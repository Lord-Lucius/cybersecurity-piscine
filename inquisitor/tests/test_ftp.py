"""FTP sniffing tests (Module A / mandatory).

The subject requires a protocol-specific test suite: while inquisitor performs the
MITM, a file transfer on the FTP control channel must surface its filename in real
time as `[FTP] STOR <file>` / `[FTP] RETR <file>`.

Flow of each test:
  1. flush + populate the ARP caches, then start inquisitor (poison + sniffer),
     with stdout piped so we can read the [FTP] lines it prints.
  2. wait for the poison to settle (client<->server traffic now flows through us).
  3. trigger an lftp transfer from the client container.
  4. read inquisitor's stdout in stream (never communicate() — it runs until SIGINT)
     and assert the expected line appeared.

Requires the Docker lab to be up (make up); skips cleanly otherwise.
"""
import time
import unittest

from helpers import (
    lab_is_up, flush_arp_caches, populate_arp_caches,
    start_inquisitor, stop_inquisitor, sigint_inquisitor,
    read_stdout_until, lftp, ensure_hello_file,
)

# force plaintext FTP (the lab server is plaintext on port 21) so lftp does not
# waste time negotiating TLS before sending the command we want to sniff.
PLAIN = "set ftp:ssl-allow no;"


@unittest.skipUnless(lab_is_up(), "Docker lab is not running")
class TestFTPSniffing(unittest.TestCase):

    def setUp(self):
        ensure_hello_file()            # RETR needs a file to download server-side
        flush_arp_caches()
        populate_arp_caches()          # real entries first — no inquisitor yet
        self.proc = start_inquisitor(capture=True)
        time.sleep(3)                  # poison in place; traffic now relayed via us

    def tearDown(self):
        stop_inquisitor(self.proc)
        time.sleep(1)

    # ── mandatory: filenames on the control channel ─────────────────────────────
    def test_stor_filename_displayed(self):
        """A client `put` must surface as [FTP] STOR <file>."""
        lftp(PLAIN + "put /etc/hostname -o up.txt; bye")
        out = read_stdout_until(self.proc, "STOR", timeout=8)
        self.assertIn("STOR up.txt", out,
            f"'[FTP] STOR up.txt' not seen. Captured:\n{out}")

    def test_retr_filename_displayed(self):
        """A client `get` must surface as [FTP] RETR <file>."""
        lftp(PLAIN + "get hello.txt -o /tmp/dl.txt; bye")
        out = read_stdout_until(self.proc, "RETR", timeout=8)
        self.assertIn("RETR hello.txt", out,
            f"'[FTP] RETR hello.txt' not seen. Captured:\n{out}")

    def test_exit_zero_after_ftp_and_sigint(self):
        """Regression: after real sniffer activity, SIGINT must still exit 0.

        This specifically stresses the thread shutdown order (breakloop -> join ->
        close): a transfer drives pcap_loop before the CTRL+C, so a broken stop
        sequence would crash here even though test_poisoning's idle SIGINT passes.
        """
        lftp(PLAIN + "put /etc/hostname -o up.txt; bye")
        read_stdout_until(self.proc, "STOR", timeout=8)   # ensure the sniffer ran
        sigint_inquisitor()
        code = self.proc.wait(timeout=6)
        self.assertEqual(code, 0, f"exit code after FTP + SIGINT = {code}")


# ── bonus (Module E) — verbose -v shows the full control traffic, login included ─
@unittest.skip("bonus verbose -v not implemented yet (Module E)")
@unittest.skipUnless(lab_is_up(), "Docker lab is not running")
class TestFTPVerbose(unittest.TestCase):

    def setUp(self):
        flush_arp_caches()
        populate_arp_caches()
        self.proc = start_inquisitor(args=None, capture=True)  # would need the -v flag
        time.sleep(3)

    def tearDown(self):
        stop_inquisitor(self.proc)
        time.sleep(1)

    def test_verbose_shows_login(self):
        """With -v, USER/PASS of the login must be printed too."""
        lftp(PLAIN + "put /etc/hostname -o up.txt; bye")
        out = read_stdout_until(self.proc, "PASS", timeout=8)
        self.assertIn("USER test", out)
        self.assertIn("PASS 1234", out)


if __name__ == "__main__":
    unittest.main()
