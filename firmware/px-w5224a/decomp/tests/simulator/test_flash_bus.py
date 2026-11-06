"""Checks for the conditional SST39LF400A offline flash model."""

import unittest
from pathlib import Path

from tests.simulator.flash_bus import FlashBusError, Sst39Lf400aBus, image_checksum


STOCK_IMAGE = Path(__file__).resolve().parents[2] / "data/W5224A104/MONT_104.bin"


def erase_sequence(target: int, command: int = 0x3000) -> list[tuple[int, int]]:
    return [
        (0xF0AAAA, 0xAA00),
        (0xF05554, 0x5500),
        (0xF0AAAA, 0x8000),
        (0xF0AAAA, 0xAA00),
        (0xF05554, 0x5500),
        (target, command),
    ]


class FlashBusTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.stock = STOCK_IMAGE.read_bytes()

    def test_fixed_data_sector_erase_breaks_stock_checksum_under_alias(self) -> None:
        bus = Sst39Lf400aBus(self.stock)
        for address, value in erase_sequence(0xF78000):
            bus.write(address, value)
        bus.require_complete()
        self.assertEqual((0x8EF0, 0xB3BD), image_checksum(bus.image))
        self.assertEqual(
            ("sector-erase", 0xF78000, 0xF78000, 0xF79000, 3730),
            (
                bus.effects[0].operation,
                bus.effects[0].command_address,
                bus.effects[0].start_address,
                bus.effects[0].end_address,
                bus.effects[0].changed_bytes,
            ),
        )
        self.assertEqual(self.stock[0x79000:], bus.image[0x79000:])

    def test_initial_sector_command_conditionally_erases_reset_vector(self) -> None:
        bus = Sst39Lf400aBus(self.stock)
        for address, value in erase_sequence(0xF00000):
            bus.write(address, value)
        bus.require_complete()
        self.assertEqual(3682, bus.effects[0].changed_bytes)
        self.assertEqual(b"\xff" * 4, bus.image[0x50:0x54])
        self.assertEqual((0xF55A, 0xB3BD), image_checksum(bus.image))

    def test_alternate_block_command_conditionally_erases_reset_vector(self) -> None:
        bus = Sst39Lf400aBus(self.stock)
        for address, value in erase_sequence(0xF00000, 0x5000):
            bus.write(address, value)
        bus.require_complete()
        self.assertEqual(63469, bus.effects[0].changed_bytes)
        self.assertEqual((0xF00000, 0xF10000),
                         (bus.effects[0].start_address, bus.effects[0].end_address))
        self.assertEqual(b"\xff" * 4, bus.image[0x50:0x54])
        self.assertEqual((0x481A, 0xB3BD), image_checksum(bus.image))

    def test_operation_five_preservation_contract_rejects_sector_erase(self) -> None:
        allowed = ((0xF70000, 0xF78000), (0xF7C000, 0xF80000))
        bus = Sst39Lf400aBus(self.stock, allowed_effect_ranges=allowed)
        with self.assertRaisesRegex(FlashBusError, "outside declared ranges"):
            for address, value in erase_sequence(0xF78000):
                bus.write(address, value)
        self.assertEqual([], bus.effects)
        self.assertEqual(self.stock, bus.image)

    def test_block_erase_and_word_program_effects(self) -> None:
        bus = Sst39Lf400aBus(b"\xff" * 0x80000)
        for address, value in erase_sequence(0xF70000, 0x5000):
            bus.write(address, value)
        for address, value in [
            (0xF0AAAA, 0xAA00),
            (0xF05554, 0x5500),
            (0xF0AAAA, 0xA000),
            (0xF70000, 0x1234),
        ]:
            bus.write(address, value)
        bus.require_complete()
        self.assertEqual(["block-erase", "word-program"], [effect.operation for effect in bus.effects])
        self.assertEqual((0xF70000, 0xF80000),
                         (bus.effects[0].start_address, bus.effects[0].end_address))
        self.assertEqual(b"\x12\x34", bus.image[0x70000:0x70002])

    def test_empty_allowed_ranges_reject_effects(self) -> None:
        bus = Sst39Lf400aBus(self.stock, allowed_effect_ranges=())
        with self.assertRaisesRegex(FlashBusError, "outside declared ranges"):
            for address, value in erase_sequence(0xF78000):
                bus.write(address, value)
        self.assertEqual([], bus.effects)
        self.assertEqual(self.stock, bus.image)

    def test_incomplete_and_out_of_range_commands_fail_closed(self) -> None:
        bus = Sst39Lf400aBus(self.stock)
        bus.write(0xF0AAAA, 0xAA00)
        with self.assertRaisesRegex(FlashBusError, "incomplete"):
            bus.require_complete()
        bus.power_cycle()
        self.assertEqual(self.stock, bus.image)
        with self.assertRaisesRegex(FlashBusError, "invalid flash word address"):
            bus.write(0xF80000, 0x3000)
        with self.assertRaisesRegex(FlashBusError, "unexpected flash write"):
            bus.write(0xF70000, 0x3000)


if __name__ == "__main__":
    unittest.main()
