# Vaccine — Phase 8 : stockage des résultats (`-o`)

> Tout est trouvé : paramètre vulnérable, moteur, schéma, dump. Reste à le **présenter** et à l'**archiver**. La clé de conception est de séparer *collecter* de *écrire* : un `Report` accumulé pendant tout le scan, puis **une** fonction de rendu qui sert à la fois `stdout` et le fichier `-o`. Le fichier est créé automatiquement s'il n'existe pas (exigence du sujet).

---

## 1. Où on en est

**Fait :**
- Phases 1-7 : la `Config` (dont `output: Option<String>`, le `-o`), le `Verdict::Vulnerable`, le `SqlEngine`, et les extractions (`databases`/`tables`/`columns`, `dump`).

**À faire dans cette phase :**
- Un type `Report` qui rassemble **tout** ce qu'un scan a produit.
- `render` : transformer un `Report` en texte lisible (le même pour l'écran et le fichier).
- `save` : écrire ce texte dans le fichier `-o`, créé au besoin.

**Ce qui suit (phase 9) :** monter un environnement vulnérable et écrire la suite de tests (unitaires + intégration) exigée par le sujet.

> Prérequis Rust : l'écriture de fichiers de [`rust/03`](../rust/03-http-io-et-crates.md) § 5 — `OpenOptions`, `fs::write`, `writeln!`, et la conversion d'une `io::Error` en `VaccineError`.

---

## 2. Architecture cible

```
   scanner::run remplit un Report au fil du scan :
        Report { target, method, vuln: {param, technique, payload},
                 engine, databases, tables, columns, dump, notes }
                          │
                 ┌────────▼─────────┐
                 │  report::render  │   Report → String (texte unique)
                 └───┬──────────┬───┘
        stdout ◄─────┘          └────► save(path) ─► fichier -o (create + append)
     println!("{s}")                   OpenOptions.create(true).append(true)
```

**Point clé sur le flux :** `render` est la **seule** source de mise en forme. `stdout` et le fichier reçoivent exactement la même chaîne — l'un par `println!`, l'autre par `save`. Écrire le rapport « au fil de l'eau » depuis dix endroits du scan disperserait le format ; un `Report` accumulé et un unique `render` le gardent en un seul lieu.

---

## 3. Concepts à maîtriser

### 3.1 Accumuler un `Report`, écrire une fois

Le scanner découvre les informations **dans le temps** (d'abord le paramètre, puis le moteur, puis le schéma…). Plutôt que d'imprimer chaque bribe à mesure, on la **range** dans un `Report`, et on n'imprime qu'à la fin. Deux bénéfices : la mise en forme vit à un seul endroit (`render`), et le même `Report` alimente l'écran **et** le fichier sans duplication.

Documentation :
- Précédent : le `Config` de [phase 1](phase-01-cli.md) — même idée d'un type qui rassemble un état, ici enrichi pendant le scan.

> Analogie : remplir un formulaire au fur et à mesure d'une enquête, puis le photocopier une fois pour l'écran et pour les archives — au lieu de crier chaque réponse dès qu'on la trouve.

⚠️ **Piège classique** : `println!` disséminés dans `scanner`, `engine`, `extract`. **Symptôme** : changer une virgule de format oblige à toucher dix fichiers, et la sortie fichier finit différente de l'écran. **Correction** : rien n'imprime sauf `render`/`save` ; le reste **remplit** le `Report`.

### 3.2 Créer le fichier, archiver ou écraser

Le sujet veut un fichier créé automatiquement s'il n'existe pas. Deux modes, selon l'intention :

| Mode | API | Effet |
|---|---|---|
| Archiver (empiler les scans) | `OpenOptions::new().create(true).append(true)` | ajoute à la suite ; le fichier grandit |
| Écraser (dernier scan seul) | `fs::write(path, contents)` | remplace le contenu |

Documentation :
- `OpenOptions`, `fs::write`, `writeln!` : [`rust/03`](../rust/03-http-io-et-crates.md) § 5, et https://doc.rust-lang.org/std/fs/

⚠️ **Piège classique** : `unwrap()` sur l'ouverture du fichier. **Symptôme** : un `-o /root/x` (droits refusés) fait paniquer l'outil au lieu d'un message clair. **Correction** : mapper l'`io::Error` en `VaccineError::Io` et la remonter par `?`.

### 3.3 `stdout` propre, `stderr` pour les logs

Depuis la phase 3, les logs `[http]` / `[detect]` / `[extract]` vont sur **`stderr`**. Le **rapport** va sur **`stdout`**. Cette séparation permet `./vaccine … > resultat.txt` sans que les logs polluent le fichier, et laisse `-o` gérer l'archivage structuré à part.

⚠️ **Piège classique** : imprimer le rapport avec `eprintln!`. **Symptôme** : `> fichier` capture les logs et rate le rapport. **Correction** : rapport sur `stdout` (`println!`), logs sur `stderr` (`eprintln!`).

### 3.4 Lexique

| Terme | Ce qu'il désigne | Où ça vit |
|---|---|---|
| `Report` | l'agrégat de tout ce qu'un scan a trouvé | [`src/report/model.rs`](../../src/report/model.rs) |
| `render` | `Report` → texte lisible, unique source de format | `src/report/save.rs` |
| `save` | écrit le texte rendu dans le fichier `-o` | `src/report/save.rs` |
| archiver | ajouter à la suite d'un fichier existant (`append`) | `report::save` |

---

## 4. Décomposition des étapes

1. **`Report`** (type) — les champs qui rassemblent le résultat d'un scan.
2. **`render`** — pure : `Report` → `String`. Testable.
3. **`save`** — écrire le texte rendu dans le fichier.
4. **Câblage** — `scanner::run` remplit le `Report` ; `main` `println!` toujours, `save` si `-o`.

> `render` est pure et se teste sans fichier ; `save` n'ajoute que l'écriture autour d'elle.

---

## 5. Le corps du document : une section par fonction

### 5.0 · Boîte à outils

| Opération | API exacte |
|---|---|
| Ouvrir en création + ajout | `OpenOptions::new().create(true).append(true).open(path)` |
| Écrire (écraser) | `std::fs::write(path, contents)` |
| Écrire une ligne formatée | `writeln!(file, "…{}…", value)?` |
| Formater sans écrire | `format!("…")`, `String::push_str` |
| Imprimer le rapport | `println!("{}", rendered)` (stdout) |
| Mapper l'erreur d'I/O | `.map_err(|e| VaccineError::Io(e.to_string()))` |

### 5.1 · `Report` (le type)

**Ce qu'il doit contenir** (on **décrit** les champs, on ne donne pas la déclaration) :

| Champ | Contenu | Vient de |
|---|---|---|
| cible | l'URL scannée | `Config.url` |
| méthode | GET ou POST | `Config.method` |
| vulnérabilité | le paramètre, la technique, le payload gagnant — ou « aucune » | `Verdict` (phase 4) |
| moteur | `MySql` / `Sqlite` / `Unknown` | phase 5 |
| bases / tables / colonnes | les listes extraites | phase 6 |
| dump | le tableau de lignes, par table | phase 7 |
| notes | erreurs rencontrées, extractions ratées (best effort) | tout le scan |

**Décisions**

| Décision | Pourquoi |
|---|---|
| un seul type porte tout | une valeur unique à passer à `render`, à `save`, aux tests |
| les champs « optionnels » sont des `Option` / `Vec` vides | un scan qui s'arrête tôt (param non vulnérable) rend un `Report` **partiel** valide, pas une erreur |
| le scanner le **remplit**, ne l'imprime pas | sépare collecter d'écrire (§ 3.1) |

> Pas de prototype : c'est une structure de données. Décrivez ses champs d'après le tableau ci-dessus et laissez le type s'écrire — un `struct` avec des `Option`, des `Vec`, et pour le dump un `Vec<(String, Vec<Vec<String>>)>` (table → lignes).

### 5.2 · `render`

**Ce qu'elle doit accomplir :** transformer un `Report` en texte lisible, mis en forme comme l'[exemple de sortie du sujet](../vaccine.md#-exemple-de-sortie) : la cible, le verdict (paramètre + technique + payload), le moteur, puis les listes et le dump en tableau. Le **même** texte servira `stdout` et le fichier.

**Décisions**

| Décision | Pourquoi |
|---|---|
| rendre une `String`, n'imprimer ni n'écrire | pure → testable, et réutilisable pour écran **et** fichier |
| sections claires, dans l'ordre de découverte | le lecteur suit le raisonnement du scan |
| un `Report` sans vulnérabilité rend un texte « aucune injection trouvée » | l'absence est un résultat lisible, pas un vide |

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| accumuler du texte section par section | `String` + `push_str` / `write!` sur la `String` |
| formater le dump en tableau | boucler sur les lignes, joindre les cellules par ` \| ` |
| omettre une section vide | tester `Vec::is_empty` avant de l'écrire |

*Troisième temps :* `render` ne lit **que** le `Report`, jamais le réseau ni un fichier — c'est ce qui la rend testable et déterministe : à `Report` égal, texte égal. Toute donnée qu'elle affiche doit donc déjà être **dans** le `Report`, ce qui force le scanner à tout y ranger (§ 3.1).

**Prototype**

```rust
pub fn render(report: &Report) -> String;
```

**Corps**

**Déroulé.** On part d'une `String` vide qu'on enrichit section par section. On écrit d'abord l'en-tête : la cible et la méthode. Puis le verdict : si une vulnérabilité est présente, on nomme le paramètre, la technique et le payload ; sinon on écrit « aucune injection trouvée » et on s'arrête là (le reste serait vide). Ensuite le moteur, puis les listes non vides (bases, tables, colonnes), chacune sous son titre. Puis, pour chaque table dumpée, on met les lignes en tableau — les cellules jointes par un séparateur lisible (` | `). On termine par les notes (erreurs, extractions ratées) s'il y en a. On rend la `String` complète.

### 5.3 · `save`

**Ce qu'elle doit accomplir :** écrire le texte rendu dans le fichier `-o`, en le créant s'il n'existe pas. Une seule I/O, toute erreur remontée proprement.

**Décisions**

| Décision | Pourquoi |
|---|---|
| appeler `render` à l'intérieur | garantit fichier == écran ; l'appelant ne re-formate pas |
| `create(true).append(true)` | archive plusieurs scans à la suite (§ 3.2), comme le suggère « archiver » |
| séparer les scans par une ligne de démarcation | un fichier d'archive reste lisible entre deux runs |

**⚠️ Pièges**

⚠️ Écrire sans `\n` final : deux scans archivés se collent. Terminer par un saut de ligne (et un séparateur visible).
⚠️ `unwrap()` sur l'I/O : un chemin non inscriptible fait paniquer. Mapper en `VaccineError::Io` (§ 3.2).

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| obtenir le texte | `render(report)` → § 5.2 |
| ouvrir en création + ajout | `OpenOptions::new().create(true).append(true).open(path)` → rust/03 § 5 |
| écrire | `writeln!(file, "{}", text)?` |
| convertir l'erreur | `.map_err(|e| VaccineError::Io(e.to_string()))` |

*Troisième temps :* `save` **n'imprime pas** à l'écran — c'est `main` qui fait le `println!`. Garder l'écriture fichier et l'affichage écran **séparés** (mais nourris par le même `render`) évite qu'un `-o` absent supprime aussi l'affichage : sans `-o`, on `println!` seulement ; avec, on `println!` **et** on `save`.

**Lignes de log**

```rust
eprintln!("[report] saved to {path}");
```

**Prototype**

```rust
pub fn save(report: &Report, path: &str) -> Result<(), VaccineError>;
```

**Corps**

**Déroulé.** On obtient le texte via `render` (→ § 5.2). On ouvre le fichier `path` en mode création + ajout ; toute erreur d'ouverture est convertie en `VaccineError::Io` et remontée par `?`. On écrit le texte suivi d'un saut de ligne et d'un séparateur de scan (même conversion d'erreur à l'écriture). On journalise (log ①) et on rend `Ok`.

### 5.4 · Câblage dans `main`

**Ce qu'il doit accomplir :** donner à `main` sa forme **définitive** — récupérer le `Report` produit par le scan, l'afficher sur `stdout` via `render`, et, si `-o` est fourni, l'archiver via `save`. C'est le point d'intégration final du projet.

**Décisions**

| Décision | Pourquoi |
|---|---|
| affichage `stdout` **toujours**, `save` **seulement si** `-o` | un scan sans `-o` doit quand même montrer son résultat |
| `render` pour l'écran, `save` réutilise `render` | fichier == écran, une seule mise en forme |
| erreur de `save` → `stderr` + code non nul | l'échec d'archivage est un échec de commande |

**⚠️ Pièges**

⚠️ Conditionner l'affichage à `-o` : sans `-o`, plus rien ne s'afficherait. `stdout` **toujours** ; `save` **en plus** quand `-o` est là.

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| obtenir le `Report` | `scanner::run(&cfg)?` (rend le `Report` accumulé) |
| afficher | `println!("{}", render(&report))` → § 5.2 |
| archiver si demandé | `if let Some(path) = &cfg.output { save(&report, path)? }` → § 5.3 |

*Troisième temps :* `main` reste mince même à sa forme finale : il ne **formate** rien (c'est `render`) et n'**écrit** rien lui-même (c'est `save`). Il ne décide que *quand* appeler chacun — écran toujours, fichier sur `-o`. C'est l'aboutissement du choix de la phase 4 : toute l'orchestration dans `run`, tout le rendu dans `report`, `main` n'étant que l'aiguillage entre les deux.

**Prototype**

```rust
fn main();
```

> Variante idiomatique : `fn main() -> Result<(), VaccineError>` laisse le `?` remonter l'erreur et fixer le code de sortie sans `match` explicite.

**Corps**

**Déroulé.** On lit la `Config`, on appelle `scanner::run` qui rend le `Report` complet du scan. On l'affiche **toujours** sur `stdout` via `render` (→ § 5.2). Si `cfg.output` porte un chemin, on appelle `save` (→ § 5.3) pour l'archiver, en traduisant une éventuelle erreur d'I/O en code de sortie non nul. C'est le `main` **définitif** : les phases suivantes n'y touchent plus.

---

## 6. Pièges spécifiques à cette phase

- **Le câblage dans `main` est le vrai point d'intégration.** Le rapport s'affiche **toujours** sur `stdout` ; le `save` n'a lieu **que si** `cfg.output` est `Some(path)`. Ne conditionnez pas l'affichage à `-o` : un utilisateur sans `-o` veut quand même voir le résultat.
- **`render` doit tolérer un `Report` partiel.** Un scan qui s'arrête à « paramètre non vulnérable » n'a ni moteur ni dump. `render` teste chaque section (`Option`, `is_empty`) et n'écrit que ce qui existe — sinon des titres vides, ou un `unwrap` sur un `None`.
- **L'archivage change la sémantique du fichier.** Avec `append`, le fichier est un **journal** de scans, pas l'image du dernier. Si le rendu attendu est « le dernier scan seulement », c'est `fs::write` qu'il faut, pas `OpenOptions::append`. Tranchez selon la lecture du sujet et documentez le choix.

---

## 7. Tests unitaires

> [!IMPORTANT]
> `save` touche le système de fichiers → **hors périmètre unitaire pur** (ou alors avec un fichier temporaire, à ranger en intégration). Le point **purement testable** est `render` : un `Report` construit à la main → une `String`. Aucune I/O.

### 7.1 `src/report/save.rs` — `#[cfg(test)]`

**Ce que tu testes :**
- `render` d'un `Report` vulnérable mentionne le paramètre, la technique et le payload.
- `render` d'un `Report` **sans** vulnérabilité produit le texte « aucune injection trouvée » et **pas** de section moteur/dump.
- une section vide (aucune table) n'apparaît pas dans le texte.

**Ce que le test doit prouver au-delà du comportement :** que `render` reste correct sur un `Report` **partiel** (le cas « pas vulnérable »), et qu'il n'imprime jamais une section vide — deux robustesses qu'une simplification casserait.

**Stratégie.** Aucun réseau ni fichier : on construit un `Report` à la main (fabrique locale décrite) et on inspecte la `String` rendue avec `contains`.

**Les cas à vérifier** (chacun devient un `#[test]`) :

| Report (construit à la main) | Attendu dans la sortie | Pourquoi ce cas |
|---|---|---|
| vulnérable : param `id`, technique error-based, payload `'` | contient `id`, `error`, `'` | le résultat nominal est rapporté |
| sans vulnérabilité | contient « aucune injection », **pas** de « moteur » | un Report partiel se rend proprement |
| vulnérable mais `tables` vide | ne contient **pas** de titre « Tables » | pas de section vide |

### 7.2 Hors périmètre

| Fonction | Pourquoi | Comment on vérifie |
|---|---|---|
| `save` | écrit un fichier réel | test d'intégration avec un fichier temporaire (phase 9), ou vérif manuelle du `-o` |

### 7.3 Résultats attendus

- Unitaires `render` : PASS (les trois cas).
- Manuel : `./vaccine -o /tmp/r.txt "…"` affiche le rapport **et** crée `/tmp/r.txt` avec le même contenu ; un second run l'**archive** à la suite — PASS.

---

## 8. Ordre de développement recommandé

1. Le type `Report` (champs décrits au § 5.1), vide, qui compile.
2. `render` pour le cas vulnérable, + son test ; puis le cas « pas vulnérable ».
3. Faire remplir le `Report` par `scanner::run` (paramètre, moteur, schéma, dump).
4. `save` avec `create(true).append(true)`, + mapping d'erreur.
5. Câbler `main` : `println!` toujours ; `if let Some(path) = &cfg.output { save(&report, path)? }`.
6. Test manuel : sortie écran, création de fichier, archivage d'un second run.

> Quand un scan complet s'affiche à l'écran **et** s'archive dans le fichier `-o`, à l'identique, cette phase est close. On passe à la **phase 9 : tests et environnement vulnérable** ([phase-09-tests-environnement.md](phase-09-tests-environnement.md)).
