# Vaccine — Organisation des fichiers et stratégie de tests

> Comment `src/` est agencé, à quoi sert chaque fichier, et où va chaque type de test. Deux décisions structurantes : **séparer la bibliothèque (`lib.rs`) du binaire (`main.rs`)**, et **un dossier par module** avec des fichiers-feuilles courts. À lire dès la [phase 1](phases/phase-01-cli.md) : c'est la carte du dépôt.

---

## 1. L'arbre réel du dépôt

Le squelette est déjà en place. Les fichiers `*.rs` **feuilles** (à remplir) sont vides ; les `mod.rs` et `lib.rs` ne contiennent que le câblage des modules.

```
vaccine/
├── Cargo.toml
├── Cargo.lock                 committé : build reproductible (exigence du sujet)
├── Makefile
├── README.md
├── vaccine                    binaire copié à la racine par `make`
├── src/
│   ├── main.rs                binaire MINCE : argv → lib → code de sortie
│   ├── lib.rs                 racine de la bibliothèque : déclare les `pub mod`
│   ├── error.rs               VaccineError, propagé par ?  (fichier unique, pas de dossier)
│   ├── cli/                   ── phase 1 : ligne de commande
│   │   ├── mod.rs             câblage du module cli
│   │   ├── config.rs          types Config + HttpMethod
│   │   └── parse.rs           parse(argv) + print_help()
│   ├── url/                   ── phase 2 : URL et paramètres
│   │   ├── mod.rs
│   │   ├── model.rs           types Target + Param
│   │   └── parse.rs           parse(url) + with_injected()
│   ├── http/                  ── phase 3 : client HTTP
│   │   ├── mod.rs
│   │   ├── client.rs          Client + send()
│   │   └── response.rs        type Response (status + body)
│   ├── scanner/               ── phase 4 : orchestration
│   │   ├── mod.rs
│   │   └── run.rs             run() : baseline puis params × techniques
│   ├── techniques/            ── phase 4 : techniques d'injection
│   │   ├── mod.rs
│   │   ├── compare.rs         similar() + type Verdict
│   │   ├── signatures.rs      table SQL_ERRORS
│   │   ├── error_based.rs     technique error-based
│   │   └── boolean_based.rs   technique boolean-based
│   ├── engine/                ── phase 5 : fingerprint moteur
│   │   ├── mod.rs
│   │   └── fingerprint.rs     enum SqlEngine + fingerprint()
│   ├── extract/               ── phases 6-7 : extraction
│   │   ├── mod.rs
│   │   ├── union.rs           nombre de colonnes, helpers UNION
│   │   ├── schema.rs          databases / tables / columns
│   │   ├── dump.rs            dump des lignes
│   │   ├── mysql.rs           payloads spécifiques MySQL
│   │   └── sqlite.rs          payloads spécifiques SQLite
│   └── report/                ── phase 8 : rapport
│       ├── mod.rs
│       ├── model.rs           struct Report (accumulée pendant le scan)
│       └── save.rs            save() (-o) + rendu stdout
├── tests/                     tests d'INTÉGRATION (crate externe, ne voit que le pub)
│   ├── cli.rs                 parsing bout-en-bout
│   ├── url.rs                 parsing d'URL bout-en-bout
│   ├── detection.rs           scan réel (marqué #[ignore] : réseau)
│   └── common/
│       └── mod.rs             helpers partagés entre tests d'intégration
└── docs/ …
```

---

## 2. Pourquoi ce découpage

Deux principes, chacun paie une difficulté concrète.

**Un dossier par module, des fichiers-feuilles courts.** Chaque grande responsabilité (parser la CLI, parler HTTP, injecter…) est un dossier ; à l'intérieur, on sépare les **types** de la **logique**. Trois bénéfices :

| Bénéfice | Concrètement |
|---|---|
| On sait où écrire | « la comparaison de réponses » → `techniques/compare.rs`, sans hésiter |
| Les fichiers restent lisibles | un fichier = une poignée de fonctions, pas 400 lignes fourre-tout |
| Les diffs et les tests sont ciblés | un test de `similar` touche `compare.rs`, pas un module géant |

**Types d'un côté, logique de l'autre** (`config.rs` vs `parse.rs`, `model.rs` vs `parse.rs`, `response.rs` vs `client.rs`). Les types changent rarement et servent de contrat ; la logique bouge souvent. Les séparer évite qu'une retouche d'algorithme fasse défiler la définition des structures, et inversement.

> [!NOTE]
> **`error.rs` reste un fichier unique, sans dossier.** Un module atomique (un seul enum + ses `impl`) n'a rien à ranger : lui imposer un dossier serait du rangement pour le rangement. La règle « un dossier par module » vaut **quand il y a plusieurs fichiers à grouper**, pas par principe.

> [!TIP]
> **Ce découpage est une proposition, pas une prison.** Les feuilles sont vides : fusionner `config.rs` et `parse.rs` en un seul `cli.rs`, ou éclater `dump.rs` davantage, se fait en supprimant un fichier et une ligne `pub mod`. Ajustez au fil de l'écriture — mais tranchez tôt (le [template](template_phases.md) § 18 rappelle ce que coûte un changement de structure tardif).

---

## 3. La séparation bibliothèque + binaire

Un projet Rust purement binaire (`main.rs` seul) **ne peut pas** être testé par le dossier `tests/` : un test d'intégration est compilé comme une **crate séparée** qui fait `use vaccine::…`, et un binaire n'expose aucun symbole. Sans rien exposer, `tests/` ne pourrait tester `vaccine` qu'en le lançant comme un process externe et en lisant sa sortie — possible, mais lourd et incapable d'atteindre les fonctions internes.

La forme standard résout ça : **toute la logique va dans la bibliothèque**, `main.rs` n'est qu'un lanceur.

```rust
// src/lib.rs — déclare les modules et expose l'API publique
pub mod error;
pub mod cli;
pub mod url;
pub mod http;
pub mod scanner;
pub mod techniques;
pub mod engine;
pub mod extract;
pub mod report;

// (optionnel) le point d'entrée logique, testable depuis tests/ :
// pub fn run(config: cli::config::Config) -> Result<(), error::VaccineError> { … }
```

```rust
// src/main.rs — le binaire : mince exprès, rien à tester ici
use std::process;
use vaccine::cli;

fn main() {
    let config = match cli::parse::parse(std::env::args().collect()) {
        Ok(c) => c,
        Err(e) => { eprintln!("{e}"); process::exit(1); }
    };
    // vaccine::run(config)…
}
```

> [!IMPORTANT]
> **Cargo détecte `lib.rs` et `main.rs` automatiquement** : bibliothèque et binaire portent tous deux le nom du paquet (`vaccine`), et **aucune modification de `Cargo.toml` n'est nécessaire**. Le binaire importe la lib avec `use vaccine::…`, exactement comme le fait `tests/`.

> [!NOTE]
> Conséquence sur les documents de phase : c'est **`lib.rs`** qui porte les `pub mod`, pas `main.rs`. Chaque `mod.rs` de dossier déclare ensuite ses feuilles (`pub mod config; pub mod parse;`).

---

## 4. Le rôle des `mod.rs` et les re-exports

Un `mod.rs` fait deux choses : **déclarer** les fichiers-feuilles du dossier, et **ré-exposer** leurs types pour raccourcir les chemins.

Aujourd'hui les feuilles sont vides, donc les `mod.rs` ne contiennent **que** les déclarations :

```rust
// src/cli/mod.rs — état actuel
pub mod config;
pub mod parse;
```

Quand `config.rs` contiendra `Config`, ajoutez un re-export pour que le reste du code écrive `cli::Config` au lieu de `cli::config::Config` :

```rust
// src/cli/mod.rs — une fois les feuilles remplies
pub mod config;
pub mod parse;

pub use config::{Config, HttpMethod};   // raccourci : vaccine::cli::Config
pub use parse::parse;                    // raccourci : vaccine::cli::parse(...)
```

> [!CAUTION]
> **N'ajoutez le `pub use` qu'une fois le type écrit.** Un `pub use config::Config;` alors que `config.rs` est vide **ne compile pas** (`unresolved import`). C'est pour ça que les `mod.rs` livrés ne portent que les `pub mod` : ils compilent avec des feuilles vides. Le re-export est la deuxième étape, pas la première.

---

## 5. Les trois niveaux de test

`cargo test` — donc `make test` — compile et lance les trois d'un coup.

| Niveau | Emplacement | Ce qu'il voit | Exemple sur ce projet |
|---|---|---|---|
| **Unitaire** | `#[cfg(test)] mod tests` en bas de la **feuille** concernée | tout, **y compris le privé** | `similar` dans `techniques/compare.rs`, `parse` dans `url/parse.rs` |
| **Intégration** | un fichier par thème dans `tests/` | seulement le `pub` de la lib | `cli::parse` bout-en-bout, `run()` sur une entrée maîtrisée |
| **Doc-test** | blocs ` ```rust ` dans les `///` | le `pub` | garder les exemples de la doc exacts |

### 5.1 Unitaire — dans la feuille du module

Le niveau principal du projet. Le module de test est **dans le même fichier** que la fonction, donc il atteint le privé. Compilé seulement sous `cargo test` grâce à `#[cfg(test)]`.

```rust
// bas de src/url/parse.rs
pub fn parse(url: &str) -> Target { /* … */ }

#[cfg(test)]
mod tests {
    use super::*;                    // importe le module parent, privé compris

    #[test]
    fn parses_two_params() {
        let t = parse("http://h/p?id=1&cat=books");
        assert_eq!(t.params.len(), 2);
    }
}
```

### 5.2 Intégration — dans `tests/`

Chaque fichier de `tests/` est une **crate indépendante** qui utilise `vaccine` de l'extérieur : il ne teste que l'API `pub`. Bon niveau pour les enchaînements, pas pour les détails internes.

```rust
// tests/cli.rs
use vaccine::cli::{parse::parse, config::HttpMethod};

#[test]
fn parses_post_from_full_argv() {
    let argv = ["./vaccine", "-X", "POST", "http://h/"]
        .iter().map(|s| s.to_string()).collect();
    let cfg = parse(argv).unwrap();
    assert!(matches!(cfg.method, HttpMethod::Post));
}
```

> [!CAUTION]
> **`tests/common/mod.rs`, jamais `tests/common.rs`.** Cargo traite *chaque fichier* directement sous `tests/` comme une crate de test : un `tests/common.rs` produirait un « 0 test » parasite. Le sous-dossier `common/mod.rs` est reconnu comme **module partagé**, importé par `mod common;` en tête des tests qui en ont besoin.

### 5.3 Doc-test — dans les commentaires

Un exemple dans un `///` est **exécuté** par `cargo test`. Il garde la doc d'une fonction publique honnête.

---

## 6. Les tests réseau

Les techniques (phase 4+) requêtent un vrai serveur : **hors du `make test`** par défaut, sinon lent et cassé hors-ligne. On les marque `#[ignore]`, dans `tests/detection.rs`.

```rust
// tests/detection.rs
#[test]
#[ignore = "requires the local SQLi lab on :8080"]
fn detects_injection_on_lab() {
    // scan http://localhost:8080/Less-1/?id=1, assert vulnerable
}
```

- `cargo test` (= `make test`) : rapide, hors-ligne, **saute** les `#[ignore]`.
- `cargo test -- --ignored` (= `make test-net`) : lance **uniquement** les tests réseau, une fois le labo démarré.

C'est la frontière « hors périmètre » des phases 3 et 4 rendue exécutable : le pur (chaînes, comparaisons) court toujours ; le réseau court à la demande.

---

## 7. Câblage `make`

| Commande | Effet |
|---|---|
| `make test` | `cargo test` — unitaires + intégration + doc-tests, **hors** `#[ignore]`, hors-ligne |
| `make test-net` | `cargo test -- --ignored` — uniquement les tests réseau, labo requis |

> Récap : la **structure** (lib + bin) *autorise* les tests d'intégration ; le **découpage en feuilles** dit *où écrire chaque fonction* (et donc où mettre son test unitaire) ; `#[ignore]` *range* le réseau à part. Les trois ensemble donnent un `make test` qu'on lance à chaque sauvegarde sans y penser.
