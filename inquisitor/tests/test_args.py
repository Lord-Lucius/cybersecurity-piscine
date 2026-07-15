import unittest
from helpers import run

VALID = ["192.168.0.2", "02:42:c0:a8:00:02", "192.168.0.3", "02:42:c0:a8:00:03"]

class TestArgs(unittest.TestCase):

    def test_no_args(self):
        code, _, _ = run([])
        self.assertNotEqual(code, 0)

    def test_1_arg(self):
        code, _, _ = run(VALID[:1])
        self.assertNotEqual(code, 0)

    def test_2_args(self):
        code, _, _ = run(VALID[:2])
        self.assertNotEqual(code, 0)

    def test_3_args(self):
        code, _, _ = run(VALID[:3])
        self.assertNotEqual(code, 0)

    def test_4_args_valid(self):
        code, _, _ = run(VALID)
        self.assertEqual(code, 0)

    def test_5_args(self):
        code, _, _ = run(VALID + ["extra"])
        self.assertNotEqual(code, 0)

if __name__ == "__main__":
    unittest.main()
