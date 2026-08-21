# Vaccine — Phase 7 : dump des données

> Le schéma est connu (phase 6) : bases, tables, colonnes. On extrait maintenant le **contenu** — typiquement la table `users`. Même mécanique UNION que la phase 6, mais sur les vraies colonnes découvertes, avec **deux** séparateurs : un entre les colonnes d'une ligne, un entre les lignes. Le dump est « best effort » : le sujet dit qu'il n'est pas garanti, donc gérer proprement l'échec fait partie du livrable.

---

## 1. Où on en est

**Fait :**
- Phases 1-6 : détection, moteur (`SqlEngine`), et le contexte `Union { engine, count, display }` avec `run_union` + `extract_between` ([phase 6](phase-06-extraction-schema.md) § 5.4, § 5.3). On sait lister tables et colonnes.

**À faire dans cette phase :**
- `row_expression` : bâtir l'expression SQL qui concatène les colonnes d'une ligne, puis les lignes entre elles.
- `split_rows` : redécouper la chaîne obtenue en lignes puis en cellules.
- `dump` : orchestrer — expression, `run_union`, découpage — et rendre un `Vec<Vec<String>>`.

**Ce qui suit (phase 8) :** ranger tout ce qui a été trouvé (paramètre, moteur, schéma, dump) dans un `Report`, l'afficher et l'archiver (`-o`).

> Prérequis Rust : encore du découpage de chaînes imbriqué (`split('\n')` puis `split('|')`), et un `Vec<Vec<String>>` — un tableau de lignes.

---

## 2. Architecture cible

```
   Union { engine, count, display }, table = "users", columns = [id, username, password]
                          │
                 ┌────────▼─────────┐
                 │  row_expression  │   → per-engine SQL:
                 └────────┬─────────┘     GROUP_CONCAT( col1 |'|' col2 |'|' col3 , '\n')
                          ▼                 (chaque colonne enveloppée COALESCE/IFNULL)
                 ┌────────▼─────────┐
                 │    run_union     │   (phase 6) → une seule chaîne affichée
                 └────────┬─────────┘
                          ▼
                 ┌────────▼─────────┐
                 │    split_rows    │   split('\n') → lignes ; split('|') → cellules
                 └────────┬─────────┘
                          ▼
                Vec<Vec<String>>  = [[1, admin, 5f4dcc…], [2, bob, …], …]
```

**Point clé sur le flux :** une seule requête UNION rapatrie **toute** la table, aplatie en une chaîne à deux niveaux de séparateurs. Tout le travail Rust est de la **reconstituer** en tableau. Le côté SQL choisit les séparateurs, le côté Rust les respecte — les deux moitiés d'une même convention.

---

## 3. Concepts à maîtriser

### 3.1 Deux séparateurs, deux niveaux

Une table est un tableau : des lignes, chacune faite de cellules. Pour la faire tenir dans **une** chaîne, il faut deux séparateurs distincts — un entre cellules (`|`), un entre lignes (`\n`) :

| Moteur | Concaténer une ligne, puis les lignes |
|---|---|
| MySQL | `GROUP_CONCAT(CONCAT_WS(0x7c, col1, col2, col3) SEPARATOR 0x0a)` |
| SQLite | `GROUP_CONCAT(col1 \|\| char(124) \|\| col2 \|\| char(124) \|\| col3, char(10))` |

(`0x7c` / `char(124)` = `|`, `0x0a` / `char(10)` = `\n`.) Côté Rust : `split('\n')` pour les lignes, puis `split('|')` pour les cellules.

Documentation :
- `str::split` : https://doc.rust-lang.org/std/primitive.str.html
- Précédent : le `GROUP_CONCAT` à un séparateur de [phase 6](phase-06-extraction-schema.md) § 3.3 — on ajoute juste le niveau « cellule ».

> Analogie : sérialiser un tableur en texte. On sépare les cellules par des virgules et les lignes par des retours à la ligne — un CSV. Ici c'est le même principe, monté **côté SQL** avant que la donnée ne parte.

⚠️ **Piège classique** : une valeur qui **contient** le séparateur (un mot de passe avec un `|`). **Symptôme** : une ligne paraît avoir une cellule de trop, tout se décale. **Correction** : sur les labos, des séparateurs improbables (`0x7c` passe le plus souvent) ; en toute rigueur, un séparateur multi-octets rare. À signaler comme limite assumée.

### 3.2 Le `NULL` qui avale la ligne

En SQL, concaténer quoi que ce soit avec `NULL` donne `NULL` : `'a' || NULL` vaut `NULL`. Une **seule** colonne nulle efface donc **toute** la ligne concaténée. On enveloppe chaque colonne avant de concaténer :

| Moteur | Neutraliser le `NULL` |
|---|---|
| MySQL | `IFNULL(col, '')` (ou `COALESCE(col, '')`) |
| SQLite | `COALESCE(col, '')` |

Documentation :
- Ce piège est classique des cheat-sheets UNION dump — à connaître, pas à redécouvrir.

⚠️ **Piège classique** : concaténer les colonnes brutes. **Symptôme** : des lignes **manquent** dans le dump (celles qui ont une valeur nulle quelque part), sans erreur. **Correction** : `IFNULL`/`COALESCE` sur **chaque** colonne, systématiquement.

### 3.3 Le dump est « best effort »

Le sujet le dit explicitement : l'extraction des données n'est **pas garantie** (protections, type d'injection, colonnes non affichables). Gérer l'échec proprement — un message clair, pas de panique, un `Vec` vide plutôt qu'un `unwrap` — fait partie du livrable au même titre que le succès.

⚠️ **Piège classique** : traiter un dump vide comme une erreur fatale. **Symptôme** : l'outil s'arrête alors qu'il a déjà trouvé le paramètre, le moteur et le schéma. **Correction** : un dump vide est un **résultat** (« rien d'extractible ici »), pas une panne ; on l'affiche comme tel et on continue.

### 3.4 Lexique

| Terme | Ce qu'il désigne | Où ça vit |
|---|---|---|
| dump | le contenu extrait d'une table, en tableau | [`src/extract/dump.rs`](../../src/extract/dump.rs) |
| séparateur de cellule / de ligne | `\|` entre colonnes, `\n` entre lignes | `extract::row_expression` |
| `row_expression` | l'expression SQL qui aplatit la table en une chaîne | `src/extract/dump.rs` |
| best effort | le dump peut légitimement échouer sans que le scan échoue | § 3.3 |

---

## 4. Décomposition des étapes

1. **`row_expression`** — pure : colonnes + moteur → l'expression SQL de concaténation. Testable.
2. **`split_rows`** — pure : la chaîne aplatie → `Vec<Vec<String>>`. Testable.
3. **`dump`** — orchestration : expression → `run_union` → `split_rows`.
4. **Câblage** — `main` inchangé : le dump s'ajoute dans `run`.

> Les deux briques pures (`row_expression`, `split_rows`) s'écrivent et se testent **sans réseau** ; `dump` ne fait que les coudre autour de `run_union` (phase 6).

---

## 5. Le corps du document : une section par fonction

### 5.0 · Boîte à outils

| Opération | API exacte |
|---|---|
| Requête UNION marquée | `run_union(client, cfg, target, key, union, expr)?` → [phase 6](phase-06-extraction-schema.md) § 5.4 |
| Découper en lignes | `s.split('\n')` |
| Découper en cellules | `ligne.split('\|')` |
| Rassembler un tableau | `.map(...).collect::<Vec<Vec<String>>>()` |
| Enrober une colonne (données de référence) | `IFNULL(col,'')` (MySQL) / `COALESCE(col,'')` (SQLite) — § 3.2 |

### 5.1 · `row_expression`

**Ce qu'elle doit accomplir :** à partir du moteur et de la liste des colonnes, produire l'expression SQL qui concatène les colonnes d'une ligne (séparateur `|`) puis les lignes entre elles (séparateur `\n`), chaque colonne étant neutralisée contre `NULL`. Fonction de chaîne pure — testable.

**Décisions**

| Décision | Pourquoi |
|---|---|
| envelopper chaque colonne de `IFNULL`/`COALESCE` **dans** l'expression | le piège `NULL` (§ 3.2) est structurel, pas un cas limite : la protection est toujours là |
| choisir la syntaxe via `match engine` | MySQL et SQLite concatènent différemment (`CONCAT_WS` vs `\|\|`) |
| rendre une `String` (l'expression), pas exécuter | garde la fonction pure et testable ; `dump` l'exécute |

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| enrober chaque colonne | `IFNULL(<col>, '')` / `COALESCE(<col>, '')` → § 3.2 |
| joindre les colonnes d'une ligne | `CONCAT_WS(0x7c, …)` (MySQL) / `… \|\| char(124) \|\| …` (SQLite) |
| joindre les lignes | `GROUP_CONCAT(…, 0x0a / char(10))` |

*Troisième temps :* on enveloppe **avant** de concaténer, pas après : une fois que `'a' || NULL` a valu `NULL`, l'information est perdue et aucun traitement en aval ne la récupère. `COALESCE` doit s'appliquer à chaque colonne à la source, dans l'expression elle-même.

**Prototype**

```rust
pub fn row_expression(engine: SqlEngine, columns: &[String]) -> String;
```

**Corps**

**Déroulé.** Pour chaque colonne, on l'enveloppe dans le neutralisant de `NULL` du moteur (`IFNULL(col,'')` sur MySQL, `COALESCE(col,'')` sur SQLite). On joint ces colonnes enveloppées avec le séparateur de cellule `|` : sur MySQL via `CONCAT_WS(0x7c, …)`, sur SQLite en les intercalant de `char(124)`. On enveloppe enfin le tout dans un `GROUP_CONCAT(…, <séparateur de ligne>)` — `0x0a` sur MySQL, `char(10)` sur SQLite — qui agrège toutes les lignes en une chaîne. On rend cette expression, prête à passer à `run_union`.

### 5.2 · `split_rows`

**Ce qu'elle doit accomplir :** l'inverse de `row_expression` côté Rust — reconstituer le tableau depuis la chaîne aplatie : découper sur `\n` pour les lignes, puis chaque ligne sur `|` pour les cellules. Pure, testable.

**Décisions**

| Décision | Pourquoi |
|---|---|
| rendre `Vec<Vec<String>>` | un tableau : lignes de cellules, directement affichable |
| ignorer une dernière ligne vide | `GROUP_CONCAT` peut laisser un `\n` traînant → une ligne fantôme sinon |

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| découper les lignes | `dump.split('\n')`, filtrer les vides |
| découper les cellules | `ligne.split('\|').map(str::to_string)` |

*Troisième temps :* on découpe dans l'ordre inverse de la construction — lignes d'abord, cellules ensuite — parce que les deux séparateurs ne sont pas interchangeables : couper d'abord sur `|` mélangerait les cellules de lignes différentes. Respecter la hiérarchie `\n` puis `|` reconstitue exactement ce que le SQL a aplati.

**Prototype**

```rust
pub fn split_rows(dump: &str) -> Vec<Vec<String>>;
```

**Corps**

**Déroulé.** On découpe la chaîne sur `\n` pour obtenir les lignes, en écartant les segments vides (dont un éventuel `\n` final). Pour chaque ligne, on la découpe sur `|` et on collecte les cellules en `Vec<String>`. L'ensemble forme le `Vec<Vec<String>>` rendu.

### 5.3 · `dump`

**Ce qu'elle doit accomplir :** l'orchestration. Construire l'expression de ligne pour la table et ses colonnes, l'exécuter via `run_union`, et redécouper le résultat en tableau — ou rendre un tableau vide si rien ne revient (best effort).

**Décisions**

| Décision | Pourquoi |
|---|---|
| réutiliser le `Union` de la phase 6 | le compte de colonnes et la colonne affichée sont déjà connus |
| dump vide = `Ok(vec![])`, pas `Err` | l'échec d'extraction est un résultat, pas une panne (§ 3.3) |
| prendre `table` + `columns` en paramètres | découplé de la découverte du schéma (phase 6) qui les a fournis |

**⚠️ Pièges**

⚠️ Propager en `Err` un `run_union` qui rend *absent* : on transformerait un « rien à extraire » en échec du scan. Un `absent` doit devenir un `Vec` vide, pas une erreur.

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| bâtir l'expression de ligne | `row_expression(union.engine, columns)` → § 5.1 |
| exécuter la requête | `run_union(…, expr)?` → phase 6 § 5.4 |
| reconstituer le tableau | `split_rows(value)` → § 5.2 |

*Troisième temps :* `dump` ne contient **aucune** logique de séparateur — elle vit entièrement dans `row_expression` (côté SQL) et `split_rows` (côté Rust). `dump` ne fait que les enchaîner : c'est ce qui garde la convention de séparateurs en **un seul** couple d'endroits, cohérents par construction.

**Lignes de log**

```rust
eprintln!("[extract] dump {table}: {rows} rows");
```

**Prototype**

```rust
pub fn dump(client: &Client, cfg: &Config, target: &Target, key: &str,
            union: &Union, table: &str, columns: &[String])
    -> Result<Vec<Vec<String>>, VaccineError>;
```

**Corps**

**Déroulé.** On bâtit l'expression de ligne avec `row_expression` pour le moteur et les colonnes données (→ § 5.1). On la passe à `run_union` (→ phase 6) : si le résultat est *absent*, on journalise et on rend un tableau **vide** — best effort, pas d'erreur. Sinon, on le redécoupe avec `split_rows` (→ § 5.2), on journalise le nombre de lignes (log ①), et on rend le `Vec<Vec<String>>`.

### 5.4 · Câblage dans `main`

**Ce qu'il doit accomplir :** encore rien de neuf côté `main`. Le dump (`dump` sur les tables intéressantes) s'ajoute **dans** `scanner::run`, après l'extraction du schéma, et complète le résultat que `main` affiche.

**Décisions**

| Décision | Pourquoi |
|---|---|
| `dump` appelé depuis `run`, pas depuis `main` | l'orchestration reste centralisée ; `main` reste mince |

**Prototype**

```rust
fn main();
```

**Corps**

**Déroulé.** `main` ne bouge pas : lire la `Config`, appeler `scanner::run`, afficher le résultat. Le dump des tables se branche **dans** `run`, à la suite du schéma, et garnit le résultat rendu. Le câblage de `main` prendra sa forme **définitive** à la phase 8, quand ce résultat deviendra un `Report` à afficher et à archiver.

---

## 6. Pièges spécifiques à cette phase

- **Le couple `row_expression` / `split_rows` doit rester symétrique.** Si un jour vous changez le séparateur côté SQL (`0x7c` → autre chose), changez le `split` côté Rust dans le même geste. Un seul des deux modifié, et le tableau se reconstitue de travers, sans erreur. C'est le piège d'interaction propre à cette phase.
- **Choisir les colonnes à dumper.** La phase 6 a pu découvrir dix colonnes ; en dumper trois pertinentes (`id`, `username`, `password`) suffit au rendu et raccourcit la chaîne (moins de risque de troncature `GROUP_CONCAT`). Ne dumpez pas tout par réflexe.
- **Un `|` ou un `\n` dans une donnée casse l'alignement** (§ 3.1). Sur les labos c'est rare ; hors labo, c'est la limite connue de l'aplatissement par séparateur. Assumez-la explicitement dans le rapport plutôt que de la masquer.

---

## 7. Tests unitaires

> [!IMPORTANT]
> `dump` appelle le réseau (via `run_union`) → **hors périmètre unitaire**. Les points **purement testables** sont `row_expression` (construction d'une chaîne SQL) et `split_rows` (découpage) : aucune I/O. On les a isolés exprès.

### 7.1 `src/extract/dump.rs` — `#[cfg(test)]`

**Ce que tu testes :**
- `row_expression` enveloppe chaque colonne d'un neutralisant de `NULL` et emploie les bons séparateurs selon le moteur.
- `split_rows` reconstitue le bon tableau, et ignore une ligne finale vide.

**Ce que le test doit prouver au-delà du comportement :** que `row_expression` ne laisse **aucune** colonne nue (sinon le piège `NULL` réapparaît), et que `split_rows` respecte la hiérarchie `\n` puis `|` — deux règles qu'une « simplification » casserait silencieusement.

**Stratégie.** Deux fonctions pures : on appelle `row_expression` avec un `SqlEngine` et des noms de colonnes littéraux, et on vérifie que la chaîne produite contient l'enveloppe attendue ; on appelle `split_rows` avec une chaîne aplatie littérale.

**Les cas à vérifier** (chacun devient un `#[test]`) :

| Fonction | Entrée | Attendu | Pourquoi ce cas |
|---|---|---|---|
| `row_expression` | `MySql`, `["id","name"]` | contient `IFNULL(id` **et** `IFNULL(name` | aucune colonne nue |
| `row_expression` | `Sqlite`, `["id"]` | contient `COALESCE(id` et `char(10)` | bons séparateurs SQLite |
| `split_rows` | `"1\|admin\n2\|bob"` | `[["1","admin"],["2","bob"]]` | reconstitution nominale |
| `split_rows` | `"1\|a\n"` (|`\n` final) | `[["1","a"]]` | la ligne vide finale est ignorée |

### 7.2 Hors périmètre

| Fonction | Pourquoi | Comment on vérifie |
|---|---|---|
| `dump` | requête réseau (via `run_union`) | test manuel contre un labo, lecture des logs `[extract]` |

### 7.3 Résultats attendus

- Unitaires `row_expression` / `split_rows` : PASS.
- Manuel : sur un labo MySQL, `dump("users", [...])` rend les lignes attendues ; sur une injection non-UNION, `dump` rend un tableau vide **sans** planter — PASS.

---

## 8. Ordre de développement recommandé

1. `split_rows` + ses tests — le plus simple, purement Rust.
2. `row_expression` + ses tests, un moteur à la fois (MySQL puis SQLite).
3. `dump` : coudre `row_expression` → `run_union` → `split_rows`.
4. Test manuel sur `users` d'un labo ; vérifier une ligne à valeur `NULL` (elle ne doit **pas** disparaître).
5. Vérifier le chemin d'échec : sur une cible sans UNION exploitable, `dump` rend `[]` proprement.

> Quand l'outil dumpe une table sur un labo MySQL **et** gère un dump vide sans planter, cette phase est close. On passe à la **phase 8 : stockage des résultats (`-o`)** ([phase-08-stockage.md](phase-08-stockage.md)).
