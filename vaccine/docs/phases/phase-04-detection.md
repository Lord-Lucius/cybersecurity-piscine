# Vaccine — Phase 4 : détection de l'injection

> Le cœur du projet. On envoie des payloads dans chaque paramètre et on décide, par comparaison de réponses, s'il est injectable. On implémente les **deux techniques** exigées : **error-based** (le serveur crache un message SQL) et **boolean-based** (deux payloads logiquement opposés produisent deux pages différentes). À la fin, on sait *quel* paramètre, *quelle* méthode, *quel* payload.

---

## 1. Où on en est

**Fait :**
- Phases 1-3 : `Config`, `Target`/`with_injected`, `http::send` rendant une `Response`.

**À faire dans cette phase :**
- Une **réponse de référence** (baseline) : l'URL non injectée.
- `error_based` : injecter un `'`, chercher une signature d'erreur SQL.
- `boolean_based` : comparer `… AND 1=1` (vrai) à `… AND 1=2` (faux).
- `scanner::run` : boucler sur les params × techniques, rendre un verdict.

**Ce qui suit (phase 5) :** une fois vulnérable, identifier le moteur SQL.

> Prérequis Rust : les trois fiches. Cette phase mobilise `enum` (le verdict), `match`, et la comparaison de chaînes.

---

## 2. Architecture cible

```
   Target { params: [id, cat] }, baseline = send(non-injected URL)
                          │
          for each param in params:
                          │
           ┌──────────────┴───────────────┐
           ▼                              ▼
     error_based(param)            boolean_based(param)
   inject  "1'"                   send "1 AND 1=1"  → resp_true
   send, look for SQL error       send "1 AND 1=2"  → resp_false
   in body                        compare true vs false vs baseline
           │                              │
           └──────────────┬───────────────┘
                          ▼
             first technique that fires wins
                          │
                 ┌────────▼──────────┐
                 │ Verdict::Vulnerable│  { param, method, technique, payload }
                 │      or ::Safe      │
                 └────────────────────┘
```

**Point clé sur le flux :** la baseline se calcule **une fois**, avant la boucle. Chaque test se juge **par rapport à elle**, jamais dans le vide. Un corps « différent » n'a de sens que relativement à un corps « normal » de référence.

### Le schéma qui explique boolean-based

Trois réponses, deux comparaisons. La vulnérabilité se lit dans le **contraste** :

```
             BASELINE            AND 1=1  (vrai)      AND 1=2  (faux)
   status      200                  200                  200
   length     1240                 1240                  318
   contenu   liste produits     liste produits       page vide
              │                     │                    │
              └── identique ────────┘                    │
                     resp_true ≈ baseline                │
                                                         ▼
                        resp_false ≠ baseline  →  INJECTABLE (boolean-based)
```

Si `AND 1=1` ressemble à la baseline **et** que `AND 1=2` en diffère nettement, le paramètre passe dans la clause SQL : vulnérable. Si les trois sont identiques, le param est filtré ou inexistant : safe.

---

## 3. Concepts à maîtriser

### 3.1 Comparer deux réponses sans se faire avoir

Un site vivant renvoie rarement deux fois **exactement** le même octet (jetons CSRF, horodatage, pub). Comparer les corps caractère par caractère produit des faux positifs. On compare donc sur des signaux robustes, combinés :

1. **le statut HTTP** (200 vs 500) ;
2. **la longueur** du corps, avec une **tolérance** (ex. ±5 %) ;
3. **la présence/absence de marqueurs** (message d'erreur, ou un texte attendu de la page normale).

Documentation :
- `str::contains`, `str::len` : https://doc.rust-lang.org/std/primitive.str.html
- Précédent : la baseline vient de `http::send` ([phase 3](phase-03-client-http.md))

> Analogie : reconnaître si deux photos montrent la même pièce. On ne compare pas pixel par pixel (la lumière change) ; on compare le nombre de meubles, leur position, la couleur des murs. Plusieurs indices grossiers valent mieux qu'un seul indice exact.

⚠️ **Piège classique** : décider « vulnérable » sur une égalité stricte des corps. **Symptôme** : le même param est tantôt vulnérable tantôt non, sans logique — un jeton dans la page bouge à chaque requête. **Correction** : comparer longueur (avec tolérance) + statut, pas l'octet exact.

### 3.2 Les signatures d'erreur SQL

L'error-based repose sur une table de sous-chaînes qui trahissent un SGBD renvoyant son erreur brute. Ces motifs servent ici (détecter *qu'il y a* une erreur) et resserviront en [phase 5](README.md) (dire *quel* moteur).

| Moteur | Signature dans le corps |
|---|---|
| MySQL | `You have an error in your SQL syntax`, `MySQL server version` |
| SQLite | `SQLITE_ERROR`, `unrecognized token`, `SQL logic error` |
| PostgreSQL | `PSQLException`, `syntax error at or near`, `PostgreSQL` |

Documentation :
- Ces motifs sont documentés dans les cheat-sheets d'injection (PayloadsAllTheThings) — à recopier, pas à deviner.

⚠️ **Piège classique** : une signature trop générique, comme `error`. **Symptôme** : faux positifs sur toute page contenant le mot « error » (un formulaire de login, un 404 stylé). **Correction** : des motifs **spécifiques au SGBD**, jamais des mots courants.

### 3.3 Modéliser le verdict avec un `enum`

Le résultat d'un test n'est pas un `bool` : c'est « sûr » **ou** « vulnérable *avec* ces détails ». Un `enum` à variante porteuse de données l'exprime exactement (voir [`rust/02`](../rust/02-structurer-le-code.md) § 2).

⚠️ **Piège classique** : rendre `bool` puis stocker le payload dans une variable à côté. **Symptôme** : à la compilation rien ; à l'exécution, un `true` sans payload associé, ou un payload périmé. **Correction** : l'`enum` colle le verdict et sa preuve dans la même valeur.

### 3.4 Lexique

| Terme | Ce qu'il désigne | Où ça vit |
|---|---|---|
| baseline | la `Response` à l'URL non injectée | `scanner::run` |
| signature | sous-chaîne qui trahit une erreur SQL | `techniques::SQL_ERRORS` |
| tolérance | marge de longueur admise entre deux réponses « identiques » | `techniques::similar` |
| `Verdict` | `enum` : `Safe` ou `Vulnerable { … }` | [`src/techniques.rs`](../../src/techniques.rs) |
| baseline vraie/fausse | réponses à `AND 1=1` / `AND 1=2` | `boolean_based` |

---

## 4. Décomposition des étapes

1. **`similar`** — comparateur de deux réponses (statut + longueur tolérante).
2. **`error_based`** — un payload, une recherche de signature.
3. **`boolean_based`** — deux payloads, trois comparaisons.
4. **`scanner::run`** — orchestration params × techniques, baseline en tête.

---

## 5. Le corps du document : une section par fonction

### 5.0 · Boîte à outils

| Opération | API exacte |
|---|---|
| Envoyer une requête | `client.send(method, url, form)?` → [phase 3](phase-03-client-http.md) |
| URL injectée (GET) | `target.with_injected(key, payload)` → [phase 2](phase-02-url-et-parametres.md) |
| Chercher une sous-chaîne | `body.contains("…")` |
| Longueur | `body.len()` |
| Valeur absolue d'un écart | `(a as isize - b as isize).unsigned_abs()` |
| Table de signatures | `const SQL_ERRORS: &[&str] = &["You have an error…", …];` |

### 5.1 · `similar`

**Ce qu'elle doit accomplir :** dire si deux réponses sont « la même page » au sens robuste : même statut, et longueurs proches à la tolérance près. Brique commune à boolean-based.

**Décisions**

| Décision | Pourquoi |
|---|---|
| tolérance **relative** (%) et non absolue | 5 octets d'écart n'ont pas le même sens sur 100 o et sur 100 Ko |
| statut d'abord | un 200 vs 500 tranche sans même regarder le corps |

**🧭 Quoi utiliser, et pourquoi**

| Ce que dit le pseudo-code | Où trouver comment |
|---|---|
| `same status` | comparer `a.status == b.status` |
| `lengths within tolerance` | écart de `len` rapporté au plus grand, comparé au seuil |

**Prototype**

```rust
pub fn similar(a: &Response, b: &Response, tolerance: f64) -> bool;
```

**Corps**

```
similar(a : &Response, b : &Response, tolerance : f64) -> bool:
    if a.status != b.status:
        return false
    longest : usize = max(a.body.len(), b.body.len())
    if longest == 0:
        return true                                  // both empty
    diff : usize = absolute difference of the two lengths
    return (diff as ratio of longest) <= tolerance
```

### 5.2 · `error_based`

**Ce qu'elle doit accomplir :** injecter un caractère qui casse la syntaxe SQL (`'`), envoyer, et déclarer vulnérable si le corps contient une signature d'erreur SQL. La technique la moins chère : une seule requête.

**Décisions**

| Décision | Pourquoi |
|---|---|
| payload = un simple `'` (puis variantes `"`, `')`) | déclenche l'erreur avec le minimum de bruit |
| on rend aussi la signature trouvée | elle pré-identifie le moteur → réutilisée en phase 5 |

**⚠️ Pièges**

⚠️ Une appli qui affiche une page d'erreur **générique** (« une erreur est survenue ») sans le message SQL brut : error-based échoue **légitimement** → c'est à boolean-based de prendre le relais. Ne pas conclure « safe » sur ce seul échec.

**🧭 Quoi utiliser, et pourquoi**

| Ce que dit le pseudo-code | Où trouver comment |
|---|---|
| `send with a broken-syntax payload` | `with_injected(key, base + "'")` puis `client.send(...)` |
| `any SQL error signature present` | boucler sur `SQL_ERRORS`, `body.contains(sig)` |

*Troisième temps :* on cherche des signatures **spécifiques** (§ 3.2) et non le mot « error » : la présence d'un message *propre au SGBD* est une preuve quasi certaine d'injection, là où un mot générique ne prouve rien. C'est ce qui rend error-based fiable malgré sa simplicité.

**Lignes de log**

```rust
eprintln!("[detect] error-based on '{key}' with payload {payload}");
```

**Prototype**

```rust
pub fn error_based(client: &Client, cfg: &Config, target: &Target, key: &str)
    -> Result<Verdict, VaccineError>;
```

**Corps**

```
error_based(client, cfg, target, key : &str) -> Result<Verdict, VaccineError>:
    for each payload in ["'", "\"", "')"]:
        url  : String   = inject payload into key                → Target · with_injected
        resp : Response = client.send(cfg.method, url, form)?    → phase 3
        for each sig in SQL_ERRORS:
            if resp.body contains sig:
                log ①
                return Ok(Vulnerable { key, method: cfg.method,
                                       technique: ErrorBased, payload })
    return Ok(Safe)
```

### 5.3 · `boolean_based`

**Ce qu'elle doit accomplir :** distinguer un paramètre qui entre dans la logique SQL. On envoie une condition vraie (`AND 1=1`) et une fausse (`AND 1=2`) ; si la vraie ressemble à la baseline et la fausse en diffère, c'est injectable.

**Décisions**

| Décision | Pourquoi |
|---|---|
| comparer aux **trois** réponses (baseline, vrai, faux) | `vrai ≈ baseline` **et** `faux ≠ baseline` : les deux conditions ensemble écartent le hasard |
| payloads suffixés au bout de la valeur (`1 AND 1=1`) | reste dans le contexte de la requête d'origine |

**⚠️ Pièges**

⚠️ Ne tester que `faux ≠ baseline` sans vérifier `vrai ≈ baseline` : une page qui change pour **toute** entrée (pas seulement la fausse) donnerait un faux positif. Les deux conditions sont nécessaires.
⚠️ Tolérance trop serrée : un jeton variable dans la page fait échouer `vrai ≈ baseline`. Calibrer sur le labo.

**🧭 Quoi utiliser, et pourquoi**

| Ce que dit le pseudo-code | Où trouver comment |
|---|---|
| `send the true and false variants` | deux `with_injected` + deux `send` |
| `true looks like baseline` | `similar(resp_true, baseline, tol)` → § 5.1 |
| `false differs from baseline` | `not similar(resp_false, baseline, tol)` |

*Troisième temps :* la double condition encode le raisonnement pentesteur : si le paramètre **modifie** la requête, alors une condition toujours-vraie ne change rien au résultat (donc ≈ baseline) tandis qu'une condition toujours-fausse vide le résultat (donc ≠ baseline). Un paramètre inerte, lui, ignore les deux : les trois réponses se ressemblent, et la double condition n'est jamais remplie. C'est pourquoi tester une seule des deux laisse passer des faux positifs.

**Lignes de log**

```rust
eprintln!("[detect] boolean-based on '{key}': true~{t} false~{f}");
```

**Prototype**

```rust
pub fn boolean_based(client: &Client, cfg: &Config, target: &Target,
                     key: &str, baseline: &Response)
    -> Result<Verdict, VaccineError>;
```

**Corps**

```
boolean_based(client, cfg, target, key, baseline : &Response)
    -> Result<Verdict, VaccineError>:

    true_payload  : &str = original value + " AND 1=1"
    false_payload : &str = original value + " AND 1=2"

    resp_true  : Response = send with true_payload injected into key    → phase 3
    resp_false : Response = send with false_payload injected into key

    true_matches  : bool = similar(resp_true,  baseline, tolerance)     → F · similar
    false_differs : bool = not similar(resp_false, baseline, tolerance)

    log ①
    if true_matches and false_differs:
        return Ok(Vulnerable { key, method: cfg.method,
                               technique: BooleanBased, payload: true_payload })
    return Ok(Safe)
```

### 5.4 · `scanner::run`

**Ce qu'elle doit accomplir :** l'orchestration. Calculer la baseline, parcourir chaque paramètre, tenter chaque technique, s'arrêter au premier verdict `Vulnerable` (ou conclure `Safe`), puis passer la main à l'extraction (phases suivantes).

**Décisions**

| Décision | Pourquoi |
|---|---|
| baseline calculée **avant** la boucle | une seule requête de référence pour tous les tests |
| error-based avant boolean-based | moins chère (1 requête vs 2) et plus décisive quand elle marche |
| on s'arrête au premier param vulnérable | suffisant pour le rendu ; scanner tous les params est un raffinement |

**Prototype**

```rust
pub fn run(cfg: &Config) -> Result<Option<Verdict>, VaccineError>;
```

**Corps**

```
run(cfg : &Config) -> Result<Option<Verdict>, VaccineError>:

    client   : Client   = Client::new()                    → phase 3
    target   : Target   = parse cfg.url                    → phase 2
    baseline : Response = send the non-injected request     → phase 3

    if target.params is empty:
        return Ok(absent)                                   // nothing to inject

    for each param in target.params:
        verdict : Verdict = error_based(client, cfg, target, param.key)?   → F · error_based
        if verdict is Vulnerable:
            return Ok(Some(verdict))

        verdict = boolean_based(client, cfg, target, param.key, baseline)? → F · boolean_based
        if verdict is Vulnerable:
            return Ok(Some(verdict))

    return Ok(absent)                                       // all params safe
```

---

## 6. Pièges spécifiques à cette phase

- **La baseline elle-même peut être instable.** Si la page varie à chaque chargement même sans injection, `similar` renvoie des verdicts erratiques. Vérifiez d'abord, à la main, que deux chargements identiques donnent `similar == true`. Sinon, la tolérance est mal réglée ou la page est inadaptée au boolean-based (basculez sur error-based).
- **Interaction error × boolean.** Une appli peut être vulnérable mais masquer ses erreurs : error-based échoue, boolean-based réussit. L'ordre (error puis boolean) et le fait de **ne pas conclure `Safe` avant d'avoir essayé les deux** sont ce qui rend la détection robuste. C'est un piège d'orchestration, pas de technique isolée.

---

## 7. Tests unitaires

> [!IMPORTANT]
> Les techniques appellent le réseau — donc **hors périmètre unitaire** (même raison qu'en phase 3). Ce qui est **testable purement**, c'est `similar` et la recherche de signatures : des fonctions de chaîne sans I/O. On les isole exprès pour ça.

### 7.1 `src/techniques.rs` — `#[cfg(test)]`

**Ce que tu testes :**
- `similar` : deux corps de même longueur/statut → `true` ; écart au-delà de la tolérance → `false` ; statuts différents → `false`.
- détection de signature : un corps contenant `You have an error in your SQL syntax` est reconnu ; un corps quelconque ne l'est pas.

**Ce que le test doit prouver au-delà du comportement :** que la **tolérance** fait bien son travail (petit écart accepté, grand écart rejeté), et qu'aucune signature générique ne matche une page ordinaire.

```rust
#[cfg(test)]
mod tests {
    use super::*;

    fn resp(status: u16, body: &str) -> Response {
        Response { status, body: body.to_string() }
    }

    #[test]
    fn similar_accepts_small_length_diff() {
        let a = resp(200, &"x".repeat(1000));
        let b = resp(200, &"x".repeat(1010));      // +1%
        assert!(similar(&a, &b, 0.05));
    }

    #[test]
    fn similar_rejects_status_mismatch() {
        assert!(!similar(&resp(200, "ok"), &resp(500, "ok"), 0.05));
    }

    #[test]
    fn detects_mysql_signature() {
        let body = "You have an error in your SQL syntax near '''";
        assert!(SQL_ERRORS.iter().any(|s| body.contains(s)));
    }
}
```

### 7.2 Hors périmètre

| Fonction | Pourquoi | Comment on vérifie |
|---|---|---|
| `error_based`, `boolean_based`, `run` | requêtes réseau | test manuel contre DVWA/SQLi-Labs, lecture des logs `[detect]` |

### 7.3 Résultats attendus

- Unitaires `similar` / signatures : PASS.
- Manuel : sur SQLi-Labs `?id=1`, l'outil annonce le param `id` vulnérable, la technique et le payload — PASS.

---

## 8. Ordre de développement recommandé

1. `Verdict` (enum) et `SQL_ERRORS` (const), `similar` + ses tests.
2. `error_based`, testé à la main sur un labo à erreurs visibles (SQLi-Labs).
3. `boolean_based`, calibrer la tolérance sur ce labo.
4. `scanner::run` : baseline + boucle.
5. Câbler `main` pour afficher le verdict.
6. `make test` (unitaires) + passe manuelle.

> Quand l'outil nomme le paramètre vulnérable, la technique et le payload sur au moins un labo, cette phase est close. On passe à la **phase 5 : fingerprint du moteur SQL** ([esquisse](README.md)).
