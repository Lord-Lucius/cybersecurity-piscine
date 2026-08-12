# Inquisitor — Document 3 : Plan du reste à faire (par module)

> Plan opérationnel de tout ce qui reste, découpé en **modules autonomes**.
> Chacun donne : objectif, concepts, décisions, pseudo-code (à l'anglaise) et
> pièges avec leur symptôme. Rédigé selon `template_doc.md` — prose en français,
> pseudo-code et logs en anglais (§ 10 du template).
>
> **Ordre imposé :** A → B → C, puis E (bonus) uniquement si le mandatory est
> parfait. (D — cohérence README — est **fait**, voir § 6.)

---

## 1. Où on en est

**Fait (voir Doc 1 & 2) :**
- Parsing, découverte de l'interface locale, poisoning ARP full-duplex, restore
  sur CTRL+C : opérationnels et testés (suite Python ARP verte).
- **Module A (sniffing FTP) implémenté au 11/08.** `include/sniffing.h` réconcilié
  (prototypes par **pointeur**, `ftp_handler` conforme à `pcap_loop`). `t_config` a
  gagné `iface` (typé **`char[IFNAMSIZ]`**, un buffer — pas `char*`) et `verbose`.
  `discover_interface` **stocke désormais** le nom de l'interface
  (`ft_strlcpy(config->iface, ifa_name, IFNAMSIZ)`). `src/sniffing.c` est **complet
  et compile** (`-Wall -Wextra -Werror`) : `start_sniffer` (open + datalink +
  filtre + thread), `capture_loop`, `ftp_handler` (descente Eth→IP→TCP, parse
  `STOR`/`RETR`, `printf %.*s`) et `stop_sniffer` (`breakloop → join → close`).
  Le sniffer est **intégré à `main`** (démarré avant la boucle de poison, arrêté
  avant `restore_arp`), et `setvbuf(stdout, _IOLBF)` garantit la visibilité en pipe.

**À faire dans cette phase :**
- Module A : **valider en conditions réelles** dans le lab (`make up` / `make run`
  + `put`/`get` → `[FTP] STOR/RETR` en direct, CTRL+C propre). Compilation et revue
  statique sont vertes ; il ne reste que la **preuve fonctionnelle**.
- Module B : une suite de tests FTP (`tests/test_ftp.py`).
- Module C : corrections de robustesse (mémoire, CTRL+C, structs).

**Ce qui suit :**
- Module E (bonus `-v`) : n'entre en jeu que si A→C sont parfaits.

> **Norme** — cette section périme à chaque avancée. L'architecture d'ensemble est
> dans le README ; le détail du « fait » est dans le Doc 2.

### Vue d'ensemble des modules

| Module | Titre | Sévérité | Bloque le rendu ? |
|--------|-------|----------|-------------------|
| **A** | Sniffing FTP + affichage des fichiers | 🟢 | *Implémenté au 11/08 — reste la **validation live*** |
| **B** | Tests FTP | 🔴 | **Oui** (exigés par le sujet) |
| **C** | Robustesse (mémoire, CTRL+C, trames) | 🟠 | Non, mais attendu en soutenance |
| ~~**D**~~ | ~~Cohérence documentaire (README)~~ | ✅ | **Fait** |
| **E** | Bonus verbose `-v` | 🟢 | Non (bonus) |

---

## 2. Architecture cible

Le sniffer vit **à côté** de la boucle de poison, dans un thread dédié, parce que
`pcap_loop` bloque. `main` orchestre les deux et garantit l'**ordre d'arrêt**.

```
main()
 ├─ setup_signals()                          [fait]
 ├─ parse_arguments()          (+ flag -v, module E)
 ├─ discover_interface()       (+ stocke iface, module A.1)
 ├─ build_arp_trame() x2 (in/out)            [fait]
 ├─ open_inject_socket()                     [fait]
 ├─ start_sniffer(&s, &config) ──▶ pthread_create ──▶ capture_loop (thread)
 │                                                       └─ pcap_loop → ftp_handler(paquet)
 ├─ while (g_running) { send in ; send out ; sleep }     [fait]   ║ tourne en parallèle
 ├─ stop_sniffer(&s)   =  pcap_breakloop → pthread_join → pcap_close
 ├─ restore_arp()                            [fait]
 └─ close(fd) ; free_ressources()            [fait]
```

**Point clé sur le flux :** l'arrêt est le seul endroit non trivial. `main` (thread
principal) et `capture_loop` (thread sniffer) partagent le `pcap_t*`. Fermer le
handle pendant que le thread est encore dans `pcap_loop` provoque un
use-after-free. L'ordre `breakloop → join → close` est donc **impératif** et doit
s'exécuter **avant** `restore_arp` (voir § 5).

---

## 3. Concepts à maîtriser

### 3.1 libpcap — le cycle de vie complet

Capturer, ce n'est pas juste « ouvrir et lire ». C'est une **chaîne d'appels
ordonnée** dont chaque maillon peut échouer et doit être testé : ouvrir →
vérifier le datalink → compiler le filtre → l'appliquer → boucler → arrêter →
fermer. Le **pourquoi** de l'ordre : on ne peut pas filtrer un handle pas ouvert,
ni fermer un handle qu'un thread lit encore.

Documentation :
- « Programming with pcap » (le tuto fondateur, à lire en entier) : https://www.tcpdump.org/pcap.html
- pcap(3) puis le man de **chaque** fonction appelée : https://www.tcpdump.org/manpages/pcap.3pcap.html
- Précédent dans le dépôt : aucun — c'est la première fois qu'on appelle `pcap_*`.

> Analogie : un micro qu'on branche sur le câble. On l'installe, on règle la
> fréquence (le filtre), on écoute, puis on **débranche dans l'ordre** avant de
> ranger — sinon on arrache le fil.

⚠️ **Piège classique** : oublier `promisc=1`. **Symptôme** : le MITM marche (les
tables ARP sont bien empoisonnées, `restore` fonctionne) mais **l'affichage FTP
reste vide**, parce que la carte ne remonte que le trafic qui lui est adressé, pas
le trafic relayé. On croit à un bug de parsing alors que rien n'est capturé.

### 3.2 Descente des couches par offsets calculés (Eth → IP → TCP → payload)

Le payload FTP n'est pas à une position fixe. On « descend » les couches en
ajoutant des offsets **calculés à partir des champs de longueur**, jamais en dur.
Le **pourquoi** : IP et TCP ont des champs d'options de taille variable, donc
`+14+20+20` ne tombe juste que sur les paquets sans options.

Documentation :
- Les structs, directement dans les headers système (plus fiable que tout tuto) :
  `/usr/include/net/ethernet.h`, `/usr/include/netinet/ip.h`,
  `/usr/include/netinet/tcp.h`.

> Analogie : un train dont chaque wagon annonce sa propre longueur sur sa porte.
> Pour atteindre le wagon 4, on lit les longueurs 1, 2, 3 — on ne suppose pas que
> tous les wagons font la même taille.

⚠️ **Piège classique** : lire `ip_hl` ou `doff` comme un nombre d'octets. Ce sont
des **mots de 4 octets** : la longueur réelle est `× 4`. **Symptôme** : sur un
paquet à options TCP, le payload est lu 12 octets trop tôt, et `[FTP] STOR …`
affiche des caractères parasites collés devant le nom, de façon **intermittente**
(seulement quand il y a des options) — le pire genre de bug à diagnostiquer.

### 3.3 Payload non terminé par `\0`

Un buffer réseau n'est **pas** une chaîne C. Il n'y a pas de `\0` final : sa fin
est donnée par une **longueur** (`caplen − entêtes`). Le **pourquoi** : `strlen`,
`strstr`, `printf("%s")` liront au-delà du paquet, dans la mémoire suivante.

> Analogie : une découpe dans un rouleau de papier continu. On sait où couper
> parce qu'on a mesuré, pas parce qu'il y a un bord.

⚠️ **Piège classique** : `printf("[FTP] %s\n", payload)`. **Symptôme** : la ligne
attendue s'affiche **suivie d'octets aléatoires** (ou l'ASan crie
`heap-buffer-overflow READ`). Il faut itérer avec la longueur et borner chaque
`printf` à `%.*s`.

### 3.4 Protocole FTP de contrôle (RFC 959, port 21, texte clair)

Le canal de contrôle transporte des **commandes texte, une par ligne**, terminées
par `\r\n`. Le mandatory ne cible que `STOR <fichier>` (upload) et `RETR
<fichier>` (download). Le **pourquoi** de la tolérance nécessaire : un segment TCP
peut porter **0, 1 ou plusieurs lignes**, ou une ligne coupée en deux segments.
La casse est indifférente (`stor`, `STOR`).

Documentation :
- RFC 959, § 4 (commandes) & § 5 : https://www.rfc-editor.org/rfc/rfc959

> Analogie : un télégraphe. Chaque message finit par un « STOP » (`\r\n`) ;
> plusieurs messages peuvent arriver dans la même dépêche.

⚠️ **Piège classique** : traiter un segment = une ligne. **Symptôme** : un client
qui envoie `USER x\r\nPASS y\r\n` en un seul segment ne fait afficher **que la
première** ligne (bonus `-v`), et un `STOR` collé à une réponse serveur est raté.
Itérer sur les `\n` avec `memchr`.

### 3.5 Concurrence sniffer ↔ poison (pthreads)

`pcap_loop` bloque, donc il tourne dans un **thread dédié** pendant que `main`
empoisonne. Le **pourquoi** de l'ordre d'arrêt : deux threads partagent le
`pcap_t*` ; il faut d'abord faire **sortir** le thread de `pcap_loop`
(`breakloop`), **attendre** qu'il ait fini (`join`), et **seulement après**
libérer le handle (`close`).

Documentation :
- `man 3 pthread_create`, `man 3 pthread_join`, `pcap_breakloop`(3).

> Analogie : on ne débranche pas la platine (`close`) pendant que le bras de
> lecture y est encore posé (`pcap_loop`) ; on relève le bras d'abord.

⚠️ **Piège classique** : `pcap_close` avant `pthread_join`. **Symptôme** : au
CTRL+C, crash intermittent (`use-after-free` sous ASan, ou SIGSEGV nu), et le test
`test_exit_code_zero_after_sigint` passe du vert au rouge **de façon aléatoire**
selon le timing des deux threads.

### 3.6 Lexique

> **Norme — obligatoire.** Tout terme employé dans le document sans être redéfini
> figure ici.

| Terme | Ce qu'il désigne | Où ça vit |
|---|---|---|
| `handle` | le `pcap_t*` rendu par `pcap_open_live`, jeton de toute opération pcap | `t_sniffer.handle` |
| datalink | type de couche liaison d'une interface (`DLT_EN10MB` = Ethernet) | `pcap_datalink()` |
| filtre BPF | programme qui laisse passer certains paquets (`"tcp port 21"`) | `pcap_compile/setfilter` |
| promiscuous | mode où la carte remonte **tout** le trafic vu, pas seulement le sien | `pcap_open_live` arg 3 |
| `snaplen` | nombre max d'octets capturés par paquet (on prend 65535) | `pcap_open_live` arg 2 |
| `caplen` | octets **réellement capturés** de ce paquet (borne sûre de lecture) | `pcap_pkthdr.caplen` |
| `len` | taille du paquet **sur le fil** (peut dépasser `caplen`) | `pcap_pkthdr.len` |
| payload | les octets FTP après les entêtes Eth+IP+TCP | calculé dans `ftp_handler` |
| entêtes | `14 + ip_len + tcp_len`, l'offset du payload | calculé dans `ftp_handler` |
| `ip_hl` / `doff` | longueur d'entête IP / TCP **en mots de 4 octets** (× 4 = octets) | `struct ip` / `struct tcphdr` |

---

## 4. Module A — le corps, une section par fonction 🟢 **IMPLÉMENTÉ (11/08)**

> **Objectif sujet** : « afficher en temps réel les noms des fichiers échangés
> entre un client et un serveur FTP ». Seul verrou du mandatory.

> **État au 11/08 :** ce module est **écrit, compile et intégré à `main`** ; les
> tests de parsing restent verts. Cette section garde toute sa valeur de
> **référence / checklist** (elle a servi à le construire). **Deux écarts assumés**
> vs. le guide ci-dessous, tous deux sans danger :
> - `iface` a été fait en **buffer `char[IFNAMSIZ]`** (au lieu de `char*` +
>   `ft_strdup`) → rempli par `ft_strlcpy`, **aucun `free` à prévoir**.
> - `start_sniffer` garde **`error()`** (au lieu de `return -1`) — OK ici car il
>   s'exécute **avant** tout empoisonnement, donc un échec ne laisse aucune victime
>   empoisonnée.
>
> Reste : la **validation live** en lab (voir § 4.6 et test manuel du README).

**Ordre de dépendance** (on code de haut en bas) : la boîte à outils, puis
`discover_interface` (delta), `start_sniffer`, `capture_loop`, `ftp_handler`,
`stop_sniffer`, et enfin l'intégration dans `main`.

### 4.0 · Boîte à outils

Les accès API exacts, à écrire une fois et à référencer par lien depuis les
sections d'algorithme.

| Opération | Appel exact | Notes |
|---|---|---|
| Ouvrir la capture | `pcap_open_live(iface, 65535, 1, 1000, errbuf)` | `iface` = **`char*`** ; rend `NULL` + `errbuf` si échec |
| Vérifier Ethernet | `pcap_datalink(handle) == DLT_EN10MB` | sinon les offsets sont faux |
| Compiler le filtre | `pcap_compile(handle, &fp, "tcp port 21", 1, PCAP_NETMASK_UNKNOWN)` | rend `-1` → `pcap_geterr(handle)` |
| Appliquer le filtre | `pcap_setfilter(handle, &fp)` | rend `-1` → `pcap_geterr(handle)` |
| Libérer le bytecode | `pcap_freecode(&fp)` | **après** setfilter, succès **comme** échec |
| Boucler | `pcap_loop(handle, -1, ftp_handler, user)` | `-1` = infini ; `user` passé tel quel |
| Arrêter la boucle | `pcap_breakloop(handle)` | fait sortir `pcap_loop` |
| Fermer | `pcap_close(handle)` | **après** `pthread_join` uniquement |
| Lancer le thread | `pthread_create(&s->thread, NULL, capture_loop, s)` | rend `0` si ok |
| Attendre le thread | `pthread_join(s->thread, NULL)` | avant `pcap_close` |
| Chercher un `\n` | `memchr(base, '\n', reste)` | rend `absent` si aucun |
| Comparer préfixe insensible casse | `strncasecmp(ligne, "STOR ", 5)` | `0` = égal |

**Structs système** (champs réels, headers à inclure) :

| Struct | Header | Champs utilisés |
|---|---|---|
| `struct ether_header` | `<net/ethernet.h>` | `ether_type` (tester `ntohs(...) == ETHERTYPE_IP`) |
| `struct ip` | `<netinet/ip.h>` | `ip_hl` (× 4 = octets), `ip_p` (tester `== IPPROTO_TCP`) |
| `struct tcphdr` | `<netinet/tcp.h>` | `doff` (× 4 = octets) — nom Linux, **pas** `th_off` |

> [!CAUTION]
> **`struct tcphdr` : utiliser `doff`, pas `th_off`.** Les noms BSD (`th_off`,
> `th_*`) n'existent que si `__FAVOR_BSD` est défini. Sur la glibc par défaut,
> seul `doff` compile. Idem `struct ip` (`ip_hl`) vs `struct iphdr` (`ihl`) :
> choisir `struct ip`/`ip_hl` et inclure `<netinet/ip.h>`.

### 4.1 · discover_interface (delta : stocker l'interface) ✅ fait

**Ce qu'elle doit accomplir.** Elle découvre déjà IP/MAC/index locaux. Il manquait
une chose : **conserver le nom** de l'interface (`tmp->ifa_name`) pour que
`start_sniffer` sache quoi ouvrir. **C'est fait** (`config->iface` rempli dans le
bloc `AF_INET`).

**Décisions** *(choix retenu : le buffer)*

| Décision | Pourquoi |
|---|---|
| `iface` en **buffer `char[IFNAMSIZ]`** (choix retenu) | un nom d'IF est court et borné (`IFNAMSIZ` = 16) → pas de `malloc`, donc **pas de `free`** à gérer. `#include <net/if.h>` pour `IFNAMSIZ` |
| *(alternative)* `char *iface` + `ft_strdup` | valable aussi, cohérent avec `local_ip`/`local_mac`, mais impose un `free(config->iface)` dans `free_ressources` |
| `ft_strlcpy(config->iface, ifa_name, IFNAMSIZ)` | `interface` est libéré par `freeifaddrs` avant le retour — on **copie** le nom, pas un pointeur (qui deviendrait dangling) |

**🧭 Quoi utiliser, et pourquoi**

| Ce que dit le pseudo-code | Où trouver comment |
|---|---|
| `remember the interface name` | `ft_strdup(tmp->ifa_name)` ; le libérer dans `free_ressources` |

**Prototype** — inchangé, seul le corps gagne une ligne :

```c
int discover_interface(t_config *config);   /* + config->iface = ft_strdup(ifa_name) */
```

**Corps** (delta seulement, **fait**) :

```
// inside the AF_INET branch, right after local_ip is set:
copy the interface name into config.iface     // ft_strlcpy, survives freeifaddrs → §4.0
```

> Avec le buffer `char[IFNAMSIZ]` retenu, **rien à libérer**. (Si tu repasses un
> jour à `char*` + `ft_strdup`, pense au `free(config->iface)` dans
> `free_ressources`.)

### 4.2 · start_sniffer

**Ce qu'elle doit accomplir.** Ouvrir la capture sur l'interface, vérifier que
c'est de l'Ethernet, installer le filtre `tcp port 21`, puis lancer `capture_loop`
dans un thread. Elle rend `0` si tout est prêt, `-1` sinon.

**Décisions**

| Décision | Pourquoi |
|---|---|
| Rendre `-1` plutôt qu'appeler `error()` | `error()` fait `exit()` **sans** restaurer les tables ARP → victimes laissées empoisonnées. Le sniffer doit rendre la main à `main`, qui décidera (voir piège § 5) |
| Travailler sur une variable locale `handle`, l'affecter à `s->handle` **en dernier** | tant que l'ouverture n'a pas réussi, `s->handle` doit rester `NULL` pour que `stop_sniffer` sache qu'il n'y a rien à fermer |
| `pcap_freecode(&fp)` sur **tous** les chemins après compile | le bytecode est inutile dès qu'il est posé ; l'oublier fuit à chaque lancement |

⚠️ **Piège** : appeler `pcap_datalink`/`pcap_compile` sur `s->handle` alors que
`s->handle` n'est pas encore affecté (il vaut encore `NULL`/garbage). **Symptôme** :
`pcap_datalink(NULL)` → segfault immédiat au démarrage. Utiliser la locale
`handle` partout, n'écrire `s->handle = handle` qu'à la toute fin.

**🧭 Quoi utiliser, et pourquoi**

| Ce que dit le pseudo-code | Où trouver comment |
|---|---|
| `open ... promiscuous` | `pcap_open_live`, → §4.0. Le `1` en 3ᵉ arg **est** le promiscuous — le crux du concept § 3.1 |
| `if link type is not Ethernet` | `pcap_datalink(handle) != DLT_EN10MB`, → §4.0 |
| `compile / install / free filter` | trio `pcap_compile` / `pcap_setfilter` / `pcap_freecode`, → §4.0 |
| `start the capture thread` | `pthread_create(..., capture_loop, s)`, → §4.0 |

**Lignes de log** (littérales — chemins d'erreur) :

```c
fprintf(stderr, "start_sniffer: %s\n", errbuf);            // ① open failed
fprintf(stderr, "start_sniffer: non-Ethernet interface\n"); // ② wrong datalink
fprintf(stderr, "start_sniffer: %s\n", pcap_geterr(handle)); // ③ compile/filter failed
```

**Prototype**

```c
int start_sniffer(t_sniffer *s, t_config *config);
```

**Corps**

```
start_sniffer(s : t_sniffer*, config : t_config*) -> int:            // 0 ok, -1 error
    errbuf : char[PCAP_ERRBUF_SIZE]
    s.verbose = config.verbose
    s.handle  = absent                                    // nothing to close yet

    handle : pcap_t* = open config.iface, promiscuous, snaplen 65535, 1000 ms   → §4.0
    if handle is absent:
        return -1                                                       log ①

    if link-layer type of handle is not Ethernet:                       → §4.0
        close handle
        return -1                                                       log ②

    fp : bpf_program
    if compiling "tcp port 21" into fp fails:                           → §4.0
        close handle
        return -1                                                       log ③
    if installing fp on handle fails:                                   → §4.0
        free fp ; close handle
        return -1                                                       log ③
    free fp                                                 // done, whether or not it succeeded

    s.handle = handle                                      // NOW it is safe to publish
    if creating the capture thread fails:                               → §4.0
        close handle ; s.handle = absent
        return -1

    return 0
```

### 4.3 · capture_loop

**Ce qu'elle doit accomplir.** C'est le corps du thread. Elle ne fait qu'une
chose : lancer `pcap_loop`, qui bloque jusqu'à ce que `stop_sniffer` appelle
`breakloop`. Sa signature suit `void *(*)(void *)` (contrat de `pthread_create`).

**Prototype**

```c
void *capture_loop(void *arg);   /* thread entry: (t_sniffer *) */
```

**Corps**

```
capture_loop(arg : void*) -> void*:
    s : t_sniffer* = arg
    loop over packets of s.handle forever, calling ftp_handler with s   → §4.0
    return absent
```

### 4.4 · ftp_handler

**Ce qu'elle doit accomplir.** Appelée **une fois par paquet** par `pcap_loop`.
Elle descend Eth → IP → TCP par offsets calculés, isole le payload, le découpe en
lignes, et pour chaque `STOR`/`RETR` affiche le nom de fichier. La signature est
**imposée** par `pcap_loop`.

**Décisions**

| Décision | Pourquoi |
|---|---|
| Borner sur `header.caplen`, pas `header.len` | `len` est la taille sur le fil ; lire jusque-là sort du buffer si le paquet a été tronqué au `snaplen` |
| Un garde-fou de taille **avant chaque** déréférencement | un paquet trop court déréférencerait au-delà du buffer → crash ou lecture de mémoire voisine |
| Itérer avec `memchr('\n')`, jamais `strstr`/`%s` nu | le payload n'a pas de `\0` (concept § 3.3) |
| `strncasecmp` pour le préfixe | la casse FTP est indifférente (concept § 3.4) |

⚠️ **Pièges** (chacun avec son symptôme) :

- `ip_hl * 4` / `doff * 4` oubliés → payload décalé, **noms préfixés de parasites**
  seulement sur les paquets à options (§ 3.2).
- `printf("%s", payload)` → **octets aléatoires** après le nom, ou ASan
  `heap-buffer-overflow` (§ 3.3). Utiliser `printf("%.*s", (int)n, ligne)`.
- Ne pas re-tester `len <= entetes` → un **ACK pur** (payload vide) fait lire une
  ligne fantôme.

**🧭 Quoi utiliser, et pourquoi**

| Ce que dit le pseudo-code | Où trouver comment |
|---|---|
| `ether_type is IPv4` | `ntohs(eth.ether_type) == ETHERTYPE_IP`, → §4.0. **`ntohs`, pas `stohs`** (voir § 5) |
| `ip header length in bytes` | `ip.ip_hl * 4`, → §4.0 |
| `tcp header length in bytes` | `tcp.doff * 4`, → §4.0 |
| `next line` | `memchr(cur, '\n', end - cur)`, → §4.0 |
| `line starts with STOR/RETR` | `strncasecmp(line, "STOR ", 5) == 0`, → §4.0 |

**Lignes de log** (littérales — ce que les tests du Module B grep) :

```c
printf("[FTP] STOR %.*s\n", (int)name_len, name);   // mandatory
printf("[FTP] RETR %.*s\n", (int)name_len, name);   // mandatory
printf("[FTP] %.*s\n",      (int)line_len, line);    // verbose -v (module E)
fflush(stdout);                                       // stdout is a PIPE in tests → flush
```

> [!IMPORTANT]
> **`fflush(stdout)` après chaque ligne.** En test (Module B), `inquisitor` tourne
> avec `stdout` redirigé vers un `PIPE` : la libc bufferise alors par blocs, pas
> par lignes, et le test qui lit « en flux » ne voit **jamais** la ligne → échoue
> sur un timeout alors que le code est correct.

**Prototype** (imposé par `pcap_loop`) :

```c
void ftp_handler(u_char *user,
                 const struct pcap_pkthdr *header,
                 const u_char *packet);
```

**Corps**

```
ftp_handler(user : u_char*, header : pcap_pkthdr*, packet : u_char*):
    s   : t_sniffer* = user
    len : uint       = header.caplen                  // SAFE bound (really captured)

    // (1) size guards BEFORE any dereference
    if len < 14 + 20 + 20:                            // Eth + min IP + min TCP
        return

    eth : ether_header* = packet
    if ether_type of eth is not IPv4:                                   → §4.0
        return

    ip     : ip* = packet + 14
    ip_len : uint = ip.ip_hl * 4                       // words of 4 bytes
    if ip_len < 20 or ip.ip_p is not TCP:                               → §4.0
        return

    tcp     : tcphdr* = packet + 14 + ip_len
    tcp_len : uint = tcp.doff * 4                       // words of 4 bytes (Linux name)
    if tcp_len < 20:
        return

    headers : uint = 14 + ip_len + tcp_len
    if len <= headers:                                 // pure ACK, no payload
        return
    payload     : u_char* = packet + headers
    payload_len : uint    = len - headers

    // (2) payload is NOT NUL-terminated: walk it by length
    cur : u_char* = payload
    end : u_char* = payload + payload_len
    while cur < end:
        nl : u_char* = next '\n' in [cur, end)                          → §4.0
        line_end : u_char* = nl if nl is set else end
        line_len : uint = line_end - cur
        if line_len > 0 and byte before line_end is '\r':
            line_len = line_len - 1                     // drop trailing CR
        if line_len == 0:
            cur = line_end + 1 ; continue
        if s.verbose:                                   // module E: everything
            print "[FTP] " + line[0..line_len]                          log verbose
        else if line[0..line_len] starts with "STOR " or "RETR " (case-insensitive):
            cmd  : text = first 4 letters, uppercased
            name : slice = line after the first space, up to line_len
            print "[FTP] " + cmd + " " + name                           log STOR/RETR
        if nl is absent:
            break                                       // last (maybe partial) line
        cur = nl + 1
```

### 4.5 · stop_sniffer

**Ce qu'elle doit accomplir.** Arrêter le thread et libérer le handle, dans
l'**ordre impératif** `breakloop → join → close`. Idempotente : si `s->handle`
est déjà `absent`, elle ne fait rien.

**Décisions**

| Décision | Pourquoi |
|---|---|
| Garder l'ordre `breakloop → join → close` | fermer un handle qu'un thread lit encore = use-after-free (§ 3.5) |
| Mettre `s->handle = absent` après `close` | rend un double appel sans danger |

✅ **Désormais implémentée** (`breakloop → join → close` + `s->handle = NULL`, avec
garde si `s->handle` est déjà `NULL`). Rappel du pourquoi : sans elle, le CTRL+C
laisserait le thread dans `pcap_loop` et le process ne se terminerait pas proprement
→ `test_exit_code_zero_after_sigint` casserait. À confirmer par le test live.

**Prototype**

```c
void stop_sniffer(t_sniffer *s);
```

**Corps**

```
stop_sniffer(s : t_sniffer*):
    if s.handle is absent:
        return
    break the capture loop of s.handle          // 1. makes pcap_loop return   → §4.0
    join s.thread                                // 2. wait for the thread       → §4.0
    close s.handle                               // 3. release the handle        → §4.0
    s.handle = absent
```

### 4.6 · Intégration dans `main` (ordre exact)

```
setup_signals() ; parse_arguments() ; print_config()
discover_interface()                    // now fills config.iface
build_arp_trame() x2 ; fd = open_inject_socket()

s : t_sniffer = {0}
if start_sniffer(&s, &config) != 0:     // BEFORE the poison loop
    restore nothing yet, just cleanup and leave // sniffer never started
    close(fd) ; free_ressources() ; return 1

while g_running:
    send in ; send out ; sleep

stop_sniffer(&s)                         // BEFORE restore_arp (§ 3.5 order)
restore_arp() ; close(fd) ; free_ressources()
```

> [!CAUTION]
> **Si `start_sniffer` échoue, ne pas empoisonner.** On n'a encore rien envoyé,
> donc rien à restaurer. En revanche, si l'échec survient **après** le début du
> poison, il faut `restore_arp` avant de sortir — d'où l'importance de démarrer le
> sniffer **avant** la boucle.

> **Implémentation réelle (11/08) :** `discover_interface()` est appelé **par
> `parse_arguments()`** (pas séparément dans `main`). Et comme `start_sniffer`
> garde `error()`, le garde `if … != 0 { cleanup ; return 1 }` est simplifié en un
> simple `start_sniffer(&s, &config);` (l'échec fait `exit()` de lui-même, sans
> victime empoisonnée puisque c'est avant la boucle). Le reste de l'ordre —
> `stop_sniffer` **avant** `restore_arp` — est respecté tel quel.

---

## 5. Pièges spécifiques à cette phase

Les **bugs qui étaient présents** dans `sniffing.c`/`inquisitor.h`/`main.c` sont
**tous corrigés au 11/08** (historique conservé pour la soutenance) :

| Bug (historique) | Symptôme | Statut |
|---|---|---|
| `t_config.iface` typé **`char`** | `pcap_open_live` recevait un octet, pas une chaîne | ✅ corrigé → `char[IFNAMSIZ]` (§ 4.1) |
| `discover_interface` ne stockait pas `ifa_name` | `iface` restait vide | ✅ corrigé → `ft_strlcpy` |
| `stohs(eth->ether_type)` | ne compilait pas | ✅ corrigé → `ntohs` |
| `pcap_*` sur `s->handle` non affecté | segfault au démarrage | ✅ corrigé → locale `handle`, publiée en dernier |
| casts manquants (`eth`, `ip`, `tcp`) | erreur `-Werror` | ✅ corrigé |
| `pcap_freecode(&fp)` absent (succès) | petite fuite | ✅ corrigé |
| `"RET "` au lieu de `"RETR "` (compare sur 5) | détection `RETR` cassée | ✅ corrigé |
| `stop_sniffer` en commentaire | CTRL+C sale | ✅ corrigé → implémentée + appelée dans `main` |
| `main` : `if start_sniffer == 0` inversé + `close(ifindex)` | s'auto-tuait avant de poison | ✅ corrigé → appel simple, boucle atteignable |
| `discover_interface` appelé 2× dans `main` | fuite `local_ip`/`local_mac` | ✅ corrigé → un seul appel (via `parse_arguments`) |
| sortie invisible en pipe | tests ne voyaient rien | ✅ corrigé → `setvbuf(_IOLBF)` |

> Choix **assumé** (pas un bug) : `start_sniffer` garde `error()` (→ `exit`) au lieu
> de `return -1`. Sans risque **tant que** le sniffer démarre avant tout
> empoisonnement (cas actuel) : un échec ne laisse aucune victime empoisonnée.

Pièges transverses (non-bugs, à ne pas introduire) :
- Promiscuous obligatoire (§ 3.1) — sinon affichage vide.
- `ip_forward` doit rester à 1 (déjà réglé dans le lab) sinon le FTP se coupe.
- Réutiliser l'interface découverte, jamais `eth0` en dur.

---

## 6. Modules B, C, E — le reste

### 6.1 Module B — Tests FTP 🔴

> **Objectif sujet** : « La démonstration se fait via le protocole FTP. Une suite
> de tests spécifique à ce protocole est donc requise. » → `tests/test_ftp.py`.

**Ce que tu testes :**
- Un `put` client → serveur fait apparaître `[FTP] STOR <fichier>` sur le stdout
  d'`inquisitor`.
- Un `get` fait apparaître `[FTP] RETR <fichier>`.

**Ce que le test doit prouver au-delà du comportement :** que l'ajout du sniffer
**ne casse pas** l'arrêt propre — d'où l'insistance sur `test_exit_code_zero_after_sigint`
qui doit rester vert. Un sniffer qui affiche bien mais crashe au CTRL+C ne passe
pas le sujet.

**Stratégie :** lancer `inquisitor` avec `stdout=PIPE` (variante des helpers de
`test_poisoning.py` : `flush_arp_caches`, `populate_arp_caches`, `stop_inquisitor`),
déclencher un transfert `lftp` depuis le conteneur `client`, lire le stdout **en
flux** (le process tourne toujours), chercher la ligne attendue.

```python
@unittest.skipUnless(lab_is_up(), "Docker lab is not running")
class TestFTPSniffing(unittest.TestCase):
    def setUp(self):
        flush_arp_caches(); populate_arp_caches()
        self.proc = start_inquisitor_capturing_stdout()   # stdout=subprocess.PIPE
        time.sleep(3)                                      # poison in place

    def _lftp(self, script):                               # comment in English (code)
        compose_exec("client",
            ["lftp", "-u", "test,1234", "-e", script, "192.168.0.2"], timeout=10)

    def test_stor_filename_displayed(self):
        self._lftp("put /etc/hostname -o up.txt; bye")
        out = read_stdout_until(self.proc, "STOR", timeout=5)
        self.assertIn("STOR up.txt", out)

    def test_retr_filename_displayed(self):
        self._lftp("get hello.txt -o /tmp/dl.txt; bye")
        out = read_stdout_until(self.proc, "RETR", timeout=5)
        self.assertIn("RETR hello.txt", out)

    def tearDown(self):
        stop_inquisitor(self.proc)
```

> [!CAUTION]
> **Ne pas faire `proc.communicate()`** : il attend la fin du process, or
> `inquisitor` tourne jusqu'au CTRL+C → blocage jusqu'au timeout. Lire ligne à
> ligne avec un timeout (`read_stdout_until`).

**Pré-requis :** `data/hello.txt` doit exister côté serveur pour le `RETR` (le
créer s'il manque). Dépend de `fflush(stdout)` côté C (§ 4.4).

**Résultats attendus :**
- `test_stor_filename_displayed` : PASS
- `test_retr_filename_displayed` : PASS
- Suite ARP (`test_poisoning.py`), dont `test_exit_code_zero_after_sigint` :
  inchangée, PASS.

### 6.2 Module C — Robustesse 🟠

Aucune fonctionnalité nouvelle, uniquement de la fiabilité (défauts repérés au
Doc 2).

| # | Où | Correction |
|---|---|---|
| C.1 | `discover_interface` (`parsing.c`) | libérer `local_ip` sur chaque `return -1` postérieur à son allocation (ou label `goto cleanup`) |
| C.2 | `build_arp_trame` (`poisoning.c`) | `config` passé par valeur → `error(&config)` libère des pointeurs partagés avec `main`. Passer `t_config*`, ou documenter que `error()` termine le process |
| C.3 | boucle / `restore_arp` (`main.c`) | `sleep(1)` rend le CTRL+C lent → `nanosleep` en petites tranches qui teste `g_running` |
| C.4 | `poisoning.c` | `struct sockaddr_ll sockadr = {0}` et `memset(frame, 0, sizeof(*frame))` (défensif) |
| C.5 | `parsing.c` | supprimer les tests morts `converted_value < 0` (≈ l.112, 143) ; commenter la convention `return 1 = invalide / 0 = valide` |

**Validation :**
```sh
make re && make debug        # build asan/ubsan
make up && make run          # put/get + CTRL+C : pas de leak, exit 0 rapide
python3 tests/run_all.py     # tout reste vert
```

### 6.3 Module D — Cohérence documentaire ✅ **FAIT**

README réaligné : arborescence des **vrais** fichiers, `docker-compose.yml`
corrigé, section « Project status » marquant le sniffing comme WIP avec le format
`[FTP] STOR/RETR` présenté comme **cible**. **À revalider** une fois le Module A
livré : le format documenté doit devenir le format réellement produit.

### 6.4 Module E — Bonus verbose `-v` 🟢

> **⚠️ À ne traiter que si A→C sont parfaits.**

Le sujet veut que `-v` affiche **tout** le trafic FTP, **login inclus** (`USER`,
`PASS`). Le pseudo-code de `ftp_handler` porte déjà la branche `if s.verbose`
(§ 4.4) ; `t_sniffer` et `t_config` ont déjà `verbose`. Il reste à **parser le
flag** et le **propager**.

**Corps (delta parsing)** :

```
parse_arguments(ac : int, av : char**, config : t_config*):
    config.verbose = 0
    if ac == 6 and av[5] is "-v":
        config.verbose = 1
        ac = 5                          // normalise, then reuse existing validation
    if ac != 5:
        error("invalid number of arguments", 1, config)
    ... existing validation unchanged ...
```

**Test (extension de `test_ftp.py`)** :

```python
def test_verbose_shows_login(self):     # inquisitor launched with -v
    self._lftp("put /etc/hostname -o up.txt; bye")
    out = read_stdout_until(self.proc, "PASS", timeout=5)
    self.assertIn("USER test", out)
    self.assertIn("PASS 1234", out)
```

**Résultat attendu :**
```
[FTP] USER test
[FTP] PASS 1234
[FTP] STOR up.txt
```

---

## 7. Un schéma qui porte le § 3.2 : la descente des couches

Un `STOR up.txt` réel, avec des options TCP (donc `tcp_len = 32`, pas 20). Les
valeurs sont concrètes exprès — c'est le contraste `20` vs `32` qui rend le piège
visible.

| Couche | Offset début | Longueur | D'où vient la longueur | Contenu |
|---|---|---|---|---|
| Ethernet | 0 | 14 (fixe) | constante | `ether_type = 0x0800` (IPv4) |
| IP | 14 | **20** | `ip_hl = 5` → `5 × 4` | `ip_p = 6` (TCP) |
| TCP | 34 | **32** | `doff = 8` → `8 × 4` | (avec options) |
| payload | **66** | 13 | `caplen(79) − 66` | `STOR up.txt\r\n` |

Cas dégénéré à opposer — un **ACK pur** sur la même connexion :

| Couche | Offset | Longueur | Contenu |
|---|---|---|---|
| Eth+IP+TCP | 0 | 54 | entêtes seuls |
| payload | 54 | **0** | `caplen(54) == headers` → `return` (pas de ligne fantôme) |

Et le découpage d'un segment qui porte **deux lignes** (bonus `-v`) :

```
  payload (payload_len = 21)
  ┌──────────────────────────────────────────────┐
  │ U S E R   t e s t \r \n P A S S   1 2 3 4 \r \n │
  └───────────────────┬────────────────┬───────────┘
       memchr '\n' ────┘   memchr '\n' ─┘
   tour 1 : "USER test"   tour 2 : "PASS 1234"
```

---

## 8. Ordre de développement recommandé

1. **Corriger les bugs bloquants** du § 5 pour que `sniffing.c` **compile**
   (`char *iface`, `ntohs`, casts, `handle` local).
2. **Module A**, sous-ordre : `discover_interface` (iface) → `start_sniffer`
   (ouverture + filtre, sans thread d'abord) → `ftp_handler` (parse STOR/RETR) →
   thread + `stop_sniffer` → intégration `main`.
3. **Test manuel** :
   ```sh
   make up ; make run                                       # terminal 1
   docker compose exec client lftp -u test,1234 \
     -e "put /etc/hostname -o up.txt; bye" 192.168.0.2      # terminal 2
   # attendu terminal 1 : [FTP] STOR up.txt
   ```
4. **Module B** — `tests/test_ftp.py` (automatise la validation de A).
5. **Module C** — robustesse (mémoire, CTRL+C, structs).
6. **Module D** — revalider le README contre la sortie réelle.
7. **Module E** — bonus `-v`, seulement si 1→6 parfaits.

> Quand `[FTP] STOR/RETR <fichier>` s'affiche en direct **et** que le CTRL+C reste
> propre (exit 0, `test_exit_code_zero_after_sigint` vert), le mandatory est
> soutenable. C renforce, E est l'extra.
```