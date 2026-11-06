import unittest

from tools.audit_keil_absolute_data import audit_map


def segment(address: int, name: str, size: int = 1) -> str:
    end = address + size - 1
    return (
        f"{address:06X}H   {end:06X}H   {size:06X}H   "
        f"BYTE   AT..     DATA           {name}"
    )


class AbsoluteDataAuditTests(unittest.TestCase):
    def test_accepts_bit_ram_aliases(self):
        lines = []
        for address in range(0x20, 0x29):
            lines.append(segment(address, f"?DT?AT_{address:02X}_?1?A"))
            lines.append(segment(address, f"?DT?AT_{address:02X}_?1?B"))
        self.assertEqual(audit_map("\n".join(lines)), (18, 9))

    def test_rejects_overlap_outside_bit_ram(self):
        text = "\n".join(
            [segment(0x30, "?DT?AT_30_?1?A"), segment(0x30, "?DT?AT_30_?1?B")]
        )
        with self.assertRaises(ValueError):
            audit_map(text)


if __name__ == "__main__":
    unittest.main()
