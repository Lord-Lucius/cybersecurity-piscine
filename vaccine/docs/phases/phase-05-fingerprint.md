# Vaccine — Phase 5 : fingerprint du moteur SQL

> Un paramètre vient d'être déclaré vulnérable (phase 4). Reste à dire **quel** SGBD répond derrière. Le sujet en exige au moins deux : on vise **MySQL** et **SQLite**. Deux voies, dans l'ordre : d'abord lire le moteur dans le message d'erreur (gratuit si error-based a marché), sinon l'interroger avec une fonction qui n'existe que sur un moteur. À la fin, on sait router les payloads d'extraction des phases 6-7.

---

## 1. Où on en est

**Fait :**
- Phases 1-4 : `Config`, `Target`/`with_injected`, `client.send` → `Response`, et la détection qui rend un `Verdict::Vulnerable { key, method, technique, payload }`.
- La brique `similar` et la table `SQL_ERRORS` ([phase 4](phase-04-detection.md) § 5.1, § 3.2).

**À faire dans cette phase :**
- `engine_from_signature` : lire un moteur dans un corps d'erreur (route gratuite).
- `probe` : envoyer une sonde booléenne propre à un moteur, dire si elle passe.
- `fingerprint` : orchestrer les deux voies, croiser les sondes, rendre un `SqlEngine`.

**Ce qui suit (phase 6) :** avec le moteur connu, construire les payloads UNION d'extraction du schéma — `information_schema` sur MySQL, `sqlite_master` sur SQLite.

> Prérequis Rust : [`rust/02`](../rust/02-structurer-le-code.md) § 2 (`enum` porteur) et § 4 (`trait`). Le `trait Extractor` n'est **pas** pour cette phase (§ 3.3) ; on s'en tient à `SqlEngine` + `match`.

---

## 2. Architecture cible

```
   Verdict::Vulnerable { key, … }, baseline (de la phase 4)
                          │
                 ┌────────▼─────────┐
                 │ engine::fingerprint│
                 └────────┬─────────┘
                          │
        VOIE 1 (gratuite) │ provoquer une erreur, lire sa signature
                          ▼
             err = send( key + "'" )
             engine_from_signature(err.body)  →  Some(MySql | Sqlite)  ─┐
                          │ None                                          │
        VOIE 2 (sondes)   ▼                                              │
             probe "sqlite_version() > '0'"   → is_sqlite : bool         │
             probe "VERSION() LIKE '%'"       → is_mysql  : bool         │
                          │                                              │
                  croiser les deux sondes                               │
                          ▼                                              ▼
              SqlEngine::{ MySql | Sqlite | Unknown } ────────────► rapport / extraction
```

**Point clé sur le flux :** les deux voies visent le même résultat mais coûtent différemment. La **voie 1 est gratuite** quand error-based a déjà fait fuiter un message : on relit ce qu'on a. La **voie 2 paie deux requêtes** et ne sert que si aucune erreur ne fuite. On tente donc toujours la voie 1 d'abord, et on ne descend aux sondes que sur son échec — exactement la même logique de coût croissant qu'entre error-based et boolean-based en phase 4.

### Le schéma qui explique les sondes

Une sonde est une condition qui n'est **valide** que sur un moteur. On la colle en `AND` à la valeur d'origine : si la fonction existe, la requête reste valide et la condition est vraie → la page « vraie » (au sens `similar`) revient ; si la fonction est inconnue, tout le `SELECT` casse → la page **diffère**. Le moteur se lit dans le croisement :

```
                     sonde MySQL                 sonde SQLite
                 "VERSION() LIKE '%'"       "sqlite_version() > '0'"

   MySQL         fonction connue → vraie      fonction inconnue → erreur
                 similar == true              similar == false        →  MYSQL
                        ▲
   SQLite        fonction inconnue → erreur   fonction connue → vraie
                 similar == false             similar == true         →  SQLITE

   (autre/WAF)   false                        false                   →  UNKNOWN
```

Une seule sonde vraie, l'autre fausse : le moteur est celui de la sonde vraie. Les deux fausses : ni MySQL ni SQLite (ou une protection avale la condition) → `Unknown`, et on ne bluffe pas.

---

## 3. Concepts à maîtriser

### 3.1 Lire le moteur dans l'erreur, avant de le sonder

Un message d'erreur SQL brut nomme presque toujours son moteur : `You have an error in your SQL syntax` est signé MySQL/MariaDB, `SQL logic error` est signé SQLite. La phase 4 avait une table plate `SQL_ERRORS` qui répondait à « *y a-t-il* une erreur ? ». Ici on veut « *quel* moteur ? » : il faut donc associer chaque signature à **son** moteur, pas seulement les mettre en vrac.

Documentation :
- `str::contains` : https://doc.rust-lang.org/std/primitive.str.html
- Précédent : la table `SQL_ERRORS` de [phase 4](phase-04-detection.md) § 3.2 — mêmes chaînes, mais indexées par moteur ici.

> Analogie : une panne de voiture. Le tableau de bord qui affiche « défaut moteur » (phase 4 : il y a un problème) n'est pas le code constructeur qui dit « injecteur cylindre 3 » (phase 5 : lequel exactement). On lit un cran plus fin la même information.

⚠️ **Piège classique** : réutiliser la table plate `SQL_ERRORS` et conclure « MySQL » parce qu'une de ses entrées matche, alors qu'elle mélange les moteurs. **Symptôme** : une erreur SQLite classée MySQL parce que la première signature de la table appartenait à MySQL. **Correction** : une table **par moteur** (`ENGINE_SIGNATURES`), et on rend le moteur dont une signature matche.

### 3.2 La sonde : une fonction qui n'existe que sur un moteur

`VERSION()` est une fonction MySQL/MariaDB ; `sqlite_version()` une fonction SQLite ; `@@version` une variable MySQL **et** MSSQL. Injecter `AND <fonction> LIKE '%'` transforme « ce moteur existe-t-il ? » en « la page vraie revient-elle ? » — une question à laquelle `similar` (phase 4) répond déjà.

Documentation :
- Précédent : `similar` de [phase 4](phase-04-detection.md) § 5.1 — on réutilise le comparateur tel quel.
- Ces fonctions sont documentées dans les cheat-sheets d'injection (PayloadsAllTheThings) — à recopier, pas à deviner.

> Analogie : demander un mot d'argot local pour deviner d'où vient quelqu'un. Poser un mot que seul un Marseillais comprend : s'il réagit, il est de Marseille ; s'il ne réagit pas, il est d'ailleurs. La **réaction** (ici : la page vraie qui revient) porte l'information, pas la réponse elle-même.

⚠️ **Piège classique** : sonder avec `@@version`, ambigu entre MySQL et MSSQL. **Symptôme** : un MSSQL classé MySQL, et toute l'extraction de la phase 6 qui part sur `information_schema` avec la mauvaise syntaxe. **Correction** : préférer une fonction **discriminante** (`VERSION()` existe sur MySQL, pas MSSQL) et **croiser deux sondes** avant de conclure.

### 3.3 Un `match SqlEngine`, pas encore un `trait`

Le `trait Extractor` de [`rust/02`](../rust/02-structurer-le-code.md) § 4 est tentant ici : « chaque moteur fournit ses payloads ». Mais tant qu'il n'y a que deux moteurs et une seule fonction qui décide, un `match SqlEngine` se lit mieux et évite d'éparpiller la logique. La fiche le dit explicitement (§ 4, encadré) : on introduit le trait quand le `match` **se répète** dans plusieurs fonctions d'extraction — c'est-à-dire en phase 6, pas ici.

⚠️ **Piège classique** : poser le `trait Extractor` dès la phase 5 « pour être propre ». **Symptôme** : une abstraction à deux implémentations dont une seule ligne varie, plus dure à lire que le `match`. **Correction** : `match` maintenant, `trait` quand la répétition apparaît.

### 3.4 Lexique

| Terme | Ce qu'il désigne | Où ça vit |
|---|---|---|
| `SqlEngine` | `enum` : `MySql`, `Sqlite`, `Unknown` | [`src/engine/fingerprint.rs`](../../src/engine/fingerprint.rs) |
| signature | sous-chaîne d'un corps d'erreur qui nomme un moteur | `engine::ENGINE_SIGNATURES` |
| sonde (probe) | condition SQL valide sur un seul moteur, injectée en `AND` | `engine::probe` |
| voie gratuite | identification par le message d'erreur, sans requête neuve dédiée | `engine::fingerprint` |
| sonde ambiguë | sonde vraie sur plusieurs moteurs (`@@version`) | § 3.2 |

---

## 4. Décomposition des étapes

1. **`SqlEngine`** (enum) et **`ENGINE_SIGNATURES`** (table moteur → signatures).
2. **`engine_from_signature`** — un corps d'erreur → `Option<SqlEngine>`, pur, testable.
3. **`probe`** — une sonde → `bool`, via `similar`.
4. **`fingerprint`** — voie 1 (signature) puis voie 2 (sondes croisées).

> `engine_from_signature` est le seul morceau sans réseau : on l'écrit et on le teste en premier, comme `similar` en phase 4.

---

## 5. Le corps du document : une section par fonction

### 5.0 · Boîte à outils

| Opération | API exacte |
|---|---|
| Envoyer une requête | `client.send(cfg.method, url, form)?` → [phase 3](phase-03-client-http.md) |
| URL injectée | `target.with_injected(key, payload)` → [phase 2](phase-02-url-et-parametres.md) |
| Comparer à la baseline | `similar(&resp, baseline, tolerance)` → [phase 4](phase-04-detection.md) § 5.1 |
| Chercher une sous-chaîne | `body.contains("…")` |
| Table moteur → signatures | une constante `ENGINE_SIGNATURES` : chaque entrée associe un `SqlEngine` à ses sous-chaînes d'erreur — forme décrite en § 5.1 |
| Parcourir la table | une boucle sur les couples (moteur, signatures) |

### 5.1 · `engine_from_signature`

**Ce qu'elle doit accomplir :** recevoir le corps d'une réponse d'erreur et rendre le moteur dont une signature y apparaît, ou `absent` si aucune. Pas de réseau : une fonction de chaîne, donc testable unitairement.

**Décisions**

| Décision | Pourquoi |
|---|---|
| rendre `Option<SqlEngine>`, pas `SqlEngine` | « aucune signature » n'est pas `Unknown` : c'est « la voie 1 n'a rien dit, essaie la voie 2 ». Deux cas distincts |
| table `(SqlEngine, &[&str])` | associe la signature à **son** moteur, contre le piège de la table plate (§ 3.1) |
| première correspondance gagne | une erreur ne mêle pas deux moteurs ; le premier match est le bon |

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| une signature de ce moteur est dans le corps | boucler sur `sigs`, `body.contains(sig)` |
| aucun moteur ne matche | la boucle finit sans rien rendre → *absent* |

*Troisième temps :* on rend `Option` plutôt que `SqlEngine::Unknown` parce que les deux « je ne sais pas » ne sont pas le même. Ici, `absent` veut dire « pas d'indice **dans l'erreur**, mais les sondes peuvent encore trancher » ; `Unknown` (rendu par `fingerprint`) veut dire « même les sondes ont échoué ». Écraser le premier en second ferait sauter la voie 2 : on conclurait `Unknown` sans avoir sondé.

**Prototype**

```rust
pub fn engine_from_signature(body: &str) -> Option<SqlEngine>;
```

**Corps**

**Déroulé.** On parcourt la table `ENGINE_SIGNATURES`, moteur par moteur. Pour chaque moteur, on regarde si l'une de ses signatures apparaît dans le corps ; à la première trouvée, on rend ce moteur. Si l'on épuise la table sans aucune correspondance, on rend *absent* — pas d'indice de moteur dans ce corps, à distinguer de `Unknown` (§ 5.1).

### 5.2 · `probe`

**Ce qu'elle doit accomplir :** injecter une condition propre à un moteur (`AND <expr>`) et dire si la page « vraie » revient — c'est-à-dire si le moteur **comprend** l'expression. Une requête, un booléen.

**Décisions**

| Décision | Pourquoi |
|---|---|
| réutiliser `similar` contre la baseline | la sonde vraie doit ressembler à la page normale ; c'est exactement ce que `similar` mesure |
| suffixer `AND <expr>` à la valeur d'origine | reste dans le contexte de la requête, comme boolean-based (phase 4) |
| rendre `bool`, pas `SqlEngine` | une sonde répond oui/non à **une** hypothèse ; c'est `fingerprint` qui combine |

**⚠️ Pièges**

⚠️ Une expression toujours vraie **quel que soit le moteur** (ex. `AND 1=1`) ne discrimine rien : `probe` rendrait `true` partout. La valeur de la sonde tient à ce que l'expression **casse** sur les autres moteurs. Ne sonder qu'avec des fonctions spécifiques (§ 3.2).

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| injecter ` AND <expr>` dans le paramètre | `with_injected(key, valeur + " AND " + expr)` → phase 2 |
| la page paraît « vraie » | `similar(resp, baseline, tolerance)` → phase 4 § 5.1 |

*Troisième temps :* pourquoi `similar == true` signifie « le moteur connaît la fonction » : si la fonction existe, `<expr>` est une condition valide qui vaut vrai, donc la requête rend la **même** ligne que sans injection → page ≈ baseline ; si la fonction est inconnue, le moteur rejette tout le `SELECT` → page d'erreur ou vide, nettement différente. Le `similar` transforme donc « la fonction existe-t-elle ? » en « la page a-t-elle survécu ? », qu'on sait déjà mesurer.

**Lignes de log**

```rust
eprintln!("[engine] probe {expr} on '{key}': true={matched}");
```

**Prototype**

```rust
pub fn probe(client: &Client, cfg: &Config, target: &Target, key: &str,
             baseline: &Response, expr: &str) -> Result<bool, VaccineError>;
```

**Corps**

**Déroulé.** On forme le payload en suffixant à la valeur d'origine de `key` un ` AND ` suivi de l'expression sonde, on l'injecte dans le paramètre (→ phase 2) et on envoie la requête (→ phase 3). On compare la réponse à la baseline avec `similar` (→ phase 4 § 5.1) : le booléen qu'il rend dit si la page est « vraie », donc si le moteur comprend l'expression. On journalise (log ①) et on rend ce booléen.

### 5.3 · `fingerprint`

**Ce qu'elle doit accomplir :** l'orchestration. Tenter la voie gratuite (provoquer une erreur, lire sa signature) ; sur échec, lancer les deux sondes, les croiser, et rendre `MySql`, `Sqlite` ou `Unknown`.

**Décisions**

| Décision | Pourquoi |
|---|---|
| voie signature **avant** les sondes | gratuite quand l'erreur fuite ; les sondes coûtent deux requêtes |
| **deux** sondes croisées, pas une | une seule sonde vraie ne prouve pas que l'autre est fausse ; le croisement écarte un troisième moteur (§ 3.2) |
| `Unknown` si les deux sondes se contredisent ou échouent | on ne devine pas un moteur qu'on n'a pas confirmé : mieux vaut `Unknown` honnête qu'un faux MySQL |

**⚠️ Pièges**

⚠️ Conclure `Unknown` dès que la voie 1 échoue, sans sonder : on raterait tous les moteurs qui **cachent** leurs erreurs mais restent injectables (le cas même où boolean-based avait pris le relais en phase 4). La voie 2 est là pour ça.
⚠️ Rendre le moteur d'une sonde vraie **sans vérifier que l'autre est fausse** : deux sondes vraies (page instable, WAF qui laisse tout passer) donneraient un moteur arbitraire. Exiger `l'une vraie ET l'autre fausse`.

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| provoquer une erreur et la lire | injecter `valeur + "'"`, envoyer, puis `engine_from_signature(resp.body)` → § 5.1 |
| lancer les deux sondes | deux appels à `probe` → § 5.2 |
| croiser les résultats | un `match` sur le couple `(is_sqlite, is_mysql)`, quatre bras |

*Troisième temps :* le `match` sur le couple `(is_sqlite, is_mysql)` encode le tableau de croisement du § 2 tel quel. `(true, false)` → SQLite, `(false, true)` → MySQL, et les deux diagonales `(true, true)` / `(false, false)` tombent sur `Unknown`. Écrire le couple plutôt que deux `if` imbriqués rend l'exhaustivité visible : le compilateur vérifie que les quatre cas sont traités.

**Lignes de log**

```rust
eprintln!("[engine] fingerprint on '{key}': signature={sig_hit}, sqlite={is_sqlite}, mysql={is_mysql}");
```

**Prototype**

```rust
pub fn fingerprint(client: &Client, cfg: &Config, target: &Target,
                   key: &str, baseline: &Response) -> Result<SqlEngine, VaccineError>;
```

**Corps**

**Déroulé.** *Voie 1, gratuite.* On provoque une erreur en injectant la valeur d'origine suivie d'un `'`, on envoie (→ phase 3), et on passe le corps obtenu à `engine_from_signature` (→ § 5.1). S'il rend un moteur, on journalise (log ①) et on le rend aussitôt.

*Voie 2, les sondes.* Sinon, on lance deux sondes croisées via `probe` (→ § 5.2) : l'une pour SQLite (l'expression ` sqlite_version() > '0'`), l'autre pour MySQL (` VERSION() LIKE '%'`), chacune rendant un booléen. On journalise (log ①). On **croise** alors les deux résultats : la vraie seule côté SQLite donne `Sqlite`, la vraie seule côté MySQL donne `MySql`, et les deux autres cas — les deux vraies, ou les deux fausses — tombent sur `Unknown`, car on ne devine pas un moteur non confirmé.

---

## 6. Pièges spécifiques à cette phase

- **La voie 1 et la phase 4 partagent leurs chaînes, pas leur table.** Les signatures sont les mêmes mots, mais `SQL_ERRORS` (plate, phase 4) répond « il y a une erreur » et `ENGINE_SIGNATURES` (indexée, phase 5) répond « quel moteur ». Garder deux tables distinctes plutôt qu'en dériver une de l'autre : leur question diffère, et les fusionner rouvrirait le piège du § 3.1.
- **Le fingerprint hérite de l'instabilité de la baseline.** `probe` s'appuie sur `similar`, donc sur la même tolérance calibrée en phase 4. Si la page est trop instable pour boolean-based, les sondes seront tout aussi erratiques — c'est un symptôme commun, pas un bug de cette phase. Le régler en phase 4 le règle ici.
- **Ambiguïté résiduelle assumée.** On ne distingue MySQL que de SQLite, pas de MariaDB (compatible) ni de MSSQL (écarté par le choix de `VERSION()`). C'est conforme au sujet (deux moteurs) ; tout moteur non confirmé tombe en `Unknown`, ce que l'extraction (phase 6) traite comme « pas d'extraction fiable ».

---

## 7. Tests unitaires

> [!IMPORTANT]
> `probe` et `fingerprint` appellent le réseau → **hors périmètre unitaire** (même raison qu'en phases 3-4). La fonction **purement testable** est `engine_from_signature` : une chaîne en entrée, un `Option<SqlEngine>` en sortie, aucun I/O. On l'a isolée exprès pour ça.

### 7.1 `src/engine/fingerprint.rs` — `#[cfg(test)]`

**Ce que tu testes :**
- un corps contenant `You have an error in your SQL syntax` → `Some(SqlEngine::MySql)`.
- un corps contenant `SQL logic error` → `Some(SqlEngine::Sqlite)`.
- un corps quelconque (« welcome home ») → `None`.

**Ce que le test doit prouver au-delà du comportement :** que « aucune signature » rend bien `None` et **non** `Unknown` — c'est la distinction (§ 5.1) qui garde la voie 2 vivante. Un relecteur tenté de « simplifier » en renvoyant `Unknown` casserait le fallback ; le test l'en empêche.

**Stratégie.** `engine_from_signature` prend une `&str` et rend un `Option<SqlEngine>` : aucun réseau, on l'appelle directement avec des corps littéraux.

**Les cas à vérifier** (chacun devient un `#[test]`) :

| Entrée (corps) | Attendu | Pourquoi ce cas |
|---|---|---|
| `You have an error in your SQL syntax near '''` | `Some(SqlEngine::MySql)` | l'erreur MySQL nomme son moteur |
| `SQL logic error` | `Some(SqlEngine::Sqlite)` | l'erreur SQLite nomme le sien |
| `welcome home` (page banale) | `None` — surtout **pas** `Unknown` | c'est la distinction (§ 5.1) qui garde la voie 2 vivante |

> `SqlEngine` doit dériver `PartialEq` et `Debug` pour être comparé dans un test. C'est un réglage qui ne se devine pas : sans, l'erreur est un `E0369` sur `==` dans le test, pas dans le code testé.

### 7.2 Hors périmètre

| Fonction | Pourquoi | Comment on vérifie |
|---|---|---|
| `probe`, `fingerprint` | requêtes réseau | test manuel contre SQLi-Labs (MySQL) et un labo SQLite, lecture des logs `[engine]` |

### 7.3 Résultats attendus

- Unitaires `engine_from_signature` : PASS (les trois cas).
- Manuel : sur SQLi-Labs `?id=1`, le rapport annonce `MySQL` ; sur un labo SQLite, `SQLite` — PASS.

---

## 8. Ordre de développement recommandé

1. `SqlEngine` (enum, `#[derive(Debug, PartialEq)]`) et `ENGINE_SIGNATURES`.
2. `engine_from_signature` + ses tests — la seule brique hors-ligne.
3. `probe`, essayé à la main sur SQLi-Labs (`VERSION()` doit passer, `sqlite_version()` échouer).
4. `fingerprint` : voie 1 puis voie 2, en vérifiant le croisement des sondes sur les deux labos.
5. Câbler le moteur trouvé dans le rapport (phase 8).

> Quand l'outil annonce le bon moteur sur un labo MySQL **et** un labo SQLite, cette phase est close. On passe à la **phase 6 : extraction du schéma** ([phase-06-extraction-schema.md](phase-06-extraction-schema.md)).
