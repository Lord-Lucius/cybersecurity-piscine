# Vaccine — Feuille de route

> Ce document est le **plan de vol** du projet : l'architecture cible, les décisions transverses, l'ordre des phases et leurs dépendances. Chaque phase a ensuite son propre document dans [`phases/`](phases/). L'architecture d'ensemble vit ici et dans le [README](../README.md) ; les documents de phase, eux, descendent au niveau des fonctions.

---

## 1. Ce que le sujet demande, en une page

`vaccine` prend une URL, y injecte des payloads SQL, et décide si un paramètre est vulnérable. Si oui, il identifie le moteur SQL, puis extrait ce qu'il peut de la base. Le [sujet complet est ici](vaccine.md). Le minimum noté :

| Obligatoire | Détail |
|---|---|
| URL en argument | `./vaccine URL` |
| Méthodes HTTP | `GET` (défaut) et `POST` (`-X POST`) |
| ≥ 2 moteurs SQL | on vise **MySQL** + **SQLite** |
| ≥ 2 techniques d'injection | on vise **error-based** + **boolean-based** |
| Paramètre vulnérable | le nommer, avec méthode + payload |
| Schéma | bases, tables, colonnes quand c'est possible |
| Dump | données accessibles quand c'est possible |
| Stockage | archiver les résultats (`-o`) |
| Tests | uniquement sur labos volontairement vulnérables |

> [!IMPORTANT]
> **Contrainte de la norme 42 : les bibliothèques qui automatisent l'injection SQL sont interdites.** `sqlmap` et consorts sont hors-jeu, et toute crate qui « scanne » à votre place aussi. En revanche une **bibliothèque HTTP** (envoyer une requête, lire une réponse) est autorisée — c'est de la plomberie réseau, pas de la logique d'injection. C'est *vous* qui écrivez les payloads, la comparaison des réponses, le fingerprint et l'extraction.

---

## 2. Architecture cible

Le flux, de l'argument de ligne de commande jusqu'au fichier de résultats :

```
                 argv
                  │
          ┌───────▼────────┐
          │  cli::parse    │   -X, -o, -h, URL      → Config
          └───────┬────────┘
                  │
          ┌───────▼────────┐
          │  url::parse    │   sépare base + params → Target { params: [id, cat] }
          └───────┬────────┘
                  │
          ┌───────▼────────┐        ┌──────────────────────┐
          │  http::Client  │◄──────►│  serveur cible (labo) │
          └───────┬────────┘        └──────────────────────┘
                  │  (envoie/reçoit à chaque test)
          ┌───────▼────────┐
          │ scanner::detect│   pour CHAQUE param, essaie CHAQUE technique
          │  ├ error_based │
          │  └ bool_based  │        → un param vulnérable + le payload gagnant
          └───────┬────────┘
                  │  (seulement si vulnérable)
          ┌───────▼────────┐
          │ engine::finger │   signatures d'erreur           → SqlEngine::MySql
          └───────┬────────┘
          ┌───────▼────────┐
          │ extract::schema│   information_schema / sqlite_master
          │  extract::dump │        → databases, tables, columns, rows
          └───────┬────────┘
          ┌───────▼────────┐
          │  report::save  │   écrit le fichier -o
          └────────────────┘
```

**Point clé sur le flux :** chaque test est **un aller-retour HTTP**. Un scan naïf en fait des centaines. Deux conséquences qui structurent tout le code : (1) il faut une **requête de référence** (la réponse « normale », non injectée) à laquelle comparer, calculée **une fois** ; (2) l'ordre des techniques compte — on tente d'abord la moins coûteuse et la plus bruyante (error-based : un seul `'`), avant la comparaison fine (boolean-based : deux requêtes à diffé­rencier).

### Découpage en modules

Un **dossier** = un module, dans `src/`, avec des fichiers-feuilles courts (types d'un côté, logique de l'autre). C'est la granularité retenue — l'arbre complet est dans [`organisation-et-tests.md`](organisation-et-tests.md) :

| Module | Fichier | Responsabilité |
|---|---|---|
| `cli` | `src/cli/` | parser `argv` → `Config` |
| `url` | `src/url/` | parser l'URL cible → `Target` + ses `Param` |
| `http` | `src/http/` | envoyer une requête, rendre une `Response` |
| `scanner` | `src/scanner/` | orchestrer techniques × paramètres |
| `techniques` | `src/techniques/` | `error_based`, `boolean_based` |
| `engine` | `src/engine/` | fingerprint du moteur SQL |
| `extract` | `src/extract/` | schéma + dump |
| `report` | `src/report/` | affichage + sauvegarde |
| `error` | `src/error.rs` | le type d'erreur du projet |

> Voir [`rust/02-structurer-le-code.md`](rust/02-structurer-le-code.md) pour *comment* un `mod` devient un dossier (`mod.rs` + feuilles), et [`organisation-et-tests.md`](organisation-et-tests.md) pour l'arbre complet, la séparation bibliothèque/binaire et l'emplacement des tests.

---

## 3. Décisions transverses (tranchées une fois, pour tout le projet)

Le template le dit (§ 18) : changer de convention en cours de route coûte une passe sur tout. On tranche donc **maintenant**.

| Décision | Choix | Pourquoi |
|---|---|---|
| Client HTTP | crate **`ureq`** (bloquant) | pas d'`async`/`tokio` à apprendre ; une requête = un appel de fonction. Voir [`rust/03-http-io-et-crates.md`](rust/03-http-io-et-crates.md) |
| Modèle de concurrence | **séquentiel** d'abord | un scan correct avant un scan rapide. Le parallélisme est un bonus, pas le socle |
| Gestion d'erreur | un **enum `VaccineError`** + `Result<T, VaccineError>` partout | messages clairs, `?` propre, pas de `unwrap()` dans le chemin principal |
| Comparaison de réponses | sur le **corps** + le **code HTTP** + la **longueur** | un seul de ces trois signaux suffit rarement ; les trois ensemble discriminent |
| Langue | prose FR, code/logs/commentaires EN | c'est la règle du [template](template_phases.md) § 10 |
| Corps des fonctions | **décrit en prose FR**, pas de code (§ 11 du template) | comprendre pour écrire ; le seul code littéral est le prototype (§ 12) |

> [!CAUTION]
> **Ne pas partir sur `reqwest` + `tokio` « parce que c'est le standard ».** L'`async` en Rust est un sujet à part entière (`Future`, `.await`, runtime, `Pin`). Pour un scanner séquentiel il n'apporte rien et ajoute des heures de compréhension. `ureq` fait exactement le même travail en synchrone. Le jour où le parallélisme devient un bonus, on ajoute des threads `std::thread`, pas un runtime async.

---

## 4. Les phases, et ce qui dépend de quoi

```
  Phase 1 ─ CLI ────────────┐
  (args → Config)           │
                            ▼
  Phase 2 ─ URL & params ── Phase 3 ─ HTTP ──┐
  (Target, Param)          (Client, Response)│
                            │                 ▼
                            └────────► Phase 4 ─ Détection ── vulnérable ?
                                       (error + boolean)      │
                                                              ▼
                                              Phase 5 ─ Fingerprint moteur
                                                              │
                                                              ▼
                                              Phase 6 ─ Extraction schéma
                                                              │
                                                              ▼
                                              Phase 7 ─ Dump des données
                                                              │
                                                              ▼
                                              Phase 8 ─ Stockage (-o)
                                              Phase 9 ─ Tests (transverse)
```

**On ne peut pas tester la détection sans le HTTP, ni le HTTP sans les params.** L'ordre 1 → 9 n'est donc pas indicatif : chaque phase compile et se teste seule, et livre une brique dont la suivante a besoin.

Documents détaillés (niveau fonction, façon template) :

| Phase | Document | État |
|---|---|---|
| 1 | [phase-01-cli.md](phases/phase-01-cli.md) | rédigé |
| 2 | [phase-02-url-et-parametres.md](phases/phase-02-url-et-parametres.md) | rédigé |
| 3 | [phase-03-client-http.md](phases/phase-03-client-http.md) | rédigé |
| 4 | [phase-04-detection.md](phases/phase-04-detection.md) | rédigé |
| 5 | [phase-05-fingerprint.md](phases/phase-05-fingerprint.md) | rédigé |
| 6 | [phase-06-extraction-schema.md](phases/phase-06-extraction-schema.md) | rédigé |
| 7 | [phase-07-dump.md](phases/phase-07-dump.md) | rédigé |
| 8 | [phase-08-stockage.md](phases/phase-08-stockage.md) | rédigé |
| 9 | [phase-09-tests-environnement.md](phases/phase-09-tests-environnement.md) | rédigé |

Les neuf documents sont désormais rédigés au niveau des fonctions. Chaque corps de fonction y est **décrit en prose** (plus de pseudo-code) ; seuls les prototypes et les lignes de log restent en Rust littéral (template § 11-12).

---

## 5. Apprendre le Rust en même temps

Ce projet est aussi une porte d'entrée dans le langage. Trois fiches, à lire dans l'ordre, chacune calée sur ce dont une phase a besoin :

1. [`rust/01-les-bases.md`](rust/01-les-bases.md) — ownership, `String`/`&str`, `Option`, `Result`, `?`, `cargo`. **À lire avant la phase 1.**
2. [`rust/02-structurer-le-code.md`](rust/02-structurer-le-code.md) — `struct`, `enum`, `match`, `trait`, modules, type d'erreur. **À lire avant la phase 2.**
3. [`rust/03-http-io-et-crates.md`](rust/03-http-io-et-crates.md) — ajouter une crate, faire une requête, lire/écrire des fichiers. **À lire avant la phase 3.**

> Chaque document de phase a aussi une section « Concepts à maîtriser » qui pointe vers la bonne fiche. On n'apprend pas tout le Rust d'un coup : on apprend ce que la phase courante exige.

---

## 6. Rappel de cadre légal

`vaccine` ne se teste **que** sur des cibles volontairement vulnérables et sur lesquelles on a l'autorisation explicite : DVWA, SQLi-Labs, bWAPP, ou une appli PHP montée pour l'occasion en local. Le sujet est clair là-dessus (§ « Sécurité et limites ») et c'est aussi la seule posture défendable. On monte l'environnement de test à la [phase 9](phases/phase-09-tests-environnement.md).
