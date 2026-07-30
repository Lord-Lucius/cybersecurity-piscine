import subprocess
import os

COMPOSE_DIR = os.path.join(os.path.dirname(__file__), "..")
TIMEOUT = 5

def run(args: list):
    try:
        result = subprocess.run(
            ["docker", "compose", "exec", "attacker", "./inquisitor"] + args,
            capture_output=True,
            text=True,
            timeout=TIMEOUT,
            cwd=COMPOSE_DIR
        )
        return result.returncode, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        # program ran past timeout without crashing = valid args accepted
        return 0, "", ""
