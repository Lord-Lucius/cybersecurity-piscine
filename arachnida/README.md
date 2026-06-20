<div align="center">

# Arachnida

An introductory **web scraping & metadata** project — a recursive image
crawler and an EXIF metadata reader, built from scratch.

`Python` · `requests` · `BeautifulSoup` · `Pillow`

</div>

---

## Overview

Arachnida is made of two independent programs:

- **spider** recursively downloads every image from a website, following links
  up to a configurable depth while staying on the same domain.
- **scorpion** parses image files and displays their metadata — basic
  filesystem attributes and embedded EXIF data.

The logic of each program is implemented by hand: ready-made scraping tools
such as `wget` or `scrapy` are forbidden. Only HTTP and file-handling
libraries are allowed.

---

## Why Python

Python was chosen because it keeps the focus on the project's own logic rather
than on low-level plumbing:

- **HTTP & HTML** — `requests` and `BeautifulSoup` cover requests and parsing
  cleanly, which are explicitly the parts the subject allows libraries for.
- **EXIF** — `Pillow` exposes embedded metadata directly, so the work is about
  understanding and presenting it, not decoding binary headers by hand.
- **Familiarity** — already used in the Data Science track, so no syntax
  friction and faster iteration.

---

## Project structure

```
arachnida/
├── ex00/
│   └── spider.py        # recursive image web crawler
├── ex01/
│   └── scorpion.py      # image metadata (EXIF) reader
├── .gitignore           # excludes downloaded data, venv
└── README.md
```

> Each exercise is self-contained in its own folder, as required by the subject.

---

## Prerequisites

- `Python 3.x`
- The following libraries: `requests`, `beautifulsoup4`, `Pillow`

```sh
python3 -m venv .venv
source .venv/bin/activate
pip install requests beautifulsoup4 Pillow
```

---

## ex00 — Spider

Extracts and downloads all images from a website, optionally following links
recursively.

### Usage

```sh
./spider [-rlp] URL
```

| Option | Description | Default |
|---|---|---|
| `-r` | Recursively download images by following links | disabled |
| `-l [N]` | Maximum recursion depth (requires `-r`) | 5 |
| `-p [PATH]` | Directory where images are saved | `./data/` |

### Examples

```sh
# single page
python3 spider.py https://example.com

# recursive crawl, depth 2, custom output folder
python3 spider.py -r -l 2 -p ./downloads/ https://example.com
```

### How it works

1. Sends an HTTP GET request (with a `User-Agent` header).
2. Parses the HTML to extract `<img>` sources and `<a>` links.
3. Resolves relative URLs into absolute ones.
4. Downloads images with a supported extension **and** an `image/`
   Content-Type, ignoring disguised HTML responses.
5. In recursive mode, follows links that stay on the same domain, skips
   already-visited URLs, and strips fragments (`#...`) to avoid duplicates.

> Downloaded files are prefixed with a short hash of their source URL to avoid
> name collisions between images coming from different pages.

---

## ex01 — Scorpion

Receives image files as arguments and displays their metadata.

### Usage

```sh
./scorpion FILE1 [FILE2 ...]
```

### Example

```sh
python3 scorpion.py photo1.jpg photo2.png ../ex00/data/image.jpg
```

### How it works

1. Validates each file (existence and supported extension).
2. Reads basic filesystem metadata via `os.stat` — size, creation and
   modification dates.
3. Extracts EXIF data with Pillow and translates numeric tag codes into
   readable names.
4. Displays everything in a clean framed box, one file at a time.

> Files without EXIF data (common for PNG, GIF, BMP) are reported as
> "No EXIF data found" rather than treated as an error.

---

## Supported formats

Both programs handle the same image extensions:

| Format | Extensions |
|---|---|
| JPEG | `.jpg` `.jpeg` |
| PNG | `.png` |
| GIF | `.gif` |
| BMP | `.bmp` |

---

## Error handling

Neither program crashes on bad input:

- **spider** — handles unreachable URLs, non-200 responses, timeouts,
  non-image content, overly long filenames, and `Ctrl+C` cleanly.
- **scorpion** — skips invalid or non-image files and continues with the
  rest, reporting issues without stopping.

---

## Security model

Metadata is *data about data*. EXIF information embedded in an image can reveal
sensitive details about its author: exact capture date and time, device model,
editing software, and even GPS coordinates.

This makes metadata a common vector of information leakage in **OSINT**
(Open Source Intelligence) and a key consideration before publishing images
online. Scorpion demonstrates how easily this hidden information can be read.

---

## Submission

Turn in the work in the Git repository as usual. Only the work inside the
repository is evaluated during the defense. Double-check folder and file names.
