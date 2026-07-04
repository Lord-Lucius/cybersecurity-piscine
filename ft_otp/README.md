<div align="center">

# ft_otp

A C implementation of a **TOTP** generator: stores a master key encrypted,
then produces an ephemeral **6-digit password** that rotates every 30 seconds —
the mechanism behind apps like Google Authenticator.

`C` · `OpenSSL` · `HMAC-SHA1` · `AES-256-CBC`

</div>

---

## Overview

The program is built around two cooperating modes and one strict rule: the
master key is **never written in plaintext**.

- **`-g` (generate):** reads a hexadecimal key, encrypts it with AES-256-CBC and
  stores it in `ft_otp.key`.
- **`-k` (key):** decrypts the stored key and prints a temporary 6-digit password
  derived from the current time.

The algorithm is implemented by hand following RFC
[4226](https://datatracker.ietf.org/doc/html/rfc4226) (HOTP) and RFC
[6238](https://datatracker.ietf.org/doc/html/rfc6238) (TOTP); HMAC and SHA1 are
used only as primitives. The output matches the reference tool `oathtool`.

---

## Why these choices

The subject leaves the language and encryption scheme open, as long as they are
justified.

- **C + OpenSSL** — `libcrypto` provides HMAC-SHA1 and AES directly, keeping the
  work focused on the protocol logic rather than reinventing cryptography.
- **AES-256-CBC** — a well-documented, widely-supported symmetric cipher; the IV
  guarantees that encrypting the same key twice yields different ciphertext.
- **Hand-written HOTP/TOTP** — the time counter, HMAC call, dynamic truncation
  and formatting are all explicit, as required (no TOTP library).

---

## Project structure

```
ft_otp/
├── Makefile               # builds ft_otp, links libcrypto, cross-platform OpenSSL detection
├── include/
│   └── ft_otp.h           # params_t struct + prototypes
└── src/
    ├── main.c             # entry point: -g / -k dispatch
    ├── parsing.c          # argument parsing + key validation
    ├── storage.c          # encryption / decryption of ft_otp.key
    ├── totp.c             # HOTP/TOTP core
    └── utils.c            # error handling + helpers
```

> The encrypted key file `ft_otp.key` is generated at runtime by `-g`; it is not
> part of the repository.

---

## Prerequisites

- A C compiler and `make`
- **OpenSSL** development headers:
  - Debian/Ubuntu: `sudo apt install libssl-dev`
  - Fedora: `sudo dnf install openssl-devel`
  - macOS: `brew install openssl@3` *(the Makefile detects the Homebrew path)*
- `oathtool` *(recommended)* to cross-check generated codes

---

## Build & run

```sh
make
```

Available targets:

| Target | Effect |
|---|---|
| `make` | Build the `ft_otp` executable. |
| `make clean` | Remove object files. |
| `make fclean` | Remove objects + binary. |
| `make re` | Full rebuild. |
| `make debug` | Build with `-g3` + AddressSanitizer + UBSan. |

---

## Usage

**1. Prepare a key** — at least 64 hexadecimal characters (32 bytes):

```sh
xxd -l 32 -p /dev/urandom | tr -d '\n' > key.hex
```

**2. Store it (encrypted):**

```sh
$ ./ft_otp -g key.hex
Key was successfully saved in ft_otp.key.
```

A `cat ft_otp.key` now shows only unreadable bytes — the key never appears in
plaintext.

**3. Generate a password:**

```sh
$ ./ft_otp -k ft_otp.key
836492
$ sleep 30
$ ./ft_otp -k ft_otp.key
123518
```

The code changes with every new 30-second time step.

---

## How it works

The `-k` pipeline turns the stored key and the current time into six digits:

```
ft_otp.key ─┬─► decryption (AES-256-CBC)
            │        │
            │        ▼
            │   hexadecimal key ──► hex decoding ──► raw key bytes ─┐
            │                                                       ├─► HMAC-SHA1
   current time / 30 ──────────────► 8 bytes (big-endian) ─────────┘      │
                                                                          ▼
                                              dynamic truncation ──► 6 digits
```

| Step | What happens |
|---|---|
| Hex decoding | the text key becomes raw bytes |
| Time counter | `unix_time / 30`, encoded as 8 big-endian bytes |
| HMAC-SHA1 | combines key and counter into 20 bytes |
| Dynamic truncation | RFC 4226 §5.3 — extracts a 31-bit integer |
| Formatting | `integer % 1 000 000`, printed as 6 digits with leading zeros |

---

## Testing it live

Every command below is reproducible. Because TOTP codes depend on the current
time, don't expect a fixed number — the meaningful check is that **`ft_otp` and
`oathtool` always print the same code at the same instant**.

### Correctness against the reference tool

Use a fixed example key so both tools work from the same secret:

```sh
echo -n "3132333435363738393031323334353637383930313233343536373839303132" > key.hex
./ft_otp -g key.hex
oathtool --totp $(cat key.hex)
./ft_otp -k ft_otp.key
```

The last two commands print an identical 6-digit code. Wait for a new 30-second
window and they roll over together:

```sh
sleep 30
oathtool --totp $(cat key.hex)
./ft_otp -k ft_otp.key
```

### The key is stored encrypted

```sh
./ft_otp -g key.hex
cat ft_otp.key          # unreadable binary — the hex key never appears
```

### Error handling (nothing should crash)

| Command | Expected |
|---|---|
| `./ft_otp` | usage error, exit code ≠ 0 |
| `./ft_otp -x key.hex` | unknown option error |
| `./ft_otp -g missing.hex` | file-not-found error |
| `echo -n "tooshort" > s.hex && ./ft_otp -g s.hex` | length error |
| `echo -n "zzzz...(64 chars)" > bad.hex && ./ft_otp -g bad.hex` | non-hex error |
| `./ft_otp -k missing.key` | file-not-found error |
| `head -c 10 ft_otp.key > bad.key && ./ft_otp -k bad.key` | corrupted-file error |

Each prints a clear `ERROR:` message and exits cleanly — no segfault.

---

## Encryption

The master key is protected with **AES-256-CBC** via the OpenSSL EVP API. A
random IV is generated on every store and written at the head of the file:

```
ft_otp.key  =  [ IV: 16 bytes ][ encrypted key ]
```

The IV is not secret (it only ensures ciphertext uniqueness); the encryption key
is embedded in the program.

> **Acknowledged limitation.** Because the encryption key is embedded in the
> binary, security relies on the confidentiality of the executable. A hardened
> version would derive the key from a user-supplied passphrase (PBKDF2) instead
> of hardcoding it.

---

## Requirements met

| Requirement | Status |
|---|---|
| Executable named `ft_otp`, rebuilt via `make` | ✓ |
| HOTP/TOTP implemented by hand (no TOTP library) | ✓ |
| Key validation: ≥ 64 chars, hexadecimal, even length | ✓ |
| Key stored **encrypted**, never in plaintext | ✓ |
| Constant 6-digit output with leading zeros | ✓ |
| Output matches `oathtool` | ✓ |

---

## Resources

- [RFC 4226 — HOTP](https://datatracker.ietf.org/doc/html/rfc4226)
- [RFC 6238 — TOTP](https://datatracker.ietf.org/doc/html/rfc6238)
- [OpenSSL EVP documentation](https://docs.openssl.org/3.0/man3/EVP_EncryptInit/)
- [oathtool](https://www.nongnu.org/oath-toolkit/man-oathtool.html)
