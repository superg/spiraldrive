# PX-W5224A firmware reconstruction

This repository contains a behavioral C reconstruction of all 1,717 canonical
functions in the Plextor PX-W5224A 1.04 main firmware. The existing recovery
firmware is not built or modified. The reconstructed source is not yet
flash-ready: the locator, fixed-address layout, runtime helpers, and hardware
validation still require work.

## Build on Windows

Requires CMake 3.24+, Python 3.10+, a Windows CMake generator, and the installed
Keil C251 toolchain. Visual Studio/MSBuild is supported; Ninja is optional.

```powershell
cmake --preset windows
cmake --build --preset firmware
cmake --build --preset verify
```

`KEIL_C251_ROOT` defaults to `C:/Keil_v5/C251`. Override it at configuration:

```powershell
cmake --preset windows -DKEIL_C251_ROOT="C:/another installation/C251"
```

On Linux, the build driver automatically runs PE-format Keil tools through
Wine. Point CMake at the installation inside the Wine prefix, for example:

```sh
cmake -S . -B out/agent-tests/firmware \
  -DKEIL_C251_ROOT="$HOME/.wine/drive_c/Keil_v5/C251"
cmake --build out/agent-tests/firmware --target firmware
```

The `windows` preset lets CMake select the installed generator. To select one
explicitly, use a fresh build directory, for example:

```powershell
cmake -S . -B out/vs2022 -G "Visual Studio 17 2022"
cmake --build out/vs2022 --config Release --target verify
```

With the default Visual Studio preset, artifacts are in
`out/windows/firmware/Release/`:

- `MONT_reconstructed.bin`: 512-KiB downloadable-format main image.
- `main.abs`, `main.hex`, `main.map`: linked OMF251, addressed HEX, and locator map.
- `*.obj`, `*.lst`, `*.log`: objects, C/assembly listings, and tool diagnostics.
- `validation.json`: container-layout and checksum validation.
- Per-step `*.json`: tool versions, hashes, arguments, and output hashes.

`build/` is reserved for the user's VS Code workflow. Presets write to `out/`;
agent probes and temporary test builds belong under `out/agent-tests/`.

`firmware` compiles, links, packages, and validates the firmware container.
`verify` also runs the host tests. These checks do not establish operational or
flash safety. A changed source/header/toolchain input rebuilds its dependent
outputs.

## Host tests and image commands

Host tests do not require Keil:

```powershell
cmake --preset host-tests
ctest --preset host-tests
# Or without any CMake generator:
python -m unittest discover -s tests -v
```

Image packaging is also standalone:

```powershell
python tools/firmware_image.py pack out/windows/firmware/Release/main.hex out/example.bin --code-only
python tools/firmware_image.py verify out/example.bin
python tools/firmware_image.py compare out/example.bin data/W5224A104/MONT_104.bin --require-vectors
```

Add `--report PATH` for JSON. `compare --require-identical` requires every byte
to match; ordinary `compare` reports differences without failing. The packer
accepts explicit high fixed-data records without `--code-only`, but always
rejects records in the recovery hole, header, checksum word, and erased tail.
Original images are comparison/test inputs only, never filler for this build.

See [the current build status](docs/BUILD_MILESTONE.md) and
[the image format](docs/W5224A_FIRMWARE_FORMAT.md). There is no flash or activation
target. Byte/layout tests do not establish physical recovery after a bad flash.
