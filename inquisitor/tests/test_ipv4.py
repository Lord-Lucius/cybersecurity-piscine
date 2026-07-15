import unittest
from helpers import run

VALID_MAC_SRC  = "02:42:c0:a8:00:02"
VALID_MAC_DST  = "02:42:c0:a8:00:03"
VALID_IP_SRC   = "192.168.0.2"
VALID_IP_DST   = "192.168.0.3"

def args(ip_src=VALID_IP_SRC, ip_dst=VALID_IP_DST):
    return [ip_src, VALID_MAC_SRC, ip_dst, VALID_MAC_DST]


class TestIPv4Valid(unittest.TestCase):

    def test_standard(self):
        code, _, _ = run(args())
        self.assertEqual(code, 0)

    def test_class_a(self):
        code, _, _ = run(args(ip_src="10.0.0.1"))
        self.assertEqual(code, 0)

    def test_all_zeros(self):
        code, _, _ = run(args(ip_src="0.0.0.0"))
        self.assertEqual(code, 0)

    def test_broadcast(self):
        code, _, _ = run(args(ip_src="255.255.255.255"))
        self.assertEqual(code, 0)

    def test_loopback(self):
        code, _, _ = run(args(ip_src="127.0.0.1"))
        self.assertEqual(code, 0)

    def test_single_digit_octets(self):
        code, _, _ = run(args(ip_src="1.2.3.4"))
        self.assertEqual(code, 0)

    def test_valid_target(self):
        code, _, _ = run(args(ip_dst="10.0.0.2"))
        self.assertEqual(code, 0)


class TestIPv4Invalid(unittest.TestCase):

    def test_src_missing_octet(self):
        code, _, _ = run(args(ip_src="192.168.0"))
        self.assertNotEqual(code, 0)

    def test_src_extra_octet(self):
        code, _, _ = run(args(ip_src="192.168.0.2.5"))
        self.assertNotEqual(code, 0)

    def test_src_missing_dot(self):
        code, _, _ = run(args(ip_src="192168.0.2"))
        self.assertNotEqual(code, 0)

    def test_src_octet_overflow(self):
        code, _, _ = run(args(ip_src="999.999.999.999"))
        self.assertNotEqual(code, 0)

    def test_src_first_octet_overflow(self):
        code, _, _ = run(args(ip_src="256.0.0.1"))
        self.assertNotEqual(code, 0)

    def test_src_last_octet_overflow(self):
        code, _, _ = run(args(ip_src="192.168.0.256"))
        self.assertNotEqual(code, 0)

    def test_src_alpha(self):
        code, _, _ = run(args(ip_src="abc.def.ghi.jkl"))
        self.assertNotEqual(code, 0)

    def test_src_empty(self):
        code, _, _ = run(args(ip_src=""))
        self.assertNotEqual(code, 0)

    def test_src_space_in_octet(self):
        code, _, _ = run(args(ip_src="192.168. .1"))
        self.assertNotEqual(code, 0)

    def test_src_ipv6(self):
        code, _, _ = run(args(ip_src="fe80::1"))
        self.assertNotEqual(code, 0)

    def test_src_cidr(self):
        code, _, _ = run(args(ip_src="192.168.0.1/24"))
        self.assertNotEqual(code, 0)

    def test_dst_missing_octet(self):
        code, _, _ = run(args(ip_dst="192.168.0"))
        self.assertNotEqual(code, 0)

    def test_dst_octet_overflow(self):
        code, _, _ = run(args(ip_dst="256.0.0.1"))
        self.assertNotEqual(code, 0)

    def test_dst_ipv6(self):
        code, _, _ = run(args(ip_dst="::1"))
        self.assertNotEqual(code, 0)

    def test_dst_empty(self):
        code, _, _ = run(args(ip_dst=""))
        self.assertNotEqual(code, 0)

if __name__ == "__main__":
    unittest.main()
