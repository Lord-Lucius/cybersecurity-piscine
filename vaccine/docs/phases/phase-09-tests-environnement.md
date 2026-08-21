# Vaccine — Phase 9 : tests et environnement vulnérable

> La phase transverse. On rassemble ici les tests d'**intégration** (l'outil complet contre un vrai serveur) et la **démonstration** exigée par le sujet — les tests unitaires, eux, ont été écrits à chaque phase. Tout se fait **uniquement** sur des cibles volontairement vulnérables et autorisées : DVWA, SQLi-Labs, ou un labo local. C'est la seule posture légale, et c'est aussi ce que le sujet impose.

---

## 1. Où on en est

**Fait :**
- Phases 1-8 : l'outil complet — CLI, URL, HTTP, détection, moteur, schéma, dump, rapport. Chaque phase a livré ses **tests unitaires** (`#[cfg(test)]`) sur ses fonctions pures.

**À faire dans cette phase :**
- Monter un **environnement vulnérable** reproductible (Docker de préférence).
- Écrire les tests d'**intégration** dans `tests/` : l'outil lancé comme un vrai binaire.
- Couvrir les **cinq tests obligatoires** du sujet et savoir les démontrer.
- Câbler `make test` (hors-ligne) et `make test-net` (réseau, labo requis).

**Ce qui suit :** rien — c'est la dernière phase. Le projet est complet et défendable.

> Prérequis Rust : lancer un processus et lire sa sortie (`std::process::Command`), et la distinction tests unitaires (`#[cfg(test)]` dans `src/`) vs tests d'intégration (crate séparée dans `tests/`), détaillée dans [`organisation-et-tests.md`](../organisation-et-tests.md).

---

## 2. Architecture cible

```
   TESTS UNITAIRES                    TESTS D'INTÉGRATION
   #[cfg(test)] dans src/             crate séparée dans tests/
   voient l'intérieur (pub + privé)   ne voient que le pub, lancent le binaire
        │                                   │
   fonctions pures :                    scénarios bout-en-bout :
   parse, similar,                      tests/cli.rs      (offline)  ── make test
   extract_between,                     tests/url.rs      (offline)  ──┘
   row_expression, render…              tests/detection.rs #[ignore]  ── make test-net
        │                                   │        (réseau, labo requis)
        └──────────► make test ◄────────────┘ (les #[ignore] restent hors du run par défaut)

   LABO (cible autorisée)
   docker run … sqli-labs   →  http://localhost:8080/Less-1/?id=1
   docker run … dvwa        →  http://localhost:8081/…
```

**Point clé sur le flux :** les tests **réseau** sont marqués `#[ignore]` pour que `make test` reste **hors-ligne et déterministe** (il ne dépend d'aucun serveur). On les lance explicitement via `make test-net` quand le labo tourne. Un test qui a besoin d'un serveur mais s'exécute par défaut rendrait la suite rouge sur toute machine sans labo — pire que pas de test.

---

## 3. Concepts à maîtriser

### 3.1 Unitaire vs intégration : deux portées, deux emplacements

Un **test unitaire** vit dans `src/` (`#[cfg(test)]`), voit les fonctions privées, et cible une fonction pure sans I/O — c'est ce qu'on a fait à chaque phase (`similar`, `extract_between`, `render`…). Un **test d'intégration** vit dans `tests/`, une crate **séparée** qui ne voit que l'API `pub` (ou lance le binaire), et vérifie un **scénario complet**.

Documentation :
- [`organisation-et-tests.md`](../organisation-et-tests.md) — l'arbre `tests/` et la séparation bibliothèque/binaire.
- `std::process::Command` : https://doc.rust-lang.org/std/process/struct.Command.html

> Analogie : tester un moteur de voiture au banc (unitaire : une pièce isolée) contre tester la voiture sur circuit (intégration : tout ensemble, dans les conditions réelles).

⚠️ **Piège classique** : mettre un test qui requête le réseau dans un `#[test]` ordinaire. **Symptôme** : `make test` échoue sur la machine du correcteur qui n'a pas lancé le labo. **Correction** : `#[ignore]` sur les tests réseau, lancés à part (`make test-net` / `cargo test -- --ignored`).

### 3.2 Lancer le binaire depuis un test

Les tests d'intégration bout-en-bout **lancent `vaccine`** comme un processus et lisent sa sortie : code de retour, `stdout` (le rapport), `stderr` (les logs). On fabrique un helper (dans `tests/common/mod.rs`) qui construit la commande avec des arguments et rend la sortie capturée.

Documentation :
- `Command::new(...).args(...).output()` rend `stdout`, `stderr`, `status`.
- Le binaire compilé est trouvable via la variable `CARGO_BIN_EXE_vaccine` fournie par cargo dans les tests d'intégration.

⚠️ **Piège classique** : coder en dur le chemin `target/debug/vaccine`. **Symptôme** : le test casse en `release` ou sur un autre profil. **Correction** : lire `env!("CARGO_BIN_EXE_vaccine")`, que cargo renseigne.

### 3.3 L'environnement doit être jetable et reproductible

Un labo monté à la main dérive ; un labo **Docker** se relance à l'identique. On documente une commande unique par cible, pour que quiconque clone le dépôt reproduise l'environnement exact des tests.

⚠️ **Piège classique** : tester sur une cible « trouvée en ligne ». **Symptôme** : hors-la-loi, et non reproductible. **Correction** : uniquement des cibles **montées localement** et volontairement vulnérables — DVWA, SQLi-Labs, labo maison.

### 3.4 Lexique

| Terme | Ce qu'il désigne | Où ça vit |
|---|---|---|
| test unitaire | `#[cfg(test)]` sur une fonction pure | dans chaque fichier de `src/` |
| test d'intégration | scénario bout-en-bout, crate séparée | [`tests/`](../../tests/) |
| `#[ignore]` | marque un test hors du run par défaut (réseau) | `tests/detection.rs` |
| labo | cible volontairement vulnérable et autorisée | Docker / local |
| `CARGO_BIN_EXE_vaccine` | chemin du binaire, fourni par cargo aux tests | `tests/common/mod.rs` |

---

## 4. Décomposition des étapes

1. **Le labo** — une commande Docker par cible, documentée et vérifiée.
2. **Le helper `tests/common/mod.rs`** — lance le binaire, capture la sortie.
3. **Les tests offline** (`tests/cli.rs`, `tests/url.rs`) — scénarios sans réseau.
4. **Les tests réseau** (`tests/detection.rs`, `#[ignore]`) — contre le labo.
5. **Les cibles `make`** — `test` (offline) et `test-net` (réseau).

> Le labo d'abord : sans lui, les tests réseau ne sont pas exécutables. Les tests offline, eux, ne dépendent de rien et servent de socle vert permanent.

---

## 5. Le corps du document : monter le labo et organiser les tests

### 5.0 · Boîte à outils

| Opération | Commande / API |
|---|---|
| Lancer SQLi-Labs | `docker run -d -p 8080:80 acgpiscine/sqli-labs` (ou image équivalente) |
| Lancer DVWA | `docker run -d -p 8081:80 vulnerables/web-dvwa` (sécurité « low ») |
| Cible type | `http://localhost:8080/Less-1/?id=1` |
| Lancer les tests offline | `make test` → `cargo test` |
| Lancer les tests réseau | `make test-net` → `cargo test -- --ignored` |
| Localiser le binaire dans un test | `env!("CARGO_BIN_EXE_vaccine")` |
| Lancer et capturer | `Command::new(bin).args([...]).output()` |

### 5.1 · Le labo (environnement vulnérable)

**Ce qu'il faut obtenir :** au moins **deux** moteurs pour couvrir l'exigence du sujet — un labo **MySQL** (SQLi-Labs, DVWA) et une cible **SQLite** (un petit script PHP/Python maison, ou une image dédiée). Chaque cible se lance par **une** commande reproductible, documentée dans un court `README` de tests ou dans le `Makefile`.

**Décisions**

| Décision | Pourquoi |
|---|---|
| Docker de préférence | jetable, reproductible, isolé de la machine hôte |
| au moins un MySQL + un SQLite | démontrer le fingerprint et l'extraction sur les **deux** moteurs |
| tout en local, jamais une cible tierce | légalité et reproductibilité (§ 3.3) |

⚠️ **Rappel légal.** `vaccine` ne se lance que sur des cibles **volontairement vulnérables** et **autorisées**. Le labo local est la seule surface de test admissible ; c'est écrit dans le sujet (« Sécurité et limites ») et non négociable.

### 5.2 · Le helper d'intégration (`tests/common/mod.rs`)

**Ce qu'il doit accomplir :** offrir aux tests une façon uniforme de **lancer `vaccine`** avec des arguments et de récupérer sa sortie (code, `stdout`, `stderr`), sans répéter la plomberie `Command` dans chaque fichier.

**Décisions**

| Décision | Pourquoi |
|---|---|
| localiser le binaire par `env!("CARGO_BIN_EXE_vaccine")` | robuste au profil (debug/release), pas de chemin en dur (§ 3.2) |
| rendre un tuple (code, stdout, stderr) | les tests vérifient tantôt la sortie, tantôt le code de retour |

> Pas de prototype figé : décrivez une petite fonction locale aux tests qui prend une liste d'arguments et rend la sortie capturée. C'est de l'outillage de test, pas de l'API du projet.

### 5.3 · Les scénarios de test

On **décrit** les scénarios (norme § 8) ; le code de test se déduit du tableau. Deux familles :

**Offline** (`tests/cli.rs`, `tests/url.rs`) — aucun réseau, exécutés par `make test` :

| Fichier | Scénario | Attendu |
|---|---|---|
| `tests/cli.rs` | lancer avec `-h` | code 0, l'aide sur la sortie |
| `tests/cli.rs` | lancer sans URL | code non-nul, message d'usage sur `stderr` |
| `tests/url.rs` | (via l'API `pub`) parser une URL à deux params | `Target` à deux `Param` |

**Réseau** (`tests/detection.rs`, chaque test `#[ignore]`) — exécutés par `make test-net` contre le labo :

| Scénario | Attendu |
|---|---|
| scan GET d'une cible SQLi-Labs vulnérable | rapport nommant le paramètre, la technique, le payload |
| fingerprint sur MySQL puis SQLite | le bon moteur dans le rapport |
| extraction du schéma | au moins la liste des tables |
| dump + cible sans UNION | des lignes d'un côté, un dump vide **sans** panique de l'autre |
| scan POST (`-X POST`) | même détection que GET, paramètres dans le corps |

---

## 6. Pièges spécifiques à cette phase

- **Un test réseau non `#[ignore]` rend la suite non portable.** C'est le piège central : la frontière offline/réseau **est** le contrat de `make test`. Tout scénario qui parle à un serveur porte `#[ignore]` et ne s'exécute que via `make test-net`.
- **La démonstration n'est pas la suite automatisée.** Les cinq tests du sujet se **montrent** (captures, commandes rejouables) autant qu'ils s'automatisent. Gardez une procédure écrite « lancer le labo → ces commandes → ces sorties » à côté des tests `cargo`.
- **Les tests unitaires ne sont pas repoussés à la fin.** Cette phase **rassemble** l'intégration, mais l'unitaire s'est écrit à chaque phase (phases 1-8). Un projet qui découvre les tests en phase 9 a mal suivi le plan.

---

## 7. Compilation et configuration

```bash
# 1. lancer un labo
docker run -d -p 8080:80 acgpiscine/sqli-labs

# 2. tests hors-ligne (déterministes, aucun serveur requis)
make test          # -> cargo test

# 3. tests réseau (labo requis)
make test-net      # -> cargo test -- --ignored
```

- `make test` : ne doit **jamais** dépendre du réseau. Vert sur toute machine.
- `make test-net` : suppose un labo joignable ; échoue proprement (message clair) s'il ne l'est pas.
- ⚠️ La cible du test réseau (URL, port) se règle en tête de `tests/detection.rs` ou par variable d'environnement — à documenter, sinon le test cible un port au hasard.

---

## 8. Tests — la matrice du sujet

Le sujet exige **cinq** démonstrations ([§ Tests](../vaccine.md#-tests)). Chacune se rattache à une phase et à un scénario :

| # | Test du sujet | Couvre | Comment le montrer |
|---|---|---|---|
| 1 | Détection + payload + moteur | phases 4-5 | scan GET SQLi-Labs ; rapport avec param, payload, moteur |
| 2 | Seconde technique d'injection | phase 4 | une cible où error-based échoue mais boolean-based réussit |
| 3 | Extraction du schéma | phase 6 | lister bases/tables/colonnes d'un labo |
| 4 | Dump + gestion d'échec | phase 7 | dumper `users` ; puis une cible sans UNION → dump vide sans panique |
| 5 | Requête POST | phase 3 | `-X POST` sur un formulaire vulnérable, même détection que GET |

### 8.1 Résultats attendus

- `make test` (offline) : PASS sur toute machine, sans labo.
- `make test-net` (labo lancé) : les cinq scénarios passent contre SQLi-Labs/DVWA + cible SQLite.
- Démonstration : chaque test du sujet rejouable depuis la procédure documentée.

---

## 9. Ordre de développement recommandé

1. Monter **un** labo Docker (SQLi-Labs), vérifier la cible `?id=1` à la main.
2. Écrire le helper `tests/common/mod.rs` (lance le binaire, capture la sortie).
3. Les tests **offline** (`tests/cli.rs`, `tests/url.rs`) → `make test` vert.
4. Les tests **réseau** `#[ignore]` (`tests/detection.rs`) → `make test-net` vert avec le labo.
5. Ajouter une cible **SQLite** pour le second moteur.
6. Rédiger la procédure de démonstration des cinq tests du sujet.

> Quand `make test` est vert partout, `make test-net` valide les cinq scénarios sur les deux moteurs, et la procédure de démonstration est écrite, le projet est **complet**. Retour à la [feuille de route](../00-feuille-de-route.md) pour la revue d'ensemble.
