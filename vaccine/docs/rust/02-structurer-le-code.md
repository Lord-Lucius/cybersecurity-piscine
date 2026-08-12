# Rust pour vaccine — 2. Structurer le code

> À lire **avant la phase 2**, quand on passe d'un `main.rs` unique à plusieurs modules. On voit ici : modéliser les données avec `struct` et `enum`, décider avec `match`, factoriser un comportement avec `trait`, découper en `mod`, et fabriquer le type d'erreur du projet.

---

## 1. `struct` : regrouper des données qui vont ensemble

Une `struct` est un type qui agrège des champs nommés. C'est le bon outil dès que trois variables voyagent toujours ensemble.

```rust
struct Config {
    url: String,
    method: HttpMethod,
    output: Option<String>,   // -o est optionnel, donc Option
}

let cfg = Config {
    url: "http://localhost/?id=1".to_string(),
    method: HttpMethod::Get,
    output: None,
};
println!("{}", cfg.url);      // accès par point
```

On attache des fonctions à une struct dans un bloc `impl`. Une méthode qui prend `&self` emprunte l'instance (lecture) ; `&mut self` la modifie ; sans `self`, c'est une fonction associée (souvent un constructeur `new`).

```rust
impl Config {
    fn new(url: String) -> Self {                 // Self = Config
        Config { url, method: HttpMethod::Get, output: None }
    }
    fn is_post(&self) -> bool {                    // emprunte, ne modifie pas
        matches!(self.method, HttpMethod::Post)
    }
}
```

---

## 2. `enum` : « c'est l'un OU l'autre »

Un `enum` liste les valeurs possibles d'un type. C'est l'outil qui manque cruellement en C, où on simule ça avec des `#define` et un `int`. Ici la méthode HTTP ne peut être **que** GET ou POST — l'exprimer par un enum rend tout autre état impossible à représenter.

```rust
enum HttpMethod {
    Get,
    Post,
}

enum SqlEngine {
    MySql,
    Sqlite,
    Unknown,
}
```

La force du `enum` Rust : **chaque variante peut porter des données**. `Option` et `Result` que vous connaissez déjà (fiche 1) sont *exactement* ça :

```rust
enum Option<T> { Some(T), None }
enum Result<T, E> { Ok(T), Err(E) }
```

On s'en servira pour modéliser une technique et son verdict :

```rust
enum Verdict {
    Vulnerable { payload: String },   // porte le payload gagnant
    Safe,
}
```

---

## 3. `match` : décider de façon exhaustive

`match` compare une valeur à des motifs. Sa qualité déterminante : **il est exhaustif**. Si vous ajoutez une variante `SqlEngine::Postgres` et oubliez de la traiter quelque part, le code **ne compile pas**. Le compilateur transforme un oubli en erreur, au lieu d'un bug silencieux.

```rust
fn engine_name(engine: &SqlEngine) -> &str {
    match engine {
        SqlEngine::MySql   => "MySQL",
        SqlEngine::Sqlite  => "SQLite",
        SqlEngine::Unknown => "unknown",
        // retirer une ligne ci-dessus → erreur "non-exhaustive patterns"
    }
}
```

On peut extraire les données portées par une variante directement dans le motif :

```rust
match verdict {
    Verdict::Vulnerable { payload } => println!("vulnerable with {payload}"),
    Verdict::Safe                   => println!("safe"),
}
```

> Analogie : `match` est un aiguillage de gare où **toutes** les voies doivent être branchées. Une voie oubliée n'est pas une voie « qui ne fait rien » — c'est un chantier que le contrôleur (le compilateur) refuse de laisser ouvrir.

⚠️ **Piège classique** : le motif attrape-tout `_ => ...` désactive l'exhaustivité. Pratique, mais si vous ajoutez ensuite une variante, `_` l'avale en silence et vous perdez le filet de sécurité. **Ne mettez `_` que pour des cas réellement indifférents**, jamais « pour faire taire le compilateur ».

---

## 4. `trait` : un comportement partagé par plusieurs types

Un `trait` est un contrat : « tout type qui l'implémente sait faire X ». C'est l'équivalent des interfaces. Sur ce projet, il devient utile quand chaque moteur SQL doit fournir *ses* payloads d'extraction — même question posée, réponse spécifique au moteur.

```rust
trait Extractor {
    fn list_tables_payload(&self) -> String;   // chaque moteur répond à sa façon
    fn list_dbs_payload(&self) -> String;
}

struct MySqlExtractor;
impl Extractor for MySqlExtractor {
    fn list_tables_payload(&self) -> String {
        "UNION SELECT table_name FROM information_schema.tables-- -".to_string()
    }
    fn list_dbs_payload(&self) -> String {
        "UNION SELECT schema_name FROM information_schema.schemata-- -".to_string()
    }
}
```

> [!NOTE]
> Le `trait` est un outil de **phase 5+**, pas du démarrage. Tant qu'il n'y a qu'un moteur, un `match SqlEngine` suffit et se lit mieux. On introduit le trait le jour où le `match` commence à se répéter dans plusieurs fonctions d'extraction. Ne pas sur-architecturer avant.

---

## 5. Les modules : un fichier = un `mod`

On déclare les modules dans **`lib.rs`** (la racine de la bibliothèque) ; chaque nom correspond à un fichier `src/<nom>.rs`. Le pourquoi de cette séparation lib/binaire — et ce qu'elle apporte aux tests — est détaillé dans [`../organisation-et-tests.md`](../organisation-et-tests.md).

```rust
// dans src/lib.rs
pub mod cli;    // ← charge src/cli.rs, et l'expose hors de la crate (tests/ inclus)
pub mod url;    // ← charge src/url.rs
pub mod error;  // ← charge src/error.rs
```

```rust
// dans src/main.rs — le binaire mince, qui consomme la bibliothèque
use vaccine::cli::{self, Config};

fn main() {
    let cfg: Config = cli::parse(std::env::args().collect()).unwrap();
}
```

Deux règles de visibilité :

- Tout est **privé au module par défaut**. Pour qu'un type ou une fonction soit utilisable depuis `main.rs` ou depuis `tests/`, marquez-le **`pub`** : `pub struct Config`, `pub fn parse(...)`.
- Les champs d'une struct sont privés séparément : `pub struct Config { pub url: String }`.

```rust
// dans src/cli.rs
pub struct Config { pub url: String, /* … */ }

pub fn parse(args: Vec<String>) -> Config { /* … */ }
```

⚠️ **Piège classique** : vous créez `src/cli.rs` mais oubliez `pub mod cli;` dans `lib.rs`. **Symptôme** : `file not found for module` *ou* le fichier est tout bonnement ignoré et vous obtenez `cannot find function parse`. **Le fichier seul ne suffit pas : il faut le déclarer.**

---

## 6. Le type d'erreur du projet

La fiche 1 a montré `?` : il propage une erreur *si toutes les fonctions parlent le même langage d'erreur*. On se donne donc **un enum d'erreur unique** pour tout le projet. Chaque variante décrit une catégorie de panne.

```rust
// dans src/error.rs
#[derive(Debug)]
pub enum VaccineError {
    Usage(String),        // mauvais arguments
    Http(String),         // échec réseau
    Io(String),           // échec fichier
    Parse(String),        // URL/param illisible
}

use std::fmt;
impl fmt::Display for VaccineError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            VaccineError::Usage(m) => write!(f, "usage: {m}"),
            VaccineError::Http(m)  => write!(f, "http: {m}"),
            VaccineError::Io(m)    => write!(f, "io: {m}"),
            VaccineError::Parse(m) => write!(f, "parse: {m}"),
        }
    }
}
```

`#[derive(Debug)]` est un **attribut** : il demande au compilateur de générer automatiquement le code d'affichage `{:?}` (utile en debug et pour `.unwrap()`). Le bloc `Display` fournit, lui, le `{}` propre destiné à l'utilisateur.

**Convertir une erreur d'une crate en la nôtre**, pour que `?` fonctionne à travers la frontière : on implémente `From`. Après ça, un `?` sur une erreur `std::io::Error` la transforme tout seul en `VaccineError::Io`.

```rust
impl From<std::io::Error> for VaccineError {
    fn from(e: std::io::Error) -> Self {
        VaccineError::Io(e.to_string())
    }
}
```

> [!TIP]
> **Ce que fait vraiment `?`, complété.** Sur `Err(e)`, `?` appelle `VaccineError::from(e)` *avant* de renvoyer. C'est ce `From` qui permet à une fonction rendant `Result<_, VaccineError>` d'utiliser `?` sur un appel de `ureq` ou de `std::fs`, sans écrire la conversion à la main à chaque ligne. Une implémentation `From` par crate externe, et tout le reste du code reste propre.

On aura alors, dans `main` :

```rust
fn main() -> Result<(), VaccineError> {
    let cfg = cli::parse(std::env::args().collect())?;
    scanner::run(&cfg)?;
    Ok(())
}
```

Toute erreur, d'où qu'elle vienne, remonte jusqu'ici et s'affiche via `Display`. Aucun `unwrap()` dans le chemin principal.

---

## Récapitulatif : quel outil pour quel besoin

| Besoin | Outil |
|---|---|
| Des champs qui voyagent ensemble | `struct` |
| « L'un parmi une liste fixe » | `enum` |
| Décider sans rien oublier | `match` |
| Même question, réponse par type | `trait` |
| Découper en fichiers | `mod` + `pub` |
| Une erreur qui remonte proprement | un `enum` + `Display` + `From` + `?` |

La [phase 2](../phases/phase-02-url-et-parametres.md) met tout ça en pratique sur le parsing de l'URL.
