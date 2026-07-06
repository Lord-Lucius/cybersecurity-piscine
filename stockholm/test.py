#!/usr/bin/env python3
"""
Stockholm test environment setup & test runner.
Usage:
    python3 setup_infection.py                      # Create ~/infection
    python3 setup_infection.py --reset              # Wipe and recreate ~/infection
    python3 setup_infection.py --check              # Verify state after manual run
    python3 setup_infection.py --clean              # Delete ~/infection entirely
    python3 setup_infection.py --test <key>         # Full automated test suite
    python3 setup_infection.py --test <key> --bin ./stockholm  # Custom binary path
"""

import os
import sys
import shutil
import hashlib
import argparse
import subprocess
from pathlib import Path

INFECTION_DIR = Path.home() / "infection"
DEFAULT_BIN   = "./stockholm"

# WannaCry targeted extensions (representative subset)
WANNACRY_FILES = [
    # Office
    "documents/report.doc",
    "documents/budget.xls",
    "documents/presentation.ppt",
    "documents/contract.docx",
    "documents/invoices.xlsx",
    "documents/slides.pptx",
    # PDFs
    "documents/manual.pdf",
    # Images
    "images/photo.jpg",
    "images/avatar.jpeg",
    "images/screenshot.png",
    "images/background.bmp",
    "images/animation.gif",
    # Archives
    "archives/backup.zip",
    "archives/sources.tar",
    "archives/logs.gz",
    # Databases
    "data/users.sql",
    "data/config.mdb",
    "data/store.sqlite3",
    # Source code
    "code/main.py",
    "code/app.js",
    "code/server.php",
    "code/lib.java",
    # Media
    "media/song.mp3",
    "media/video.mp4",
    # Nested subfolders
    "documents/legal/nda.doc",
    "documents/legal/terms.pdf",
    "code/src/utils.cpp",
    "code/src/parser.c",
]

# Non-WannaCry extensions — must NOT be touched
IGNORED_FILES = [
    "readme.md",
    "notes.txt",
    "debug.log",
    "config.ini",
    "data.csv",
]

GREEN  = "\033[92m"
RED    = "\033[91m"
YELLOW = "\033[93m"
CYAN   = "\033[96m"
BOLD   = "\033[1m"
RESET  = "\033[0m"

def ok(msg):    print(f"  {GREEN}✓{RESET} {msg}")
def fail(msg):  print(f"  {RED}✗{RESET} {msg}")
def info(msg):  print(f"  {CYAN}→{RESET} {msg}")
def warn(msg):  print(f"  {YELLOW}!{RESET} {msg}")
def header(msg): print(f"\n{BOLD}{CYAN}{msg}{RESET}\n")


# ─────────────────────────────────────────────────────────────
# Environment management
# ─────────────────────────────────────────────────────────────

def create_environment():
    """Create ~/infection with a realistic file tree."""
    if INFECTION_DIR.exists():
        warn(f"{INFECTION_DIR} already exists — use --reset to wipe and recreate.")
        return

    header(f"Creating test environment in {INFECTION_DIR}")

    for rel_path in WANNACRY_FILES:
        target = INFECTION_DIR / rel_path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(f"Fake content for {target.name} — WannaCry target\n")
        ok(f"Created {rel_path}")

    for rel_path in IGNORED_FILES:
        target = INFECTION_DIR / rel_path
        target.write_text(f"Fake content for {target.name} — should be ignored\n")
        ok(f"Created {rel_path}  (non-targeted)")

    print(f"\n{GREEN}Done.{RESET}")
    print(f"  WannaCry targets : {len(WANNACRY_FILES)} files")
    print(f"  Ignored files    : {len(IGNORED_FILES)} files\n")


def reset_environment():
    """Wipe ~/infection and recreate it from scratch."""
    if INFECTION_DIR.exists():
        shutil.rmtree(INFECTION_DIR)
        info("Wiped existing ~/infection")
    create_environment()


def clean_environment():
    """Delete ~/infection entirely."""
    if not INFECTION_DIR.exists():
        warn("~/infection does not exist, nothing to clean.")
        return
    shutil.rmtree(INFECTION_DIR)
    ok("Deleted ~/infection")


# ─────────────────────────────────────────────────────────────
# Hashing helpers (for content integrity check)
# ─────────────────────────────────────────────────────────────

def hash_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def snapshot(file_list) -> dict:
    """Return {relative_path_str: sha256} for a list of paths."""
    result = {}
    for rel in file_list:
        p = INFECTION_DIR / rel
        if p.exists():
            result[rel] = hash_file(p)
    return result


# ─────────────────────────────────────────────────────────────
# Manual check (post-encryption state)
# ─────────────────────────────────────────────────────────────

def check_state():
    if not INFECTION_DIR.exists():
        fail("~/infection does not exist. Run without arguments first.")
        sys.exit(1)

    header(f"Checking state of {INFECTION_DIR}")

    encrypted   = []
    not_done    = []
    wrong_hit   = []
    correct_ign = []

    all_files        = [p for p in INFECTION_DIR.rglob("*") if p.is_file()]
    expected_targets = {INFECTION_DIR / p for p in WANNACRY_FILES}
    expected_ignored = {INFECTION_DIR / p for p in IGNORED_FILES}

    for f in all_files:
        if f.suffix == ".ft":
            base = f.with_suffix("")
            if base in expected_targets:
                encrypted.append(f)
            else:
                wrong_hit.append(f)
        else:
            if f in expected_targets:
                not_done.append(f)
            elif f in expected_ignored:
                correct_ign.append(f)

    print(f"{GREEN}Correctly encrypted ({len(encrypted)}):{RESET}")
    for f in encrypted:
        ok(str(f.relative_to(INFECTION_DIR)))

    if not_done:
        print(f"\n{RED}Missed targets ({len(not_done)}):{RESET}")
        for f in not_done:
            fail(str(f.relative_to(INFECTION_DIR)))

    if wrong_hit:
        print(f"\n{RED}Wrong hits ({len(wrong_hit)}):{RESET}")
        for f in wrong_hit:
            fail(str(f.relative_to(INFECTION_DIR)))

    print(f"\n{GREEN}Correctly ignored ({len(correct_ign)}):{RESET}")
    for f in correct_ign:
        ok(str(f.relative_to(INFECTION_DIR)))

    print(f"""
{CYAN}Summary{RESET}
  Encrypted correctly  : {len(encrypted)}/{len(WANNACRY_FILES)}
  Missed targets       : {len(not_done)}
  Wrong hits           : {len(wrong_hit)}
  Ignored correctly    : {len(correct_ign)}/{len(IGNORED_FILES)}
""")

    passed = not not_done and not wrong_hit
    if passed:
        print(f"{GREEN}All checks passed.{RESET}\n")
    else:
        print(f"{RED}Some checks failed.{RESET}\n")
        sys.exit(1)


# ─────────────────────────────────────────────────────────────
# Run helper
# ─────────────────────────────────────────────────────────────

def run_stockholm(binary: str, args: list) -> subprocess.CompletedProcess:
    cmd = [binary] + args
    info(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.stdout:
        print(result.stdout, end="")
    if result.stderr:
        print(result.stderr, end="")
    return result


# ─────────────────────────────────────────────────────────────
# Automated test suite
# ─────────────────────────────────────────────────────────────

def run_tests(key: str, binary: str):
    failures = []

    # ── 0. Sanity checks ────────────────────────────────────
    header("0 / Sanity checks")

    if not Path(binary).exists():
        fail(f"Binary not found: {binary}")
        sys.exit(1)
    ok(f"Binary found: {binary}")

    if len(key) < 16:
        fail(f"Key too short ({len(key)} chars) — minimum is 16")
        sys.exit(1)
    ok(f"Key length OK ({len(key)} chars)")

    # ── 1. --help ───────────────────────────────────────────
    header("1 / --help")
    r = run_stockholm(binary, ["--help"])
    if r.returncode == 0:
        ok("--help exits 0")
    else:
        fail(f"--help exited {r.returncode}")
        failures.append("--help exit code")

    # ── 2. --version ────────────────────────────────────────
    header("2 / --version")
    r = run_stockholm(binary, ["--version"])
    if r.returncode == 0:
        ok("--version exits 0")
    else:
        fail(f"--version exited {r.returncode}")
        failures.append("--version exit code")

    # ── 3. --reverse without key ────────────────────────────
    header("3 / --reverse without key (should fail)")
    r = run_stockholm(binary, ["--reverse"])
    if r.returncode != 0:
        ok("--reverse without key exits non-zero")
    else:
        fail("--reverse without key exited 0 — should be an error")
        failures.append("--reverse without key")

    # ── 4. Fresh environment ─────────────────────────────────
    header("4 / Setup fresh environment")
    reset_environment()
    before_hashes = snapshot(WANNACRY_FILES)
    ok(f"Snapshot taken ({len(before_hashes)} files)")

    # ── 5. Encryption ────────────────────────────────────────
    header("5 / Encryption pass")
    r = run_stockholm(binary, [key])
    if r.returncode != 0:
        fail(f"stockholm exited {r.returncode} during encryption")
        failures.append("encryption exit code")
    else:
        ok("stockholm exited 0")

    enc_ok   = 0
    enc_fail = 0
    for rel in WANNACRY_FILES:
        encrypted_path = INFECTION_DIR / (rel + ".ft")
        plain_path     = INFECTION_DIR / rel
        if encrypted_path.exists() and not plain_path.exists():
            enc_ok += 1
        else:
            fail(f"Not encrypted: {rel}")
            enc_fail += 1
            failures.append(f"encrypt: {rel}")

    ign_ok   = 0
    ign_fail = 0
    for rel in IGNORED_FILES:
        p = INFECTION_DIR / rel
        if p.exists() and not (INFECTION_DIR / (rel + ".ft")).exists():
            ign_ok += 1
        else:
            fail(f"Ignored file was touched: {rel}")
            ign_fail += 1
            failures.append(f"wrongly encrypted: {rel}")

    ok(f"Encrypted correctly : {enc_ok}/{len(WANNACRY_FILES)}")
    ok(f"Ignored correctly   : {ign_ok}/{len(IGNORED_FILES)}")

    # ── 6. Idempotency — run encryption again ───────────────
    header("6 / Idempotency (encrypt twice)")
    r = run_stockholm(binary, [key])
    double_enc = 0
    for rel in WANNACRY_FILES:
        double_path = INFECTION_DIR / (rel + ".ft.ft")
        if double_path.exists():
            fail(f"Double-encrypted: {rel}.ft.ft")
            double_enc += 1
            failures.append(f"double encrypt: {rel}")
    if double_enc == 0:
        ok("No double-encryption detected")

    # ── 7. Decryption ────────────────────────────────────────
    header("7 / Decryption pass")
    r = run_stockholm(binary, ["--reverse", key])
    if r.returncode != 0:
        fail(f"stockholm --reverse exited {r.returncode}")
        failures.append("decryption exit code")
    else:
        ok("stockholm --reverse exited 0")

    dec_ok    = 0
    dec_fail  = 0
    integrity_ok   = 0
    integrity_fail = 0

    for rel in WANNACRY_FILES:
        plain_path     = INFECTION_DIR / rel
        encrypted_path = INFECTION_DIR / (rel + ".ft")

        if plain_path.exists() and not encrypted_path.exists():
            dec_ok += 1
            # Content integrity check
            after_hash = hash_file(plain_path)
            if before_hashes.get(rel) == after_hash:
                integrity_ok += 1
            else:
                fail(f"Content mismatch after decrypt: {rel}")
                integrity_fail += 1
                failures.append(f"integrity: {rel}")
        else:
            fail(f"Not decrypted: {rel}")
            dec_fail += 1
            failures.append(f"decrypt: {rel}")

    ok(f"Decrypted correctly : {dec_ok}/{len(WANNACRY_FILES)}")
    ok(f"Content integrity   : {integrity_ok}/{len(WANNACRY_FILES)}")

    # ── 8. Wrong key ─────────────────────────────────────────
    header("8 / Decryption with wrong key")
    reset_environment()
    run_stockholm(binary, [key])
    wrong_key = "WRONGKEYABCDEFGH"
    r = run_stockholm(binary, ["--reverse", wrong_key])
    info(f"stockholm --reverse <wrong_key> exited {r.returncode}")
    # We don't enforce a specific exit code here — just report behavior
    # A well-implemented program should either fail or produce garbage output

    # ── 9. --silent ──────────────────────────────────────────
    header("9 / --silent mode")
    reset_environment()
    r = run_stockholm(binary, [key, "--silent"])
    if r.returncode == 0 and not r.stdout.strip():
        ok("--silent produced no stdout output")
    elif r.returncode == 0:
        warn("--silent ran OK but produced stdout output")
    else:
        fail(f"--silent exited {r.returncode}")
        failures.append("--silent exit code")

    # ── Final report ─────────────────────────────────────────
    header("Test Report")
    if not failures:
        print(f"{GREEN}{BOLD}All tests passed.{RESET}\n")
    else:
        print(f"{RED}{BOLD}Failures ({len(failures)}):{RESET}")
        for f in failures:
            fail(f)
        print()
        sys.exit(1)


# ─────────────────────────────────────────────────────────────
# Entry point
# ─────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="Stockholm test environment manager & test runner"
    )
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--reset", action="store_true",  help="Wipe and recreate ~/infection")
    group.add_argument("--check", action="store_true",  help="Verify state after manual run")
    group.add_argument("--clean", action="store_true",  help="Delete ~/infection entirely")
    group.add_argument("--test",  metavar="KEY",        help="Run full automated test suite with KEY")
    parser.add_argument("--bin",  default=DEFAULT_BIN,  help=f"Path to stockholm binary (default: {DEFAULT_BIN})")
    args = parser.parse_args()

    if args.reset:
        reset_environment()
    elif args.check:
        check_state()
    elif args.clean:
        clean_environment()
    elif args.test:
        run_tests(args.test, args.bin)
    else:
        create_environment()


if __name__ == "__main__":
    main()
