# Vaccine — Phase 1 : parser la ligne de commande

> Cette phase transforme `argv` en une `Config` propre : l'URL cible, la méthode HTTP, le fichier de sortie, l'aide. C'est le socle : toutes les phases suivantes reçoivent une `Config` et ne touchent plus jamais à `argv`. Le squelette actuel de [`src/main.rs`](../../src/main.rs) compte les arguments ; on remplace ce comptage par un vrai parsing.

---

## 1. Où on en est

**Fait :**
- Le projet compile (`make`), le binaire `vaccine` est produit à la racine.
- `main.rs` récupère `argv` et refuse le cas « aucun argument ».

**À faire dans cette phase :**
- Reconnaître les options `-X <méthode>`, `-o <fichier>`, `-h`.
- Isoler l'URL (le seul argument positionnel).
- Rendre une `Config` typée, ou une erreur d'usage claire.

**Ce qui suit (phase 2) :** découper l'URL de cette `Config` en paramètres injectables.

> Prérequis Rust : [`rust/01-les-bases.md`](../rust/01-les-bases.md) en entier, et la § 6 (type d'erreur) de [`rust/02-structurer-le-code.md`](../rust/02-structurer-le-code.md).

---

## 2. Architecture cible

```
     argv = ["./vaccine", "-X", "POST", "-o", "res.txt", "http://h/?id=1"]
                                   │
                          ┌────────▼─────────┐
                          │   cli::parse     │
                          └────────┬─────────┘
       boucle sur les tokens :     │
         "-X" → lire le suivant  → method = Post
         "-o" → lire le suivant  → output = Some("res.txt")
         "-h" → afficher l'aide, quitter
         autre → c'est l'URL     → url = "http://h/?id=1"
                                   │
                          ┌────────▼─────────┐
                          │ Config { url,    │
                          │  method, output }│
                          └──────────────────┘
```

**Point clé sur le flux :** une option « à valeur » (`-X`, `-o`) **consomme le token suivant**. La boucle ne peut donc pas être un simple `for` indépendant token par token : quand on voit `-X`, il faut avancer d'un cran de plus pour lire `POST`. On itère donc sur un curseur qu'on peut faire sauter de deux.

---

## 3. Concepts à maîtriser

### 3.1 Itérer sur un `Vec` avec un curseur

Un `for token in &args` ne permet pas d'avancer de deux d'un coup. On utilise soit un index `i` qu'on incrémente à la main, soit un itérateur dont on tire le prochain élément (`args.next()`) quand une option a besoin de sa valeur.

Documentation :
- Itérateurs : https://doc.rust-lang.org/book/ch13-02-iterators.html
- Précédent dans le dépôt : [`src/main.rs`](../../src/main.rs) — fonction `check_args` (récupération d'`argv`)

> Analogie : lire une liste de courses où certaines lignes disent « et la ligne d'après en est la quantité ». On ne peut pas lire chaque ligne isolément ; parfois on en avale deux.

⚠️ **Piège classique** : `-X` en dernière position, sans valeur derrière. **Symptôme** : si vous lisez `args[i + 1]` sans vérifier, `index out of bounds` et panique. **Correction** : tester la présence du token suivant et rendre une `VaccineError::Usage` explicite (« -X requires a method »).

### 3.2 `Option` pour ce qui est facultatif

`-o` est optionnel : `output` est donc `Option<String>` (`None` si absent). L'URL, elle, est **obligatoire** : son absence est une erreur d'usage, pas un `None` toléré.

Documentation :
- `Option` : https://doc.rust-lang.org/std/option/
- Précédent : [`rust/01-les-bases.md`](../rust/01-les-bases.md) § 3

⚠️ **Piège classique** : représenter l'URL manquante par `url: String` vide `""`. **Symptôme** : le bug se voit trois phases plus loin, quand on requête une URL vide. **Correction** : si aucun argument positionnel n'est trouvé à la fin du parsing, c'est une `VaccineError::Usage`, tout de suite.

### 3.3 Lexique

| Terme | Ce qu'il désigne | Où ça vit |
|---|---|---|
| token | un élément de `argv`, ex. `"-X"` ou `"POST"` | `cli::parse` |
| option à valeur | une option suivie de son argument (`-X POST`) | `cli::parse` |
| argument positionnel | l'URL : le token qui n'est pas une option | `cli::parse` |
| `Config` | le résultat typé du parsing | [`src/cli/config.rs`](../../src/cli/config.rs) |

---

## 4. Décomposition des étapes

1. **Types** — définir `HttpMethod` (enum) et `Config` (struct) dans `src/cli/config.rs`.
2. **`parse`** — la boucle qui remplit la `Config` depuis `argv`.
3. **`print_help`** — le texte d'aide de `-h`.
4. **Câblage** — `main` appelle `cli::parse`, gère l'erreur.

---

## 5. Le corps du document : une section par fonction

### 5.0 · Boîte à outils

| Opération | API exacte |
|---|---|
| Longueur d'un `Vec` | `args.len()` |
| Accès borné | `args.get(i) -> Option<&String>` (préférer à `args[i]`, qui panique) |
| Comparer un token | `token == "-X"` ou `token.as_str()` dans un `match` |
| Construire une erreur | `VaccineError::Usage("…".to_string())` |
| Afficher sur stderr | `eprintln!("…")` |
| Quitter proprement | rendre `Err(...)`, `main` fait le `exit` |

### 5.1 · `parse`

**Ce qu'elle doit accomplir :** transformer le `Vec<String>` d'`argv` (moins le nom du programme) en `Config`, ou rendre une erreur d'usage. C'est le seul endroit du projet qui connaît la syntaxe de la ligne de commande.

**Décisions**

| Décision | Pourquoi |
|---|---|
| `parse` prend `Vec<String>` en paramètre, pas `env::args()` en direct | testable : on peut lui passer un faux `argv` dans un test unitaire sans lancer un process |
| `-h` déclenche l'aide **et arrête** (via un `Err` dédié ou un flag) | l'aide n'est pas une erreur ; mais dans les deux cas on ne scanne pas |
| méthode par défaut = `Get` | imposé par le sujet |
| une option inconnue est une erreur | mieux vaut refuser `-z` que l'ignorer et scanner de travers |

**⚠️ Pièges**

⚠️ Option à valeur en fin de liste (`… -X`) : vérifier le token suivant **avant** de le lire.
⚠️ Deux URLs positionnelles (`./vaccine a b`) : décider — on prend la première et on refuse la seconde, plus sûr que d'écraser silencieusement.
⚠️ `-X get` en minuscules : normaliser (comparer en majuscules) sinon `GET` ≠ `get`.

**🧭 Quoi utiliser, et pourquoi**

| Ce que fait l'algorithme | Où trouver comment |
|---|---|
| lire le token suivant | `args.get(i + 1)` → boîte à outils ; rend `Option`, donc on teste l'absence |
| distinguer une option d'une URL | `token.starts_with('-')` |
| normaliser la méthode | comparer `token.to_uppercase()` à `"GET"`/`"POST"` |

*Troisième temps, pour la méthode :* on compare en majuscules parce que l'utilisateur peut taper `post` ou `POST`, et que la valeur ne doit exister ensuite que sous une forme canonique — l'enum `HttpMethod`. Une fois convertie en enum, la casse d'origine n'existe plus nulle part, donc plus aucun `match` à corriger ailleurs.

**Lignes de log**

*(Pas de log en phase 1 : le parsing est synchrone et immédiat. Les logs arrivent en phase 3 avec le réseau.)*

**Prototype**

```rust
pub fn parse(args: Vec<String>) -> Result<Config, VaccineError>;
```

**Corps**

**Déroulé.** On part de trois accumulateurs : la méthode, initialisée à `Get` (le défaut du sujet) ; le fichier de sortie et l'URL, tous deux *absents* au départ. On parcourt les tokens avec un curseur `i` démarré à 1, pour sauter `args[0]`, le nom du programme. À chaque tour, on lit le token courant et on décide selon sa forme :

- `-h` ou `--help` : on s'arrête tout de suite en rendant une erreur d'usage « aide demandée » — `main` l'attrapera pour afficher l'aide et sortir avec le code 0 ;
- `-X` : on lit le token suivant ; s'il est *absent*, erreur d'usage (« -X requires a method ») ; sinon on le passe en majuscules et on le convertit en `HttpMethod` (`GET` ou `POST`, toute autre valeur étant une erreur), puis on avance le curseur de deux ;
- `-o` : même schéma, le token suivant est le nom de fichier (erreur s'il manque) ; on le range dans la sortie et on avance de deux ;
- un token qui commence par `-` sans être reconnu : option inconnue, erreur d'usage ;
- sinon, c'est l'URL positionnelle : si une URL a **déjà** été retenue, on refuse la seconde ; sinon on la garde et on avance d'un cran.

La boucle finie, si l'URL est toujours *absente*, c'est une erreur d'usage (« missing target URL »). Autrement, on assemble et on rend la `Config` : l'URL, la méthode, la sortie.


### 5.2 · `print_help`

**Ce qu'elle doit accomplir :** afficher la syntaxe et les options, exactement comme le sujet les liste. Appelée sur `-h`, et par `main` quand il attrape l'erreur `help-requested`.

**Prototype**

```rust
pub fn print_help();
```

**Corps**

**Déroulé.** Écrire la ligne d'usage (`./vaccine [-X METHOD] [-o FILE] URL`), puis une ligne par option — `-X`, `-o`, `-h` — avec sa description. Du texte littéral, aucune logique.

---

## 6. Pièges spécifiques à cette phase

- **Ordre des options libre.** `-o f -X POST URL` et `-X POST -o f URL` doivent donner la même `Config`. La boucle par curseur le gère naturellement — mais un parsing « positionnel rigide » (première option = -X, deuxième = -o) casserait dessus. Ne codez pas de position fixe.
- **`-h` prioritaire ou pas ?** Décidez : `-h` rencontré n'importe où affiche l'aide et sort, même si le reste est invalide. C'est le comportement le moins surprenant.

---

## 7. Compilation et configuration

Rien de neuf : `make` compile, `make run ARGS='...'` exécute. Pour tester le parsing sans réseau :

```bash
make run ARGS='-X POST -o /tmp/r.txt "http://localhost/?id=1"'
```

---

## 8. Tests unitaires

### 8.1 `src/cli/parse.rs` — module `#[cfg(test)]` en bas de fichier

**Ce que tu testes :**
- URL seule → `method == Get`, `output == None`.
- `-X POST URL` → `method == Post`.
- `-o f URL` → `output == Some("f")`.
- Ordre inversé des options → même résultat.
- `-X` sans valeur → `Err(Usage …)`.
- Aucune URL → `Err(Usage …)`.
- Option inconnue → `Err`.

**Ce que le test doit prouver au-delà du comportement :** que l'ordre des options est **indifférent**, et que les cas d'erreur rendent bien `Err` et non une `Config` bancale. C'est là qu'un relecteur voudrait « simplifier » en supposant un ordre fixe — le test l'en empêche.

**Stratégie.** `parse` prend un `Vec<String>`, pas `env::args()` : on lui passe donc un faux `argv` (d'où la décision § 5.1). Décrire une **fabrique** locale — une fonction qui préfixe `"./vaccine"` (le nom du programme) à une liste de `&str` et rend le `Vec<String>` attendu. C'est tout l'outillage, aucune I/O.

**Les cas à vérifier** (chacun devient un `#[test]`) :

| Entrée (après `./vaccine`) | Attendu | Pourquoi ce cas |
|---|---|---|
| `http://h/?id=1` | `method == Get`, `output == None` | le défaut, sans option |
| `-X POST http://h/` | `method == Post` | l'option à valeur est lue |
| `-o f http://h/` | `output == Some("f")` | la sortie optionnelle |
| `-X POST -o f http://h/` **vs** `-o f -X POST http://h/` | même `url`, même `output` | l'ordre des options est indifférent |
| `-X POST` (sans URL) | `Err(Usage …)` | URL obligatoire manquante |
| `-z http://h/` | `Err` | option inconnue refusée |

> **Norme des tests** (rappel du [template](../template_phases.md) § 8) : module `#[cfg(test)]` en bas de fichier, fabriques **décrites** en prose et non données, un cas unique = `#[test]`, commentaires cités en **anglais**. Chaque test part d'un état neuf — rien ne se transporte de l'un à l'autre.

### 8.3 Résultats attendus

- URL seule → défaut `Get`, pas de sortie : PASS
- ordre des options indifférent → même `Config` : PASS
- URL manquante → `Err(Usage …)` : PASS

---

## 9. Ordre de développement recommandé

1. Définir `HttpMethod` et `Config` (compilent seuls, vides).
2. Écrire `parse` pour le seul cas « URL seule », faire passer le premier test.
3. Ajouter `-X`, puis `-o`, puis `-h`, un test à chaque fois.
4. Câbler `main` : collecter `env::args()`, le passer à `cli::parse`, propager l'erreur ; afficher temporairement la `Config` obtenue pour la vérifier à l'œil.
5. Test manuel : `make run ARGS='-X POST -o /tmp/r "http://localhost/?id=1"'`.
6. Test automatisé : `make test`.

> Quand `make test` est vert et que `make run` affiche la bonne `Config` pour les trois formes d'appel, cette phase est close. On passe à la **phase 2 : URL & paramètres**.
