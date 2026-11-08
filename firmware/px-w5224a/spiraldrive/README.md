# SpiralDrive PX-W5224A

Custom PX-W5224A firmware based on stock MONT 1.04, adding C4 raw capture and
movement commands. The Keil C251 build produces `payload/c4.hex` and `payload/c4.map`.
The Rust `fwpatch-plextor` tool packages them using the static
[mont-104.json](../mont-104.json) definition, which owns the `G104` revision,
inquiry text, input paths, and stock-firmware patches.

With Keil C251, CMake 3.21 or newer, and Rust edition 2024 support installed, run
from the repository root:

```sh
cmake -S firmware/px-w5224a/spiraldrive -B build/px-w5224a/spiraldrive
cmake --build build/px-w5224a/spiraldrive --config Release
```

Keil defaults to `C:/Keil_v5/C251`. Configure `KEIL_C251_ROOT` for another
installation. Stock firmware belongs at `data/px-w5224a/MONT_104.bin`, configurable
through `SPIRALDRIVE_ORIGINAL_FIRMWARE`.

The default build compiles the payload and Rust patcher, then writes
`build/px-w5224a/spiraldrive/MONT_104_patched.bin`. The JSON paths are relative to
the definition file. Update them when changing the payload output directory.
