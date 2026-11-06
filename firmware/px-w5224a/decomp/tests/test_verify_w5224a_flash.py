import unittest

from tools import firmware_update_plan
from tools import verify_w5224a_flash


class VerifyW5224AFlashTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.stock = firmware_update_plan.STOCK.read_bytes()
        cls.monitor = firmware_update_plan.MONITOR.read_bytes()

    def test_expected_stock_view_matches_live_preflash_composite(self):
        expected = verify_w5224a_flash.expected_post_flash_view(self.stock, self.monitor)
        self.assertEqual(expected, self.stock[:0x70000] + self.monitor)

    def test_expected_view_substitutes_only_operation3_ranges(self):
        candidate = bytearray(self.stock)
        candidate[0x12345] ^= 1
        candidate[firmware_update_plan.firmware_image.CHECKSUM_OFFSET:
                  firmware_update_plan.firmware_image.CHECKSUM_OFFSET + 2] = b"\0\0"
        checksum = (0x8000 + sum(candidate[:firmware_update_plan.firmware_image.CHECKSUM_OFFSET])) & 0xFFFF
        candidate[firmware_update_plan.firmware_image.CHECKSUM_OFFSET:
                  firmware_update_plan.firmware_image.CHECKSUM_OFFSET + 2] = checksum.to_bytes(2, "big")
        expected = verify_w5224a_flash.expected_post_flash_view(bytes(candidate), self.monitor)
        self.assertEqual(expected[:0x70000], candidate[:0x70000])
        self.assertEqual(expected[0x70000:0x78000], self.monitor[:0x8000])
        self.assertEqual(expected[0x78000:0x7A000], candidate[0x78000:0x7A000])
        self.assertEqual(expected[0x7A000:], self.monitor[0xA000:])

    def test_compare_detects_each_preservation_range(self):
        expected = verify_w5224a_flash.expected_post_flash_view(self.stock, self.monitor)
        result = verify_w5224a_flash.compare_view(expected, expected, self.stock, self.monitor)
        self.assertTrue(result["pass"])
        damaged = bytearray(expected)
        damaged[0x7A123] ^= 1
        result = verify_w5224a_flash.compare_view(bytes(damaged), expected, self.stock, self.monitor)
        self.assertFalse(result["pass"])
        self.assertFalse(result["checks"]["configuration_7a000_7ffff_preserved"])


if __name__ == "__main__":
    unittest.main()
