# Vaccine

**Cybersecurity Piscine — SQL Injection Scanner**
**Version:** 1.00

---

## 📋 Description

`vaccine` est un outil de détection de vulnérabilités **SQL Injection**.

Le programme prend une URL en paramètre, exécute une série de tests d'injection SQL et détermine si l'application est vulnérable.

Lorsqu'une vulnérabilité est confirmée, `vaccine` tente d'identifier le moteur de base de données utilisé et, lorsque cela est possible, d'extraire des informations sur la structure et le contenu de la base de données.

> ⚠️ **Important :** cet outil doit uniquement être utilisé sur des applications pour lesquelles vous disposez d'une autorisation explicite de test.

---

## 🎯 Objectifs

Le projet a pour objectifs de :

* Détecter les vulnérabilités SQL Injection.
* Identifier les paramètres vulnérables.
* Identifier le moteur de base de données utilisé.
* Tester plusieurs techniques d'injection SQL.
* Récupérer le schéma de la base de données lorsque cela est possible.
* Extraire les données accessibles lorsque cela est possible.
* Archiver les résultats des tests.

---

# 🛠️ Fonctionnalités obligatoires

## 1. URL en argument

Le programme doit accepter une URL en argument :

```bash
./vaccine URL
```

Exemple :

```bash
./vaccine "http://localhost/vulnerable.php?id=1"
```

---

## 2. Méthodes HTTP

Le programme doit supporter au minimum :

* `GET`
* `POST`

La méthode `GET` est utilisée par défaut.

Exemples :

```bash
./vaccine "http://localhost/vulnerable.php?id=1"
```

```bash
./vaccine -X POST "http://localhost/login.php"
```

---

## 3. Détection des moteurs de base de données

Le programme doit être capable d'identifier au minimum **2 moteurs de base de données**.

Exemples de moteurs pouvant être supportés :

* MySQL
* SQLite
* PostgreSQL
* Oracle
* Microsoft SQL Server

### Moteurs implémentés

* [ ] MySQL
* [ ] SQLite
* [ ] PostgreSQL
* [ ] Oracle
* [ ] Microsoft SQL Server

---

## 4. Techniques d'injection SQL

Le programme doit implémenter au minimum **2 techniques** parmi les suivantes :

* Union-based
* Error-based
* Boolean-based
* Time-based
* Blind

### Techniques implémentées

* [ ] Union-based
* [ ] Error-based
* [ ] Boolean-based
* [ ] Time-based
* [ ] Blind

---

# 🔎 Informations récupérées

Lorsqu'une vulnérabilité est détectée, `vaccine` doit afficher et/ou sauvegarder autant d'informations que possible.

## Paramètres vulnérables

Le programme doit identifier :

* Le paramètre vulnérable.
* La méthode HTTP utilisée.
* Le payload ayant permis de détecter la vulnérabilité.

Exemple de résultat :

```text
[+] Vulnerable parameter : id
[+] Method               : GET
[+] Payload              : ...
```

---

## 🗄️ Informations sur la base de données

Lorsque cela est possible, le programme doit tenter de récupérer :

### Bases de données

```text
[+] Databases:
    - database_1
    - database_2
```

### Tables

```text
[+] Tables:
    - users
    - products
    - orders
```

### Colonnes

```text
[+] Columns:
    users:
        - id
        - username
        - password
```

### Données

Lorsque cela est possible, le programme doit tenter de réaliser un dump des données accessibles.

```text
[+] Dump:
    users
    --------------------------------
    id | username | password
    --------------------------------
    1  | admin    | ********
    2  | user     | ********
```

> Le dump complet n'est pas garanti et dépend notamment du type d'injection, du moteur SQL et des protections présentes sur l'application cible.

---

# ⚙️ Utilisation

## Syntaxe

```bash
./vaccine [-oX] URL
```

### Options

| Option | Description                       |
| ------ | --------------------------------- |
| `-o`   | Fichier d'archivage des résultats |
| `-X`   | Méthode HTTP utilisée             |
| `-h`   | Affiche l'aide                    |

### Méthode par défaut

La méthode HTTP utilisée par défaut est :

```text
GET
```

### Exemple GET

```bash
./vaccine "http://localhost/vulnerable.php?id=1"
```

### Exemple POST

```bash
./vaccine -X POST "http://localhost/vulnerable.php"
```

### Exemple avec fichier de sortie

```bash
./vaccine -o results.txt "http://localhost/vulnerable.php?id=1"
```

---

# 💾 Stockage des résultats

Le programme doit disposer d'un fichier de stockage permettant d'archiver automatiquement les résultats.

Si le fichier n'existe pas, il doit être créé automatiquement.

Les informations sauvegardées peuvent notamment contenir :

```text
Target URL
HTTP Method
Vulnerable Parameters
Database Engine
Payloads
Databases
Tables
Columns
Dump
Errors
```

Exemple :

```text
results/
├── target_01.txt
├── target_02.txt
└── ...
```

La structure exacte du stockage est libre.

---

# 🧪 Tests

Une série de tests doit être fournie afin de démontrer le fonctionnement de `vaccine`.

Les tests doivent être réalisés uniquement sur des environnements volontairement vulnérables.

## Environnements recommandés

* DVWA
* bWAPP
* SQLi Labs
* Applications de test développées pour le projet
* Autres laboratoires SQL Injection autorisés

---

## Tests obligatoires

### Test 1 — Détection SQL Injection

* [ ] Détection d'un paramètre vulnérable.
* [ ] Affichage du payload utilisé.
* [ ] Identification du moteur SQL.

### Test 2 — Deuxième technique d'injection

* [ ] Détection avec une seconde technique.
* [ ] Comparaison des réponses.
* [ ] Confirmation de la vulnérabilité.

### Test 3 — Extraction du schéma

* [ ] Récupération des bases de données.
* [ ] Récupération des tables.
* [ ] Récupération des colonnes.

### Test 4 — Extraction des données

* [ ] Tentative de dump.
* [ ] Sauvegarde des résultats.
* [ ] Gestion des erreurs lorsque l'extraction est impossible.

### Test 5 — Requête POST

* [ ] Envoi d'une requête POST.
* [ ] Modification d'un paramètre POST.
* [ ] Détection d'une éventuelle vulnérabilité.

---

# 🔐 Sécurité et limites

`vaccine` est un outil destiné à l'apprentissage et aux tests de sécurité autorisés.

Il ne doit **pas** être utilisé contre :

* des sites publics sans autorisation ;
* des services appartenant à des tiers ;
* des applications de production sans accord préalable ;
* des systèmes dont vous n'êtes pas propriétaire ou administrateur autorisé.

Les tests doivent être effectués dans un environnement de laboratoire ou dans le cadre d'un audit explicitement autorisé.

---

# 🚀 Bonus

Les fonctionnalités suivantes peuvent être ajoutées après avoir terminé la partie obligatoire.

## Moteurs supplémentaires

**+1 point par moteur supplémentaire**

* [ ] PostgreSQL
* [ ] Oracle
* [ ] Microsoft SQL Server
* [ ] Autre : `__________`

---

## Techniques supplémentaires

**+1 point par technique supplémentaire**

* [ ] Union-based
* [ ] Error-based
* [ ] Boolean-based
* [ ] Time-based
* [ ] Blind

---

## Modification des paramètres HTTP

**+1 point maximum**

Permettre la modification de paramètres supplémentaires tels que :

* [ ] User-Agent
* [ ] Referer
* [ ] Cookies
* [ ] Headers personnalisés
* [ ] Autres paramètres HTTP

Exemple :

```bash
./vaccine -H "User-Agent: custom-agent" URL
```

---

# 📁 Structure du projet

La structure minimale attendue est :

```text
.
├── vaccine
├── Makefile
├── README.md
├── tests/
│   ├── ...
│   └── ...
└── ...
```

Si le projet est développé dans un langage ne nécessitant pas de compilation, le `Makefile` peut être adapté ou supprimé selon les exigences du sujet.

---

# 🔧 Contraintes techniques

* Le langage de programmation est libre.
* Les bibliothèques automatisant les injections SQL sont interdites.
* Les mécanismes de détection et d'injection doivent être développés par le projet.
* Si le langage nécessite une compilation, un `Makefile` doit être fourni.
* Les fichiers nécessaires au fonctionnement du programme doivent être créés automatiquement lorsque cela est nécessaire.
* Une série de tests doit être fournie.
* Le projet doit être entièrement reproductible à partir du dépôt.

---

# 📦 Installation

## Prérequis

À compléter selon le langage utilisé :

```text
Langage :
Version :
Dépendances :
```

## Compilation

Si nécessaire :

```bash
make
```

## Nettoyage

```bash
make clean
```

---

# ▶️ Exemple de lancement

```bash
make
./vaccine "http://localhost/vulnerable.php?id=1"
```

Exemple avec POST :

```bash
./vaccine -X POST "http://localhost/vulnerable.php"
```

Exemple avec sauvegarde :

```bash
./vaccine -o results.txt "http://localhost/vulnerable.php?id=1"
```

---

# 📊 Exemple de sortie

```text
========================================
             VACCINE v1.00
        SQL Injection Scanner
========================================

[*] Target  : http://localhost/vulnerable.php?id=1
[*] Method  : GET

[*] Testing parameters...

[+] Parameter found : id

[*] Testing SQL Injection techniques...

[+] Boolean-based injection : possible
[+] Error-based injection   : possible

[*] Detecting database engine...

[+] Database engine : MySQL

[*] Enumerating database...

[+] Databases found:
    - example_db

[*] Enumerating tables...

[+] Tables found:
    - users
    - products

[*] Enumerating columns...

[+] users:
    - id
    - username
    - password

[*] Saving results...

[+] Results saved.

========================================
              SCAN COMPLETE
========================================
```

---

# 👥 Équipe

À compléter :

| Nom     | Rôle          |
| ------- | ------------- |
| `Nom 1` | Développement |
| `Nom 2` | Développement |
| `Nom 3` | Tests         |
| `Nom 4` | Documentation |

---

# 📌 État du projet

| Fonctionnalité           | État |
| ------------------------ | ---- |
| URL en argument          | ⬜    |
| GET                      | ⬜    |
| POST                     | ⬜    |
| Détection SQL Injection  | ⬜    |
| Moteur SQL n°1           | ⬜    |
| Moteur SQL n°2           | ⬜    |
| Technique n°1            | ⬜    |
| Technique n°2            | ⬜    |
| Détection des paramètres | ⬜    |
| Extraction des bases     | ⬜    |
| Extraction des tables    | ⬜    |
| Extraction des colonnes  | ⬜    |
| Dump des données         | ⬜    |
| Sauvegarde des résultats | ⬜    |
| Tests                    | ⬜    |
| Bonus                    | ⬜    |

---

# 📜 Licence

Projet réalisé dans le cadre de la **Cybersecurity Piscine**.

Utilisation exclusivement dans un cadre légal et autorisé.
