# Vaccine — Phase 3 : client HTTP

> Cette phase donne au projet sa première capacité réseau : envoyer une requête (GET ou POST) et rendre une `Response` normalisée (statut + corps). C'est la brique que la détection appellera des centaines de fois. Le point subtil est unique mais critique : une page d'erreur HTTP est une **donnée**, pas un échec.

---

## 1. Où on en est

**Fait :**
- Phase 1 : `Config` (dont la `HttpMethod`).
- Phase 2 : `Target` avec `with_injected`.

**À faire dans cette phase :**
- Ajouter la crate `ureq`.
- `http::send` : émettre une requête selon la méthode, rendre une `Response`.
- Capturer les réponses d'erreur HTTP (4xx/5xx) au lieu d'abandonner.

**Ce qui suit (phase 4) :** comparer deux `Response` pour décider si un paramètre est vulnérable.

> Prérequis Rust : [`rust/03-http-io-et-crates.md`](../rust/03-http-io-et-crates.md) en entier.

---

## 2. Architecture cible

```
   Config.method + une URL (ou Target + un param)   payload
                          │
                 ┌────────▼─────────┐
                 │   http::send     │
                 └────────┬─────────┘
          GET │                    │ POST
     ureq::get(url).call()   ureq::post(base).send_form(pairs)
                 │                    │
                 └────────┬───────────┘
                          ▼
              match on the ureq result :
                Ok(r)               → keep r
                Err(Status(_, r))   → keep r        ← page d'erreur = donnée
                Err(Transport(t))   → VaccineError::Http
                          │
                 ┌────────▼─────────┐
                 │ Response {       │
                 │  status, body }  │
                 └──────────────────┘
```

**Point clé sur le flux :** GET et POST diffèrent sur **où** vont les paramètres (URL vs corps), mais rendent le **même** type `Response`. Tout le reste du projet ignore la méthode : il reçoit une `Response` et la compare. C'est ce découplage qui permet à la détection (phase 4) d'être écrite une fois pour les deux méthodes.

---

## 3. Concepts à maîtriser

### 3.1 Le résultat de `ureq` : trois cas, pas deux

`ureq::...call()` rend `Result<Response, ureq::Error>`, et l'`Error` a **deux formes** : `Status(code, response)` pour un 4xx/5xx (le serveur a répondu, mais mal), et `Transport(...)` pour un vrai problème réseau (DNS, refus, timeout). Un scanner veut le corps dans les **trois** cas sauf `Transport`.

Documentation :
- `ureq::Error` : https://docs.rs/ureq/latest/ureq/enum.Error.html
- Précédent : [`rust/03-http-io-et-crates.md`](../rust/03-http-io-et-crates.md) § 2

> Analogie : vous frappez à une porte. `Ok` = on vous ouvre en souriant. `Status` = on vous ouvre et on vous crie dessus (utile : vous avez vu l'intérieur). `Transport` = personne, la rue est barrée. Seul le dernier cas est un vrai échec pour un enquêteur.

⚠️ **Piège classique** : propager `Err(Status(...))` avec `?`. **Symptôme** : l'outil s'arrête pile quand une injection error-based réussit (le serveur renvoie un 500 avec le message SQL dans le corps). **Correction** : le `match` qui garde la réponse sur `Status` — c'est le cœur de cette phase.

### 3.2 Le coût d'un aller-retour

Chaque `send` est une vraie requête réseau : lente, visible côté serveur, et faillible. Deux conséquences de conception : on calcule **une fois** la réponse de référence (l'URL non injectée) pour comparer sans la re-télécharger à chaque payload ; et on prévoit un **timeout** pour ne pas figer le scan sur un serveur muet (utile aussi pour la future technique *time-based*).

Documentation :
- Timeout `ureq` : `ureq::AgentBuilder::new().timeout(...)` — https://docs.rs/ureq/

⚠️ **Piège classique** : pas de timeout. **Symptôme** : sur un serveur lent ou un payload qui déclenche un `SLEEP`, le scan se bloque indéfiniment. **Correction** : un agent configuré avec un timeout raisonnable (ex. 10 s), réutilisé pour toutes les requêtes.

### 3.3 Lexique

| Terme | Ce qu'il désigne | Où ça vit |
|---|---|---|
| `Response` | notre type : `status: u16` + `body: String` | [`src/http.rs`](../../src/http.rs) |
| agent | le client `ureq` réutilisable (connexion, timeout) | `http::Client` |
| `Status` | variante d'erreur `ureq` pour un 4xx/5xx | `http::send` |
| `Transport` | variante d'erreur `ureq` réseau (fatal) | `http::send` |
| réponse de référence | la réponse à l'URL non injectée, calculée une fois | phase 4 |

---

## 4. Décomposition des étapes

1. **Dépendance** — `cargo add ureq` (+ `urlencoding` plus tard si besoin).
2. **Types** — `Response`, et un `Client` qui encapsule l'agent `ureq`.
3. **`send`** — la méthode qui émet et normalise.
4. **Fumée** — un `main` temporaire qui GET une URL et affiche statut + longueur.

---

## 5. Le corps du document : une section par fonction

### 5.0 · Boîte à outils

| Opération | API exacte |
|---|---|
| Créer un agent avec timeout | `ureq::AgentBuilder::new().timeout(Duration::from_secs(10)).build()` |
| GET | `agent.get(url).call()` |
| POST formulaire | `agent.post(url).send_form(&pairs)` où `pairs: &[(&str, &str)]` |
| Statut | `resp.status() -> u16` |
| Corps | `resp.into_string()? -> String` |
| Distinguer l'erreur | `match err { ureq::Error::Status(c, r) => …, ureq::Error::Transport(t) => … }` |

**Où voir ces API à l'œuvre :** la doc `ureq` (lien § 3.1) ; le pattern `match` sur l'erreur est détaillé dans [`rust/03-http-io-et-crates.md`](../rust/03-http-io-et-crates.md) § 2.

### 5.1 · `send`

**Ce qu'elle doit accomplir :** émettre une requête pour une URL donnée et une méthode donnée, en posant les paramètres au bon endroit (query pour GET, corps pour POST), et rendre une `Response` — y compris quand le serveur répond en erreur HTTP.

**Décisions**

| Décision | Pourquoi |
|---|---|
| une seule `send`, paramétrée par la méthode | la détection l'appelle sans savoir GET/POST |
| l'agent `ureq` vit dans `Client` et est réutilisé | évite de recréer connexion + timeout à chaque requête (des centaines) |
| `Status` → on garde la réponse ; `Transport` → `Err` | une page d'erreur est une donnée ; une panne réseau est fatale |
| pour POST, on passe les paires `(clé, valeur)` déjà injectées | `send_form` encode le corps ; la substitution du payload est faite en amont |

**⚠️ Pièges**

⚠️ `into_string()` échoue sur un corps non-UTF-8 ou énorme : le `?` le remonte en `Http` — acceptable, mais ne pas l'`unwrap()`.
⚠️ Réutiliser une `Response` `ureq` deux fois : `into_string` **consomme** la réponse (elle prend `self`). On l'appelle une fois, on stocke la `String`.

**🧭 Quoi utiliser, et pourquoi**

| Ce que dit le pseudo-code | Où trouver comment |
|---|---|
| `perform the request for this method` | `agent.get(...).call()` / `agent.post(...).send_form(...)` |
| `on Status keep the response` | `match` sur `ureq::Error::Status(code, resp)` |
| `read the body as string` | `.into_string()?` — consomme la réponse |

*Troisième temps :* on lit le corps **après** avoir résolu le `match`, sur la réponse retenue (qu'elle vienne de `Ok` ou de `Status`). Lire le corps *dans* chaque branche dupliquerait l'appel ; unifier les deux branches sur une même variable `resp`, puis lire une fois, garde une seule source de corps.

**Lignes de log**

```rust
eprintln!("[http] -> {method} {url}");
eprintln!("[http] <- status {status}, {len} bytes");
```

> Ces logs sur `stderr` (pas `stdout`) laissent la sortie « propre » du rapport sur `stdout`. Ils deviennent précieux en phase 4 pour voir *quelle* requête a produit *quel* écart. Gardez-les identiques : les instructions de vérification y renvoient.

**Prototype**

```rust
pub fn send(&self, method: &HttpMethod, url: &str, form: &[(&str, &str)])
    -> Result<Response, VaccineError>;
```

**Corps**

```
send(self, method : &HttpMethod, url : &str, form : &[(&str, &str)])
    -> Result<Response, VaccineError>:

    log ①                                          // "[http] -> METHOD url"

    result : Result<ureq::Response, ureq::Error> =
        match method:
            Get  => self.agent.get(url).call()
            Post => self.agent.post(url).send_form(form)

    resp : ureq::Response = match result:
        Ok(r)                          => r
        Err(Status(_code, r))          => r         // error page is DATA, keep it
        Err(Transport(t))              => return Err(Http from t)

    status : u16 = resp.status()
    body   : String = resp.into_string()?          // consumes resp, once

    log ②                                          // "[http] <- status N, M bytes"
    return Ok(Response { status, body })
```

### 5.2 · `Client::new`

**Ce qu'elle doit accomplir :** construire le `Client` en préparant l'agent `ureq` avec son timeout. Appelée une fois, en tête de scan.

**Prototype**

```rust
pub fn new() -> Client;
```

**Corps**

```
new() -> Client:
    agent : ureq::Agent = build a ureq agent with a 10s timeout
    return Client { agent }
```

---

## 6. Pièges spécifiques à cette phase

- **GET avec payload non encodé.** Un payload contient des espaces et des `'`. En GET, ils passent dans l'URL et peuvent la casser. Pour un premier jet sur labo local, ça passe souvent tel quel ; dès qu'un payload casse, encoder la valeur (`urlencoding::encode`, voir [`rust/03`](../rust/03-http-io-et-crates.md) § 6). En POST, `send_form` encode déjà — piège absent.
- **Interaction méthode × source des params.** GET lit les params dans l'URL (déjà injectée par `with_injected`) ; POST les lit dans `form`. Ne pas mélanger : pour POST, l'URL est la base **sans** query, et les paires injectées vont dans `form`. C'est l'articulation avec la phase 2 à tenir au clair.

---

## 7. Compilation et configuration

```bash
cargo add ureq
make            # compile avec la nouvelle dépendance
```

- La première compilation télécharge `ureq` et ses dépendances : plus longue, normale.
- `Cargo.lock` est mis à jour : **le committer** pour garder le build reproductible (exigence du sujet).

---

## 8. Tests unitaires

> [!IMPORTANT]
> **`send` fait un vrai appel réseau : elle est hors périmètre du test unitaire pur.** Un test unitaire ne doit pas dépendre d'un serveur. On teste donc autour : la construction des paires POST, la logique de choix GET/POST, et on valide `send` par un **test manuel** contre un labo local.

### 8.1 Ce qui est testable sans réseau

- Si vous extrayez une fonction `build_form(target, key, payload) -> Vec<(String, String)>`, testez qu'elle injecte le bon param et garde les autres — même logique que `with_injected` (phase 2), côté POST.

### 8.2 Ce qui reste hors périmètre

| Fonction | Pourquoi elle n'est pas testée en unitaire | Comment on la vérifie |
|---|---|---|
| `send` | dépend d'un serveur HTTP réel | test manuel contre un labo local, lecture des logs `[http]` |
| `Client::new` | rien à assurer qu'un appel réussi | couverte par le test manuel de `send` |

### 8.3 Résultats attendus (test manuel)

Contre un labo local (voir [phase 9](README.md) pour le monter) :

```bash
make run ARGS='"http://localhost/vulnerable.php?id=1"'
# stderr attendu :
# [http] -> GET http://localhost/vulnerable.php?id=1
# [http] <- status 200, 1234 bytes
```

- GET sur page existante : status 200, corps non vide — PASS.
- GET sur page inexistante : status 404, corps récupéré quand même (pas d'abandon) — PASS.
- URL injoignable (port fermé) : `VaccineError::Http` propre, pas de panique — PASS.

---

## 9. Ordre de développement recommandé

1. `cargo add ureq`, `Response` et `Client` vides qui compilent.
2. `Client::new` avec le timeout.
3. `send` pour GET seulement, câbler un `main` de fumée, requêter un labo.
4. Vérifier le cas 404 : la réponse revient, l'outil ne s'arrête pas.
5. Ajouter la branche POST.
6. Ajouter les logs `[http]`.

> Quand `make run` requête une page et affiche statut + taille, y compris sur une 404, cette phase est close. On passe à la **phase 4 : détection**, qui compare enfin les réponses.
