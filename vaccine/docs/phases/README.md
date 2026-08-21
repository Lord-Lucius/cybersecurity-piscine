# Documents de phase

> Index des phases de `vaccine`. **Les neuf phases ont désormais un document complet**, au niveau des fonctions, selon la [norme du template](../template_phases.md) : chaque corps de fonction est décrit en prose (pas de pseudo-code), seuls prototypes et lignes de log restent en Rust littéral.

Pour la vue d'ensemble et l'ordre, voir la [feuille de route](../00-feuille-de-route.md).

| Phase | Sujet | Document |
|---|---|---|
| 1 | CLI | [phase-01-cli.md](phase-01-cli.md) |
| 2 | URL & paramètres | [phase-02-url-et-parametres.md](phase-02-url-et-parametres.md) |
| 3 | Client HTTP | [phase-03-client-http.md](phase-03-client-http.md) |
| 4 | Détection (error + boolean) | [phase-04-detection.md](phase-04-detection.md) |
| 5 | Fingerprint moteur | [phase-05-fingerprint.md](phase-05-fingerprint.md) |
| 6 | Extraction du schéma | [phase-06-extraction-schema.md](phase-06-extraction-schema.md) |
| 7 | Dump des données | [phase-07-dump.md](phase-07-dump.md) |
| 8 | Stockage `-o` | [phase-08-stockage.md](phase-08-stockage.md) |
| 9 | Tests & environnement | [phase-09-tests-environnement.md](phase-09-tests-environnement.md) |

---

## Fil rouge

Les phases s'enchaînent en livrant chacune une brique dont la suivante a besoin :

- **1-3 — le socle** : `argv` → `Config`, URL → `Target`, requête → `Response`.
- **4-5 — décider** : un paramètre est-il vulnérable (error-based / boolean-based), et derrière quel moteur (MySQL / SQLite) ?
- **6-7 — extraire** : le schéma (UNION-based), puis le dump des données.
- **8 — livrer** : accumuler un `Report`, l'afficher et l'archiver (`-o`).
- **9 — prouver** : environnement vulnérable reproductible + suite de tests (unitaires par phase, intégration ici).

Chaque document descend au niveau des fonctions ; l'architecture d'ensemble reste dans la [feuille de route](../00-feuille-de-route.md) et le [README](../../README.md).
