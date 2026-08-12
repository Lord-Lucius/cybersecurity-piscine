# Rust pour vaccine — 3. HTTP, entrées/sorties et crates

> À lire **avant la phase 3**. On voit : ajouter une dépendance externe, faire une requête HTTP GET et POST avec `ureq`, lire la réponse, et écrire le fichier de résultats. C'est la fiche la plus « pratique » — elle donne les gestes exacts.

---

## 1. Ajouter une crate (dépendance externe)

Une *crate* est un paquet Rust publié sur https://crates.io. On l'ajoute au projet avec :

```bash
cargo add ureq
```

Cela modifie `Cargo.toml` :

```toml
[dependencies]
ureq = "2"
```

Puis `cargo build` la télécharge et la compile **une fois** ; les builds suivants la réutilisent. La version exacte est gelée dans `Cargo.lock` (déjà présent dans le dépôt), ce qui garantit que le projet est **reproductible** — exigence explicite du sujet.

> [!IMPORTANT]
> **Rappel de la contrainte 42** (voir la [feuille de route](../00-feuille-de-route.md) § 1) : une crate HTTP est autorisée car elle transporte des octets ; une crate qui *automatise l'injection SQL* est interdite. `ureq` est de la première catégorie. On l'utilise pour envoyer/recevoir ; toute la logique d'injection reste écrite à la main.

**Pourquoi `ureq` et pas `reqwest` :** `ureq` est **synchrone** (bloquant). Une requête est un simple appel de fonction qui rend la réponse. `reqwest` est asynchrone : il faut un runtime (`tokio`), des `async fn`, des `.await` — tout un modèle à apprendre pour, ici, aucun gain. On reste synchrone.

---

## 2. Une requête GET

```rust
fn fetch_get(url: &str) -> Result<Response, VaccineError> {
    let resp = ureq::get(url).call();       // envoie, bloque jusqu'à la réponse
    // ... transformer resp en notre type Response ...
}
```

Le point délicat : **ureq considère un code HTTP 4xx/5xx comme une *erreur*, pas comme une réponse.** Or pour un scanner, une page d'erreur 500 est une **information capitale** (elle peut trahir une injection). Il faut donc récupérer la réponse *y compris* quand le statut est « erreur ».

```rust
use ureq::Error;

let result = ureq::get(url).call();
let response = match result {
    Ok(r) => r,                              // 2xx / 3xx
    Err(Error::Status(_code, r)) => r,       // 4xx / 5xx : on VEUT quand même le corps
    Err(Error::Transport(t)) => {            // DNS, connexion refusée, timeout…
        return Err(VaccineError::Http(t.to_string()));
    }
};
let status = response.status();              // u16, ex. 200 ou 500
let body = response.into_string()?;          // le corps HTML/texte, en String
```

> [!CAUTION]
> **Ne traitez pas `Err(Error::Status(...))` comme un échec fatal.** C'est le piège spécifique du scanner : si vous laissez le `?` propager cette erreur, l'outil abandonne dès qu'il déclenche une erreur SQL côté serveur… c'est-à-dire exactement au moment où il vient de *réussir* une injection error-based. Le `match` ci-dessus est obligatoire, pas cosmétique.

---

## 3. Une requête POST

Le corps POST classique d'un formulaire est du `application/x-www-form-urlencoded` : `id=1&cat=books`. Avec `ureq` :

```rust
let response = ureq::post(url)
    .send_form(&[("id", "1"), ("cat", "books")]);
```

`send_form` prend un tableau de paires `(clé, valeur)` et pose le bon `Content-Type` tout seul. Comme pour le GET, enveloppez l'appel dans le même `match` sur `Ok` / `Error::Status` / `Error::Transport`.

Pour la [phase 4](../phases/phase-04-detection.md), on injectera en remplaçant la valeur d'**un** paramètre à la fois par un payload, en laissant les autres intacts.

---

## 4. Modéliser la réponse

On ne fait jamais circuler l'objet `ureq` dans tout le code : on le convertit tout de suite en un type à nous, découplé de la crate. Ça isole le reste du projet du choix de la bibliothèque.

```rust
pub struct Response {
    pub status: u16,
    pub body: String,
}

impl Response {
    pub fn len(&self) -> usize { self.body.len() }
}
```

La détection (phase 4) comparera deux `Response` sur trois axes : `status`, `body.len()`, et la présence de certaines sous-chaînes dans `body`.

---

## 5. Lire et écrire des fichiers

Le stockage des résultats (`-o`, phase 8) tient en deux fonctions de `std::fs`. Pas de crate nécessaire.

**Écrire (en écrasant) :**

```rust
use std::fs;

fn save(path: &str, content: &str) -> Result<(), VaccineError> {
    fs::write(path, content)?;    // crée le fichier s'il n'existe pas, sinon écrase
    Ok(())
}
```

**Ajouter à la fin (sans écraser)**, plus adapté pour archiver plusieurs scans :

```rust
use std::fs::OpenOptions;
use std::io::Write;

fn append(path: &str, content: &str) -> Result<(), VaccineError> {
    let mut file = OpenOptions::new()
        .create(true)      // crée si absent — exigé par le sujet
        .append(true)      // écrit à la suite
        .open(path)?;
    writeln!(file, "{content}")?;   // writeln! ajoute un \n
    Ok(())
}
```

Le sujet impose que **le fichier soit créé automatiquement s'il n'existe pas** : c'est le rôle de `.create(true)`.

⚠️ **Piège classique** : `writeln!(file, ...)` exige `use std::io::Write;` dans la portée. **Symptôme** : `no method named write_fmt found` — un message qui ne mentionne jamais l'import manquant. Le trait doit être *importé* pour que ses méthodes deviennent visibles.

---

## 6. Construire les payloads et les URLs

Encoder les caractères spéciaux d'une valeur injectée dans une query string (`'`, espace, `=`) évite qu'ils cassent l'URL. `ureq` encode déjà les corps de formulaire ; pour les query strings GET, une petite crate comme `urlencoding` (`cargo add urlencoding`) rend ça trivial :

```rust
let encoded = urlencoding::encode("1' OR '1'='1");   // → "1%27%20OR%20%271%27%3D%271"
```

> [!TIP]
> **Commencez sans encodage, sur un labo local, pour voir les payloads « en clair » passer.** L'encodage cache ce qui se passe et complique le débogage des premières injections. Ajoutez-le une fois la détection qui marche, quand un payload contient un caractère qui casse l'URL. Un problème à la fois.

---

## Récapitulatif des gestes

| Besoin | Geste |
|---|---|
| Ajouter une crate | `cargo add <nom>` |
| GET | `ureq::get(url).call()` + `match` sur `Error::Status` |
| POST formulaire | `ureq::post(url).send_form(&[(k, v)])` |
| Corps de la réponse | `.into_string()?` |
| Écraser un fichier | `fs::write(path, content)?` |
| Archiver à la suite | `OpenOptions::new().create(true).append(true)` |
| Encoder un payload d'URL | `urlencoding::encode(...)` |

Avec ces trois fiches, vous avez le Rust nécessaire pour les phases 1 à 4. La suite (traits d'extraction, itérateurs avancés) s'apprend au fil des phases 5+, quand le besoin apparaît.
