# Inquisitor — Document 2 : Ce qui est fait & problèmes à corriger

> Revue détaillée, module par module, de ce qui est réalisé, avec les
> **problèmes / points d'attention** repérés dans le code existant. Sévérité :
> 🔴 bloquant · 🟠 à corriger avant soutenance · 🟡 amélioration / robustesse.

---

## 1. Parsing & validation — `src/parsing.c` ✅

**Fait :**
- `parse_arguments` : exige exactement 4 arguments (`ac == 5`), sinon erreur
  propre.
- `is_ipv4` : split sur `.`, exige 4 octets, chaque octet 1–3 chiffres, valeur
  0–255, refuse les non-chiffres.
- `is_mac_addr` : split sur `:`, exige 6 groupes de 2 hex.
- Toutes les erreurs passent par `error()` → **pas d'arrêt inattendu** (conforme
  au sujet).

**Problèmes :**
- 🟡 Convention de retour inversée : `is_ipv4`/`is_mac_addr` renvoient `1` en cas
  d'erreur et `0` en cas de succès. C'est cohérent partout, mais contre-intuitif
  (l'inverse du booléen « est valide »). À documenter d'un commentaire au minimum.
- 🟡 Les tests `converted_value < 0` (lignes 112, 143) sont **morts** :
  `ft_parse_octet` / `ft_parse_hex_octet` ne renvoient jamais de négatif (borne
  haute 255, jamais de signe). Sans danger, mais code inutile.
- 🟡 `is_mac_addr` rejette `a:b:c:d:e:f` (exige 2 caractères pile par octet).
  C'est un choix strict acceptable, mais à connaître pour la soutenance (les MAC
  passées par le lab sont bien en `%02x`, donc OK).

---

## 2. Découverte interface locale — `discover_interface` ✅

**Fait :**
- Parcourt `getifaddrs`, ignore le loopback, prend la première IF `AF_INET`.
- Récupère l'IP (`inet_ntoa`), la MAC (`ioctl SIOCGIFHWADDR`) et l'index
  (`SIOCGIFINDEX`).
- `local_ip` / `local_mac` sont `ft_strdup` → libérés dans `free_ressources`.

**Problèmes :**
- 🟠 **Fuite mémoire sur chemin d'erreur** : entre le `ft_strdup(local_ip)` et le
  `ft_strdup(local_mac)`, si un `ioctl` échoue, on `return -1` **sans libérer
  `local_ip`** déjà alloué. `error()` appellera bien `free_ressources`, donc en
  pratique c'est rattrapé — mais la fonction seule fuit. À nettoyer.
- 🟡 Prend **la première interface non-loopback** : dans le lab Docker c'est
  `eth0`, donc OK, mais fragile si plusieurs interfaces. Acceptable pour le sujet.

---

## 3. Empoisonnement ARP — `src/poisoning.c` ✅

**Fait :**
- `build_arp_trame` : construit une trame Ethernet + ARP **reply** (opcode 2)
  correctement remplie (`htons` sur les champs multi-octets, `inet_pton` pour les
  IP, MAC parsées via `get_hex_from_mac_addr`).
- `open_inject_socket` : `socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))`.
- `send_arp_frame` : `sendto` via `sockaddr_ll` (ifindex + MAC destination).
- **Full-duplex** : dans `main`, une trame « in » et une trame « out » sont
  envoyées à chaque tour de boucle (poison des deux victimes).

**Problèmes :**
- 🟠 **`error()` reçoit `&config` où `config` est une copie locale** (passée par
  valeur à `build_arp_trame`). `free_ressources` libère alors `local_ip`/
  `local_mac`, **pointeurs partagés avec le `config` de `main`**. Comme `error()`
  fait `exit()`, il n'y a pas de double-free en pratique, mais c'est fragile : si
  un jour on retire le `exit`, on a un use-after-free. À rendre explicite.
- 🟡 `send_arp_frame` remplit `struct sockaddr_ll sockadr` **partiellement** (pas
  de `memset` initial ; `sll_protocol`, `sll_hatype`, `sll_pkttype` laissés non
  initialisés). Pour l'émission via `sendto` le noyau n'en a pas besoin, donc ça
  marche, mais un `= {0}` serait plus propre.
- 🟡 `build_arp_trame` ne fait pas de `memset(frame, 0, ...)` : tous les champs
  utiles sont bien écrits donc la trame est complète, mais dépendre de ça est
  risqué si la struct évolue.

---

## 4. Restauration ARP + signaux — `restore_arp`, `src/signals.c` ✅

**Fait :**
- `signalHandler` met `g_running = 0` (variable `volatile sig_atomic_t`), boucle
  principale s'arrête proprement — **pattern correct**.
- `restore_arp` reconstruit des trames avec les **vraies** MAC des victimes
  (`local_mac = spoof/target_mac`) pour réécrire les bonnes associations, et les
  envoie 5 fois.
- `main` : `restore_arp` → `close(fd)` → `free_ressources` → `return 0`.

**Problèmes :**
- 🟠 Le `sleep(1)` de la boucle principale et des boucles de restore rend le
  `CTRL+C` **lent à répondre** (jusqu'à 1 s pour sortir, puis 5 s de restore avec
  sleeps). Fonctionnel, mais à la soutenance ça paraît « bloqué ». Envisager un
  `nanosleep` interruptible ou un intervalle plus court.
- 🟡 Restore = 5 trames espacées d'1 s. Si la victime a un cache très frais, ça
  suffit, mais aucun contrôle que la table est réellement revenue. Acceptable.

---

## 5. Sniffing FTP — `src/sniffing.c`, `include/sniffing.h` 🔴 **NON FAIT**

**État réel :**
- `src/sniffing.c` : **100 % commenté**. Contient un brouillon `pcap_open_live` +
  `packet_handler` qui dumpe le payload brut, mais **rien n'est compilé ni
  appelé**.
- `include/sniffing.h` : **fichier vide (0 octet)**.
- **Aucune fonction de sniffing n'est déclarée, définie ou intégrée à `main`.**

**Problèmes :**
- 🔴 **Exigence mandatory non satisfaite** : « afficher en temps réel les noms des
  fichiers échangés entre un client et un serveur FTP ». Rien n'est implémenté.
- 🔴 `-lpcap` est lié dans le Makefile mais **jamais utilisé** → dette prête à
  l'emploi, mais code absent.
- 🔴 Le sniffing doit tourner **en même temps** que le poisoning. Le Makefile
  utilise déjà `-pthread` (donc un thread de capture était prévu), mais aucun
  thread n'existe. Décision d'architecture à prendre (thread vs `pcap` non-bloquant
  dans la boucle).

---

## 6. Mode verbose `-v` (bonus) ❌ **NON FAIT**

**État réel :**
- `usage()` (dans `utils.c`) mentionne `[-v]`, mais :
  - `usage()` **n'est jamais appelée**,
  - `parse_arguments` **refuse tout ce qui n'a pas exactement 4 args** → lancer
    avec `-v` donne actuellement une erreur.
- Aucune logique verbose.

**Problème :**
- 🟡 Bonus, donc non bloquant — mais à ne traiter **qu'après** un mandatory
  parfait (sniffing inclus).

---

## 7. Infrastructure (Docker / Makefile / tests) ✅

**Fait :**
- `docker-compose.yml` : lab 3 conteneurs propre (plan d'adressage documenté,
  MAC déterministes, `ip_forward=1`, `send_redirects=0`, `NET_RAW`/`NET_ADMIN`).
- `attacker/Dockerfile` : image avec `build-essential`, `libpcap-dev`, outils FTP.
- `Makefile` : build local + orchestration lab (`up/run/test/down/...`), flags
  `-Wall -Wextra -Werror`, cible `debug` avec asan/ubsan.
- Tests Python : `test_args.py`, `test_ipv4.py`, `test_mac.py`,
  `test_poisoning.py` (ARP overwrite + restore + champs de trame), `run_all.py`.

**Problèmes :**
- 🟠 **Aucun test FTP** alors que le sujet l'exige explicitement (« une suite de
  tests spécifique à ce protocole est donc requise »). À créer avec le module 5.
- 🟡 Le `README.md` décrit une arborescence `src/` (args.c, netinfo.c, arp.c,
  inject.c, sniff.c, ftp.c…) qui **ne correspond pas** aux fichiers réels
  (`parsing.c`, `poisoning.c`, `sniffing.c`…). À réaligner avant rendu pour ne pas
  induire l'évaluateur en erreur.
- 🟡 Le `README` promet un affichage `[FTP] STOR ...` / `[FTP] RETR ...` qui
  n'existe pas encore dans le code. Cohérence à rétablir une fois le module 5 fait.

---

## 8. Synthèse des corrections prioritaires

| Sévérité | Où | Correction |
|----------|-----|-----------|
| 🔴 | `sniffing.c` / `.h` | Implémenter tout le module 5 (voir Doc 3) |
| 🔴 | tests | Ajouter une suite de tests FTP |
| 🟠 | `discover_interface` | Libérer `local_ip` sur chemin d'erreur |
| 🟠 | `build_arp_trame` | Clarifier le passage de `config` à `error()` (copie vs original) |
| 🟠 | boucle / restore | Rendre le `CTRL+C` plus réactif (sleep interruptible) |
| 🟠 | `README.md` | Réaligner l'arborescence et les exemples sur le code réel |
| 🟡 | `parsing.c` | Retirer les tests morts `< 0`, documenter la convention de retour |
| 🟡 | `send_arp_frame` | `memset` du `sockaddr_ll` / de la trame |
