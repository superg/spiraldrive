import contextlib
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import firmware_image as fw

REFERENCE = fw.ROOT / "data/W5224A104/MONT_104.bin"


def record(kind, offset=0, payload=b""):
    data = bytes([len(payload)]) + offset.to_bytes(2, "big") + bytes([kind]) + payload
    return ":" + (data + bytes([-sum(data) & 255])).hex().upper()


def hex_payload(chunks):
    lines = []
    for address, data in chunks:
        for index in range(0, len(data), 16):
            here = address + index
            lines += [record(4, payload=(here >> 16).to_bytes(2, "big")),
                      record(0, here & 0xFFFF, data[index:index + 16])]
    return "\n".join(lines + [record(1)]) + "\n"


def fix_checksum(image):
    image[fw.CHECKSUM_OFFSET:fw.CHECKSUM_OFFSET + 2] = (
        (0x8000 + sum(image[:fw.CHECKSUM_OFFSET])) & 0xFFFF).to_bytes(2, "big")


class ImageTests(unittest.TestCase):
    def setUp(self):
        self.memory = {fw.BASE + 0x50 + i: value for i, value in enumerate(b"\x8a\xfd\xce\xa9")}
        self.image = fw.pack(self.memory, code_only=True)

    def test_header_layout_checksum_and_independent_monitor_sum(self):
        self.assertEqual(len(fw.HEADER), 80)
        self.assertEqual(len(self.image), 0x80000)
        report = fw.verify(self.image)
        self.assertEqual(report["checksum"], report["monitor_checksum"])
        self.assertFalse(report["stock_image_match"])
        self.assertEqual(self.image[0x70000:0x78000], b"\xff" * 0x8000)
        self.assertEqual(self.image[0x7A000:], b"\xff" * 0x6000)

    def test_sparse_hex_preserves_absolute_addresses_across_banks(self):
        chunks = [(0xF80050, b"abcd"), (0xFDCEA9, b"efgh")]
        memory = fw.read_hex(hex_payload(chunks))
        self.assertEqual(memory[0xFDCEA9], ord("e"))
        self.assertEqual(fw.pack(memory)[0x5CEA9:0x5CEAD], b"efgh")

    def test_segment_address_records(self):
        text = "\n".join([record(2, payload=b"\x12\x34"), record(0, 0x50, b"a"), record(1)])
        self.assertEqual(fw.read_hex(text), {0x12390: ord("a")})

    def test_start_records_do_not_relocate_payload(self):
        for kind, payload in ((3, bytes.fromhex("12340050")), (5, bytes.fromhex("00f80050"))):
            with self.subTest(kind=kind):
                text = record(kind, payload=payload) + "\n" + hex_payload([(0xF80050, b"a")])
                self.assertEqual(fw.read_hex(text), {0xF80050: ord("a")})

    def test_reject_malformed_hex(self):
        good = record(0, 0x50, b"a")
        bad_cases = ["not hex", ":0", ":GG", good[:-2] + "00\n" + record(1),
                     ":02005000614D\n" + record(1), good,
                     record(1), record(1) + "\n" + good,
                     record(7) + "\n" + record(1),
                     record(1, 1), record(4, payload=b"\x01") + "\n" + record(1),
                     record(0, 0xFFFF, b"ab") + "\n" + record(1),
                     record(4, payload=b"\x01\x00") + "\n" + good + "\n" + record(1),
                     record(5, payload=b"\xff\xff\xff\xff") + "\n" + good + "\n" + record(1)]
        for text in bad_cases:
            with self.subTest(text=text):
                with self.assertRaises(fw.ImageError):
                    fw.read_hex(text)

    def test_duplicate_bytes_are_rejected_even_if_equal(self):
        with self.assertRaisesRegex(fw.ImageError, "overlap"):
            fw.read_hex(hex_payload([(0xF80050, b"a"), (0xF80050, b"a")]))

    def test_reject_payload_in_header_recovery_checksum_padding_and_outside_image(self):
        for address in (0, 0xF7FFFF, 0xF80000, 0xF8004F, 0xFF0000, 0xFF7FFF,
                        0xFF9FFE, 0xFFA000, 0xFFFFFF, 0x1000000):
            with self.subTest(address=address):
                with self.assertRaises(fw.ImageError):
                    fw.pack(self.memory | {address: 0})

    def test_high_fixed_data_allowed_but_executable_rejected(self):
        memory = self.memory | {0xFF8000: 42, 0xFF9FFD: 99}
        self.assertEqual(fw.pack(memory)[0x78000], 42)
        with self.assertRaisesRegex(fw.ImageError, "executable payload"):
            fw.pack(memory, code_only=True)

    def test_fixed_data_can_be_supplied_outside_linker_payload(self):
        fixed_data = bytes((index & 0xff) for index in range(0x1FFE))
        image = fw.pack(self.memory, code_only=True, fixed_data=fixed_data)
        self.assertEqual(image[0x78000:fw.CHECKSUM_OFFSET], fixed_data)

    def test_fixed_data_must_cover_exact_canonical_range(self):
        with self.assertRaisesRegex(fw.ImageError, "cover RAM:ff8000"):
            fw.pack(self.memory, fixed_data=b"short")

    def test_reject_corrupted_container(self):
        for offset in (0, 0x43, 0x4F, 0x100, 0x70000, 0x79FFE, 0x7A000):
            with self.subTest(offset=offset):
                corrupted = bytearray(self.image)
                corrupted[offset] ^= 1
                with self.assertRaises(fw.ImageError):
                    fw.verify(bytes(corrupted))
        for image in (self.image[:-1], self.image + b"\0"):
            with self.assertRaisesRegex(fw.ImageError, "image size"):
                fw.verify(image)

    def test_monitor_entry_guard_even_with_valid_checksum(self):
        for value in (0, 255):
            image = bytearray(self.image)
            image[0x50:0x200] = bytes([value]) * 0x1B0
            fix_checksum(image)
            with self.assertRaisesRegex(fw.ImageError, "entry window"):
                fw.verify(bytes(image))

    def test_compare_reports_contiguous_ranges_and_vector_match(self):
        changed = bytearray(self.image)
        changed[0x200:0x202] = b"ab"
        changed[0x204] = 0
        result = fw.compare(bytes(changed), self.image)
        self.assertEqual(result["differing_bytes"], 3)
        self.assertEqual(result["difference_ranges_file_offsets"],
                         [{"start": 0x200, "end_exclusive": 0x202},
                          {"start": 0x204, "end_exclusive": 0x205}])
        self.assertTrue(result["vector_region"]["identical"])

    def test_cli_round_trip_and_input_protection(self):
        with tempfile.TemporaryDirectory() as directory:
            source, output, report = [Path(directory) / name for name in ("in.hex", "out.bin", "report.json")]
            source.write_text(hex_payload([(0xF80050, b"abcd")]))
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(fw.main(["pack", str(source), str(output), "--report", str(report)]), 0)
                self.assertEqual(fw.main(["verify", str(output)]), 0)
                self.assertEqual(fw.main(["compare", str(output), str(output), "--require-identical"]), 0)
                original_source = source.read_bytes()
                self.assertEqual(fw.main(["pack", str(source), str(source)]), 1)
                self.assertEqual(source.read_bytes(), original_source)
                original_image = output.read_bytes()
                self.assertEqual(fw.main(["verify", str(output), "--report", str(output)]), 1)
                self.assertEqual(output.read_bytes(), original_image)

    def test_canonical_inputs_cannot_be_output_paths(self):
        for directory in ("data", "ghidra"):
            with self.assertRaisesRegex(fw.ImageError, "immutable"):
                fw.check_output(fw.ROOT / directory / "must-not-create.bin")

    def test_failed_exact_comparison_still_writes_difference_report(self):
        with tempfile.TemporaryDirectory() as directory:
            original, changed, report = [Path(directory) / name for name in ("a.bin", "b.bin", "diff.json")]
            original.write_bytes(self.image)
            altered = bytearray(self.image)
            altered[0x50] ^= 1
            changed.write_bytes(altered)
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(fw.main(["compare", str(changed), str(original), "--require-vectors",
                                          "--report", str(report)]), 1)
            self.assertEqual(json.loads(report.read_text())["differing_bytes"], 1)

    @unittest.skipUnless(REFERENCE.exists(), "original image not available")
    def test_stock_fixed_data_gate_rejects_checksum_valid_mutation(self):
        original = REFERENCE.read_bytes()
        reconstructed = fw.pack(self.memory, code_only=True,
                                fixed_data=original[0x78000:fw.CHECKSUM_OFFSET])
        self.assertTrue(fw.verify(reconstructed, fixed_data_reference=original)
                        ["stock_fixed_data_match"])
        mutated = bytearray(reconstructed)
        mutated[0x78000] ^= 1
        fix_checksum(mutated)
        self.assertTrue(fw.verify(bytes(mutated))["format_valid"])
        with self.assertRaisesRegex(fw.ImageError, "stock fixed data mismatch"):
            fw.verify(bytes(mutated), fixed_data_reference=original)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "mutated.bin"
            path.write_bytes(mutated)
            with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(fw.main(["verify", str(path)]), 0)
                self.assertEqual(fw.main(["verify", str(path),
                                          "--fixed-data-from", str(REFERENCE)]), 1)

    @unittest.skipUnless(REFERENCE.exists(), "original image not available")
    def test_original_payload_round_trip_is_byte_identical(self):
        original = REFERENCE.read_bytes()
        self.assertEqual(fw.sha256(original), fw.STOCK_SHA256)
        self.assertEqual(original[:80], fw.HEADER)
        chunks = [(fw.BASE + 0x50, original[0x50:0x70000]),
                  (fw.BASE + 0x78000, original[0x78000:fw.CHECKSUM_OFFSET])]
        rebuilt = fw.pack(fw.read_hex(hex_payload(chunks)))
        self.assertEqual(rebuilt, original)
        self.assertTrue(fw.verify(rebuilt)["stock_image_match"])


if __name__ == "__main__":
    unittest.main()
