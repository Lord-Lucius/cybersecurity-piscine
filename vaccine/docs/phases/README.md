# Documents de phase

> Index des phases de `vaccine`. Les phases 1 à 4 ont un document complet, au niveau des fonctions, selon la [norme du template](../template_phases.md). Les phases 5 à 9 sont **esquissées ici** : objectif, concepts, payloads clés. On promeut une esquisse en document complet **au moment de l'attaquer** — pas avant, pour qu'elle ne vieillisse pas dans un tiroir.

Pour la vue d'ensemble et l'ordre, voir la [feuille de route](../00-feuille-de-route.md).

| Phase | Sujet | Document |
|---|---|---|
| 1 | CLI | [phase-01-cli.md](phase-01-cli.md) — complet |
| 2 | URL & paramètres | [phase-02-url-et-parametres.md](phase-02-url-et-parametres.md) — complet |
| 3 | Client HTTP | [phase-03-client-http.md](phase-03-client-http.md) — complet |
| 4 | Détection | [phase-04-detection.md](phase-04-detection.md) — complet |
| 5 | Fingerprint moteur | esquisse ci-dessous |
| 6 | Extraction du schéma | esquisse ci-dessous |
| 7 | Dump des données | esquisse ci-dessous |
| 8 | Stockage `-o` | esquisse ci-dessous |
| 9 | Tests & environnement | esquisse ci-dessous |

---

## Phase 5 — Fingerprint du moteur SQL

**Objectif.** Une fois un paramètre confirmé vulnérable, dire **quel** SGBD tourne derrière. Le sujet en exige au moins deux : on vise **MySQL** et **SQLite**.

**Deux voies, à essayer dans cet ordre :**

1. **Par le message d'erreur** (gratuit si error-based a marché) — la signature déjà trouvée en [phase 4](phase-04-detection.md) § 3.2 nomme souvent le moteur directement.
2. **Par une fonction spécifique au moteur**, quand aucune erreur ne fuite. On injecte une expression qui n'est valide que sur un SGBD et on regarde si la page « vraie » revient :

   | Sonde | Vrai uniquement sur |
   |---|---|
   | `AND 1=1 AND VERSION() LIKE '%'` | MySQL (`VERSION()`) |
   | `AND sqlite_version() > '0'` | SQLite (`sqlite_version()`) |
   | `AND 1=1 AND @@version LIKE '%'` | MySQL/MSSQL (`@@version`) |

   La sonde qui laisse la page « vraie » (au sens boolean-based) identifie le moteur.

**Concepts Rust.** Le `trait Extractor` de [`rust/02`](../rust/02-structurer-le-code.md) § 4 prend son sens ici : un type par moteur, chacun sachant fournir ses sondes puis ses payloads d'extraction. Tant qu'il n'y a que deux moteurs, un `match SqlEngine` suffit — n'introduisez le trait que quand le `match` se répète.

**Piège.** Ne pas conclure sur une seule sonde ambiguë (`@@version` marche sur MySQL **et** MSSQL). Croiser deux sondes pour lever l'ambiguïté.

**Livrable.** `engine::fingerprint(...) -> SqlEngine`, affiché dans le rapport.

---

## Phase 6 — Extraction du schéma

**Objectif.** Récupérer bases, tables, colonnes. La technique reine ici est **UNION-based** : on prolonge la requête d'origine par un `UNION SELECT` qui va lire les métadonnées.

**Les deux préalables de l'UNION :**

1. **Nombre de colonnes.** `UNION SELECT` exige le **même nombre de colonnes** que la requête d'origine. On le trouve en incrémentant : `ORDER BY 1`, `ORDER BY 2`… jusqu'à l'erreur ; le dernier rang sans erreur est le compte. (Variante : `UNION SELECT NULL`, `UNION SELECT NULL,NULL`…)
2. **Colonne affichée.** Toutes les colonnes ne s'affichent pas dans la page. On repère laquelle en injectant des marqueurs (`UNION SELECT 1,2,3`) et en cherchant lequel apparaît.

**Les payloads de métadonnées, par moteur :**

| Info | MySQL | SQLite |
|---|---|---|
| Bases | `SELECT schema_name FROM information_schema.schemata` | *(une seule base ; concept absent)* |
| Tables | `SELECT table_name FROM information_schema.tables WHERE table_schema=database()` | `SELECT name FROM sqlite_master WHERE type='table'` |
| Colonnes | `SELECT column_name FROM information_schema.columns WHERE table_name='users'` | `SELECT sql FROM sqlite_master WHERE name='users'` (parser le DDL) |

**Concept clé — récupérer plusieurs lignes via une seule colonne affichée.** On concatène les résultats côté SQL avec un séparateur, puis on découpe côté Rust :

- MySQL : `GROUP_CONCAT(table_name SEPARATOR 0x0a)` → une chaîne, `split('\n')` côté Rust.
- SQLite : `GROUP_CONCAT(name, char(10))`.

**Concepts Rust.** Beaucoup de découpage de chaînes (`split`, `.lines()`, `.collect()`), et le `trait Extractor` pour router selon le moteur. C'est ici que [`rust/02`](../rust/02-structurer-le-code.md) est le plus sollicité.

**Piège.** Le `GROUP_CONCAT` MySQL est **tronqué** à 1024 octets par défaut (`group_concat_max_len`). Sur beaucoup de tables, augmenter via `SET SESSION group_concat_max_len=...` en préambule, ou paginer avec `LIMIT`.

**Livrable.** `extract::databases / tables / columns`, chaque fonction rendant un `Vec<String>`.

---

## Phase 7 — Dump des données

**Objectif.** Extraire le contenu des tables intéressantes (typiquement `users`). Même mécanique UNION que la phase 6, mais sur les vraies colonnes découvertes.

**Technique.** Concaténer les colonnes d'une ligne, et les lignes entre elles, avec deux séparateurs distincts :

- MySQL : `GROUP_CONCAT(CONCAT_WS(0x7c, id, username, password) SEPARATOR 0x0a)` → `|` entre colonnes, `\n` entre lignes.
- SQLite : `GROUP_CONCAT(id || '|' || username || '|' || password, char(10))`.

Côté Rust : `split('\n')` pour les lignes, puis `split('|')` pour les cellules → un `Vec<Vec<String>>` qu'on met en tableau à l'affichage.

**Piège — le `NULL` avale la ligne.** En SQL, `'a' || NULL` vaut `NULL` : une seule colonne nulle efface toute la ligne concaténée. Envelopper chaque colonne (`COALESCE(col, '')` / `IFNULL(col,'')`) avant de concaténer.

**Cadre.** Le dump est « best effort » : le sujet dit explicitement qu'il n'est pas garanti (protections, type d'injection). Gérer proprement l'échec fait partie du livrable — pas de panique, un message clair.

**Livrable.** `extract::dump(table, columns) -> Vec<Vec<String>>`.

---

## Phase 8 — Stockage des résultats (`-o`)

**Objectif.** Archiver tout ce qui a été trouvé dans un fichier, créé automatiquement s'il n'existe pas (exigence du sujet).

**Contenu à sauver** (cf. sujet) : URL cible, méthode, paramètre vulnérable, moteur, payloads, bases/tables/colonnes, dump, erreurs.

**Concepts Rust.** Écriture de fichier — tout est dans [`rust/03`](../rust/03-http-io-et-crates.md) § 5 : `OpenOptions::new().create(true).append(true)` pour archiver plusieurs scans à la suite, ou `fs::write` pour écraser. `writeln!` pour formater.

**Décision de conception.** Un `struct Report` accumulé pendant tout le scan (le scanner le remplit au fur et à mesure), puis **une** fonction `report::save(&report, path)` qui le sérialise. Cela sépare *collecter* de *écrire*, et permet d'afficher le même `Report` sur `stdout` et dans le fichier sans dupliquer la logique.

**Piège.** Ne pas écrire au fil de l'eau depuis dix endroits : dix points à corriger si le format change. Un `Report` accumulé, une seule fonction d'écriture.

**Livrable.** `report::save(report, path)` + affichage `stdout` façon [exemple de sortie du sujet](../vaccine.md#-exemple-de-sortie).

---

## Phase 9 — Tests et environnement vulnérable

**Objectif.** Fournir la série de tests exigée, **uniquement** sur cibles volontairement vulnérables et autorisées.

**Monter un labo local.** Le plus simple et reproductible :

- **SQLi-Labs** via Docker : `docker run -d -p 8080:80 acgpiscine/sqli-labs` (ou une image équivalente). Cible type : `http://localhost:8080/Less-1/?id=1`.
- **DVWA** via Docker : `docker run -d -p 8081:80 vulnerables/web-dvwa`, niveau de sécurité « low ».
- Une petite page PHP + MySQL maison pour maîtriser exactement la requête.

**Les cinq tests obligatoires du sujet** ([§ Tests](../vaccine.md#-tests)) : détection + payload + moteur ; seconde technique ; extraction du schéma ; dump + gestion d'échec ; requête POST.

**Structure.** Un dossier `tests/` (tests d'intégration Rust, qui voient le binaire) + un script/README décrivant comment lancer le labo. Les tests unitaires purs restent dans chaque module (`#[cfg(test)]`, comme aux phases 1-4).

**Rappel.** Cette phase est transverse : on écrit des tests unitaires **à chaque phase**, pas seulement à la fin. Le § 9 rassemble surtout les tests d'**intégration** (bout en bout, contre le labo) et la démonstration du sujet.

**Livrable.** Environnement documenté + suite de tests reproductible depuis le dépôt (`make test` + procédure labo).
