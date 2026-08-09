# Inquisitor — Document 3 : Plan du reste à faire (par module)

> Plan opérationnel de tout ce qui reste, découpé en **5 modules autonomes**
> (A→E), chacun avec : objectif, concepts, étapes, pseudo-code / corrections
> concrètes, pièges et tests. Format inspiré de `template_doc.md`.
>
> **Ordre imposé :** A → B → C → D, puis E (bonus) uniquement si le mandatory est
> parfait.

---

## 1. Vue d'ensemble des modules

| Module | Titre | Priorité | Sévérité | Bloque le rendu ? |
|--------|-------|----------|----------|-------------------|
| **A** | Sniffing FTP + affichage des fichiers | 1 | 🔴 | **Oui** (exigence mandatory) |
| **B** | Tests FTP | 2 | 🔴 | **Oui** (exigés par le sujet) |
| **C** | Robustesse (mémoire, CTRL+C, trames) | 3 | 🟠 | Non, mais attendu en soutenance |
| ~~**D**~~ | ~~Cohérence documentaire (README)~~ | — | ✅ | **Fait (09/08)** |
| **E** | Bonus verbose `-v` | 5 | 🟢 | Non (bonus) |

**Architecture cible globale (après A) :**

```
main()
 ├─ setup_signals()                     [fait]
 ├─ parse_arguments()                   [fait] (+ flag -v au module E)
 ├─ build_arp_trame() x2 (in/out)       [fait]
 ├─ open_inject_socket()                [fait]
 ├─ start_sniffer()   ◄── Module A : thread pcap "tcp port 21"
 ├─ while (g_running) { send in/out ; sleep }   [fait]
 ├─ stop_sniffer()    ◄── Module A : breakloop -> join -> close
 ├─ restore_arp()                       [fait]
 └─ free / close                        [fait]
```

---
---

# MODULE A — Sniffing FTP + affichage des fichiers 🔴

> **Objectif sujet :** « afficher en temps réel les noms des fichiers échangés
> entre un client et un serveur FTP ». C'est le seul verrou du mandatory.

> **État au 09/08 :** le **squelette d'interface** est posé — `include/sniffing.h`
> déclare `t_sniffer` (handle `pcap_t*` + `pthread_t` + `verbose`) et les
> prototypes `start_sniffer`, `capture_loop`, `ftp_handler`, `stop_sniffer`.
> **Rien n'est implémenté** (`sniffing.c` = stub `test()`), rien n'est intégré à
> `main`, et **certaines signatures sont à réconcilier** avant de coder (voir
> A.2). Le reste de ce module reste donc entièrement à faire.

## A.1 Concepts à maîtriser

### libpcap — cycle de vie complet
La chaîne d'appels obligatoire : **ouvrir → vérifier le datalink → compiler le
filtre → appliquer → boucler → arrêter → fermer**. Chaque étape peut échouer et
doit être testée.
- `pcap_open_live(iface, snaplen, promisc, to_ms, errbuf)` — ouvre l'IF. On passe
  `snaplen=65535` (capturer tout le paquet), `promisc=1` (**voir le trafic
  relayé**, pas seulement le sien — indispensable en MITM), `to_ms=1000`. Renvoie
  `NULL` + message dans `errbuf` en cas d'échec.
- `pcap_datalink(handle)` — type de couche liaison. Ton parsing suppose de
  l'Ethernet (offset 14) → **vérifier `== DLT_EN10MB`**, sinon les offsets sont
  faux (ex. Linux « cooked », loopback…).
- `pcap_loop(handle, -1, handler, user)` — boucle infinie (`-1`), appelle
  `handler` à chaque paquet ; `user` est un pointeur passé **tel quel** au handler.
- `pcap_breakloop` / `pcap_close` — arrêt (voir concept concurrence).

Docs (man, **une par fonction** — lis celles que tu appelles) :
- Vue d'ensemble pcap(3) : https://www.tcpdump.org/manpages/pcap.3pcap.html
- `pcap_open_live`(3), `pcap_datalink`(3), `pcap_loop`(3), `pcap_breakloop`(3),
  `pcap_close`(3), `pcap_geterr`(3) — toutes sur `tcpdump.org/manpages/`.
- **Le meilleur tuto pour débuter (lis-le en entier) :** « Programming with pcap »
  — https://www.tcpdump.org/pcap.html

> Analogie : un micro branché sur le câble ; le filtre ne laisse passer que « la
> fréquence 21 ».
> Piège : oublier le promiscuous (`promisc=1`) → affichage FTP vide alors que le
> MITM marche.

### Filtre BPF (`pcap_compile` + `pcap_setfilter`)
`pcap_compile(handle, &fp, "tcp port 21", 1, PCAP_NETMASK_UNKNOWN)` **traduit** la
chaîne en bytecode (`struct bpf_program fp`) ; `pcap_setfilter(handle, &fp)`
**l'installe** ; `pcap_freecode(&fp)` le libère une fois posé. Les deux renvoient
`-1` en cas d'échec → afficher `pcap_geterr(handle)`.
- Syntaxe des filtres : https://www.tcpdump.org/manpages/pcap-filter.7.html
- `pcap_compile`(3), `pcap_setfilter`(3), `pcap_freecode`(3) sur tcpdump.org.

### En-têtes Ethernet → IP → TCP → payload (offsets + structs exactes)
Tu « descends » les couches en ajoutant des offsets **calculés** (jamais en dur) :
- `struct ether_header` (`<net/ethernet.h>`) — 14 octets fixes. Champ
  `ether_type` : ne continuer que si `ntohs(ether_type) == ETHERTYPE_IP` (0x0800).
- `struct ip` (`<netinet/ip.h>`) — `ip_hl` = longueur de l'en-tête IP **en mots de
  4 octets** → longueur réelle = `ip_hl * 4` (≥ 20). Champ `ip_p` : ne continuer
  que si `== IPPROTO_TCP` (6).
- `struct tcphdr` (`<netinet/tcp.h>`) — `doff` (data offset) = longueur de l'en-tête
  TCP **en mots de 4 octets** → `doff * 4` (≥ 20).
- payload FTP = `paquet + 14 + ip_len + tcp_len` ; sa taille =
  `caplen − 14 − ip_len − tcp_len`.

> Piège n°1 : coder `+14+20+20` en dur — les options IP/TCP changent la taille, on
> lit alors à côté. Recalcule via `ip_hl` / `doff`.
> Piège n°2 (glibc) : les noms BSD `th_off`/`th_*` de `struct tcphdr` n'existent
> que si `__FAVOR_BSD` est défini ; par défaut utilise **`doff`**. Idem `struct ip`
> (BSD, champ `ip_hl`) vs `struct iphdr` (Linux, champ `ihl`) — choisis-en une et
> inclus le bon header.
> Piège n°3 : borne tes lectures avec `header->caplen` (octets réellement
> capturés), **pas** `header->len` (taille sur le fil, potentiellement plus grande).

### Protocole FTP (RFC 959, port 21, texte clair)
Le canal de contrôle (port 21) transporte des commandes **texte, une par ligne**,
terminées par `\r\n`. Mandatory : `STOR <fichier>` (upload) et `RETR <fichier>`
(download) → afficher le nom. Casse indifférente. Un même segment TCP peut contenir
0, 1 ou **plusieurs** lignes (ou une ligne coupée) → itérer, rester tolérant.
- RFC 959, §4 (commandes) & §5 : https://www.rfc-editor.org/rfc/rfc959
- (bonus `-v`) le login `USER`/`PASS` passe par les mêmes lignes texte.

### Concurrence sniffer ↔ poison (pthreads)
`pcap_loop` **bloque** → il tourne dans un **thread dédié** (le Makefile lie déjà
`-pthread`) pendant que `main` empoisonne en boucle. Ordre d'arrêt **impératif** :
`pcap_breakloop` (fait sortir `pcap_loop`) → `pthread_join` (attend la fin du
thread) → `pcap_close` (libère le handle).
- `man 3 pthread_create`, `man 3 pthread_join`.
> Piège : `pcap_close` pendant que le thread est encore dans `pcap_loop` → crash /
> use-after-free. Toujours `breakloop` puis `join` **avant** `close`.
> `verbose` partagé en lecture seule entre threads = OK ; ne partage rien d'écrit
> sans protection.

### Docs à lire, dans l'ordre (pour avoir les clés en main)
1. **« Programming with pcap »** (tuto fondateur, en entier) — https://www.tcpdump.org/pcap.html
2. pcap(3), puis le man de **chaque** fonction que tu appelles (open_live, datalink,
   compile, setfilter, freecode, loop, breakloop, close, geterr).
3. **pcap-filter(7)** pour comprendre `tcp port 21`.
4. **RFC 959 §4–5** pour `STOR`/`RETR` (et `USER`/`PASS` pour le bonus).
5. `man 3 pthread_create` / `pthread_join`.
6. Les structs directement dans les headers système :
   `/usr/include/net/ethernet.h`, `/usr/include/netinet/ip.h`,
   `/usr/include/netinet/tcp.h` (lis les champs, ça vaut tous les tutos).

## A.2 Étapes

1. **`include/sniffing.h` — squelette posé, à réconcilier** ✅⚠️ : la struct
   `t_sniffer` et les prototypes existent déjà. **Corrections à faire avant
   d'implémenter :**
   - passer `t_sniffer *` (pointeur) à `capture_loop`/`stop_sniffer` plutôt que
     par valeur — sinon on manipule une copie du `pcap_t*`/`pthread_t` ;
   - donner à `ftp_handler` la signature imposée par `pcap_loop` :
     `void ftp_handler(u_char *user, const struct pcap_pkthdr *h, const u_char *pkt)`.
2. **Stocker l'interface** dans `t_config` (❌ pas encore fait) :
   `discover_interface` connaît déjà `ifa_name` (variable `tmp->ifa_name`) mais ne
   le garde pas → ajouter `char *iface;` à `t_config`, le `ft_strdup`, et le
   libérer dans `free_ressources`.
3. **Ouvrir la capture** : `pcap_open_live(iface, 65535, 1, 1000, errbuf)`,
   compiler + appliquer `tcp port 21`.
4. **Handler** : sauter Ethernet/IP/TCP via `ip_hl`/`th_off`, isoler le payload.
5. **Parser mandatory** : lignes `STOR `/`RETR ` (insensible casse) → nom →
   `printf("[FTP] STOR %s\n")`.
6. **Threader** : lancer `pcap_loop` dans un `pthread` avant la boucle de poison.
7. **Arrêt propre** : `stop_sniffer` = `breakloop` → `join` → `close`, appelé
   avant `restore_arp`.

## A.3 Prototypes (C)

> Contrat d'interface uniquement (structs + signatures). C'est la **cible** à
> viser : elle corrige les signatures actuelles de `sniffing.h` (passage par
> pointeur, `ftp_handler` conforme à `pcap_loop`) et ajoute `iface` à `t_config`.

```c
/* inquisitor.h — ajout au t_config existant */
typedef struct s_config {
    /* ... champs existants ... */
    char *iface;      /* nom de l'IF découverte (ft_strdup, libéré au cleanup) */
    int   verbose;    /* 0 = mandatory (STOR/RETR), 1 = verbose -v (bonus)     */
} t_config;

/* sniffing.h */
typedef struct s_sniffer {
    pcap_t    *handle;
    pthread_t  thread;
    int        verbose;
} t_sniffer;

int   start_sniffer(t_sniffer *s, t_config *config);
void *capture_loop(void *arg);   /* thread : (t_sniffer *) -> pcap_loop        */
void  ftp_handler(u_char *user,
                  const struct pcap_pkthdr *header,
                  const u_char *packet);      /* prototype imposé par pcap_loop */
void  stop_sniffer(t_sniffer *s);            /* breakloop -> join -> close      */
```

> Note : `capture_loop` respecte la signature `void *(*)(void *)` attendue par
> `pthread_create` ; `ftp_handler` respecte celle attendue par `pcap_loop`. Les
> deux structs et `t_config` sont passés **par pointeur** pour éviter de
> travailler sur une copie du `pcap_t*` / `pthread_t`.

## A.4 Pseudo-code (logique) — détaillé

> Chaque appel pcap peut échouer : on teste **tous** les retours. Les noms de
> champs ci-dessous sont ceux des **vraies** structs système (voir A.1 / A.3).

```
// ─── start_sniffer : ouvrir, vérifier, filtrer, lancer le thread ───
FONCTION start_sniffer(s, config) -> int (0 = ok, -1 = erreur) :
    char errbuf[PCAP_ERRBUF_SIZE]
    s.verbose = config.verbose
    s.handle  = pcap_open_live(config.iface, 65535, 1 /*promisc*/, 1000, errbuf)
    SI s.handle == NULL : afficher errbuf ; RETOURNER -1

    // le parsing suppose de l'Ethernet (offset 14) -> le vérifier
    SI pcap_datalink(s.handle) != DLT_EN10MB :
        afficher "interface non-Ethernet" ; pcap_close(s.handle) ; RETOURNER -1

    struct bpf_program fp
    SI pcap_compile(s.handle, &fp, "tcp port 21", 1, PCAP_NETMASK_UNKNOWN) == -1 :
        afficher pcap_geterr(s.handle) ; pcap_close(s.handle) ; RETOURNER -1
    SI pcap_setfilter(s.handle, &fp) == -1 :
        afficher pcap_geterr(s.handle) ; pcap_freecode(&fp) ; pcap_close(s.handle) ; RETOURNER -1
    pcap_freecode(&fp)                       // bytecode inutile une fois posé

    SI pthread_create(&s.thread, NULL, capture_loop, s) != 0 :
        afficher "pthread_create" ; pcap_close(s.handle) ; RETOURNER -1
    RETOURNER 0

// ─── capture_loop : exécuté DANS le thread ───
FONCTION capture_loop(arg) -> void* :
    s = (t_sniffer*) arg
    pcap_loop(s.handle, -1, ftp_handler, (u_char*) s)   // bloque jusqu'au breakloop
    RETOURNER NULL

// ─── ftp_handler : appelé une fois par paquet ───
FONCTION ftp_handler(user, header, packet) :
    s   = (t_sniffer*) user
    len = header.caplen                      // borne SÛRE (octets réellement capturés)

    // (1) gardes-fous de taille AVANT tout déréférencement
    SI len < 14 + 20 + 20 : RETOURNER        // au minimum Eth + IP min + TCP min
    eth = (struct ether_header*) packet
    SI ntohs(eth.ether_type) != ETHERTYPE_IP : RETOURNER

    ip     = (struct ip*) (packet + 14)
    ip_len = ip.ip_hl * 4                     // ip_hl = mots de 4 octets
    SI ip_len < 20 OU ip.ip_p != IPPROTO_TCP : RETOURNER

    tcp     = (struct tcphdr*) (packet + 14 + ip_len)
    tcp_len = tcp.doff * 4                     // doff = mots de 4 octets (nom Linux)
    SI tcp_len < 20 : RETOURNER

    entetes = 14 + ip_len + tcp_len
    SI len <= entetes : RETOURNER             // pas de payload (ACK pur, etc.)
    payload     = packet + entetes
    payload_len = len - entetes

    // (2) le payload N'EST PAS terminé par \0 -> itérer avec la longueur
    //     (memchr pour '\n'), jamais de strlen/strstr direct dessus.
    POUR CHAQUE ligne DANS payload[0 .. payload_len] séparée par '\n' :
        retirer le '\r' final éventuel
        SI ligne vide : CONTINUER
        SI s.verbose :                        // bonus -v : tout le trafic
            afficher "[FTP] " + ligne
        SINON SI ligne commence (insensible casse) par "STOR " OU "RETR " :
            cmd = les 4 premières lettres (en majuscules)
            nom = la ligne après le premier espace
            afficher "[FTP] " + cmd + " " + nom

// ─── stop_sniffer : ORDRE IMPÉRATIF ───
FONCTION stop_sniffer(s) :
    SI s.handle == NULL : RETOURNER
    pcap_breakloop(s.handle)                  // 1. fait sortir pcap_loop
    pthread_join(s.thread, NULL)              // 2. attend la fin du thread
    pcap_close(s.handle)                      // 3. libère le handle
    s.handle = NULL
```

Intégration dans `main` (ordre exact) :

```
setup_signals() ; parse_arguments() ; print_config()
build_arp_trame() x2 ; fd = open_inject_socket()
t_sniffer s ;
SI start_sniffer(&s, &config) != 0 : error(...)   // AVANT la boucle de poison
TANT QUE g_running : send in ; send out ; sleep
stop_sniffer(&s)                                    // AVANT restore_arp
restore_arp() ; close(fd) ; free_ressources()
```

## A.5 Pièges spécifiques
- Promiscuous obligatoire (sinon affichage vide).
- Offsets `ip_hl`/`th_off` = valeur × 4, jamais en dur.
- Ordre d'arrêt `breakloop → join → close` (sinon crash au CTRL+C → casse
  `test_exit_code_zero_after_sigint`).
- Réutiliser l'interface découverte (pas `eth0` en dur).
- `ip_forward` doit rester à 1 (déjà réglé) sinon le FTP se coupe.

## A.6 Test manuel de validation
```sh
make up
make run                                   # terminal 1
docker compose exec client lftp -u test,1234 \
  -e "put /etc/hostname -o up.txt; bye" 192.168.0.2   # terminal 2
# attendu terminal 1 : [FTP] STOR up.txt
```

> **Module A clos quand** `[FTP] STOR/RETR <fichier>` s'affiche en direct et que
> l'arrêt CTRL+C reste propre (exit 0).

---
---

# MODULE B — Tests FTP 🔴

> **Objectif sujet :** « La démonstration se fait via le protocole FTP. Une suite
> de tests spécifique à ce protocole est donc requise. » → nouveau fichier
> `tests/test_ftp.py`.

## B.1 Ce qu'on teste
- Un `put` client → serveur fait apparaître `[FTP] STOR <fichier>` sur le stdout
  de `inquisitor`.
- Un `get` fait apparaître `[FTP] RETR <fichier>`.
- Les tests ARP existants **restent verts** (l'ajout du sniffer ne change pas le
  code de sortie 0).

## B.2 Stratégie
Lancer `inquisitor` avec `stdout=PIPE` (réutiliser les helpers de
`test_poisoning.py` : `flush_arp_caches`, `populate_arp_caches`, `stop_inquisitor`),
déclencher un transfert `lftp` depuis le conteneur `client`, lire le stdout
capturé, chercher la ligne attendue.

## B.3 Pseudo-code (`tests/test_ftp.py`)
```python
@unittest.skipUnless(lab_is_up(), "Docker lab is not running")
class TestFTPSniffing(unittest.TestCase):
    def setUp(self):
        flush_arp_caches(); populate_arp_caches()
        # variante de start_inquisitor() avec stdout=subprocess.PIPE
        self.proc = start_inquisitor_capturing_stdout()
        time.sleep(3)                        # poison en place

    def _lftp(self, script):
        compose_exec("client",
            ["lftp","-u","test,1234","-e",script,"192.168.0.2"], timeout=10)

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

## B.4 Pré-requis / pièges
- Le fichier `data/hello.txt` doit exister côté serveur pour tester le `RETR`
  (le README le mentionne ; le créer si absent).
- Lire le stdout **en flux** (le process tourne toujours) : ne pas faire
  `proc.communicate()` qui bloque — lire ligne à ligne avec un timeout.
- Garder `test_exit_code_zero_after_sigint` (dans `test_poisoning.py`) vert :
  régression directe si l'arrêt du thread sniffer casse le code 0.

## B.5 Résultats attendus
- `test_stor_filename_displayed` : PASS
- `test_retr_filename_displayed` : PASS
- suite ARP (`test_poisoning.py`) : inchangée, PASS

---
---

# MODULE C — Robustesse 🟠

> **Objectif :** corriger les défauts repérés dans le code existant (voir Doc 2).
> Aucune nouvelle fonctionnalité, uniquement de la fiabilité.

## C.1 Fuite mémoire dans `discover_interface` (`src/parsing.c`)
Sur le chemin d'erreur entre `ft_strdup(local_ip)` et la fin, `local_ip` alloué
n'est pas libéré si un `ioctl` échoue.
```
Correction : avant chaque `return (-1)` postérieur à l'allocation de local_ip,
             free(config->local_ip); config->local_ip = NULL;
   (ou centraliser via un label goto cleanup).
```

## C.2 `config` copie vs original dans `build_arp_trame` (`src/poisoning.c`)
`build_arp_trame(t_arp_frame*, t_config config)` reçoit `config` **par valeur** ;
`error(&config)` libère `local_ip/local_mac` du **copie** (mêmes pointeurs que le
`main`). Sans danger tant que `error()` fait `exit()`, mais fragile.
```
Correction (au choix) :
  - passer t_config* (pointeur) à build_arp_trame pour lever l'ambiguïté, OU
  - documenter clairement par un commentaire que error() termine le process.
```

## C.3 CTRL+C peu réactif (`src/main.c`, `restore_arp`)
`sleep(1)` dans la boucle → jusqu'à 1 s pour sortir, puis 5 s de restore.
```
Correction : remplacer sleep(1) par une attente interruptible
  (nanosleep en petites tranches qui teste g_running, ou intervalle plus court).
Le restore peut aussi passer à un intervalle < 1 s pour accélérer la soutenance.
```

## C.4 Initialisation des structures (`src/poisoning.c`)
`send_arp_frame` remplit `struct sockaddr_ll sockadr` partiellement ;
`build_arp_trame` n'initialise pas la trame.
```
Correction : struct sockaddr_ll sockadr = {0};  et  memset(frame, 0, sizeof(*frame));
             (défensif si les structs évoluent)
```

## C.5 Code mort (`src/parsing.c`)
Les tests `converted_value < 0` (lignes ~112 et ~143) ne peuvent jamais être
vrais.
```
Correction : les supprimer, et ajouter un commentaire sur la convention
             « return 1 = invalide / return 0 = valide » de is_ipv4 / is_mac_addr.
```

## C.6 Validation
```sh
make re && make debug         # build asan/ubsan
make up && make run           # puis put/get + CTRL+C : pas de leak, exit 0 rapide
python3 tests/run_all.py      # tout reste vert
```

---
---

# MODULE D — Cohérence documentaire ✅ **FAIT (09/08)**

> **Objectif :** aligner le `README.md` sur le code réel pour ne pas induire
> l'évaluateur en erreur. **Réalisé.**

## D.1 Arborescence ✅
Le README listait `src/args.c`, `netinfo.c`, `arp.c`, `inject.c`, `sniff.c`,
`ftp.c` — **fichiers inexistants**. La section « Project structure » a été
réécrite avec les vrais fichiers (`main.c`, `parsing.c`, `poisoning.c`,
`sniffing.c`, `signals.c`, `utils.c`, + `include/` et `tests/`), un commentaire
par fichier, et une note précisant que parsing = args+IPv4+MAC+interface et
poisoning = build+inject+restore.

## D.2 Exemples d'affichage ✅
Le README montre toujours `[FTP] STOR ...` / `[FTP] RETR ...`, mais une section
**« Project status »** a été ajoutée : elle marque le sniffing comme **WIP** et
précise que ce format est la **cible**, pas la sortie actuelle. À revalider une
fois le Module A implémenté (le format documenté = le format produit).

## D.3 `docker-compose.yaml` vs `.yml` ✅
Corrigé : le README cite désormais `docker-compose.yml` (le vrai fichier).

## D.4 Validation
Relecture croisée README ↔ `ls src/` ↔ sortie réelle de `make run` : **OK**.
`make help` a aussi été ajouté à la liste des cibles.

---
---

# MODULE E — Bonus verbose `-v` 🟢

> **Objectif sujet :** un mode `-v` qui affiche **tout** le trafic FTP (login
> inclus : `USER`, `PASS`, …), pas seulement les noms de fichiers.
> **⚠️ À ne traiter que si A→D sont parfaits** — le bonus n'est évalué que dans ce
> cas.

## E.1 Étapes
1. **Parser le flag** dans `parse_arguments` : accepter un 5ᵉ argument optionnel
   `-v` (donc `ac == 5` **ou** `ac == 6 && strcmp(av[5], "-v") == 0`).
2. **Stocker** `int verbose` dans `t_config`.
3. **Propager** `verbose` à `start_sniffer` → `ftp_handler` (déjà prévu dans le
   pseudo-code du Module A : la branche `SI *verbose` existe).
4. En mode verbose, afficher **toutes** les lignes FTP, pas seulement STOR/RETR.

## E.2 Pseudo-code (delta parsing)
```
FONCTION parse_arguments(ac, av, config) :
    config.verbose = 0
    SI ac == 6 ET av[5] == "-v" : config.verbose = 1 ; ac = 5   // normalise
    SI ac != 5 : error("invalid number of arguments")
    ... (validation existante inchangée)
```

## E.3 Pièges
- Le sujet insiste : **login inclus** → capturer aussi `USER`/`PASS`, donc ne pas
  filtrer sur STOR/RETR en mode `-v`.
- Un paquet peut contenir plusieurs lignes ou une ligne coupée → itérer sur les
  lignes et rester tolérant.

## E.4 Test (extension de `test_ftp.py`)
```python
def test_verbose_shows_login(self):
    # inquisitor lancé avec -v
    self._lftp("put /etc/hostname -o up.txt; bye")
    out = read_stdout_until(self.proc, "PASS", timeout=5)
    self.assertIn("USER test", out)
    self.assertIn("PASS 1234", out)
```

## E.5 Résultat attendu
```
[FTP] USER test
[FTP] PASS 1234
[FTP] STOR up.txt
[FTP] RETR hello.txt
```

---
---

## Ordre de développement recommandé (global)

1. **Module A** — sniffing FTP (le verrou). Sous-ordre : `sniffing.h` → ouverture
   pcap seule → parsing STOR/RETR → thread + arrêt propre → test manuel.
2. **Module B** — `tests/test_ftp.py` (automatiser la validation de A).
3. **Module C** — corrections de robustesse (mémoire, CTRL+C, structs).
4. **Module D** — réalignement du README.
5. **Module E** — bonus `-v` (seulement si 1→4 parfaits).

> **Mandatory soutenable** dès que A + B sont verts (et que les tests ARP
> existants ne régressent pas). C + D renforcent la soutenance. E est l'extra.
