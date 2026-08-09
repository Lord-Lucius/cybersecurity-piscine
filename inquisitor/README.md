# inquisitor

> ARP poisoning MITM tool — Cybersecurity Piscine @ 42

Inquisitor performs a full-duplex ARP poisoning attack between two hosts on the same
LAN, placing itself transparently in the middle of their traffic. While the attack is
running, it sniffs the FTP control channel and prints the names of files being
transferred in real time. On exit (`Ctrl-C`) the ARP tables of both victims are
restored to their original state.

---

## Project status

| Feature | State |
|---|---|
| Argument parsing & IPv4 / MAC validation | ✅ done |
| Local interface / MAC / ifindex discovery | ✅ done |
| Full-duplex ARP poisoning | ✅ done |
| ARP restore on `Ctrl-C` (exit 0) | ✅ done |
| Docker lab (`make up`) | ✅ done |
| **FTP sniffing — filename display** | 🔶 **work in progress** (interface declared in `sniffing.h`, not yet implemented) |
| FTP-specific test suite | ❌ to do |
| `-v` verbose mode (bonus) | ❌ to do |

> The ARP layer is complete and tested. The FTP sniffing described below is the
> intended behaviour and is **not functional yet** — the `[FTP] STOR/RETR ...`
> output shown in the walkthrough is the target format, not current output.

---

## Requirements

| Dependency | Purpose |
|---|---|
| `cc` (gcc / clang) | C compiler |
| `libpcap-dev` | packet capture / injection |
| `Docker` + `docker compose` | lab environment |

Install locally (Debian / Ubuntu):

```sh
sudo apt install build-essential libpcap-dev
```

---

## Usage

```
./inquisitor <IP-src> <MAC-src> <IP-target> <MAC-target> [-v]
```

| Argument | Description |
|---|---|
| `IP-src` | IP address of the first victim (e.g. the FTP server) |
| `MAC-src` | MAC address of the first victim |
| `IP-target` | IP address of the second victim (e.g. the FTP client) |
| `MAC-target` | MAC address of the second victim |
| `-v` | *(bonus)* verbose mode — prints all FTP control traffic, including login |

> Must be run as **root** (raw socket / AF_PACKET). IPv4 only.

---

## Quick start — Docker lab

The lab spins up three containers on a shared bridge network (`192.168.0.0/24`):

| Container | Role | IP | MAC |
|---|---|---|---|
| `server` | FTP server (vsftpd, plaintext) | `192.168.0.2` | `02:42:c0:a8:00:02` |
| `client` | FTP victim | `192.168.0.3` | `02:42:c0:a8:00:03` |
| `attacker` | runs `inquisitor` | `192.168.0.4` | `02:42:c0:a8:00:04` |

```sh
# 1 — build image + start lab + compile the binary inside the container
make up

# 2 — [terminal 1] start the attack
make run
# equivalent to:
# docker compose exec attacker ./inquisitor \
#   192.168.0.2 02:42:c0:a8:00:02 \
#   192.168.0.3 02:42:c0:a8:00:03

# 3 — [terminal 2] trigger FTP transfers from the client
make test

# 4 — stop the lab
make down
```

---

## Manual test walkthrough

### Step 1 — verify the poisoning

Before running `inquisitor`, check the ARP cache of the client:

```sh
docker compose exec client arp -n
# 192.168.0.2 -> 02:42:c0:a8:00:02   (real MAC of the server)
```

Start `inquisitor` (`make run`), then check again:

```sh
docker compose exec client arp -n
# 192.168.0.2 -> 02:42:c0:a8:00:04   (now points to the ATTACKER)
```

### Step 2 — FTP file transfer

While `inquisitor` is running, open a second terminal and upload/download a file:

```sh
docker compose exec client sh
# inside the client container:
lftp -u test,1234 192.168.0.2
lftp test@192.168.0.2:~> put /etc/hostname -o uploaded.txt
lftp test@192.168.0.2:~> get hello.txt -o /tmp/dl.txt
lftp test@192.168.0.2:~> bye
```

Expected output in the `inquisitor` terminal:

```
[FTP] STOR uploaded.txt
[FTP] RETR hello.txt
```

With `-v` (verbose / bonus):

```
[FTP] USER test
[FTP] PASS 1234
[FTP] STOR uploaded.txt
[FTP] RETR hello.txt
```

### Step 3 — verify the restore

Press `Ctrl-C` in the `inquisitor` terminal, then check the client's ARP cache:

```sh
docker compose exec client arp -n
# 192.168.0.2 -> 02:42:c0:a8:00:02   (back to the real MAC)
```

---

## Makefile targets

```sh
make          # build the binary locally (needs libpcap-dev)
make debug    # local build with -fsanitize=address,undefined
make re       # clean local rebuild
make up       # start the Docker lab + compile inside the attacker
make run      # launch inquisitor with lab defaults
make test     # trigger an FTP upload/download from the client
make logs     # follow container logs
make shell    # open a shell in the attacker container
make down     # stop the lab
make lab-clean# stop lab + remove volumes
make help     # list every target
```

---

## Project structure

```
.
├── Makefile                    # local build + Docker lab orchestration
├── docker-compose.yml          # 3-container lab (server / client / attacker)
├── attacker/
│   └── Dockerfile              # attacker image (cc + libpcap-dev + FTP tools)
├── data/                       # FTP server root (bind-mounted)
│   └── hello.txt
├── libft/                      # 42 utility library (static, linked into the binary)
├── include/
│   ├── inquisitor.h            # t_config + shared types
│   ├── parsing.h               # arg parsing / validation / interface discovery
│   ├── poisoning.h             # ARP frame structs + poisoning prototypes
│   ├── signals.h               # SIGINT handler
│   ├── sniffing.h              # libpcap sniffer interface (WIP)
│   └── utils.h                 # error / usage / cleanup
├── src/
│   ├── main.c                  # entry point: setup → poison loop → restore
│   ├── parsing.c               # 4-arg parsing, IPv4/MAC validation, local IF discovery
│   ├── poisoning.c             # ARP reply build + AF_PACKET raw inject + restore
│   ├── signals.c               # SIGINT → graceful stop (g_running)
│   ├── sniffing.c              # libpcap FTP capture (WIP — not yet implemented)
│   └── utils.c                 # error handling, usage, resource cleanup
└── tests/
    ├── helpers.py              # shared test helpers (compose exec, ARP cache)
    ├── test_args.py            # argument-count validation
    ├── test_ipv4.py            # IPv4 parser
    ├── test_mac.py             # MAC parser
    ├── test_poisoning.py       # ARP overwrite / restore / frame fields
    ├── dump_frame.c            # helper to dump a captured ARP frame
    └── run_all.py              # runs the whole Python suite
```

> Note: parsing, interface discovery and validation all live in `parsing.c`
> (there is no separate `args.c` / `netinfo.c`); ARP build, raw injection and
> restore all live in `poisoning.c`. FTP payload parsing will live in
> `sniffing.c` once implemented.
