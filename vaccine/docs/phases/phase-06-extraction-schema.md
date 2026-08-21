# Vaccine — Phase 6 : extraction du schéma

> Le moteur est identifié (phase 5). On passe de « *est-ce* vulnérable » à « *que contient* la base ». La technique reine ici est **UNION-based** : on prolonge la requête d'origine par un `UNION SELECT` qui lit les métadonnées du SGBD — bases, tables, colonnes. Deux préalables la conditionnent (nombre de colonnes, colonne affichée), et un même mécanisme sert ensuite pour les trois niveaux de métadonnées.

---

## 1. Où on en est

**Fait :**
- Phases 1-5 : `Config`, `Target`/`with_injected`, `client.send` → `Response`, un `Verdict::Vulnerable`, et le `SqlEngine` (`MySql` / `Sqlite`).
- La brique `similar` ([phase 4](phase-04-detection.md) § 5.1) qui juge si une page « vraie » revient.

**À faire dans cette phase :**
- `column_count` : trouver le nombre de colonnes de la requête d'origine (`ORDER BY n`).
- `find_display_column` : repérer **laquelle** de ces colonnes s'affiche dans la page.
- `run_union` + `extract_between` : injecter un `UNION SELECT` et récupérer la valeur affichée, isolée par des marqueurs.
- `databases` / `tables` / `columns` : router selon le moteur vers les bonnes tables de métadonnées, rendre chacune un `Vec<String>`.

**Ce qui suit (phase 7) :** rejouer ce même mécanisme UNION, mais sur les vraies colonnes découvertes, pour **dumper** les données.

> Prérequis Rust : beaucoup de découpage de chaînes (`split`, `.lines()`, `.collect()`), et le `match SqlEngine` de [`rust/02`](../rust/02-structurer-le-code.md) § 3-4. C'est ici que le `trait Extractor` (§ 3.4) commence à gagner sa place.

---

## 2. Architecture cible

```
   Verdict::Vulnerable { key, … }, SqlEngine (phase 5)
                          │
          ┌───────────────┴────────────────┐
          ▼                                 ▼
   column_count(key)                 (une fois, en préambule)
   ORDER BY 1, 2, …  jusqu'à l'erreur      │
          │  → count : usize               │
          ▼                                 │
   find_display_column(key, count)          │
   UNION SELECT 1,2,…,count ; lequel apparaît ?
          │  → display : usize              │
          ▼                                 ▼
                    Union { engine, count, display }   (contexte réutilisé)
                          │
        ┌─────────────────┼──────────────────┐
        ▼                 ▼                   ▼
   databases()         tables()           columns(table)
   expr moteur →   run_union(union, expr)  → body
                   extract_between(body, "~")  → "a\nb\nc"
                   split('\n')                 → Vec<String>
```

**Point clé sur le flux :** `count` et `display` se calculent **une fois**, juste après la détection, et forment un contexte `Union` que **toutes** les extractions réutilisent. Sans ce préambule, chaque `UNION SELECT` échouerait : l'UNION exige le **même nombre de colonnes** que la requête d'origine, et la donnée n'est visible que si on l'écrit dans la colonne **affichée**. Le reste (databases, tables, columns) n'est plus qu'une variation de l'expression injectée.

---

## 3. Concepts à maîtriser

### 3.1 Les deux préalables de l'UNION

`UNION SELECT` colle une seconde requête à la première. Le SGBD n'accepte l'union que si les deux ont le **même nombre de colonnes** — d'où `column_count`. Et parmi ces colonnes, une page n'en **affiche** souvent qu'une (les autres servent en interne) : écrire la donnée ailleurs revient à ne rien voir — d'où `find_display_column`.

Documentation :
- Ces deux étapes sont le B.A.-BA de l'UNION-based, documentées dans PayloadsAllTheThings (UNION injection) — à recopier, pas à deviner.
- Précédent : `similar` ([phase 4](phase-04-detection.md) § 5.1) sert à détecter l'erreur d'`ORDER BY`.

> Analogie : pour glisser une page dans un classeur à intercalaires, il faut **le bon nombre de perforations** (le compte de colonnes) et savoir **quel intercalaire est visible** quand on referme (la colonne affichée). Se tromper de l'un ou de l'autre, et la page est là mais illisible.

⚠️ **Piège classique** : chercher la colonne affichée avant d'avoir le bon nombre de colonnes. **Symptôme** : tous les `UNION SELECT` échouent en bloc, `find_display_column` ne voit jamais aucun marqueur. **Correction** : `column_count` **d'abord**, son résultat nourrit toutes les requêtes suivantes.

### 3.2 Découvrir le nombre de colonnes : `ORDER BY` qui escalade

On demande `ORDER BY 1`, `ORDER BY 2`… en montant. Tant que le rang existe, la page reste « normale » ; au premier rang qui dépasse le nombre de colonnes, le SGBD renvoie une erreur (ou une page différente). Le **dernier rang sans erreur** est le compte.

Documentation :
- Variante sans `ORDER BY` : `UNION SELECT NULL`, `UNION SELECT NULL,NULL`… jusqu'à ce que la page « vraie » revienne.

⚠️ **Piège classique** : ne pas borner la montée. **Symptôme** : sur une page qui ne signale pas l'erreur, la boucle grimpe à l'infini. **Correction** : une borne haute raisonnable (ex. 30 colonnes) ; au-delà, abandonner proprement.

### 3.3 Récupérer plusieurs lignes par une seule colonne : `GROUP_CONCAT`

La colonne affichée ne montre qu'**une** valeur. Pour lire *toutes* les tables d'un coup, on concatène leurs noms côté SQL avec un séparateur, puis on découpe côté Rust.

| Moteur | Concaténer avec `\n` comme séparateur |
|---|---|
| MySQL | `GROUP_CONCAT(table_name SEPARATOR 0x0a)` |
| SQLite | `GROUP_CONCAT(name, char(10))` |

Documentation :
- `str::split`, `str::lines` : https://doc.rust-lang.org/std/primitive.str.html

⚠️ **Piège classique** : le `GROUP_CONCAT` de MySQL est **tronqué à 1024 octets** par défaut (`group_concat_max_len`). **Symptôme** : la liste des tables s'arrête net au milieu d'un nom. **Correction** : préfixer `SET SESSION group_concat_max_len = 1000000` quand c'est possible, ou paginer avec `LIMIT`.

### 3.4 Router selon le moteur : `match` aujourd'hui, `trait` demain

Chaque niveau (databases, tables, columns) a une syntaxe **différente par moteur** (`information_schema` sur MySQL, `sqlite_master` sur SQLite). Un `match SqlEngine` par fonction suffit tant qu'il ne se répète pas. Comme trois fonctions vont porter le même `match`, c'est le moment où le `trait Extractor` de [`rust/02`](../rust/02-structurer-le-code.md) § 4 devient rentable : un type par moteur, chacun fournissant ses expressions.

⚠️ **Piège classique** : dupliquer le `match SqlEngine` dans `databases`, `tables` **et** `columns`. **Symptôme** : ajouter un moteur oblige à toucher trois fonctions, et on en oublie une. **Correction** : centraliser les expressions par moteur (dans `mysql.rs` / `sqlite.rs`), les fonctions génériques ne routant qu'une fois.

### 3.5 Lexique

| Terme | Ce qu'il désigne | Où ça vit |
|---|---|---|
| UNION-based | prolonger la requête par un `UNION SELECT` pour lire d'autres données | [`src/extract/union.rs`](../../src/extract/union.rs) |
| nombre de colonnes | combien de colonnes rend la requête d'origine | `extract::column_count` |
| colonne affichée | l'index de colonne dont la valeur apparaît dans la page | `extract::find_display_column` |
| `Union` | le contexte `{ engine, count, display }`, calculé une fois | `src/extract/union.rs` |
| marqueur | délimiteur (`~`) encadrant la valeur injectée pour l'isoler | `extract::extract_between` |
| métadonnées | `information_schema` (MySQL) / `sqlite_master` (SQLite) | `src/extract/mysql.rs`, `src/extract/sqlite.rs` |

---

## 4. Décomposition des étapes

1. **`column_count`** — `ORDER BY n` qui escalade jusqu'à l'erreur.
2. **`find_display_column`** — marqueurs `1..count`, repérer celui qui apparaît.
3. **`extract_between`** — pur : isoler le texte entre deux marqueurs. Testable.
4. **`run_union`** — bâtir le payload UNION (expr marquée dans la colonne affichée), envoyer, rendre le corps.
5. **`databases` / `tables` / `columns`** — fournir l'expression de métadonnées du moteur, découper le résultat.

> `count` puis `display` sont **ordonnés** : le second a besoin du premier. `extract_between` est la seule brique hors réseau — on l'écrit et on la teste en premier.

---

## 5. Le corps du document : une section par fonction

### 5.0 · Boîte à outils

| Opération | API exacte |
|---|---|
| Envoyer une requête | `client.send(cfg.method, url, form)?` → [phase 3](phase-03-client-http.md) |
| URL injectée | `target.with_injected(key, payload)` → [phase 2](phase-02-url-et-parametres.md) |
| Comparer à la baseline | `similar(&resp, baseline, tolerance)` → [phase 4](phase-04-detection.md) § 5.1 |
| Chercher / positionner une sous-chaîne | `body.find("…") -> Option<usize>` |
| Découper sur un séparateur | `s.split('\n')`, `s.lines()` |
| Rassembler | `.map(str::to_string).collect::<Vec<_>>()` |
| Le marqueur | une constante `MARKER` valant `"~~"` (une paire improbable dans une page) |

**Les expressions de métadonnées, par moteur** (données de référence, à recopier — comme les signatures de la phase 4) :

| Niveau | MySQL | SQLite |
|---|---|---|
| Bases | `SELECT GROUP_CONCAT(schema_name SEPARATOR 0x0a) FROM information_schema.schemata` | *(une seule base ; concept absent — rendre une liste vide)* |
| Tables | `SELECT GROUP_CONCAT(table_name SEPARATOR 0x0a) FROM information_schema.tables WHERE table_schema=database()` | `SELECT GROUP_CONCAT(name, char(10)) FROM sqlite_master WHERE type='table'` |
| Colonnes | `SELECT GROUP_CONCAT(column_name SEPARATOR 0x0a) FROM information_schema.columns WHERE table_name='<t>'` | `SELECT sql FROM sqlite_master WHERE name='<t>'` *(parser le DDL)* |

### 5.1 · `column_count`

**Ce qu'elle doit accomplir :** trouver combien de colonnes la requête d'origine renvoie, en injectant `ORDER BY n` pour `n` croissant jusqu'à ce que la page bascule (erreur ou différence nette avec la baseline). Le dernier `n` valide est le compte.

**Décisions**

| Décision | Pourquoi |
|---|---|
| escalade bornée (ex. 1 à 30) | sans borne, une page muette fait boucler à l'infini (§ 3.2) |
| juger la bascule via `similar` contre la baseline | même critère robuste que la détection ; pas de nouvelle heuristique |
| rendre `Result<usize, …>` ; 0 possible | un compte de 0 (aucune escalade valide) signale un contexte d'injection inattendu |

**⚠️ Pièges**

⚠️ Confondre « l'erreur d'`ORDER BY` » avec une erreur SQL générique : ici on ne cherche pas une signature, on compare à la baseline. La page « valide » ressemble à la baseline ; la première qui en diffère marque le dépassement.

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| injecter `ORDER BY n` | `with_injected(key, valeur + " ORDER BY " + n)` → phase 2 |
| la page a basculé | négation de `similar(resp, baseline, tol)` → phase 4 § 5.1 |

*Troisième temps :* on garde le **dernier rang valide**, pas le premier invalide : le compte de colonnes est le plus grand `n` pour lequel `ORDER BY n` tient. Retourner `n` au moment de l'échec donnerait un de trop ; retourner `n-1` au premier échec est le compte exact.

**Lignes de log**

```rust
eprintln!("[extract] column_count on '{key}': {count} columns");
```

**Prototype**

```rust
pub fn column_count(client: &Client, cfg: &Config, target: &Target,
                    key: &str, baseline: &Response) -> Result<usize, VaccineError>;
```

**Corps**

**Déroulé.** On monte un compteur de 1 jusqu'à une borne haute. À chaque rang, on injecte `ORDER BY <rang>` dans `key` et on envoie. Tant que la réponse **ressemble** à la baseline (au sens `similar`), le rang est valide et on continue. Dès qu'une réponse en **diffère**, le rang précédent était le dernier valide : on journalise (log ①) et on rend ce précédent comme nombre de colonnes. Si l'on atteint la borne sans jamais basculer, on rend la borne (ou une erreur `Parse`, au choix, en signalant que le compte est incertain).

### 5.2 · `find_display_column`

**Ce qu'elle doit accomplir :** une fois le nombre de colonnes connu, injecter un `UNION SELECT 1,2,…,count` et repérer **lequel** de ces entiers apparaît dans la page — c'est la colonne où poser la donnée ensuite.

**Décisions**

| Décision | Pourquoi |
|---|---|
| marquer chaque entier (`~1~`, `~2~`…) plutôt que `1,2,3` bruts | un `2` nu se confond avec un `2` déjà présent dans la page ; encadré de marqueurs, il est sans ambiguïté |
| rendre `Option<usize>` | certaines injections n'affichent **aucune** colonne (donnée en aveugle) → `absent`, et l'appelant décide |

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| bâtir `UNION SELECT ~1~, ~2~, …, ~count~` | concaténer les marqueurs colonne par colonne |
| repérer le marqueur présent | `body.find("~<i>~")` pour chaque `i` |

*Troisième temps :* on entoure chaque numéro de marqueurs parce que la seule question est « **quel** numéro ressort », et qu'un numéro nu est un faux ami — les pages contiennent des chiffres partout. Le marqueur transforme « le chiffre 3 est-il là » (souvent oui, par hasard) en « la séquence `~3~` est-elle là » (seulement si le SGBD l'a affichée).

**Lignes de log**

```rust
eprintln!("[extract] display column on '{key}': index {index}");
```

**Prototype**

```rust
pub fn find_display_column(client: &Client, cfg: &Config, target: &Target,
                           key: &str, count: usize) -> Result<Option<usize>, VaccineError>;
```

**Corps**

**Déroulé.** On construit une liste de `count` expressions, où chaque position `i` porte son propre numéro encadré de marqueurs (`~i~`) — un `UNION SELECT` complet avec autant de colonnes que le compte. On injecte ce payload dans `key`, on envoie, puis on cherche dans le corps lequel des marqueurs `~1~`…`~count~` apparaît. Le premier trouvé donne l'index de la colonne affichée, qu'on journalise (log ①) et qu'on rend ; si aucun n'apparaît, on rend *absent*.

### 5.3 · `extract_between`

**Ce qu'elle doit accomplir :** isoler, dans un corps de réponse, le texte compris entre deux occurrences du marqueur. Fonction de chaîne pure, sans réseau — le point testable de la phase.

**Décisions**

| Décision | Pourquoi |
|---|---|
| encadrer la valeur injectée de marqueurs, puis découper ici | localiser la donnée dans une vraie page HTML sans marqueur est fragile ; le marqueur la rend triviale à extraire |
| rendre `Option<String>` | marqueurs absents = extraction ratée, pas une valeur vide ; deux cas distincts |

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| trouver le premier marqueur | `body.find(MARKER) -> Option<usize>` |
| trouver le marqueur de fin après lui | `body[start..].find(MARKER)` |

*Troisième temps :* on cherche le **second** marqueur *à partir de la fin du premier*, pas depuis le début du corps — sinon on retrouve le premier marqueur lui-même et on isole une chaîne vide. Décaler la recherche après le premier marqueur est ce qui rend l'extraction correcte.

**Prototype**

```rust
pub fn extract_between(body: &str, marker: &str) -> Option<String>;
```

**Corps**

**Déroulé.** On cherche la première occurrence du marqueur dans le corps ; si elle est *absente*, on rend *absent*. Sinon, on repart **juste après** ce premier marqueur et on cherche la suivante ; *absente* aussi ⇒ *absent*. Entre les deux positions se trouve la valeur injectée : on la rend, telle quelle, en `String`.

### 5.4 · `run_union`

**Ce qu'elle doit accomplir :** bâtir un payload `UNION SELECT` qui place une expression **marquée** dans la colonne affichée (et `NULL` partout ailleurs, au bon nombre de colonnes), l'injecter, envoyer, et rendre la valeur isolée par `extract_between`.

**Décisions**

| Décision | Pourquoi |
|---|---|
| prendre un contexte `Union` (engine, count, display) déjà calculé | ne pas refaire `column_count`/`find_display_column` à chaque requête |
| envelopper l'expression : `CONCAT(MARKER, expr, MARKER)` (MySQL) / `MARKER \|\| expr \|\| MARKER` (SQLite) | rend la valeur repérable quel que soit le HTML autour |
| remplir les autres colonnes de `NULL` | satisfait le compte de colonnes sans polluer l'affichage |

**⚠️ Pièges**

⚠️ Oublier de marquer l'expression : la valeur revient noyée dans la page, et `extract_between` ne trouve rien. Le marquage est **dans** `run_union`, pas optionnel.

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| bâtir la liste de colonnes (NULL sauf la colonne affichée) | placer l'expression marquée à l'index `union.display`, `NULL` ailleurs |
| envelopper avec le marqueur selon le moteur | `CONCAT` (MySQL) / `\|\|` (SQLite) → tables du § 5.0 |
| isoler la réponse | `extract_between(resp.body, MARKER)` → § 5.3 |

*Troisième temps :* on assemble les colonnes une fois, dans l'ordre, en n'écrivant l'expression qu'à `union.display` et des `NULL` ailleurs — parce que l'UNION échoue si le compte ne colle pas, et n'affiche rien si l'expression n'est pas à la bonne place. Les deux contraintes de § 3.1 se satisfont dans cette seule construction.

**Lignes de log**

```rust
eprintln!("[extract] union on '{key}' col {display}: expr {expr}");
```

**Prototype**

```rust
pub fn run_union(client: &Client, cfg: &Config, target: &Target,
                 key: &str, union: &Union, expr: &str)
    -> Result<Option<String>, VaccineError>;
```

**Corps**

**Déroulé.** On enveloppe d'abord l'expression demandée entre deux marqueurs, avec la concaténation propre au moteur (`CONCAT` pour MySQL, `||` pour SQLite). On construit ensuite la liste des colonnes de l'UNION : autant que `union.count`, toutes à `NULL` sauf celle d'index `union.display`, qui reçoit l'expression marquée. On suffixe ce `UNION SELECT` à la valeur d'origine de `key`, on injecte et on envoie (→ phase 3). On journalise (log ①), puis on isole la valeur du corps avec `extract_between` (→ § 5.3) et on la rend — *absente* si le marqueur n'a pas été trouvé.

### 5.5 · `databases` / `tables` / `columns`

**Ce qu'elles doivent accomplir :** chacune fournit l'expression de métadonnées propre au moteur (table du § 5.0), la passe à `run_union`, puis découpe la chaîne concaténée en `Vec<String>`. `columns` prend en plus le nom de la table visée.

**Décisions**

| Décision | Pourquoi |
|---|---|
| une fonction par niveau, le routage moteur factorisé | évite trois `match SqlEngine` dupliqués (§ 3.4) |
| découper sur `\n` (le séparateur du `GROUP_CONCAT`) | c'est le séparateur qu'on a **choisi** côté SQL ; les deux bouts sont cohérents |
| SQLite `databases` rend une liste vide | le concept de « plusieurs bases » n'existe pas ; ne pas simuler |

**⚠️ Pièges**

⚠️ Interpoler un nom de table dans l'expression `columns` sans le protéger : un nom exotique casse la requête. Pour le rendu du projet (labos), l'interpolation simple suffit ; le signaler comme limite assumée.
⚠️ Sur SQLite, les colonnes se lisent en **parsant le DDL** (`sqlite_master.sql`), pas dans une table `columns`. `columns` doit donc, pour SQLite, extraire les noms depuis le `CREATE TABLE` — un découpage de chaîne, pas une requête de plus.

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| choisir l'expression selon le moteur | `match union.engine` → expressions du § 5.0 (centralisées en `mysql.rs`/`sqlite.rs`) |
| exécuter et isoler | `run_union(union, expr)?` → § 5.4 |
| découper en lignes | `value.split('\n').map(...).collect()` |

**Prototype**

```rust
pub fn databases(client: &Client, cfg: &Config, target: &Target,
                 key: &str, union: &Union) -> Result<Vec<String>, VaccineError>;

pub fn tables(client: &Client, cfg: &Config, target: &Target,
              key: &str, union: &Union) -> Result<Vec<String>, VaccineError>;

pub fn columns(client: &Client, cfg: &Config, target: &Target,
               key: &str, union: &Union, table: &str) -> Result<Vec<String>, VaccineError>;
```

**Corps**

**Déroulé (les trois, même forme).** On choisit, selon `union.engine`, l'expression de métadonnées du niveau voulu (les chaînes du § 5.0) ; pour `columns`, on y interpole le nom de `table`. On passe cette expression à `run_union` (→ § 5.4), qui rend la chaîne concaténée affichée. Si elle est *absente*, on rend une liste vide (rien d'extractible). Sinon on la découpe sur `\n`, on nettoie les entrées vides, et on rend le `Vec<String>`. Cas particuliers : `databases` sur SQLite rend directement une liste vide ; `columns` sur SQLite reçoit un DDL `CREATE TABLE …` et en extrait les noms de colonnes par découpage de chaîne plutôt que par un simple `split('\n')`.

---

## 6. Pièges spécifiques à cette phase

- **L'ordre `count` → `display` → extractions est un invariant.** Les trois extractions supposent un `Union` complet et correct. Calculez-le **une fois**, juste après la phase 5, et passez-le partout ; le recalculer dans chaque fonction multiplie les requêtes et les points de casse.
- **UNION n'est pas la seule voie.** Si l'injection ne permet pas l'UNION (contexte, filtres), l'extraction bascule sur du **boolean-based caractère par caractère** — beaucoup plus lent, hors périmètre de cette phase mais à mentionner : l'échec de l'UNION n'est pas l'échec de l'extraction, c'est le signal d'une autre technique. Pour le rendu, l'UNION sur labo suffit.
- **La troncature `GROUP_CONCAT` est silencieuse** (§ 3.3) : la liste paraît complète alors qu'elle est coupée. Sur une base à beaucoup de tables, vérifiez la longueur et augmentez `group_concat_max_len` avant de conclure.

---

## 7. Tests unitaires

> [!IMPORTANT]
> `column_count`, `find_display_column`, `run_union`, `databases`/`tables`/`columns` appellent le réseau → **hors périmètre unitaire**. Le point **purement testable** est `extract_between` : une chaîne en entrée, un `Option<String>` en sortie. On l'a isolée exprès.

### 7.1 `src/extract/union.rs` — `#[cfg(test)]`

**Ce que tu testes :**
- `extract_between` isole bien la valeur entre deux marqueurs.
- marqueur unique (pas de fin) → `None`.
- aucun marqueur → `None`.

**Ce que le test doit prouver au-delà du comportement :** que la recherche du **second** marqueur repart *après* le premier (sinon une valeur non vide serait rendue comme vide), et que « marqueurs absents » rend `None` et non `Some("")` — c'est la distinction qui laisse l'appelant conclure « extraction ratée ».

**Stratégie.** Aucun réseau : on appelle `extract_between` avec des chaînes littérales imitant un corps de page avec, ou sans, les marqueurs.

**Les cas à vérifier** (chacun devient un `#[test]`, marqueur `~~`) :

| Entrée (corps) | Attendu | Pourquoi ce cas |
|---|---|---|
| `...~~users\ntokens~~...` | `Some("users\ntokens")` | extraction nominale, séparateur `\n` compris |
| `...~~users` (une seule occurrence) | `None` | marqueur de fin manquant |
| `page sans marqueur` | `None` | rien à extraire, pas `Some("")` |

### 7.2 Hors périmètre

| Fonction | Pourquoi | Comment on vérifie |
|---|---|---|
| `column_count`, `find_display_column`, `run_union`, `databases`/`tables`/`columns` | requêtes réseau | test manuel contre SQLi-Labs (MySQL) et un labo SQLite, lecture des logs `[extract]` |

### 7.3 Résultats attendus

- Unitaires `extract_between` : PASS (les trois cas).
- Manuel : sur SQLi-Labs `?id=1`, l'outil affiche le compte de colonnes, la colonne affichée, puis la liste des tables — PASS.

---

## 8. Ordre de développement recommandé

1. La constante `MARKER` et `extract_between` + ses tests — la seule brique hors-ligne.
2. `column_count`, essayé à la main sur SQLi-Labs (le compte doit être stable).
3. `find_display_column`, vérifier que le marqueur attendu ressort.
4. Le contexte `Union` (décrit : engine + count + display), assemblé une fois.
5. `run_union`, puis `tables` — la première liste réelle extraite.
6. `databases` et `columns`, en routant par `union.engine` ; refactorer en `trait Extractor` si le `match` se répète.

> Quand l'outil liste les tables d'un labo MySQL **et** d'un labo SQLite, cette phase est close. On passe à la **phase 7 : dump des données** ([phase-07-dump.md](phase-07-dump.md)).
