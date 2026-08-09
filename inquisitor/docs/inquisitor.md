# Inquisitor — Résumé du sujet

> Cybersecurity Piscine · École 42 · ARP Poisoning · Version 1.00

---

## Contexte

Le modèle OSI est l'architecture suivie par les réseaux informatiques du monde
entier. Il comporte 7 couches, chacune exposant des risques et vulnérabilités
propres.

Au niveau réseau, la **passerelle par défaut** (souvent appelée *router*) reçoit
le trafic externe et le distribue aux nœuds du réseau local. Si un nœud parvient
à **usurper l'identité de la passerelle**, il peut prendre le contrôle du trafic,
l'intercepter, décider à qui le retransmettre, et éventuellement le modifier ou
le bloquer.

Cette technique — l'**ARP spoofing** — est aussi utilisée légitimement, par
exemple pour rediriger les nouvelles connexions vers une page d'enregistrement
(portails captifs des aéroports, cafés, lieux publics).

---

## Objectif

Créer un programme `inquisitor` qui réalise une attaque par empoisonnement ARP
sur un réseau local.

Le travail se fait obligatoirement à l'intérieur d'un **conteneur ou d'une VM**,
puisque la manipulation de *raw sockets* nécessite des privilèges bas niveau.

---

## Partie obligatoire

### Environnement

Si l'implémentation utilise des conteneurs, le rendu doit inclure :
- Le `Dockerfile` **ou** le `docker-compose.yaml`.
- Un `Makefile` qui **démarre l'environnement complet sans intervention utilisateur**.

### Le programme `inquisitor`

Caractéristiques imposées :

- Développé pour la plateforme **Linux**.
- Prend au minimum **4 paramètres** :
  - `<IP-src>`
  - `<MAC-src>`
  - `<IP-target>`
  - `<MAC-target>`
- Fonctionne uniquement avec des adresses **IPv4**.
- Ne s'arrête **jamais de manière inattendue** — toutes les erreurs d'entrée
  doivent être gérées.

### Comportement attendu

Le programme doit :

- Effectuer un empoisonnement ARP **dans les deux directions** (*full duplex*).
- **Restaurer les tables ARP** lorsque l'attaque est arrêtée (CTRL+C).
- Afficher **en temps réel** les noms des fichiers échangés entre un client et
  un serveur FTP.

### Bibliothèque et langage

La bibliothèque **libpcap** peut être utilisée pour sniffer les paquets. En
conséquence, tout langage l'implémentant est autorisé (C, C++, Python, etc.).

### Tests

La démonstration se fait via le **protocole FTP**. Une suite de tests
spécifique à ce protocole est donc requise en plus des autres tests.

---

## Partie bonus

Un mode **`-v` (verbose)** qui affiche **tout le trafic FTP** — pas uniquement
les noms de fichiers — y compris le trafic généré lors du **login au serveur
FTP**.

> **Important :** le bonus n'est évalué que si la partie obligatoire est
> **parfaite** (intégralement réalisée et fonctionnant sans anomalie). Toute
> défaillance sur le mandatory annule l'évaluation du bonus.

---

## Rendu et évaluation

Le projet est rendu dans le **dépôt Git**. Seul le contenu du dépôt est évalué
en soutenance. Les noms des dossiers et fichiers doivent être vérifiés avec
attention.
