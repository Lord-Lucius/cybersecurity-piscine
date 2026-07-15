import subprocess

BINARY = "../inquisitor"
TIMEOUT = 2

def run(args: list):
    result = subprocess.run(
        [BINARY] + args,
        capture_output=True,
        text=True,
        timeout=TIMEOUT
    )
    return result.returncode, result.stdout, result.stderr
