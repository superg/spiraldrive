# Plextor PX-W5224A firmware image format

This is the image-generation contract for the downloadable PX-W5224A firmware.
Function and data semantics remain authoritative in Ghidra.

## Immutable inputs

| Version | File | SHA-256 | Checksum |
|---|---|---|---:|
| 1.01 | `data/W5224A101/Mont_101.bin` | `cb48fb90a18731e89ca8335371b72c178c50723f8330b7478b4961ce87695514` | `0x9bd8` |
| 1.02 | `data/W5224A102/Mont_102.bin` | `492cac48f6eeb453d3c94ee16cf3e9ca1cc88ab8e4b342f05dde5220390cc03c` | `0x1fd2` |
| 1.03 | `data/W5224A103/MONT_103.bin` | `7ab5497b4c3e4bb1a17099b5b4af3d52af7eaf9eac9b120b3aee78ecf4fb4853` | `0x58d6` |
| 1.04 | `data/W5224A104/MONT_104.bin` | `622e1abd99894e913300f3b737d0cb82eb217061f342d26536b88125769e50b2` | `0xb3bd` |

Every image is exactly `0x80000` bytes. Version 1.04 is the reconstruction
target and maps at `RAM:f80000`.

## Layout

| File range | Mapped range | Contents |
|---|---|---|
| `0x00000..0x0004f` | `RAM:f80000..f8004f` | 80-byte identification header |
| `0x00050..0x6ffff` | `RAM:f80050..feffff` | main source-mode code and data |
| `0x70000..0x77fff` | `RAM:ff0000..ff7fff` | erased `0xff` recovery hole, not monitor code |
| `0x78000..0x79ffd` | `RAM:ff8000..ff9ffd` | main-image fixed data |
| `0x79ffe..0x79fff` | `RAM:ff9ffe..ff9fff` | big-endian checksum |
| `0x7a000..0x7ffff` | `RAM:ffa000..ffffff` | `0x6000` erased `0xff` bytes |

The 1.04 header is:

```text
PLEXTOR CD-R   PX-W5224A1.0404/10/06  17:00     PLEXTOR MONT    000
```

Execution begins with `8a fd ce a9` at file offset `0x50`, an `EJMP` from
`RAM:f80050`. The runtime monitor captured at `RAM:ff0000..ffffff` is a
separate ROM/overlay mapping; it is not present in the downloadable image's
erased tail. A direct comparison of the immutable 1.04 files finds all
32,768 bytes of the download at file offsets `0x70000..0x77fff` erased, while
the corresponding `RAM:ff0000..ff7fff` runtime capture contains 28,717
non-`0xff` bytes. The next `RAM:ff8000..ff9fff` 8,192 bytes match exactly.
At `RAM:ffa000..ffffff` the download is erased again, while the runtime
capture has 118 non-`0xff` bytes, including configuration data. This is
file-level evidence for distinct views at overlapping addresses; it does not
measure chip select, write protection, or flash aliasing.

## Checksum and generation

The checksum covers every byte before its word and excludes erased padding:

```text
checksum = (0x8000 + sum(image[0x00000:0x79ffe])) & 0xffff
image[0x79ffe] = checksum >> 8
image[0x79fff] = checksum & 0xff
```

An image generator must:

1. Produce exactly `0x80000` bytes.
2. Place linked code and fixed data at assigned addresses.
3. Fill `0x7a000..0x7ffff` with `0xff`.
4. Calculate and store the big-endian checksum only after preceding bytes are
   final.
5. Independently verify the completed image before packaging or transport.

The checksum provides integrity only; it is not authentication or evidence of
behavioral safety.

The recovery program's `ValidateMainFirmwareImage` at `RAM:ff4dc9` independently
sums main offsets `0..0x6ffff` and `0x78000..0x79ffd`, then compares the stored
word. The erased hole contributes `0x8000` modulo 65536 to the container sum,
canceling its initial `0x8000` bias. The monitor also rejects an entry window
`0x50..0x1ff` whose byte sum is zero or `0x1ae50` (entirely erased).

`tools/firmware_image.py` implements addressed Intel HEX packaging, format
verification, and comparison. The source build emits relocatable copies only
of main-ROM tables whose canonical locations are overwritten by reconstructed
code. The header and `RAM:ff8000..ff9ffd` fixed data remain packer-owned at
their canonical addresses; the build supplies the latter from the hash-pinned
1.04 reference with `--fixed-data-from`. The final `verify` step now also
uses `--fixed-data-from` to require all 8,190 fixed-data bytes in the packed
image to match the hash-pinned stock file, independently of the checksum.
A checksum-valid one-byte fixed-data mutation fails this gate. Its host tests
round-trip the original 1.04 payload to the exact original image hash. See root
`README.md` for command examples and `BUILD_MILESTONE.md` for source-match
boundaries.

## Canonical analysis

The reviewed program is `/validated/MONT_104.bin` in `ghidra/gplextor`, using
`80251-fixed:BE:24:default`, `srcMode=1`. Use
`tools/validate_canonical_ghidra_project.sh` rather than recreating analysis
from loader facts in this document.
