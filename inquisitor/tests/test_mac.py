import unittest
from helpers import run

VALID_IP_SRC  = "192.168.0.2"
VALID_IP_DST  = "192.168.0.3"
VALID_MAC_SRC = "02:42:c0:a8:00:02"
VALID_MAC_DST = "02:42:c0:a8:00:03"

def args(mac_src=VALID_MAC_SRC, mac_dst=VALID_MAC_DST):
    return [VALID_IP_SRC, mac_src, VALID_IP_DST, mac_dst]


class TestMACValid(unittest.TestCase):

    def test_standard(self):
        code, _, _ = run(args())
        self.assertEqual(code, 0)

    def test_broadcast(self):
        code, _, _ = run(args(mac_src="ff:ff:ff:ff:ff:ff"))
        self.assertEqual(code, 0)

    def test_null_mac(self):
        code, _, _ = run(args(mac_src="00:00:00:00:00:00"))
        self.assertEqual(code, 0)

    def test_uppercase(self):
        code, _, _ = run(args(mac_src="AA:BB:CC:DD:EE:FF"))
        self.assertEqual(code, 0)

    def test_lowercase(self):
        code, _, _ = run(args(mac_src="aa:bb:cc:dd:ee:ff"))
        self.assertEqual(code, 0)

    def test_mixed(self):
        code, _, _ = run(args(mac_src="0a:1b:2c:3d:4e:5f"))
        self.assertEqual(code, 0)

    def test_valid_dst(self):
        code, _, _ = run(args(mac_dst="aa:bb:cc:dd:ee:ff"))
        self.assertEqual(code, 0)


class TestMACInvalid(unittest.TestCase):

    def test_src_5_octets(self):
        code, _, _ = run(args(mac_src="02:42:c0:a8:00"))
        self.assertNotEqual(code, 0)

    def test_src_7_octets(self):
        code, _, _ = run(args(mac_src="02:42:c0:a8:00:02:ff"))
        self.assertNotEqual(code, 0)

    def test_src_dash_separator(self):
        code, _, _ = run(args(mac_src="02-42-c0-a8-00-02"))
        self.assertNotEqual(code, 0)

    def test_src_no_separator(self):
        code, _, _ = run(args(mac_src="0242c0a80002"))
        self.assertNotEqual(code, 0)

    def test_src_invalid_chars(self):
        code, _, _ = run(args(mac_src="zz:zz:zz:zz:zz:zz"))
        self.assertNotEqual(code, 0)

    def test_src_single_digit_octet(self):
        code, _, _ = run(args(mac_src="02:42:c0:a8:00:2"))
        self.assertNotEqual(code, 0)

    def test_src_triple_digit_octet(self):
        code, _, _ = run(args(mac_src="02:42:c0:a8:00:002"))
        self.assertNotEqual(code, 0)

    def test_src_empty(self):
        code, _, _ = run(args(mac_src=""))
        self.assertNotEqual(code, 0)

    def test_src_out_of_range_hex(self):
        code, _, _ = run(args(mac_src="02:42:c0:a8:GG:02"))
        self.assertNotEqual(code, 0)

    def test_dst_5_octets(self):
        code, _, _ = run(args(mac_dst="02:42:c0:a8:00"))
        self.assertNotEqual(code, 0)

    def test_dst_invalid_chars(self):
        code, _, _ = run(args(mac_dst="zz:zz:zz:zz:zz:zz"))
        self.assertNotEqual(code, 0)

    def test_dst_empty(self):
        code, _, _ = run(args(mac_dst=""))
        self.assertNotEqual(code, 0)

if __name__ == "__main__":
    unittest.main()
