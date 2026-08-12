"""ARP poisoning / restore / frame-field tests.

Shared plumbing lives in helpers.py (lab constants, arp cache, start/stop, ...).
"""
import time
import unittest

from helpers import (
    ATTACKER_MAC, SERVER_IP, SERVER_MAC, CLIENT_IP, CLIENT_MAC, INQUISITOR_ARGS,
    compose_exec, get_arp_mac, flush_arp_caches, populate_arp_caches,
    start_inquisitor, sigint_inquisitor, stop_inquisitor, lab_is_up,
)


# ── ARP poisoning tests ────────────────────────────────────────────────────────
@unittest.skipUnless(lab_is_up(), "Docker lab is not running")
class TestARPPoisoning(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        flush_arp_caches()
        populate_arp_caches()    # real entries first — no inquisitor yet
        cls.proc = start_inquisitor()
        time.sleep(3)            # inquisitor overwrites entries with attacker MAC

    @classmethod
    def tearDownClass(cls):
        stop_inquisitor(cls.proc)
        time.sleep(1)   # let tables restore before next class

    def test_client_sees_attacker_for_server(self):
        """Client ARP cache: server IP must point to attacker MAC."""
        mac = get_arp_mac("client", SERVER_IP)
        self.assertEqual(mac, ATTACKER_MAC,
            f"client arp: {SERVER_IP} -> {mac} (expected {ATTACKER_MAC})")

    def test_server_sees_attacker_for_client(self):
        """Server ARP cache: client IP must point to attacker MAC."""
        mac = get_arp_mac("server", CLIENT_IP)
        self.assertEqual(mac, ATTACKER_MAC,
            f"server arp: {CLIENT_IP} -> {mac} (expected {ATTACKER_MAC})")

    def test_poison_persists_after_5s(self):
        """Poison must hold after 5 s (re-poisoning loop active)."""
        time.sleep(5)
        self.assertEqual(get_arp_mac("client", SERVER_IP), ATTACKER_MAC)
        self.assertEqual(get_arp_mac("server", CLIENT_IP), ATTACKER_MAC)

    def test_forwarding_ping_works(self):
        """Ping client -> server must succeed (ip_forward=1).
        Validated manually via make run + ping; skipped in CI (Mac Docker limits)."""
        self.skipTest("validated manually — Mac Docker Desktop ICMP forwarding unreliable in exec")


# ── ARP restore tests ─────────────────────────────────────────────────────────
@unittest.skipUnless(lab_is_up(), "Docker lab is not running")
class TestARPRestore(unittest.TestCase):

    def setUp(self):
        flush_arp_caches()
        populate_arp_caches()    # real entries first — no inquisitor yet
        self.proc = start_inquisitor()
        time.sleep(3)            # inquisitor overwrites entries with attacker MAC

    def tearDown(self):
        stop_inquisitor(self.proc)
        time.sleep(1)

    def test_restore_client_after_sigint(self):
        """After SIGINT, client ARP cache must show server's real MAC.

        Wait for the process to fully exit (restore runs during shutdown, ~5 s)
        before reading the cache, so the check is not racing the restore frames.
        """
        self.assertEqual(get_arp_mac("client", SERVER_IP), ATTACKER_MAC)
        sigint_inquisitor()
        self.proc.wait(timeout=12)   # restore completes before exit
        time.sleep(1)
        mac = get_arp_mac("client", SERVER_IP)
        self.assertEqual(mac, SERVER_MAC,
            f"client non restauré: {mac} (attendu {SERVER_MAC})")

    def test_restore_server_after_sigint(self):
        """After SIGINT, server ARP cache must show client's real MAC."""
        self.assertEqual(get_arp_mac("server", CLIENT_IP), ATTACKER_MAC)
        sigint_inquisitor()
        self.proc.wait(timeout=12)
        time.sleep(1)
        mac = get_arp_mac("server", CLIENT_IP)
        self.assertEqual(mac, CLIENT_MAC,
            f"server non restauré: {mac} (attendu {CLIENT_MAC})")

    def test_exit_code_zero_after_sigint(self):
        """inquisitor must exit with code 0 after SIGINT (restore takes ~5 s)."""
        sigint_inquisitor()
        code = self.proc.wait(timeout=12)
        self.assertEqual(code, 0, f"exit code = {code}")


# ── frame field tests (needs dump_frame compiled in attacker) ─────────────────
@unittest.skipUnless(lab_is_up(), "Docker lab is not running")
class TestARPFrameFields(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        compose_exec("attacker",
            ["cc", "-Iinclude", "-Ilibft/include",
             "tests/dump_frame.c", "-o", "dump_frame"],
            timeout=10)

    def _get_frame(self, args):
        stdout, _, code = compose_exec(
            "attacker", ["./dump_frame"] + args, timeout=5)
        if code != 0:
            self.skipTest("dump_frame not available")
        return bytes.fromhex(stdout.strip())

    def test_ethertype_is_arp(self):
        self.assertEqual(self._get_frame(INQUISITOR_ARGS)[12:14], b'\x08\x06')

    def test_arp_opcode_is_reply(self):
        self.assertEqual(self._get_frame(INQUISITOR_ARGS)[20:22], b'\x00\x02')

    def test_hardware_type_is_ethernet(self):
        self.assertEqual(self._get_frame(INQUISITOR_ARGS)[14:16], b'\x00\x01')

    def test_sender_mac_is_attacker(self):
        frame = self._get_frame(INQUISITOR_ARGS)
        mac = ":".join(f"{b:02x}" for b in frame[22:28])
        self.assertEqual(mac, ATTACKER_MAC)

    def test_sender_ip_is_spoofed(self):
        frame = self._get_frame(INQUISITOR_ARGS)
        ip = ".".join(str(b) for b in frame[28:32])
        self.assertEqual(ip, SERVER_IP)


if __name__ == "__main__":
    unittest.main()
