import unittest
from helpers import run


class TestIPv4(unittest.TestCase):
    def test_ip_valid(self):
        code, _, _ = run(["192.168.0.2", "02:42:c0:a8:00:02", "192.168.0.3", "02:42:c0:a8:00:03"])
        self.assertEqual(code, 0)
