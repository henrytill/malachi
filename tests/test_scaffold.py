"""Scaffold for future tests"""

import unittest


class TestScaffold(unittest.TestCase):
    """Throwaway tests"""

    def test_trivial_assertion(self) -> None:
        """Zero equals zero"""
        self.assertEqual(0, 0)


if __name__ == "__main__":
    unittest.main()
