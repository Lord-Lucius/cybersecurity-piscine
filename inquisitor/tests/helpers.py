"""Shared test helpers for the inquisitor suite.

Single source of truth for: lab constants, docker-compose plumbing, ARP cache
inspection, launching/stopping inquisitor, and streaming its stdout (FTP tests).
Imported by test_args / test_ipv4 / test_mac / test_poisoning / test_ftp.
"""
import os
import select
import subprocess
import time

# ── paths / timeouts ────────────────────────────────────────────────────────────
COMPOSE_DIR = os.path.join(os.path.dirname(__file__), "..")
RUN_TIMEOUT = 3          # seconds a "valid args" run is allowed before we treat it as accepted
EXEC_TIMEOUT = 5         # default docker-compose-exec timeout

# ── lab constants (see docker-compose.yml) ──────────────────────────────────────
ATTACKER_MAC = "02:42:c0:a8:00:04"
SERVER_IP    = "192.168.0.2"
SERVER_MAC   = "02:42:c0:a8:00:02"
CLIENT_IP    = "192.168.0.3"
CLIENT_MAC   = "02:42:c0:a8:00:03"
INQUISITOR_ARGS = [SERVER_IP, SERVER_MAC, CLIENT_IP, CLIENT_MAC]

FTP_USER = "test"
FTP_PASS = "1234"


# ── low-level docker plumbing ───────────────────────────────────────────────────
def compose_exec(container, cmd, timeout=EXEC_TIMEOUT):
    """Run a command in a lab container. Returns (stdout, stderr, returncode)."""
    result = subprocess.run(
        ["docker", "compose", "exec", "-T", container] + cmd,
        capture_output=True, text=True, timeout=timeout,
        cwd=COMPOSE_DIR,
    )
    return result.stdout, result.stderr, result.returncode


def lab_is_up():
    """True when the attacker container is running (tests otherwise skip)."""
    result = subprocess.run(
        ["docker", "compose", "ps", "--services", "--filter", "status=running"],
        capture_output=True, text=True, cwd=COMPOSE_DIR,
    )
    return "attacker" in result.stdout


# ── argument-validation harness (test_args / test_ipv4 / test_mac) ──────────────
def run(args, timeout=RUN_TIMEOUT):
    """Run inquisitor with `args` in the attacker container.

    Invalid args -> inquisitor exits fast with its real return code.
    Valid args   -> it runs forever; hitting the timeout means "accepted", so we
                    SIGINT the orphaned in-container process (clean ARP restore)
                    and report success.
    """
    try:
        result = subprocess.run(
            ["docker", "compose", "exec", "-T", "attacker", "./inquisitor"] + args,
            capture_output=True, text=True, timeout=timeout, cwd=COMPOSE_DIR,
        )
        return result.returncode, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        # ran past the timeout without crashing = valid args accepted.
        # kill the orphan left running inside the container so it does not keep
        # poisoning the lab across tests.
        sigint_inquisitor()
        return 0, "", ""


# ── ARP cache inspection ────────────────────────────────────────────────────────
def get_arp_mac(container, ip):
    """Parse `arp -n` — handles both Alpine (BSD) and Debian (table) formats."""
    stdout, _, _ = compose_exec(container, ["arp", "-n"])
    for line in stdout.splitlines():
        if ip not in line:
            continue
        parts = line.split()
        # Alpine/BSD: ? (IP) at MAC [ether] on eth0
        if "at" in parts:
            idx = parts.index("at")
            if idx + 1 < len(parts):
                mac = parts[idx + 1]
                if mac not in ("(incomplete)", "<incomplete>"):
                    return mac
        # Debian table: IP  ether  MAC  C  Iface
        elif len(parts) >= 3 and parts[0] == ip:
            if parts[2] not in ("(incomplete)", "<incomplete>", "HWaddress"):
                return parts[2]
    return None


def flush_arp_caches():
    """Flush ARP caches on both victims to start from a clean state."""
    compose_exec("client", ["sh", "-c", f"arp -d {SERVER_IP} 2>/dev/null; true"])
    compose_exec("server", ["sh", "-c", f"arp -d {CLIENT_IP} 2>/dev/null; true"])


def populate_arp_caches():
    """Trigger real ARP resolution BEFORE inquisitor starts.
    Creates dynamic entries — the kernel then accepts gratuitous ARP overwrites."""
    for src, dst in (("client", SERVER_IP), ("server", CLIENT_IP)):
        try:
            compose_exec(src, ["ping", "-c", "1", "-W", "2", dst], timeout=8)
        except Exception:
            pass
    time.sleep(0.5)


# ── launching / stopping inquisitor ─────────────────────────────────────────────
def start_inquisitor(args=None, capture=False):
    """Launch inquisitor in the attacker container.

    capture=False -> stdout discarded (ARP tests).
    capture=True  -> stdout piped, line-buffered, for read_stdout_until (FTP tests).
    """
    if args is None:
        args = INQUISITOR_ARGS
    return subprocess.Popen(
        ["docker", "compose", "exec", "-T", "attacker", "./inquisitor"] + args,
        stdout=(subprocess.PIPE if capture else subprocess.DEVNULL),
        stderr=subprocess.DEVNULL,
        text=True, bufsize=1,
        cwd=COMPOSE_DIR,
    )


def sigint_inquisitor():
    """Send SIGINT to inquisitor inside the container (triggers ARP restore)."""
    compose_exec("attacker", ["pkill", "-INT", "-x", "inquisitor"])


def stop_inquisitor(proc):
    """Signal inquisitor properly, then wait for `docker compose exec` to exit
    and release its pipes (avoids ResourceWarning noise)."""
    sigint_inquisitor()
    try:
        proc.wait(timeout=4)
    except (subprocess.TimeoutExpired, ProcessLookupError):
        compose_exec("attacker", ["pkill", "-9", "-x", "inquisitor"])
        try:
            proc.kill()
            proc.wait(timeout=4)
        except Exception:
            pass
    finally:
        for stream in (proc.stdout, proc.stderr, proc.stdin):
            if stream:
                try:
                    stream.close()
                except Exception:
                    pass


# ── stdout streaming (FTP tests) ────────────────────────────────────────────────
def read_stdout_until(proc, needle, timeout=8):
    """Read inquisitor's piped stdout line by line until `needle` appears.

    The process keeps running, so we must NOT use communicate() (it would block
    until exit). We poll the pipe with select and stop as soon as we see needle
    or the timeout elapses. Returns everything captured so far.
    """
    deadline = time.time() + timeout
    captured = ""
    while time.time() < deadline:
        remaining = deadline - time.time()
        ready, _, _ = select.select([proc.stdout], [], [], min(0.3, max(0.0, remaining)))
        if not ready:
            continue
        line = proc.stdout.readline()
        if line == "":          # EOF: process ended
            break
        captured += line
        if needle in line:
            break
    return captured


# ── FTP helpers ─────────────────────────────────────────────────────────────────
def ensure_hello_file(name="hello.txt", content="inquisitor lab file\n"):
    """Ensure a file exists in the FTP server root (./data is bind-mounted to it),
    so RETR/get tests have something to download."""
    path = os.path.join(COMPOSE_DIR, "data", name)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if not os.path.exists(path):
        with open(path, "w") as f:
            f.write(content)
    return name


def lftp(script, host=None, timeout=15):
    """Run an lftp one-liner from the client container against the FTP server."""
    if host is None:
        host = SERVER_IP
    return compose_exec(
        "client",
        ["lftp", "-u", f"{FTP_USER},{FTP_PASS}", "-e", script, host],
        timeout=timeout,
    )
