import subprocess


def run(args: list):
    result = subprocess.run(["../inquisitor"] + args, capture_output=True, text=True, timeout=2)
    return result.returncode, result.stdout, result.stderr
