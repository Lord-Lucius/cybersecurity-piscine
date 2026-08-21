# vaccine

**Cybersecurity Piscine — 42** · Scanner d'injection SQL, écrit en Rust.

`vaccine` prend une URL, injecte des payloads SQL dans ses paramètres, décide si l'application est vulnérable, identifie le moteur de base de données, puis extrait ce qu'il peut du schéma et des données. Le [sujet complet](docs/vaccine.md) détaille les exigences.

> ⚠️ Outil d'apprentissage. À utiliser **exclusivement** sur des cibles volontairement vulnérables et pour lesquelles vous avez une autorisation explicite (DVWA, SQLi-Labs, bWAPP, labo local). Jamais sur un service tiers ou de production.

---

## Démarrage rapide

```bash
make                                              # compile (release), binaire à la racine
./vaccine "http://localhost/vulnerable.php?id=1"   # scan GET
./vaccine -X POST "http://localhost/login.php"     # scan POST
./vaccine -o results.txt "http://localhost/?id=1"  # archive les résultats
```

Options : `-X <méthode>` (GET par défaut), `-o <fichier>` (archivage), `-h` (aide).

---

## Par où commencer (développement)

Le projet se construit phase par phase. Le point d'entrée de toute la documentation est la **feuille de route** :

### 👉 [`docs/00-feuille-de-route.md`](docs/00-feuille-de-route.md)

Elle contient l'architecture cible, les décisions transverses (client HTTP, gestion d'erreur…), le graphe de dépendances des phases, et l'ordre de développement.

Ensuite, deux pistes en parallèle :

**Apprendre le Rust** — trois fiches calées sur les besoins des phases :
1. [`docs/rust/01-les-bases.md`](docs/rust/01-les-bases.md) — ownership, `String`/`&str`, `Option`, `Result`, `?`, cargo.
2. [`docs/rust/02-structurer-le-code.md`](docs/rust/02-structurer-le-code.md) — `struct`, `enum`, `match`, `trait`, modules, type d'erreur.
3. [`docs/rust/03-http-io-et-crates.md`](docs/rust/03-http-io-et-crates.md) — crates, requêtes HTTP, fichiers.

**Construire, phase par phase** — [`docs/phases/`](docs/phases/) :
1. [CLI](docs/phases/phase-01-cli.md) · 2. [URL & paramètres](docs/phases/phase-02-url-et-parametres.md) · 3. [Client HTTP](docs/phases/phase-03-client-http.md) · 4. [Détection](docs/phases/phase-04-detection.md) · 5. [Fingerprint](docs/phases/phase-05-fingerprint.md) · 6. [Schéma](docs/phases/phase-06-extraction-schema.md) · 7. [Dump](docs/phases/phase-07-dump.md) · 8. [Stockage](docs/phases/phase-08-stockage.md) · 9. [Tests](docs/phases/phase-09-tests-environnement.md).

**Organisation & tests** — l'arbre `src/`, la séparation bibliothèque/binaire et où va chaque type de test : [`docs/organisation-et-tests.md`](docs/organisation-et-tests.md).

Les documents de phase descendent au **niveau des fonctions** (prototype, algorithme décrit en prose, pièges, tests). Ce README, lui, reste au niveau de l'**architecture d'ensemble**.

---

## Architecture cible

```
argv → cli::parse → url::parse → http::Client → scanner::detect → engine::fingerprint
                                                       │                    │
                                                  (si vulnérable)     extract::schema/dump → report::save
```

| Module | Fichier | Rôle |
|---|---|---|
| `cli` | `src/cli/` | parse `argv` → `Config` |
| `url` | `src/url/` | URL → `Target` + `Param`, réinjection |
| `http` | `src/http/` | requêtes GET/POST → `Response` |
| `scanner` / `techniques` | `src/scanner/`, `src/techniques/` | orchestration + error-based / boolean-based |
| `engine` | `src/engine/` | fingerprint du moteur (MySQL, SQLite) |
| `extract` | `src/extract/` | schéma + dump (UNION-based) |
| `report` | `src/report/` | affichage + archivage `-o` |
| `error` | `src/error.rs` | `enum VaccineError`, propagé par `?` |

Toute la logique vit dans une **bibliothèque** (`src/lib.rs`) ; `src/main.rs` n'est qu'un lanceur mince. Les tests unitaires sont dans chaque module (`#[cfg(test)]`), les tests d'intégration dans `tests/`. Détail : [organisation & tests](docs/organisation-et-tests.md). Justification des autres choix : [feuille de route § 2-3](docs/00-feuille-de-route.md).

---

## Commandes `make`

| Commande | Effet |
|---|---|
| `make` | compile en release, binaire `vaccine` à la racine |
| `make debug` | compile en debug |
| `make run ARGS='...'` | compile puis exécute avec les arguments donnés |
| `make test` | lance la suite de tests (hors-ligne) |
| `make test-net` | lance les tests réseau (labo requis) |
| `make fmt` / `make clippy` | formatage / lint |
| `make check` | `fmt-check` + `clippy` + `test` — le gate avant commit |
| `make clean` / `make fclean` / `make re` | nettoyage / nettoyage complet / reconstruction |
| `make help` | liste les cibles |

Exemple : `make run ARGS='-X POST -o /tmp/r.txt "http://localhost/?id=1"'`.

---

## Prérequis

- **Langage :** Rust (édition 2024) — `rustc` + `cargo` (installer via [rustup](https://rustup.rs)).
- **Dépendances :** ajoutées au fil des phases (`ureq` pour le HTTP à la phase 3). Gelées dans `Cargo.lock` pour un build reproductible.
- **Contrainte 42 :** aucune bibliothèque n'automatisant l'injection SQL. Le HTTP est une crate, la logique d'injection est écrite à la main. Voir [feuille de route § 1](docs/00-feuille-de-route.md).

---

## État du projet

| Fonctionnalité | État |
|---|---|
| URL en argument | 🟡 squelette (phase 1 en cours) |
| GET / POST | ⬜ phase 3 |
| Détection (error-based, boolean-based) | ⬜ phase 4 |
| Moteurs (MySQL, SQLite) | ⬜ phase 5 |
| Schéma (bases, tables, colonnes) | ⬜ phase 6 |
| Dump | ⬜ phase 7 |
| Stockage `-o` | ⬜ phase 8 |
| Tests | ⬜ phase 9 (transverse) |

---

## Licence & cadre

Projet réalisé dans le cadre de la **Cybersecurity Piscine** de 42. Usage strictement légal et autorisé — voir la section « Sécurité et limites » du [sujet](docs/vaccine.md#-sécurité-et-limites).
