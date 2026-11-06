import tempfile
import unittest
from pathlib import Path

from tools.report_keil_size_deltas import canonical_sizes, listing_sizes


class ReportKeilSizeDeltasTests(unittest.TestCase):
    def test_parses_canonical_extent(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "functions.tsv"
            path.write_text("RAM:f80050\tResetVector\tRAM:f80050\tRAM:f80053\t1\n")
            self.assertEqual(canonical_sizes(path), {"ResetVector": 4})

    def test_parses_listing_instruction_lengths(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "sample.lst"
            path.write_text(
                ";       FUNCTION ResetVector? (BEGIN)\n"
                "000000 7E080007       MOV DR0,#7\n"
                "000004 AA             ERET\n"
                ";       FUNCTION ResetVector? (END)\n",
                encoding="latin-1")
            self.assertEqual(listing_sizes(path), [("ResetVector", 5)])


if __name__ == "__main__":
    unittest.main()
