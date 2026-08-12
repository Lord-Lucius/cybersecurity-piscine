# Rust pour vaccine — 1. Les bases

> À lire **avant la phase 1**. On ne couvre pas tout le langage : seulement ce qui revient dès la première fonction — la propriété des valeurs, les deux types de chaîne, `Option`, `Result`, l'opérateur `?`, et `cargo`. Le reste arrive dans les fiches [2](02-structurer-le-code.md) et [3](03-http-io-et-crates.md).

Ce sont des fiches d'apprentissage, préfixées mentalement `to_delete_` : quand le langage est acquis, elles ne servent plus. Ce qui doit survivre remonte dans le [README](../../README.md).

---

## 1. Le modèle qui change tout : la propriété (ownership)

En C, une variable est un nom pour une case mémoire ; c'est à vous de savoir qui la libère. En Rust, **chaque valeur a un unique propriétaire**, et quand le propriétaire sort de portée, la valeur est libérée automatiquement. Pas de `free`, pas de garbage collector. Le compilateur vérifie ça à la compilation.

Trois règles suffisent pour démarrer :

1. **Une valeur a un seul propriétaire à la fois.**
2. **Passer une valeur non-`Copy` à une fonction la *déplace* (move)** : l'ancienne variable n'est plus utilisable.
3. **On peut prêter une valeur (borrow) avec `&`** au lieu de la déplacer.

```rust
fn main() {
    let s = String::from("http://localhost/?id=1");
    takes_ownership(s);       // s est DÉPLACÉ dans la fonction
    // println!("{s}");       // ← ERREUR de compilation : s a été déplacé

    let n = 42;
    makes_copy(n);            // n est COPIÉ (les entiers sont Copy)
    println!("{n}");          // ← OK, n est toujours là
}
```

> Analogie : une valeur `String`, c'est un livre unique. Le *déplacer*, c'est le donner : vous ne l'avez plus. Le *prêter* (`&String`), c'est laisser quelqu'un le lire par-dessus votre épaule ; il vous revient. Un entier, lui, c'est un post-it : on en refait une copie sans y penser.

### Emprunt partagé vs emprunt mutable

```rust
fn length(s: &str) -> usize { s.len() }        // emprunt PARTAGÉ (&) : lecture seule
fn push_bang(s: &mut String) { s.push('!'); }  // emprunt MUTABLE (&mut) : écriture
```

La règle du compilateur : **soit plusieurs emprunts partagés, soit un seul emprunt mutable, jamais les deux en même temps.** C'est ce qui rend impossible, à la compilation, la classe de bugs « quelqu'un lit pendant qu'un autre modifie ».

⚠️ **Piège classique** : vous passez `config` à une fonction, puis vous la réutilisez, et le compilateur crie `value borrowed here after move`. **Symptôme** : l'erreur pointe la *deuxième* utilisation, pas la fonction. **La correction n'est presque jamais `.clone()`** (qui recopie tout) : neuf fois sur dix, la fonction n'avait besoin que d'emprunter — changez sa signature de `config: Config` en `config: &Config`.

---

## 2. Les deux chaînes : `String` et `&str`

C'est la source n°1 de confusion en début de Rust. Il y a **deux types** là où d'autres langages en ont un.

| | `String` | `&str` (prononcé « string slice ») |
|---|---|---|
| Possède ses données ? | oui, sur le tas | non, c'est une *vue* empruntée |
| Modifiable ? | oui (`push`, `+=`) | non |
| D'où ça vient | `String::from(...)`, `.to_string()`, `format!(...)` | littéral `"..."`, ou emprunt d'une `String` |
| Analogie | le seau d'eau | un tuyau qui pointe vers le seau |

**La règle pratique** qui vous évite 90 % des ennuis :

- **En paramètre de fonction, prenez `&str`** (le plus souple : accepte les littéraux *et* les `String` empruntées).
- **En champ de struct / en valeur de retour construite, stockez `String`** (elle possède ses données, elle survit à la fonction).

```rust
fn is_error_page(body: &str) -> bool {          // &str en entrée
    body.contains("SQL syntax")
}

fn build_payload(base: &str) -> String {        // String en sortie (on l'a fabriquée)
    format!("{base} AND 1=1")
}
```

`format!` est votre outil de construction de chaînes : comme `println!` mais rend une `String` au lieu d'afficher. Les accolades interpolent : `format!("id={id}")`.

⚠️ **Piège classique** : vous renvoyez `&str` d'une fonction qui a fabriqué la chaîne localement → `cannot return reference to local variable`. **Symptôme** : le compilateur parle de *lifetime*. **Cause** : vous rendez un tuyau vers un seau qui va être détruit à la fin de la fonction. **Correction** : renvoyez `String` (le seau lui-même).

---

## 3. L'absence : `Option<T>` au lieu de `null`

Rust n'a **pas** de `null`. Une valeur qui peut manquer a le type `Option<T>`, qui vaut soit `Some(valeur)` soit `None`. Le compilateur vous **oblige** à traiter le cas `None` — c'est la raison pour laquelle un programme Rust ne plante pas sur un « null pointer ».

```rust
fn find_param<'a>(url: &'a str, name: &str) -> Option<&'a str> {
    // rend Some("1") pour ?id=1, ou None si le param n'existe pas
}

match find_param(url, "id") {
    Some(value) => println!("id = {value}"),
    None        => println!("pas de paramètre id"),
}
```

Raccourcis utiles : `.unwrap_or("défaut")`, `.map(|v| ...)`, `if let Some(v) = opt { ... }`.

⚠️ **Piège classique** : `.unwrap()` sur un `Option`. Ça marche… jusqu'à ce que ce soit `None`, et là le programme **panique** (`called Option::unwrap() on a None value`). **Réservez `.unwrap()` aux tests et aux cas réellement impossibles.** Dans le chemin principal, utilisez `match`, `if let`, ou `?` (section suivante).

---

## 4. L'échec : `Result<T, E>` et l'opérateur `?`

Une opération qui peut échouer (réseau, fichier, parsing) rend `Result<T, E>` : soit `Ok(valeur)`, soit `Err(erreur)`. Là encore, le compilateur exige que vous traitiez l'échec.

Écrire un `match` à chaque appel serait insupportable. D'où l'opérateur **`?`** : placé après une expression `Result`, il fait *« si c'est `Ok`, donne-moi la valeur ; si c'est `Err`, arrête la fonction et renvoie cette erreur »*.

```rust
use std::fs;

fn read_config(path: &str) -> Result<String, std::io::Error> {
    let content = fs::read_to_string(path)?;   // ← le ? propage l'erreur
    Ok(content.trim().to_string())
}
```

Sans `?`, la même fonction ferait dix lignes de `match`. Avec `?`, le chemin heureux se lit d'un trait, et les erreurs remontent seules jusqu'à `main`.

> Analogie : `?` est une trappe d'évacuation. Tant que tout va bien, on descend l'escalier ligne par ligne. Au premier problème, la trappe s'ouvre et on ressort directement par le haut, avec l'erreur en main.

> [!IMPORTANT]
> **`?` ne marche que dans une fonction qui rend elle-même `Result` (ou `Option`).** On construira à la [phase 2](../phases/phase-02-url-et-parametres.md) un seul type d'erreur maison, `VaccineError`, pour que *toutes* les fonctions puissent faire remonter leurs erreurs vers `main` avec un `?` uniforme. C'est le sujet de la fiche [2](02-structurer-le-code.md).

⚠️ **Piège classique** : `?` dans `fn main()` alors que `main` rend `()`. **Symptôme** : `the ? operator can only be used in a function that returns Result`. **Correction** : donnez à `main` la signature `fn main() -> Result<(), VaccineError>` et terminez-la par `Ok(())`.

---

## 5. `cargo`, votre couteau suisse

Cargo est à la fois le compilateur (via `rustc`), le gestionnaire de dépendances et le lanceur de tests. Les commandes du quotidien :

| Commande | Ce qu'elle fait | Équivalent Makefile |
|---|---|---|
| `cargo build` | compile (debug) dans `target/debug/` | `make debug` |
| `cargo build --release` | compile optimisé dans `target/release/` | `make` |
| `cargo run -- ARGS` | compile puis exécute (le `--` sépare les args du programme) | `make run ARGS=...` |
| `cargo test` | lance les tests | `make test` |
| `cargo fmt` | reformate selon la norme officielle | `make fmt` |
| `cargo clippy` | linter : détecte les tournures maladroites | `make clippy` |
| `cargo add ureq` | ajoute une dépendance à `Cargo.toml` | — |

> [!TIP]
> **`cargo clippy` est un professeur gratuit.** Il ne se contente pas de trouver des bugs : il propose la tournure idiomatique (« utilise `if let` ici », « ce `.clone()` est inutile »). Lancez-le souvent au début — c'est la façon la plus rapide d'apprendre à écrire du Rust qui *ressemble* à du Rust. Le [Makefile](../../Makefile) l'a en cible `make clippy`, et `make check` refuse de passer s'il reste un warning.

---

## 6. Le minimum pour lire le code de départ

Le [`src/main.rs`](../../src/main.rs) actuel utilise déjà :

- `use std::env;` — importe le module `env` de la bibliothèque standard.
- `env::args().collect()` — récupère les arguments ; `.collect()` transforme un itérateur en `Vec<String>` (un tableau redimensionnable).
- `Vec<String>` — un vecteur de chaînes possédées.
- `process::exit(code)` — quitte avec un code de sortie.
- `eprintln!` — comme `println!` mais écrit sur `stderr` (le bon canal pour les erreurs).
- `{msg=msg}` dans `eprintln!` — interpolation nommée.

Vous avez maintenant de quoi lire, comprendre et étendre ce squelette. La suite se passe dans la [phase 1](../phases/phase-01-cli.md).

---

## Pour aller plus loin

- *The Rust Book* (référence officielle, en anglais) : https://doc.rust-lang.org/book/
- *Rust by Example* (exemples exécutables) : https://doc.rust-lang.org/rust-by-example/
- *Rustlings* (exercices progressifs) : https://github.com/rust-lang/rustlings
- Le livre en français (traduction communautaire) : https://jimskapt.github.io/rust-book-fr/
