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
| **D** | Cohérence documentaire (README) | 4 | 🟠 | Non, mais trompe l'évaluateur |
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

## A.1 Concepts à maîtriser

### libpcap (ouverture + filtre BPF + boucle)
`pcap_open_live` ouvre l'interface en **promiscuous** (indispensable : en MITM on
veut voir le trafic *relayé*, pas seulement le sien). `pcap_compile` +
`pcap_setfilter` appliquent le filtre `tcp port 21` (canal de contrôle FTP).
`pcap_loop` appelle un handler par paquet.
- Manuel : https://www.tcpdump.org/manpages/pcap.3pcap.html
- Filtres BPF : https://www.tcpdump.org/manpages/pcap-filter.7.html

> Analogie : un micro branché sur le câble ; le filtre BPF ne laisse passer que
> « la fréquence 21 ».
> Piège : oublier le promiscuous (3ᵉ arg à `1`) → affichage FTP vide alors que le
> MITM marche.

### Empilement des en-têtes Ethernet → IP → TCP → payload
Sauter 14 octets Ethernet, puis `ip_hl * 4` (variable), puis `th_off * 4`
(variable). Le reste = ligne FTP ASCII.
> Piège : coder `+ 14 + 20 + 20` en dur. Les options IP/TCP changent la taille →
> on lit à côté. Toujours recalculer via `ip_hl` / `th_off`.

### Protocole FTP (port 21, texte clair)
Une commande par ligne terminée par `\r\n`. Mandatory = `STOR <f>` (upload) et
`RETR <f>` (download) → afficher le nom de fichier.
- RFC 959 : https://www.rfc-editor.org/rfc/rfc959

### Concurrence sniffer ↔ poison
`pcap_loop` bloque → **thread dédié** (le Makefile lie déjà `-pthread`). Arrêt :
`pcap_breakloop()` → `pthread_join` → `pcap_close`, dans cet ordre.
> Piège : `pcap_close` pendant que le thread est encore dans `pcap_loop` → crash.

## A.2 Étapes

1. **Remplir `include/sniffing.h`** : `t_sniffer` (handle pcap + `pthread_t`),
   prototypes `start_sniffer(t_config*)`, `stop_sniffer(t_sniffer*)`,
   `ftp_handler(...)`.
2. **Stocker l'interface** dans `t_config` : `discover_interface` connaît déjà
   `ifa_name` mais ne le garde pas → ajouter `char *iface;` et le `ft_strdup`.
3. **Ouvrir la capture** : `pcap_open_live(iface, 65535, 1, 1000, errbuf)`,
   compiler + appliquer `tcp port 21`.
4. **Handler** : sauter Ethernet/IP/TCP via `ip_hl`/`th_off`, isoler le payload.
5. **Parser mandatory** : lignes `STOR `/`RETR ` (insensible casse) → nom →
   `printf("[FTP] STOR %s\n")`.
6. **Threader** : lancer `pcap_loop` dans un `pthread` avant la boucle de poison.
7. **Arrêt propre** : `stop_sniffer` = `breakloop` → `join` → `close`, appelé
   avant `restore_arp`.

## A.3 Pseudo-code

```
// sniffing.h
STRUCT t_sniffer { pcap_t *handle ; pthread_t thread ; int verbose ; }

// sniffing.c
FONCTION start_sniffer(config, verbose) -> t_sniffer :
    handle = pcap_open_live(config.iface, 65535, PROMISC=1, 1000, errbuf)
    SI handle == NULL : error("pcap_open_live")
    compiler filtre "tcp port 21" ; appliquer au handle
    s = { handle, verbose }
    pthread_create(&s.thread, capture_loop, &s)
    RETOURNER s

FONCTION capture_loop(s) :        // exécuté dans le thread
    pcap_loop(s.handle, -1, ftp_handler, (u_char*)&s.verbose)

FONCTION ftp_handler(user, header, packet) :
    ip  = packet + 14
    ip_len  = (ip.version_ihl & 0x0F) * 4
    tcp = ip + ip_len
    tcp_len = (tcp.data_offset >> 4) * 4
    payload = tcp + tcp_len
    payload_len = header.len - 14 - ip_len - tcp_len
    SI payload_len <= 0 : RETOURNER
    POUR CHAQUE ligne (séparée par \r\n) DANS payload :
        SI *verbose : afficher "[FTP] " + ligne          // (préparé pour module E)
        SINON SI ligne commence par "STOR " OU "RETR " :
            afficher "[FTP] " + commande + " " + (ligne après l'espace)

FONCTION stop_sniffer(s) :
    pcap_breakloop(s.handle) ; pthread_join(s.thread) ; pcap_close(s.handle)
```

Intégration `main` : `s = start_sniffer(...)` juste après `open_inject_socket`,
et `stop_sniffer(&s)` juste avant `restore_arp`.

## A.4 Pièges spécifiques
- Promiscuous obligatoire (sinon affichage vide).
- Offsets `ip_hl`/`th_off` = valeur × 4, jamais en dur.
- Ordre d'arrêt `breakloop → join → close` (sinon crash au CTRL+C → casse
  `test_exit_code_zero_after_sigint`).
- Réutiliser l'interface découverte (pas `eth0` en dur).
- `ip_forward` doit rester à 1 (déjà réglé) sinon le FTP se coupe.

## A.5 Test manuel de validation
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

# MODULE D — Cohérence documentaire 🟠

> **Objectif :** aligner le `README.md` sur le code réel pour ne pas induire
> l'évaluateur en erreur.

## D.1 Arborescence
Le README liste `src/args.c`, `netinfo.c`, `arp.c`, `inject.c`, `sniff.c`,
`ftp.c`, `signals.c` — **fichiers qui n'existent pas**. Les fichiers réels sont :
`main.c`, `parsing.c`, `poisoning.c`, `sniffing.c`, `signals.c`, `utils.c`.
```
Correction : réécrire la section « Project structure » avec les vrais fichiers
             et un commentaire par fichier (parsing = args+IPv4+MAC+interface, etc.).
```

## D.2 Exemples d'affichage
Le README montre `[FTP] STOR ...` / `[FTP] RETR ...` : à valider une fois le
Module A fait, pour que le format documenté = le format réellement produit.

## D.3 `docker-compose.yaml` vs `.yml`
Le README parle de `docker-compose.yaml` ; le fichier réel est `docker-compose.yml`.
Harmoniser le nom cité (ou le fichier).

## D.4 Validation
Relecture croisée README ↔ `ls src/` ↔ sortie réelle de `make run`.

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
