# stockholm

A WannaCry-inspired ransomware simulation built for educational purposes as part of the 42 Cybersecurity Piscine.

---

## Overview

`stockholm` targets files in `~/infection` and encrypts them using AES-256-CBC (OpenSSL).
Only file extensions affected by the original WannaCry ransomware are targeted.
Encrypted files receive the `.ft` extension. The operation is fully reversible with the original key.

---

## Requirements

- Linux
- OpenSSL (`libssl-dev`)
- C++17 compiler

---

## Build

```
make
```

---

## Usage

```
./stockholm <key>                  Encrypt files in ~/infection
./stockholm --reverse <key>        Decrypt files in ~/infection
./stockholm --silent <key>         Encrypt without output
./stockholm --help                 Show help
./stockholm --version              Show version
```

---

## Options

| Option            | Description                          |
|-------------------|--------------------------------------|
| `-h`, `--help`    | Display usage information            |
| `-v`, `--version` | Display program version              |
| `-r`, `--reverse` | Decrypt files using the provided key |
| `-s`, `--silent`  | Suppress output during encryption    |

---

## Notes

> The key must be at least 16 characters long.
> The program only acts on `~/infection` and never outside of it.
> Running encryption twice on the same files has no effect (`.ft` extension is not re-applied).

---

## Disclaimer

This project is strictly educational. It was created as part of a cybersecurity curriculum
and must not be used outside of a controlled environment.
