# 📄 Template de document de phase

> Ce fichier sert à deux choses : c'est le **squelette à copier** pour écrire un document de phase (partie A), et la **norme de rédaction** à respecter en le remplissant (parties B et C).
>
> Les règles des parties B et C ont été **payées** : chacune corrige une erreur réellement commise en écrivant les guides de ce dossier. Les passages marqués 💥 racontent l'erreur.

## Sommaire

**Partie A — Le squelette**
1. [Où on en est](#1-où-on-en-est)
2. [Architecture cible](#2-architecture-cible)
3. [Concepts à maîtriser](#3-concepts-à-maîtriser)
4. [Décomposition des étapes](#4-décomposition-des-étapes)
5. [Le corps du document : une section par fonction](#5-le-corps-du-document--une-section-par-fonction)
6. [Pièges spécifiques à cette phase](#6-pièges-spécifiques-à-cette-phase)
7. [Compilation et configuration](#7-compilation-et-configuration)
8. [Tests unitaires](#8-tests-unitaires)
9. [Ordre de développement recommandé](#9-ordre-de-développement-recommandé)

**Partie B — Les normes transverses**

10. [La langue : où le français, où l'anglais](#10-la-langue--où-le-français-où-langlais)
11. [La norme du pseudo-code](#11-la-norme-du-pseudo-code)
12. [Ce qui reste en langage cible littéral](#12-ce-qui-reste-en-langage-cible-littéral)
13. [Les blocs d'explication](#13-les-blocs-dexplication)
14. [Les schémas](#14-les-schémas)
15. [Encadrés et tableaux](#15-encadrés-et-tableaux)
16. [Liens et ancres](#16-liens-et-ancres)

**Partie C — Contrôle qualité**

17. [Vérifier un document mécaniquement](#17-vérifier-un-document-mécaniquement)
18. [Les erreurs à ne pas refaire](#18-les-erreurs-à-ne-pas-refaire)

---

## Comment se servir de ce template

Copiez la **partie A** dans un nouveau fichier, remplacez les `[crochets]`, supprimez les sections optionnelles inutilisées. Gardez ce fichier-ci ouvert à côté pour les parties B et C.

**Le document s'adresse à quelqu'un qui va taper le code lui-même**, et qui doit comprendre en le tapant. Toute la norme découle de cette tension : en dire assez pour qu'on ne bloque pas, pas assez pour qu'on puisse copier.

| Le document contient | Il ne contient pas |
|---|---|
| Une section par fonction, dans l'ordre où on l'écrira | L'architecture d'ensemble — elle est dans le README |
| Les décisions de conception et leurs raisons | Le contrat des API du socle — il est dans les sources |
| Les pièges, avec leur symptôme | Le code final |
| Du pseudo-code | Des corps de fonction dans le langage cible |

**Le préfixe `to_delete_`** marque un document d'apprentissage, destiné à disparaître quand la personne qui apprenait connaît le sujet. Ce qui doit survivre remonte dans le README.

---

# Partie A — Le squelette

# [Nom du projet] — Phase [N] : [Titre court et descriptif]

> [Une phrase de contexte : ce que fait cette phase et pourquoi elle importe.
> Éventuellement une ligne sur ce qui existe déjà et qu'on va remplacer ou étendre.]

---

## 1. Où on en est

**Fait :**
- [Résumé des phases précédentes, une ligne par phase.]

**À faire dans cette phase :**
- [Point 1 opérationnel]
- [Point 2 opérationnel]
- [Point 3 opérationnel]

**Ce qui suit (phase suivante) :**
- [Ce qui viendra après, pour donner la perspective globale.]

> **Norme** — cette section est la seule qui périme à chaque phase. Ne dupliquez pas ici ce que le README dit de l'architecture : une ligne par phase suffit, avec un lien.

---

## 2. Architecture cible

```
[Schéma ASCII ou pseudo-arborescence qui montre le flux ou la structure.
Projet de fonctions : montrer les appels entre elles.
Projet de composants : montrer leur interaction.]
```

**Point clé sur le flux :** [Une ou deux phrases sur ce qui rend l'architecture non triviale : ordre d'appel, gestion d'erreur transverse, concurrence, coût d'un aller-retour…]

> **Norme** — voir § 14 pour les trois situations où un schéma est obligatoire, et la règle des valeurs concrètes.

---

## 3. Concepts à maîtriser

De 3 à 6 concepts. Chacun suit la même structure en quatre temps.

### 3.1 [Nom du premier concept]

[Explication en 2 à 4 phrases. Aller au fond du **pourquoi**, pas seulement du comment.]

Documentation :
- [Titre du lien] : [URL]
- [Précédent dans le dépôt] : [`Fichier.cs`](chemin) — fonction `NomDeLaFonction`

> Analogie : [Une comparaison avec quelque chose de tangible et non technique.]

⚠️ **Piège classique** : [Le piège précis qu'un débutant va rencontrer, **avec son symptôme observable**, puis comment l'éviter.]

### 3.2 [Nom du deuxième concept]

[Même structure : explication → documentation → analogie → piège.]

> **Norme** — un précédent réel du dépôt vaut mieux qu'un lien externe : citez-le **par fichier et par fonction**. Et le piège doit donner le **symptôme**, pas seulement l'erreur : « le log ① tombe à chaque appel au lieu d'être rare » se reconnaît en production, « la fonction rend `false` » non.

### 3.3 Lexique

| Terme | Ce qu'il désigne | Où ça vit |
|---|---|---|
| [terme] | [définition en une ligne] | [`Fichier.cs`](chemin) |

> **Norme — obligatoire.** Tout mot employé dans le document sans être redéfini ensuite figure ici.
>
> 💥 L'erreur commise : *« journaliser la bascule sur la chaîne d'ancres »* écrit alors que rien n'avait défini « ancre ». Le lecteur a dû demander.

---

## 4. Décomposition des étapes

1. **[Étape 1]** — [Ce qui doit être produit, en une ligne.]
2. **[Étape 2]** — [Idem.]
3. **[Étape 3]** — [Idem.]

> [Note transverse si utile : ordre imposé, dépendance entre étapes.]

---

## 5. Le corps du document : une section par fonction

**Un fichier source à la fois.** À l'intérieur, les fonctions dans l'**ordre de dépendance** : ce dont les autres ont besoin d'abord. On peut ainsi coder de haut en bas sans référence à un morceau pas encore écrit.

### 5.0 · Boîte à outils

Avant les algorithmes, une section qui liste les **accès aux API exactement**, chemins littéraux compris : lire une propriété, naviguer dans une structure, lire et écrire le format de persistance, journaliser, comparer des chaînes. Plus une table « où voir ces API à l'œuvre dans le dépôt ».

**Pourquoi séparer.** Le lecteur assemble, la boîte à outils fournit les pièces. Un chemin d'API répété dans dix sections devient dix endroits à corriger, et surtout il transforme chaque algorithme en copie à recopier. Les sections d'algorithme n'y renvoient **que par lien**.

### 5.x · [NomDeLaFonction]

L'ordre suivant n'est **pas négociable au cas par cas** :

```
1. Ce qu'elle doit accomplir, et pourquoi
2. Les décisions de conception, et leurs raisons     (tableau « Décision | Pourquoi »)
3. ⚠️ Les pièges                                     (encadrés)
4. 🧭 Quoi utiliser, et pourquoi                     (voir § 13)
5. Les lignes de log — langage cible littéral
6. Prototype — langage cible exact
7. Corps — pseudo-code (voir § 11)
```

**Pourquoi cet ordre.** Qui descend directement au corps écrira ce qu'il aurait deviné de toute façon, et sautera précisément ce qui coûte des heures. Les pièges sont donc **au-dessus** du code, pas en dessous.

**Prototype et Corps restent collés**, tous les deux à la fin : le contrat juste au-dessus de l'algorithme qui le remplit.

**Les logs viennent avant le prototype**, parce que le corps y renvoie par `log ①`. On les lit comme du matériel de référence, pas comme une étape à exécuter.

⚠️ Une section qui inverse cet ordre casse l'habitude de lecture, et le lecteur cesse de faire confiance à la structure — donc lit tout, ou ne lit rien.

**Décisions**

| Décision | Pourquoi |
|---|---|
| [choix retenu] | [ce que l'alternative aurait coûté] |

**🧭 Quoi utiliser, et pourquoi**

| Ce que dit le pseudo-code | Où trouver comment |
|---|---|
| [la ligne] | [la famille d'API, la section de la boîte à outils] |

[Puis, en prose, les quatre temps du § 13 pour les lignes qui le méritent.]

**Lignes de log**

```csharp
Log.Information("[Composant] ① message, avec un {Champ} nommé", valeur);
```

**Prototype**

```csharp
internal static XmlNode? NomDeLaFonction(AutomationElement element, ITreeWalker walker);
```

**Corps**

```
[Pseudo-code selon le § 11.]
```

---

## 6. Pièges spécifiques à cette phase

- [Piège 1 : celui qui émerge de l'**interaction entre concepts**, avec son symptôme.]
- [Piège 2.]

Section distincte des pièges par concept du § 3 : ici, ce sont ceux qu'aucun concept ne porte seul.

---

## 7. Compilation et configuration

*Section optionnelle — à inclure seulement si cette phase ajoute des flags, des variables d'environnement, une dépendance ou une étape de configuration.*

```
[Commande de compilation ou de configuration]
```

- `-flag1` : [Ce qu'il fait.]
- `-flag2` : [Ce qu'il fait.]

⚠️ Signalez les réglages **qui ne se devinent pas**, et ce qu'on voit quand ils manquent. Exemple réel : sans `Compile Remove="Tests\**"`, l'erreur est un `CS0246` sur `Xunit` dans le projet du plugin — un message qui ne pointe pas vers sa cause.

---

## 8. Tests unitaires

### 8.1 [Fichier de test à créer ou à étendre]

**Ce que tu testes :**
- [Comportement 1 à vérifier.]
- [Comportement 2.]

**Ce que le test doit prouver au-delà du comportement :** [Les règles contre-intuitives qu'un relecteur voudrait « simplifier ». C'est là qu'un test gagne sa place.]

**Stratégie :** [Ce qu'il faut fabriquer pour appeler la fonction : un `XmlDocument`, un `Dictionary<…>`, un mock. Si la réponse est « un objet qui vient d'une application réelle », la fonction est **hors périmètre** — dites-le et rangez-la dans la liste du § 8.3.]

```csharp
public class [Nom]Tests
{
    // Fabriques private static en tête de classe.
    private static NodeFacts Facts(string text, string type, string className) => new(text, type, className);

    [Theory]
    //          entrée                      attendu
    [InlineData("[valeur]",                 true)]
    [InlineData("[valeur]",                 false)]   // [pourquoi ce cas existe]
    public void Fonction_ComportementAttendu_QuandCondition(string entrée, bool attendu)
    {
        Assert.Equal(attendu, /* … */);
    }
}
```

> **Norme des tests**
>
> | Élément | Convention |
> |---|---|
> | Nommage | `Fonction_ComportementAttendu_QuandCondition` |
> | Un cas unique | `[Fact]` |
> | Jeu de valeurs | `[Theory]` + `[InlineData]` |
> | Fabriques | `private static` en tête de classe |
> | Commentaires, y compris dans les `[InlineData]` | **anglais** (§ 10) |
>
> ⚠️ `[InlineData]` n'accepte que des **constantes de compilation**. Un objet, un tableau construit ou un appel de fonction imposent `[MemberData]`.
>
> ⚠️ Chaque test est **autonome** : le framework construit une instance neuve de la classe par test. Rien ne se transporte d'un test au suivant.
>
> 💥 L'erreur commise : un squelette qui réutilisait une variable d'un test à l'autre comme si elle persistait. Correction : une fabrique qui rend un tuple, et la portée expliquée.

### 8.2 Ce qui reste hors périmètre

| Fonction | Pourquoi elle n'est pas testée | Comment on la vérifie |
|---|---|---|
| [nom] | [ce qu'il faudrait fabriquer] | [session réelle, lecture des logs] |

⚠️ **Déclarez le projet de test dans la solution.** Des tests qui ne s'exécutent pas sont pires que pas de tests : ils donnent l'illusion d'une couverture.

### 8.3 Résultats attendus

- [Test 1] : PASS — [ce qui doit se passer]
- [Test 2] : PASS — [ce qui doit se passer]

---

## 9. Ordre de développement recommandé

1. [Le squelette qui compile.]
2. [La première fonctionnalité isolée et testable.]
3. [L'assemblage avec le reste.]
4. Test manuel : [commande à taper pour valider à l'œil.]
5. Test automatisé : [fichier de test à lancer.]

> Quand [critère précis de validation] est vert, cette phase est close.
> On passe à la **phase [N+1] : [titre]**.

---

# Partie B — Les normes transverses

## 10. La langue : où le français, où l'anglais

La règle ne suit pas la langue de la conversation, mais **la nature de ce qu'on écrit**.

| Élément | Langue | Pourquoi |
|---|---|---|
| Prose explicative | **français** | C'est de la documentation pour un humain |
| Pseudo-code | **anglais** | C'est la langue dans laquelle le code sera écrit |
| Commentaires *dans* le pseudo-code | **anglais** | Ils seront recopiés tels quels dans le source |
| Lignes de log citées | **anglais** | Elles sont littérales, elles vont dans le code |
| Commentaires des fichiers de test | **anglais** | C'est du code |
| Titres, tableaux | **français** | Prose |

**Le vocabulaire technique reste en anglais, même dans la prose française** : `walker`, `sibling`, `pattern`, `fallback`, `BOM`, `XmlWriter`, `record`, `playback`, `rescan`, `ConditionFactory`. Traduire n'aide personne et casse la correspondance avec le code.

⚠️ **Ne traduisez jamais *sibling* par « frère ».** Employez « **élément adjacent** », « **élément de même niveau** », ou gardez `sibling`.

💥 **L'erreur commise** : des commentaires et des messages de log rédigés en français parce que la conversation l'était — jusque dans les blocs `[InlineData]`. Il a fallu tout reprendre.

---

## 11. La norme du pseudo-code

Pseudo-code structuré **à l'anglaise**. Ni du code recopiable, ni de la prose déguisée en code.

> [!IMPORTANT]
> **Pourquoi pas le pseudo-code français en majuscules** (`FONCTION`, `TANT QUE`, `SI … ALORS`, `FIN SI`) ?
>
> Il a été essayé sur ce projet, puis abandonné : le décalage permanent avec le C# à écrire obligeait à traduire mentalement chaque ligne, et les mots-clés français n'ont pas d'équivalent direct (`FIN SI` contre l'accolade, `POUR … FAIRE` contre `foreach`). La structure de contrôle en anglais **est** celle du langage cible ; c'est une correspondance de moins à faire.

### 11.1 La syntaxe

| Élément | Convention |
|---|---|
| En-tête | `NomFonction(param : Type, …) -> TypeRetour:` |
| Contrôle | `if` · `else if` · `else` · `for each … in …` · `while` · `try` · `catch` · `return` |
| Blocs | un `:` ouvre, l'**indentation** ferme. Aucun marqueur de fin, jamais de `end` ni de `FIN` |
| Affectation | `=` |
| Type | annoté par `:` sur la **première** affectation seulement |
| Commentaires | `//`, en anglais |
| Renvoi vers une autre fonction du document | `→ F2 · LiveFacts`, aligné à droite |
| Renvoi vers une ligne de log | `log ①` |

### 11.2 Le vocabulaire des valeurs

| Écrire | Plutôt que | Signifie |
|---|---|---|
| `absent` | `null` | absence de valeur |
| `is empty` / `is set` | `.Length == 0` | chaîne vide ou non |
| `no opinion` | `null` | abstention d'un hook à trois réponses |
| `continue` / `break` | — | itération suivante / sortie de boucle |

La distinction `absent` / `is empty` n'est pas cosmétique : dans ce plugin, « voisin absent » et « voisin présent mais sans texte » suivent deux règles différentes. Le pseudo-code doit pouvoir les dire séparément.

### 11.3 Un exemple qui porte toutes les conventions

```
FindDescribedComponent(element : AutomationElement, walker : ITreeWalker,
                       windowNode : XmlNode) -> XmlNode?:

    live : Signature = signature of the live element and its neighbourhood   → F3 · FromLiveElement
    if live.Self.Text is empty:
        return absent                                            // nothing to recognise on

    candidates : XmlNodeList? = every <composant> located UNDER this window node
    if candidates is absent or candidates is empty:
        return absent

    best : XmlNode? = absent
    bestScore : int = -1
    bestIsHidden : bool = false

    for each candidate in candidates:
        if not SelfMatches(live.Self, facts of the candidate):    → F2 · DescribedFacts
            continue

        described : Signature = signature of the candidate node   → F3 · FromDescribedNode
        // ONE call, TWO answers: the verdict by return value, the count by out parameter
        if not AreNeighborhoodsSimilar(live, described, out agreements : int):
            continue

        if agreements > bestScore
           or (agreements == bestScore and bestIsHidden and the candidate is visible):
            best = candidate
            bestScore = agreements
            bestIsHidden = the candidate is not visible

    log ③
    return best
```

Relevez, dans ce bloc :

- **les types sont annotés une fois**, à la première affectation — `bestScore : int = -1`, puis `bestScore = agreements` sans répéter le type ;
- **le `?` de `XmlNode?` et de `candidates`** dit exactement où un test d'absence est obligatoire ;
- **les noms réels apparaissent** : `Signature`, `live.Self.Text`, `SelfMatches`, `AreNeighborhoodsSimilar` ;
- **les appels n'apparaissent pas** : « signature of the live element » est une *description*, avec le nom de la fonction en annotation à droite ;
- **`out agreements : int`** est écrit dans l'appel, parce que la double sortie est le point subtil de cette fonction ;
- **le commentaire signale la double sortie** en anglais, tel qu'il finira dans le source.

### 11.4 Ce que le pseudo-code montre, et ce qu'il cache

**Il montre les noms réels.** Types, champs, membres d'énumération, noms des fonctions à écrire. Les connaître est nécessaire et ne dispense de rien : savoir qu'il faut appeler `SelfMatches` n'apprend pas à l'écrire.

**Il cache les appels et les chemins d'API.** L'opération est **décrite** :

```
live : NodeFacts = facts of the live element              → F2 · LiveFacts
```

Le lecteur sait **quoi** appeler et **ce que ça doit produire**. Écrire l'appel, choisir les paramètres et implémenter restent son travail.

> [!CAUTION]
> **Une opération = une ligne.** Si un appel rend deux choses, écrivez-le sur une ligne avec un commentaire, comme le `AreNeighborhoodsSimilar` ci-dessus.
>
> 💥 C'est l'erreur la plus coûteuse commise sur ce projet : cet appel avait été coupé en deux lignes — le compteur d'un côté, le verdict de l'autre. Le lecteur a cherché une fonction qui n'existait pas.

### 11.5 Les trois dérives, et où est le curseur

💥 Trois versions ont été écrites avant celle-ci, dans cet ordre :

| Dérive | Symptôme | Verdict |
|---|---|---|
| **Trop proche du code** — `element.Properties.Name.ValueOrDefault` en clair | Le lecteur recopie au lieu de chercher | *« c'est trop proche de ce que je dois faire, je vais quasi rien faire »* |
| **Trop maigre** — « préparer trois emplacements », sans type | On ne sait pas quoi écrire | Illisible |
| **Français scolaire** — `DEBUT` / `SI … ALORS` / `FIN SI` | Décalage constant avec le code à écrire | Abandonné (§ 11) |

Le curseur est au milieu : **la structure de contrôle en anglais et les types sont donnés, les chemins d'API ne le sont pas.**

---

## 12. Ce qui reste en langage cible littéral

Trois catégories, et seulement trois.

**Les prototypes.** Ce sont des contrats — ce qui entre, ce qui sort. Se tromper de signature coûte du temps pour rien, et il n'y a rien à apprendre en la devinant. Dans un **bloc séparé**, balisé (```csharp), placé **avant** le pseudo-code de logique.

**Les lignes de log.** Ce sont des données de sortie, pas de l'algorithmique — et surtout des **repères de calibration**. Si le lecteur en écrit une variante, les instructions de vérification qui disent « cherchez `agreements=` dans les logs » ne fonctionnent plus. Elles doivent donc être identiques, donc données à coller.

**Les expressions régulières.** Ce sont des données. Leur mécanique et leurs tableaux de couverture vont dans le README ; le document donne les motifs littéralement.

⚠️ **Tout le reste est du pseudo-code.** Y compris les corps d'une ligne, y compris les constructeurs triviaux — sinon la frontière devient une affaire de jugement, et elle dérive.

---

## 13. Les blocs d'explication

Le bloc **🧭 Quoi utiliser, et pourquoi** est ce qui rend le document utilisable sans devenir une correction. Il suit quatre temps.

```
1. Ce que la ligne demande            — reformuler l'intention
2. Où lire la réponse                 — la famille d'API, la boîte à outils, le précédent du dépôt
3. Pourquoi ça marche                 — c'est ce temps qui rend la ligne évidente
4. ⚠️ Le piège, avec son symptôme
```

**Le troisième temps est celui qu'on oublie**, et c'est le plus utile. Exemple réel : *« relisez la première instruction de `CompareNeighbor` : elle sort sur `None` dès qu'un côté est absent, donc lire l'autre est du gaspillage pur »*. Une fois ça compris, la ligne s'écrit toute seule.

**Le quatrième temps doit donner le symptôme**, pas seulement l'erreur.

### Le dosage

| Nature du point | Traitement |
|---|---|
| Un chemin d'API découvrable | **Donner la famille et où chercher**, pas l'expression. Chercher `Properties.Name` une fois apprend la forme de toutes les propriétés UIA |
| Une subtilité que la doc de l'API ne signale pas | **Écrire en clair.** Un `null` inattendu, un ordre d'évaluation, une propriété qui lève au lieu de rendre un défaut. Chercher ça sans indice est du temps perdu, pas de l'apprentissage |
| Un précédent dans le dépôt | **Le citer par fichier et fonction.** `TreeDescription.AddAttribute` vaut mieux que trois paragraphes |

💥 **Deux erreurs symétriques commises.** D'abord des explications si évasives qu'elles obligeaient à demander de l'aide. Puis, en corrigeant, des explications qui donnaient la réponse.

---

## 14. Les schémas

Un schéma ASCII vaut mieux qu'un paragraphe dans trois situations, et il faut le mettre **sans attendre qu'on le demande**.

**Deux structures à mettre en correspondance** — décrit contre vivant, avant/après, attendu/obtenu. Dessinées **côte à côte**, avec les valeurs qui diffèrent visibles :

```
        DÉCRIT (XML)                          VIVANT (UIA)

  <composant id=12                      Pane
             automationID="panelid__3">         AutomationId="panelid__7"
      │                                     │
  <composant id=13                      Button
             automationID="tab_7"               AutomationId="tab_9"
             texte="Valider">                   Name="Valider"
             ▲                                  ▲
             └──── le clic a résolu ici ────────┘
```

**Un parcours qui se déroule dans le temps** — une boucle qui remonte des niveaux, un enchaînement de filtres : un **tableau tour par tour** avec l'état des variables à chaque itération.

| Tour | `node` | `element` | Ce que la fonction constate |
|---|---|---|---|
| 1 | id=13 | Button | `tab_7` ≠ `tab_9` → marque |
| 2 | id=12 | Pane | `panelid__3` ≠ `panelid__7` → marque |
| 3 | `<fenetre>` | Window | la boucle s'arrête |

**Un cas dégénéré opposé au cas normal** — redessiner le même schéma avec l'anomalie, par exemple un niveau sauté qui désaligne les deux colonnes.

⚠️ **Les valeurs doivent être concrètes et plausibles**, jamais `X` et `Y`. C'est le contraste entre `tab_7` et `tab_9` qui rend le problème visible.

`mermaid` est réservé aux enchaînements à branches. Pour une correspondance de structures, l'ASCII côte à côte est plus clair.

---

## 15. Encadrés et tableaux

| Encadré | Usage |
|---|---|
| `> [!CAUTION]` | Une erreur qui **ne lève pas** et casse le mécanisme en silence |
| `> [!WARNING]` | Une faiblesse connue et assumée, ou un point non validé |
| `> [!IMPORTANT]` | Une distinction structurante qu'on ne peut pas ignorer |
| `> [!TIP]` | Une méthode de vérification, un raccourci de raisonnement |
| `> [!NOTE]` | Un effet de bord sans conséquence, une précision |

**⚠️ en début de phrase** pour un piège en ligne, sans encadré. Réservez-le aux vrais pièges — s'il apparaît partout, il ne signale plus rien.

### Les tableaux récurrents

| Tableau | Colonnes | Quand |
|---|---|---|
| Décisions | `Décision \| Pourquoi` | Chaque fonction dont la forme aurait pu être autre |
| Quoi utiliser | `Ce que dit le pseudo-code \| Où trouver comment` | Quand plusieurs lignes demandent des API différentes |
| À lire avant | `Fichier \| Pourquoi le lire` | En tête de chaque fichier source |
| Hors périmètre | `Fonction \| Pourquoi \| Comment on vérifie` | § 8.2 |

**Les tableaux de cas à dérouler à la main** sont la meilleure façon de verrouiller une règle sans écrire de test : cinq lignes d'entrées et de verdicts attendus, que le lecteur vérifie mentalement avant de coder. Ils deviennent ensuite les `[InlineData]` des tests.

---

## 16. Liens et ancres

**📖 pour renvoyer à une source du socle**, `§n.m` pour une section du même document, lien markdown pour un autre fichier.

⚠️ **Pas d'emoji dans un titre qui sert de cible de lien.** Les générateurs d'ancres ne traitent pas les emoji de la même façon : le lien marche sur GitHub et casse ailleurs. Un titre avec emoji est acceptable **s'il n'est jamais visé**.

⚠️ **Ne renvoyez pas au numéro de section d'un document que vous ne maîtrisez pas.** « Voir `Plugins/README.md` § 7 » casse silencieusement à la première réorganisation. Citez le titre.

⚠️ **Ne renvoyez pas depuis un fichier suivi vers un fichier non suivi.** Un lien d'un README commité vers un document local pend pour quiconque clone le dépôt. Redirigez vers la **source du socle** plutôt que vers un manuel local.

💥 L'erreur commise : 21 liens de deux README vers un dossier `docs/` retiré du commit. Il a fallu les rediriger un par un vers les fichiers `.cs` réels.

**L'ancre GitHub se calcule ainsi** : minuscules, ponctuation retirée, espaces en tirets. `## 5.4 · Lire le XML` donne `#54--lire-le-xml` — notez le double tiret laissé par le `·` supprimé.

---

# Partie C — Contrôle qualité

## 17. Vérifier un document mécaniquement

Quatre contrôles attrapent l'essentiel des dérives, sans relire le document.

**Les liens et les ancres.** Extraire toutes les cibles `](#…)` et `](fichier#…)`, calculer les ancres des titres de chaque fichier cible, comparer. Attrape les renommages, les sections déplacées et les emoji dans les titres.

💥 Un fichier renommé a cassé **13 liens** répartis sur quatre documents, sans que rien ne le signale.

**Les noms de fonction, document contre code.** Extraire les noms déclarés dans le code et ceux cités dans le document, comparer dans les **deux sens**. Attrape les fonctions renommées, supprimées, ou documentées sans exister.

> [!IMPORTANT]
> **Le code est la vérité, pas le document.** Quand les deux divergent, alignez le document — sauf si le document décrit une conception que le code a mal implémentée, ce qui arrive et se voit précisément à ce moment-là.

**Les invariants de structure.** Compter les blocs `**Prototype**` et `**Corps**` : ils s'apparient, à l'exception des types de données qui ont un prototype sans corps. Vérifier qu'aucun bloc `🧭` ou `Prototype` n'apparaît **après** le `Corps` de sa propre section — contrôle **par section**, pas global, sinon chaque section est comparée à la précédente.

**Les comptes cités dans la prose.** Grep les formulations du type « les cinq familles », « les trois documents », « les 7 membres », et vérifier qu'elles tiennent encore. 💥 « Les cinq familles » est resté après l'ajout d'un sixième motif.

---

## 18. Les erreurs à ne pas refaire

| Erreur | Conséquence | La règle qui en découle |
|---|---|---|
| Un appel coupé en deux lignes de pseudo-code | Le lecteur cherche une fonction qui n'existe pas | Une opération = une ligne. Si elle rend deux choses, dites-le en commentaire |
| Une variable déclarée dans une section, utilisée dans la suivante | Illusion de portée partagée | Chaque bloc autonome redéclare le nécessaire |
| Du vocabulaire employé sans définition | Le lecteur doit demander | Lexique en § 3.3, et rien d'employé qui n'y soit |
| Des explications qui donnent la réponse | Rien n'est appris | Expliquer pourquoi, laisser chercher comment |
| Des explications évasives | Le lecteur bloque | Le troisième temps du bloc 🧭 : *pourquoi ça marche* |
| Un compte cité en dur dans la prose | Périme en silence | Le vérifier au grep, ou ne pas le citer |
| Un renommage de fichier | Liens cassés partout | Contrôle mécanique des liens après tout renommage |
| Un lien d'un fichier suivi vers un fichier local | Pend pour qui clone le dépôt | Renvoyer vers la source du socle |
| Le code corrigé sans le document | Les deux divergent | Contrôle croisé des noms, dans les deux sens |
| Des commentaires en français parce que l'échange l'était | Incohérent avec le dépôt | § 10 : la langue suit la nature, pas la conversation |
| Changer de convention en cours de route | Une passe complète sur toutes les sections | Trancher la forme **avant** d'écrire |

> [!TIP]
> Le dernier point est celui qui coûte le plus cher. Le pseudo-code de ce projet a changé de convention **quatre fois** — français scolaire, schéma anglais terse, français d'intention, puis anglais structuré. Chaque changement a demandé une passe sur une quarantaine de sections.
>
> Si vous hésitez sur une forme, convertissez **une seule section** dans chaque candidate et comparez-les côte à côte. Dix minutes contre plusieurs heures.
