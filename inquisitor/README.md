# inquisitor

> ARP poisoning MITM tool — Cybersecurity Piscine @ 42

Inquisitor performs a full-duplex ARP poisoning attack between two hosts on the same
LAN, placing itself transparently in the middle of their traffic. While the attack is
running, it sniffs the FTP control channel and prints the names of files being
transferred in real time. On exit (`Ctrl-C`) the ARP tables of both victims are
restored to their original state.

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
```

---

## Project structure

```
.
├── Makefile
├── docker-compose.yaml
├── attacker/
│   └── Dockerfile          # attacker image (cc + libpcap-dev)
├── data/                   # FTP server root (bind-mounted)
│   └── hello.txt
├── include/
│   └── inquisitor.h
└── src/
    ├── main.c
    ├── args.c              # parsing + validation of the 4 parameters
    ├── netinfo.c           # local interface / MAC discovery
    ├── arp.c               # ARP frame construction (poison + restore)
    ├── inject.c            # AF_PACKET raw socket + sendto
    ├── sniff.c             # libpcap open / BPF filter / capture loop
    ├── ftp.c               # TCP payload parser — extracts filenames
    └── signals.c           # SIGINT handler + graceful shutdown
```
