# Vaccine — Phase 2 : URL et paramètres injectables

> Cette phase découpe l'URL de la `Config` en une cible et une liste de paramètres. Un scanner injecte **dans les paramètres**, un à la fois ; il faut donc d'abord savoir lesquels existent, et pouvoir reconstruire l'URL avec un paramètre modifié. C'est purement du traitement de chaînes, sans réseau — le bon moment pour assimiler `struct`/`enum` sur un cas concret.

---

## 1. Où on en est

**Fait :**
- Phase 1 : `cli::parse` rend une `Config { url, method, output }`.

**À faire dans cette phase :**
- Séparer `http://host/path` de la query string `?id=1&cat=books`.
- Modéliser chaque paire `clé=valeur` en `Param`.
- Savoir **reconstruire** l'URL en remplaçant la valeur d'un seul paramètre.

**Ce qui suit (phase 3) :** envoyer réellement ces URLs reconstruites au serveur.

> Prérequis Rust : [`rust/02-structurer-le-code.md`](../rust/02-structurer-le-code.md) (struct, enum, modules, type d'erreur).

---

## 2. Architecture cible

```
   "http://host/p.php?id=1&cat=books"
                │  cut on '?'
     ┌──────────┴───────────┐
     ▼                      ▼
  base                    query "id=1&cat=books"
"http://host/p.php"         │  split on '&', then on '='
                            ▼
                    params = [ Param{key:"id",  value:"1"},
                               Param{key:"cat", value:"books"} ]

   inject("id", "1' OR '1'='1")  reconstruit :
     "http://host/p.php?id=1' OR '1'='1&cat=books"
                            ▲ seul id change, cat reste intact
```

**Point clé sur le flux :** pour injecter dans `id`, il faut réémettre l'URL **complète** avec `id` modifié et **tous les autres paramètres inchangés**. La `Target` doit donc conserver l'ordre et l'intégralité des params, pas seulement celui qu'on teste. On ne stocke pas « le param vulnérable » ici — on stocke *tout*, et l'injection choisira lequel bouger.

---

## 3. Concepts à maîtriser

### 3.1 Découper une chaîne : `split`, `split_once`

`str::split_once('?')` rend `Option<(&str, &str)>` : la partie avant et après le premier `?`. `str::split('&')` rend un itérateur sur les morceaux. C'est tout l'outillage nécessaire.

Documentation :
- `str::split_once` : https://doc.rust-lang.org/std/primitive.str.html#method.split_once
- `str::split` : https://doc.rust-lang.org/std/primitive.str.html#method.split

> Analogie : `split_once('?')` coupe la ficelle au **premier** nœud et vous donne les deux bouts. `split('&')` coupe à **chaque** nœud et vous rend tous les segments.

⚠️ **Piège classique** : une URL **sans** query string (`http://host/`). **Symptôme** : `split_once('?')` rend `None` ; si vous faites `.unwrap()`, panique. **Correction** : `None` est un cas légitime (zéro paramètre à injecter) — traitez-le, ne le forcez pas.

### 3.2 De l'itérateur au `Vec` : `.map().collect()`

On transforme chaque segment `"id=1"` en `Param`, puis on rassemble en `Vec<Param>` avec `.collect()`. C'est le pattern Rust le plus courant : `iter → map(transformation) → collect`.

Documentation :
- `Iterator::map`, `collect` : https://doc.rust-lang.org/book/ch13-02-iterators.html

⚠️ **Piège classique** : une valeur contenant un `=` (`token=a=b`). **Symptôme** : un `split('=')` naïf coupe en trois. **Correction** : `split_once('=')` — coupe au **premier** `=` seulement, la valeur garde les suivants.

### 3.3 Lexique

| Terme | Ce qu'il désigne | Où ça vit |
|---|---|---|
| base | l'URL sans la query string | [`src/url.rs`](../../src/url.rs) — `Target::base` |
| query string | la partie après `?` | `url::parse` |
| `Param` | une paire `key`/`value` d'un paramètre | `src/url.rs` |
| `Target` | la base + tous les `Param`, reconstructible | `src/url.rs` |
| inject | reconstruire l'URL avec un `Param` modifié | `Target::with_injected` |

---

## 4. Décomposition des étapes

1. **Types** — `Param { key, value }` et `Target { base, params }` dans `src/url.rs`.
2. **`parse`** — URL string → `Target`.
3. **`with_injected`** — rendre l'URL string avec un paramètre remplacé.

---

## 5. Le corps du document : une section par fonction

### 5.0 · Boîte à outils

| Opération | API exacte |
|---|---|
| Couper au premier séparateur | `s.split_once('?') -> Option<(&str, &str)>` |
| Couper à chaque séparateur | `s.split('&') -> impl Iterator` |
| Transformer + rassembler | `.map(|x| …).collect::<Vec<_>>()` |
| Recomposer une query | `params.iter().map(...).collect::<Vec<_>>().join("&")` |
| Construire une String | `format!("{base}?{query}")` |

### 5.1 · `parse`

**Ce qu'elle doit accomplir :** transformer l'URL de la `Config` en `Target`. Gérer proprement l'absence de query string (zéro paramètre).

**Décisions**

| Décision | Pourquoi |
|---|---|
| `Target` garde **tous** les params, ordonnés | l'injection doit réémettre les autres intacts (§ 2) |
| une URL sans `?` donne un `Target` à `params` vide, pas une erreur | c'est un cas légal ; l'absence de param vulnérable se constatera à la détection |
| on ne décode pas l'URL-encoding ici | on manipule les valeurs telles quelles ; l'encodage se gère au moment d'émettre (phase 3, § 6) |

**⚠️ Pièges**

⚠️ Query vide après `?` (`http://h/?`) : `split('&')` sur `""` rend un segment vide → filtrer.
⚠️ Paramètre sans valeur (`?id`) : `split_once('=')` rend `None` → décider (value = `""`).

**🧭 Quoi utiliser, et pourquoi**

| Ce que dit le pseudo-code | Où trouver comment |
|---|---|
| `split url on first '?'` | `split_once('?')` → rend `Option`, gère l'absence de query |
| `for each pair in query split on '&'` | `.split('&')` |
| `split pair on first '='` | `split_once('=')` — protège les valeurs contenant `=` (§ 3.2) |

**Prototype**

```rust
pub fn parse(url: &str) -> Target;
```

**Corps**

```
parse(url : &str) -> Target:

    (base, query) : (&str, Option<&str>) = split url on first '?'   // None query if no '?'

    params : Vec<Param> = empty
    if query is set and query is not empty:
        for each pair in query split on '&':
            if pair is empty:
                continue
            (key, value) : (&str, &str) = split pair on first '='   // value = "" if no '='
            params.push(Param { key, value })

    return Target { base, params }
```

### 5.2 · `with_injected`

**Ce qu'elle doit accomplir :** rendre l'URL complète où **un seul** paramètre (repéré par son nom) prend une nouvelle valeur, tous les autres inchangés. C'est la fonction que la détection appellera en boucle avec des payloads.

**Décisions**

| Décision | Pourquoi |
|---|---|
| prend le nom du param à injecter + la nouvelle valeur | la détection itère sur les noms ; passer le nom découple des index |
| rend une `String` (URL complète), pas un `Target` | c'est directement ce que le client HTTP consomme |

**🧭 Quoi utiliser, et pourquoi**

| Ce que dit le pseudo-code | Où trouver comment |
|---|---|
| `rebuild each pair as key=value` | `format!("{}={}", p.key, chosen_value)` |
| `join pairs with '&'` | `.collect::<Vec<_>>().join("&")` |
| `stitch base and query` | `format!("{base}?{query}")` |

*Troisième temps :* on reconstruit **tous** les params, en substituant la valeur seulement pour celui dont la clé correspond. Reconstruire tout — plutôt que « patcher » une sous-chaîne dans l'URL d'origine — évite les faux positifs de remplacement (imaginez `id` présent aussi dans `path`). La source de vérité est la liste `params`, pas la chaîne originale.

**Prototype**

```rust
pub fn with_injected(&self, target_key: &str, payload: &str) -> String;
```

**Corps**

```
with_injected(self, target_key : &str, payload : &str) -> String:

    pairs : Vec<String> = empty
    for each p in self.params:
        value : &str = if p.key == target_key then payload else p.value
        pairs.push(format "key=value" for p.key and value)

    query : String = pairs joined with "&"
    return format "base?query"
```

---

## 6. Pièges spécifiques à cette phase

- **POST vs GET.** Pour GET, les params sont dans l'URL (ce document). Pour POST, ils iront dans le corps du formulaire (phase 3). Le `Target` est le **même** ; c'est le client HTTP qui décidera où poser les paramètres selon la méthode. Ne dupliquez pas la structure.
- **Ne pas confondre « aucun param » et « échec ».** Une URL sans query donne un `Target` valide à `params` vide. C'est la détection (phase 4) qui dira « rien à injecter ici » — pas le parsing.

---

## 7. Tests unitaires

### 7.1 `src/url.rs` — module `#[cfg(test)]`

**Ce que tu testes :**
- URL avec deux params → `params.len() == 2`, clés et valeurs correctes.
- URL sans `?` → `params` vide.
- Valeur contenant un `=` (`?t=a=b`) → `value == "a=b"`.
- `with_injected("id", "X")` → `id` remplacé, `cat` intact.

**Ce que le test doit prouver au-delà du comportement :** que `with_injected` **ne touche pas** aux autres paramètres, et que `split_once('=')` préserve les `=` dans la valeur. Deux règles qu'un « nettoyage » casserait.

```rust
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_two_params() {
        let t = parse("http://h/p?id=1&cat=books");
        assert_eq!(t.params.len(), 2);
        assert_eq!(t.params[0].key, "id");
        assert_eq!(t.params[1].value, "books");
    }

    #[test]
    fn value_keeps_inner_equals() {
        let t = parse("http://h/p?token=a=b");
        assert_eq!(t.params[0].value, "a=b");
    }

    #[test]
    fn inject_touches_only_target() {
        let t = parse("http://h/p?id=1&cat=books");
        let url = t.with_injected("id", "X");
        assert!(url.contains("id=X"));
        assert!(url.contains("cat=books"));   // untouched
    }
}
```

### 7.3 Résultats attendus

- `parses_two_params` : PASS
- `value_keeps_inner_equals` : PASS
- `inject_touches_only_target` : PASS

---

## 8. Ordre de développement recommandé

1. `Param` et `Target` (structs, `pub`).
2. `parse` sur le cas « deux params », premier test vert.
3. Cas limites : sans `?`, query vide, valeur avec `=`.
4. `with_injected`, test d'isolation.
5. `make test`.

> Quand les trois tests passent et que `with_injected` réémet une URL correcte, cette phase est close. On passe à la **phase 3 : client HTTP**, qui enverra enfin ces URLs.
