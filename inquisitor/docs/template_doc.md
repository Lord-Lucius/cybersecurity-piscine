
# [Nom du projet] — Phase [N] : [Titre court et descriptif]

> [Une phrase de contexte : ce que fait cette phase et pourquoi elle importe.
> Éventuellement une ligne sur ce qui existe déjà et qu'on va remplacer/étendre.]

---

## 1. Où on en est

**Fait :**
- [Résumé des phases précédentes, une ligne par phase.]

**À faire dans cette phase :**
- [Point 1 opérationnel]
- [Point 2 opérationnel]
- [Point 3 opérationnel]

**Ce qui suit (phase suivante) :**
- [Ce qui viendra après pour donner la perspective globale.]

---

## 2. Architecture cible

```
[Schéma ASCII ou pseudo-arborescence qui montre le flux/structure.
Si c'est un projet avec des fonctions : montrer l'appel entre elles.
Si c'est un projet avec des composants : montrer leur interaction.]
```

**Point clé sur le flux :** [Une ou deux phrases sur ce qui rend l'architecture
non-triviale : ordre d'appel, gestion d'erreur transverse, concurrence, etc.]

---

## 3. Concepts à maîtriser

### 3.1 [Nom du premier concept]

[Explication en 2-4 phrases. Aller au fond du "pourquoi" et pas seulement du "comment".]

Documentation :
- [Titre du lien] : [URL]
- [Titre du lien] : [URL]

> Analogie : [Une comparaison avec quelque chose de tangible et non-technique.]

Piège classique : [Le piège précis qu'un débutant va rencontrer + comment l'éviter.]

### 3.2 [Nom du deuxième concept]

[Même structure : explication → docs → analogie → piège.]

### 3.3 [etc.]

[Autant de sections que nécessaire, 3 à 6 concepts en général.]

---

## 4. Décomposition des étapes

1. **[Étape 1]** — [Description en une ligne de ce qui doit être produit.]
2. **[Étape 2]** — [Idem.]
3. **[Étape 3]** — [Idem.]
4. [etc.]

> [Note transverse si utile : ordre imposé, dépendance entre étapes, etc.]

---

## 5. Pseudo-code de cadrage

> Pseudo-code uniquement pour la **logique** — pas de code compilable. Le but est
> de guider la structure algorithmique sans donner la solution.
>
> **Exception — prototypes :** les **signatures de fonctions et définitions de
> types/structs** peuvent être données dans le **langage cible du projet, en
> clair** (C, Rust, Go, Python, TypeScript, …), car elles constituent un
> *contrat d'interface* (ce qui entre / ce qui sort), pas la solution. Les placer
> dans un **bloc séparé** du pseudo-code — une sous-section « Prototypes
> (<langage>) » ou un bloc de code balisé (```c, ```rust, ```go, ```py…) — placé
> **avant** le pseudo-code de logique, qui lui reste en français.

### 5.1 [Nom du premier fichier ou module]

> Optionnel : ouvrir par un bloc « Prototypes (<langage>) » listant les
> signatures et types concernés dans le langage du projet, puis enchaîner sur le
> pseudo-code de logique en français.

```
[Pseudo-code lisible, en français, avec majuscules pour les mots-clés
(FONCTION, TANT QUE, SI, SINON, RETOURNER, etc.).
Utiliser des noms de variables réalistes.
Commentaires inline pour clarifier les décisions non évidentes.]
```

### 5.2 [Deuxième fichier ou module]

```
[Idem.]
```

### 5.3 [etc.]

---

## 6. Pièges spécifiques à cette phase

- [Piège 1 avec explication concise + éventuel lien vers la doc qui le décrit.]
- [Piège 2 idem.]
- [Piège 3 idem.]

[Cette section est distincte des pièges par concept — ici c'est les pièges qui
émergent de l'interaction entre les concepts, ou des spécificités de cette phase.]

---

## 7. Compilation / configuration

[Section optionnelle — à inclure seulement si des flags de compilation, variables
d'environnement, ou étapes de configuration sont ajoutés cette phase.]

Nouveau flag / dépendance :
```
[Commande de compilation ou de configuration]
```

- `-flag1` : [Ce qu'il fait.]
- `-flag2` : [Ce qu'il fait.]

---

## 8. Tests unitaires

### 8.1 [Nom du nouveau fichier de test, ou fichier existant à étendre]

**Ce que tu testes :**
- [Comportement 1 à vérifier.]
- [Comportement 2 à vérifier.]

**Stratégie :** [Comment tu vas capturer/mesurer le comportement.
Ex : "Capturer stdout avec subprocess.PIPE", "Utiliser un mock", "Comparer un
fichier généré avec une référence", etc.]

```python
[Pseudo-code Python de la classe de test ou de la structure de test.
Montrer le setUp/tearDown si pertinent, et un ou deux tests représentatifs.]
```

### 8.2 [Autre fichier de test si applicable]

[Idem.]

### 8.3 Résultats attendus

- [Test 1] : PASS — [Ce qui doit se passer.]
- [Test 2] : PASS — [Ce qui doit se passer.]
- [Test 3] : PASS — [Ce qui doit se passer.]

---

## 9. Ordre de développement recommandé

1. [Étape la plus atomique — écrire le squelette qui compile.]
2. [Étape suivante — ajouter la première fonctionnalité isolée et testable.]
3. [Étape suivante — assembler avec le reste.]
4. [etc.]
5. Test manuel : [commande à taper pour valider à l'œil.]
6. Test automatisé : [nom du fichier de test à lancer.]

> Quand [critère précis de validation] est vert, cette phase est close.
> On passe à la **phase [N+1] : [titre]**.
