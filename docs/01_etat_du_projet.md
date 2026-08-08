# Inquisitor — Document 1 : Où on en est

> État des lieux du projet `inquisitor` (ARP poisoning / MITM — Cybersecurity
> Piscine @ 42) au **08/08/2026**. Ce document situe l'avancement global ; le
> détail du « fait » et des problèmes est dans le Document 2, le plan du reste à
> faire dans le Document 3.

---

## 1. Vue d'ensemble

Le projet se découpe naturellement en **6 modules**. Voici leur statut :

| # | Module | Fichier(s) | Statut | Couverture sujet |
|---|--------|-----------|--------|------------------|
| 1 | Parsing & validation | `src/parsing.c`, `include/parsing.h` | ✅ **Fait** | 4 params, IPv4, MAC, gestion d'erreurs |
| 2 | Découverte interface locale | `src/parsing.c` (`discover_interface`) | ✅ **Fait** | IP/MAC/ifindex local |
| 3 | Empoisonnement ARP (full-duplex) | `src/poisoning.c`, `include/poisoning.h` | ✅ **Fait** | poison bidirectionnel |
| 4 | Restauration ARP (CTRL+C) | `src/poisoning.c` (`restore_arp`), `src/signals.c` | ✅ **Fait** | restore tables + exit 0 |
| 5 | **Sniffing FTP + affichage fichiers** | `src/sniffing.c`, `include/sniffing.h` | ❌ **À faire** | **cœur du mandatory manquant** |
| 6 | Mode verbose `-v` (bonus) | — | ❌ **À faire** | bonus |

**Résumé en une phrase :** toute la partie **réseau bas niveau (ARP)** est
opérationnelle et testée ; il manque la partie **capture / analyse du trafic
FTP**, qui est pourtant une **exigence obligatoire du sujet**.

---

## 2. Progression estimée

```
Mandatory  [██████████████████░░░░░░]  ~70 %
  Parsing / validation      ████████████  100 %
  Interface locale          ████████████  100 %
  Poisoning full-duplex      ████████████  100 %
  Restore (CTRL+C)           ████████████  100 %
  Sniffing FTP (filenames)   ░░░░░░░░░░░░    0 %   <-- bloquant
  Tests FTP                  ░░░░░░░░░░░░    0 %

Bonus      [░░░░░░░░░░░░░░░░░░░░░░░░]    0 %
  Verbose -v                 ░░░░░░░░░░░░    0 %
```

> Rappel du sujet : **le bonus n'est évalué que si le mandatory est parfait.**
> Tant que le module 5 (sniffing FTP) n'est pas terminé, le projet n'est **pas
> soutenable** et le bonus n'entrera pas en compte.

---

## 3. Ce qui fonctionne aujourd'hui

- Le binaire `inquisitor` **compile** et **s'exécute**.
- Lancé avec 4 arguments valides, il :
  1. valide les entrées (rejette IP/MAC malformées sans crash),
  2. découvre l'IP/MAC/index de l'interface locale,
  3. affiche un tableau de configuration,
  4. **empoisonne les deux victimes en boucle** (une trame ARP reply/sec dans
     chaque sens),
  5. sur `CTRL+C`, **restaure les tables ARP** (5 trames correctives) puis quitte
     proprement avec le code 0.
- Le **lab Docker** (3 conteneurs : server FTP / client / attacker) démarre via
  le `Makefile` sans intervention (`make up`).
- Une **suite de tests Python** valide le parsing (args, IPv4, MAC) et le
  poisoning (ARP overwrite, restore, champs de trame).

## 4. Ce qui ne fonctionne pas / manque

- **`src/sniffing.c` est intégralement commenté** — aucune capture réelle.
- **`include/sniffing.h` est vide (0 octet)**.
- **Aucun affichage des noms de fichiers FTP** (exigence mandatory).
- **Aucune suite de tests spécifique FTP** (exigée par le sujet).
- Le flag **`-v` (bonus)** n'est ni parsé ni implémenté.
- `-lpcap` est lié dans le Makefile mais **jamais utilisé** dans le code.

---

## 5. Prochaine étape immédiate

Attaquer le **module 5 (sniffing FTP)** : c'est le seul verrou qui empêche le
mandatory d'être complet. Le plan détaillé (concepts, architecture, pseudo-code,
tests) est dans le **Document 3**.
