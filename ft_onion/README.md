<div align="center">

# ft_onion


A static web page served by Nginx, reachable **only through the Tor network**
as a v3 onion service — with SSH administration, and **zero exposed ports**.

`Tor` · `Nginx` · `OpenSSH` · `Docker`

</div>

---

## Overview

This project runs three daemons that cooperate inside a single container:

- **Nginx** serves a static `index.html` on `127.0.0.1:80`.
- **Tor** publishes a v3 hidden service and forwards the onion address to the
  local Nginx and SSH daemons.
- **OpenSSH** listens on port `4242` for administration.

Nothing is exposed to the host network. The service becomes reachable because
Tor makes **outbound** connections to the network (introduction points,
rendezvous) — not because any inbound port is opened. That is the whole point:
**no open ports, no firewall rules, no attack surface.**

---

## Why Docker

The subject allows any environment as long as it is justified. Docker was chosen
because it is:

- **Reproducible** — the exact same image builds and runs anywhere, so the
  evaluation does not depend on a machine the corrector has to trust.
- **Portable** — it runs identically across WSL, macOS and Fedora without
  requiring a permanent privileged setup.
- **Self-contained and pushable** — unlike a VM, the build recipe lives in the
  repository, so the corrector can rebuild everything from source.

A VM would also satisfy the subject, but it is not pushed and forces the
corrector to trust an environment they cannot inspect.

---

## Project structure

```
ft_onion/
├── Dockerfile              # builds the image: tor + nginx + openssh, perms, entrypoint
├── docker-compose.yml      # build + persistent volume for the onion keys
├── srcs/
│   ├── index.html          # the static page
│   ├── nginx.conf          # web server config (listens on 127.0.0.1:80)
│   ├── sshd_config         # SSH daemon config (Port 4242 + hardening)
│   └── torrc               # hidden service: exposes ports 80 and 4242 over Tor
├── tools/
│   └── entrypoint.sh       # starts sshd + nginx in background, tor in foreground
├── .gitignore              # excludes generated onion keys, logs
└── README.md
```

> The four files required by the subject — `index.html`, `nginx.conf`,
> `sshd_config`, `torrc` — live under `srcs/`.

---

## Prerequisites

- `docker` (and `docker compose`)
- A Tor client to reach the service: **Tor Browser**, or `tor` + `torsocks`
  on the host for command-line testing.

---

## Build & run

```sh
docker compose up --build
```

On first start, Tor generates the service keys and writes the onion address.

---

## Get the onion address

The address is written by Tor into the `hostname` file inside the
`HiddenServiceDir`:

```sh
docker compose exec <service> cat /var/lib/tor/hidden_service/hostname
```

The output is your `xxxxxxxx...onion` URL. It stays stable across restarts
because the key directory is mounted on a persistent volume.

---

## Testing

**Web — via Tor Browser:** open the `.onion` address. The page loads.

**Web — via CLI:**

```sh
torsocks curl http://<address>.onion
```

**SSH — over Tor (no port is opened on the host):**

```sh
torsocks ssh -p 4242 <user>@<address>.onion
```

**Local sanity checks (inside the container), to isolate service bugs from
Tor bugs:**

```sh
curl -v http://127.0.0.1        # should return your page, not the Nginx default
ss -tlnp                        # sshd should be listening on 4242
```

---

## Security model

- **No ports published, no firewall rules.** All services listen on localhost;
  only the local Tor daemon reaches them, and Tor itself only makes outbound
  connections.
- **SSH is reachable solely through the onion address**, never via a host port.
- **Onion private keys are never committed** — they are excluded in
  `.gitignore`. Leaking them would expose and let anyone impersonate the service.

### SSH hardening *(bonus)*

The SSH daemon is locked down: root login disabled, password authentication
disabled in favour of public-key authentication, login restricted to a single
dedicated user, and a non-default port (`4242`).

---

## Files submitted

| File | Purpose |
|---|---|
| `index.html` | the static page served |
| `nginx.conf` | web server configuration |
| `sshd_config` | SSH daemon configuration (port 4242) |
| `torrc` | Tor hidden service configuration |

Supporting files (`Dockerfile`, `docker-compose.yml`, `entrypoint.sh`) are
included to build and run the environment, as the subject requires the choices
to be justified and reproducible.
