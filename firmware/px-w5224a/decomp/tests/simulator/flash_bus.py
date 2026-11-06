"""Conditional SST39LF400A flash-bus model for offline firmware-update traces.

CPU addresses, the high-byte command lane, and the +0x80000 execution alias are
explicit modeling assumptions. This module does not establish LC898095 wiring.
"""

from __future__ import annotations

from dataclasses import dataclass


FLASH_BASE = 0xF00000
FLASH_SIZE = 0x80000
SECTOR_SIZE = 0x1000
BLOCK_SIZE = 0x10000
CHECKSUM_OFFSET = 0x79FFE
_UNLOCK1 = 0xAAAA
_UNLOCK2 = 0x5554


class FlashBusError(ValueError):
    pass


@dataclass(frozen=True)
class FlashEffect:
    operation: str
    command_address: int
    start_address: int
    end_address: int
    changed_bytes: int


class Sst39Lf400aBus:
    """Replay complete erase/program commands against a disposable image copy.

    A bus write is a CPU byte address and a 16-bit CPU word. The high byte is
    interpreted as the command byte; program data is stored in big-endian CPU
    byte order. Busy timing, status polling, protection pins, and chip-select
    behavior are outside this model.
    """

    def __init__(
        self,
        image: bytes,
        *,
        allowed_effect_ranges: tuple[tuple[int, int], ...] | None = None,
    ) -> None:
        if len(image) != FLASH_SIZE:
            raise FlashBusError("flash model requires a 512-KiB image")
        self._image = bytearray(image)
        self.allowed_effect_ranges = (
            ((FLASH_BASE, FLASH_BASE + FLASH_SIZE),)
            if allowed_effect_ranges is None else allowed_effect_ranges
        )
        for start, end in self.allowed_effect_ranges:
            if not FLASH_BASE <= start < end <= FLASH_BASE + FLASH_SIZE:
                raise FlashBusError("invalid allowed effect range")
        self.bus_writes: list[tuple[int, int]] = []
        self.effects: list[FlashEffect] = []
        self._state = "idle"

    @property
    def image(self) -> bytes:
        return bytes(self._image)

    @property
    def pending_command(self) -> bool:
        return self._state != "idle"

    def power_cycle(self) -> None:
        """Discard an incomplete command; completed array effects persist."""
        self._state = "idle"

    def require_complete(self) -> None:
        if self.pending_command:
            raise FlashBusError(f"incomplete flash command: {self._state}")

    def _effect_range(self, address: int, size: int) -> tuple[int, int]:
        offset = address - FLASH_BASE
        start = offset & ~(size - 1)
        end = start + size
        cpu_start = FLASH_BASE + start
        cpu_end = FLASH_BASE + end
        if not any(
            allowed_start <= cpu_start and cpu_end <= allowed_end
            for allowed_start, allowed_end in self.allowed_effect_ranges
        ):
            raise FlashBusError(
                f"flash effect outside declared ranges: {cpu_start:#08x}..{cpu_end:#08x}"
            )
        return start, end

    def _erase(self, address: int, size: int, operation: str) -> None:
        start, end = self._effect_range(address, size)
        changed = sum(value != 0xFF for value in self._image[start:end])
        self._image[start:end] = b"\xff" * size
        self.effects.append(
            FlashEffect(operation, address, FLASH_BASE + start, FLASH_BASE + end, changed)
        )

    def _program(self, address: int, value: int) -> None:
        start, end = self._effect_range(address, 2)
        word = value.to_bytes(2, "big")
        changed = 0
        for index in range(2):
            programmed = self._image[start + index] & word[index]
            changed += programmed != self._image[start + index]
            self._image[start + index] = programmed
        self.effects.append(
            FlashEffect("word-program", address, FLASH_BASE + start, FLASH_BASE + end, changed)
        )

    def write(self, address: int, value: int) -> None:
        if not FLASH_BASE <= address <= FLASH_BASE + FLASH_SIZE - 2 or address & 1:
            raise FlashBusError(f"invalid flash word address: {address:#08x}")
        if not 0 <= value <= 0xFFFF:
            raise FlashBusError("flash bus value must be a word")
        self.bus_writes.append((address, value))
        offset = address - FLASH_BASE
        command = value >> 8
        state = self._state

        if state == "idle" and offset == _UNLOCK1 and command == 0xAA:
            self._state = "unlock2"
        elif state == "unlock2" and offset == _UNLOCK2 and command == 0x55:
            self._state = "operation"
        elif state == "operation" and offset == _UNLOCK1 and command == 0x80:
            self._state = "erase-unlock1"
        elif state == "operation" and offset == _UNLOCK1 and command == 0xA0:
            self._state = "program-word"
        elif state == "erase-unlock1" and offset == _UNLOCK1 and command == 0xAA:
            self._state = "erase-unlock2"
        elif state == "erase-unlock2" and offset == _UNLOCK2 and command == 0x55:
            self._state = "erase-target"
        elif state == "erase-target" and command in (0x30, 0x50):
            size = SECTOR_SIZE if command == 0x30 else BLOCK_SIZE
            self._erase(address, size, "sector-erase" if command == 0x30 else "block-erase")
            self._state = "idle"
        elif state == "program-word":
            self._program(address, value)
            self._state = "idle"
        else:
            self._state = "idle"
            raise FlashBusError(
                f"unexpected flash write {address:#08x}={value:#06x} in {state}"
            )


def image_checksum(image: bytes) -> tuple[int, int]:
    if len(image) != FLASH_SIZE:
        raise FlashBusError("checksum requires a 512-KiB image")
    computed = (0x8000 + sum(image[:CHECKSUM_OFFSET])) & 0xFFFF
    stored = int.from_bytes(image[CHECKSUM_OFFSET:CHECKSUM_OFFSET + 2], "big")
    return computed, stored
