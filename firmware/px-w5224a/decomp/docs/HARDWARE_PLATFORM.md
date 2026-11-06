# PX-W5224A hardware platform

## Observed board components

A PX-W5224TA board inspection identifies the main controller as a Sanyo `LC898095`
(`XF8H 3RKN5`), external flash as an SST `39LF400A`, and SDRAM as a Winbond
`W9816168H-6` 16-Mbit part. See the
[PX-W5224TA board inspection](https://diit.cz/clanek/co-je-uvnitr-specifikace-a-technologie9).
Plextor’s [PX-W5224TA installation manual](https://www.manualslib.com/manual/311259/Plextor-Px-W5224ta.html)
names `PX-W5224TA` as the retail model and `PX-W5224A` as its ATAPI
INQUIRY identifier. That resolves the model-name difference with the 1.04
firmware image, which contains `PX-W5224A`. The teardown observes one
board; component markings and bus wiring on a future test drive still need
independent confirmation.

Sanyo describes the LC898095 as a 52x CD-R/RW encoder/decoder containing:

- an 8051-source-compatible, 80251-compatible CPU;
- CD main-channel, CIRC, CD-ROM ECC/EDC, subcode, ATIP, servo, write-strategy,
  ATAPI, and external-buffer controllers;
- configurable external-RAM areas for main channel, C2 flags, and subcode plus
  a batch-transfer facility;
- a 33.8688-MHz clock and support for 2-MiB-class external RAM.

Primary source for the controller: [Sanyo DVD/CD LSI brochure](https://bitsavers.trailing-edge.com/components/sanyo/cdrom/2005_Sanyo_DVD_CD_LSI_Brochure.pdf).
The [SST39LF400A manufacturer datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/SST39LFX00A-SST39VFX00A-2-Mbit-4-Mbit-8-Mbit-x16-Multi-Purpose-Flash-DS20005001.pdf)
specifies 512 KiB of x16 flash, uniform 4-KiB sectors (2 Kwords), and uniform
64-KiB blocks (32 Kwords). Its six-write `0x30` sequence erases one sector;
its `0x50` sequence erases one block. The board inspection identifies the
part, while the manufacturer's document supplies the device geometry and
command semantics. Neither supplies the LC898095 programming manual, register
map, or wiring/alias between CPU and flash addresses.

## Address-map confidence

| Range | Interpretation | Confidence |
|---|---|---|
| `RAM:000000..0000ff` | MCS-251 register banks/direct internal RAM | Architecture-backed |
| `RAM:000100..f7ffff` | External data address space | Analysis model confirmed; physical partition incomplete |
| `RAM:f80000..ff9fff` | 512-KiB firmware image through checksum | Image-backed |
| `RAM:ffa000..ffffff` | Erased in download; selected ROM/overlay data exists at runtime | Image and capture-backed |
| `RAM:018000+` | LC898095 controller MMIO window | Repeated-access evidence; fields unresolved |

The configuration programmer provides a concrete unresolved partition question:
firmware 1.04 relocates code to `RAM:000800..000a8d`, while the saved
coroutine-three return occupies `RAM:000900..000903` in the S251 model. S251
shows the code copy overwriting that return. In the generic
[Intel 8XC251Sx manual](https://www.keil.com/dd/docs/datashts/intel/8xc251sx_um.pdf),
SPX addresses region `00:` and that Intel part's 1,056 bytes of on-chip RAM
end at `0x041f`; `0x0900` is beyond its on-chip RAM. This does not establish
the LC898095's memory size or wiring. The Intel manual also describes
configurable external-memory bus behavior, and the
[Sanyo LC898095 brochure](https://bitsavers.trailing-edge.com/components/sanyo/cdrom/2005_Sanyo_DVD_CD_LSI_Brochure.pdf)
mentions a buffer-RAM mapping function; neither documents the PX-W5224A's
CPU-visible mapping at those addresses. The paired conditional trace at
`out/agent-tests/c3reinit_final/results.json` shows that restoring the
`InitializeCoroutine3` saved frame permits the next scheduler call in both
images, but does not determine the physical code/data relationship.

## Mode Page 03 high-bank observation candidate

Firmware 1.04 treats `RAM:2bc6ef` as the current eight-byte Mode Page 03
image. The stock startup call at `RAM:fc2a73` copies defaults from
`RAM:ff8a5a` with ACC=8, MODE SELECT can replace those eight bytes, and the
page-control selector at `RAM:fc2ee7` returns `RAM:2bc6ef` for the current
image. The immutable runtime-capture defaults are
`03 06 00 00 00 00 00 00`. This establishes the firmware's intended data
flow, not the physical backing or alias of the high bank. A paired S251
restart continuation reaches the same call arguments in stock and the
reconstruction; substituting simulator bank `0x01` for the destination makes
both copy those eight bytes
(`out/agent-tests/hostrestart_modepage_copy_probe/host-interface-main-vector-status-cause10-initialization-entry-via-timer-source/results.json`).
The native `RAM:2bc6ef` continuation remains unverified in S251.

For an independently identified PX-W5224A already running stock firmware
1.04, vendor F5 selector zero offers a candidate **DATA IN** read of the
current Mode Page 03 bytes:

```sh
sg_raw --readonly --request=8 --outfile=mode-page03-current.bin --timeout=10 /dev/sgX f5 00 00 00 00 2b c6 ef 00 08
```

Use the device-identity and status recording procedure below. A stronger
read-only cross-check is MODE SENSE(10), current page 03, on the same drive:

```sh
sg_raw --readonly --request=16 --outfile=mode-sense03-current.bin --timeout=10 /dev/sgX 5a 00 03 00 00 00 00 00 10 00
```

Firmware 1.04's `ExecuteModeSense6Or10Response` selects page control zero
from CDB byte 2 and emits an eight-byte MODE SENSE(10) header with no block
descriptors. `AppendModeSensePageToControllerBuffer` then selects the current
page image at `RAM:2bc6ef` and copies eight bytes. The immutable 1.04 CDB
mask at `RAM:ff8cc9` is `0a 00 07 00 ff ff ff ff 00 00 fc`; applying its
nine field masks to the exact CDB above produces zero in every prohibited
bit, so the request passes `ValidateScsiCdbFieldMasks` under the recovered
logic. The response length is `8 + 6 + 2 = 16` bytes. If both commands return
GOOD with their full requested lengths, compare all eight F5 bytes with MODE
SENSE response bytes 8..15; require a `03 06` page header before interpreting
the comparison. Matching payloads support the firmware-visible data-read path
without assuming that the mutable page still equals its immutable defaults.
A mismatch is a specific discrepancy to investigate, not evidence of a flash
alias. Even a match cannot identify physical RAM backing or prove writes to
flash. The later identified-drive run described below returned GOOD for both
commands and matching `03 06 00 00 00 00 00 00` payloads.

The fixed capture helper `tools/probe_readonly_w5224a.py` combines INQUIRY,
the two Mode Page 03 DATA IN views, the two firmware-window F5 reads, and a
two-byte read of the flash-ID selector at `RAM:0000f8`. It
records each exact CDB, requested/returned length, device path, command status,
response bytes and hashes in a new output directory. It refuses vendor
commands unless INQUIRY identifies `PLEXTOR`, a product containing `W5224A`,
and revision `1.04`. Preview its CDBs with
`python3 tools/probe_readonly_w5224a.py --dry-run`; on an identified drive,
pass `--device /dev/sgX --output out/agent-tests/<unique-run-name>`. The
helper now has a native Windows SPTI DATA-IN backend. The identified-drive
run and full backup described below exercised all 44 fixed read-only commands;
the helper still has no DATA OUT path.

The helper also has an explicit `--recovery` profile for a drive manually
booted into the protected monitor. It requires monitor INQUIRY revision `0.00`
instead of normal firmware revision `1.04`, then permits only nine fixed
INQUIRY/F5 DATA-IN observations; `--full-backup` optionally adds the same 32
fixed F5 chunks. On the currently normal 1.04 drive, this profile completed
one standard INQUIRY with GOOD status, rejected revision `1.04`, and stopped
before any vendor command
(`out/agent-tests/windows-recovery-profile-negative-normal-20260925-v3/report.json`).
The tool does not enter recovery mode and sends no reset or DATA OUT command.
Its report is fail-closed: all mode-specific identity, immutable-byte, alias,
and requested full-backup checks must be true for `pass` to be true.
The expected `PLEXTOR` / `CD-R   PX-W5224A` / `0.00` recovery identity is
parsed from the hash-pinned monitor's 28-byte template at capture offset
`0x7fc0`, rather than trusted as an unbound command-line assumption.
The current helper was also exercised in normal mode: all 12 fixed DATA-IN
commands completed, all 14 required checks passed, and the selector remained
`bf00` (`out/agent-tests/windows-readonly-w5224a-current-helper-20260925/report.json`).
The user then powered on with Eject held, and the same helper positively
identified the protected monitor as `PLEXTOR` / `CD-R   PX-W5224A` / `0.00`.
All 41 commands were DATA IN, all 15 recovery checks passed, and the two
complete 512-KiB CPU-data views were byte-identical (SHA-256
`c3f6dff082319e1e3ac29554c3954be4a9a05d6b2345a5114fee8a65c9f5a65c`).
The execution view equals immutable stock through `RAM:feffff` plus the
hash-pinned protected-monitor capture at `RAM:ff0000..ffffff`. Evidence is in
`out/agent-tests/windows-recovery-full-backup-current-20260925/`. This confirms
the live forced-recovery route and recovery-mode read alias. It does not expose
the transient P3 programming-state address or byte lane and sends no write.

## Read-only flash-alias measurement candidate

Firmware 1.04 `HandleScsiOpcodeF5` at `RAM:fcb61f` accepts selector zero with a
32-bit big-endian source address in CDB bytes 4..7 and a 16-bit big-endian
length in bytes 8..9. It copies from that CPU data address to the controller
buffer and starts a DATA IN transfer. Read-only headless inspection of the
canonical stock 1.04 program confirms the selector read at `RAM:fcb62b`
(`RAM:000b0d`), the mode-zero dword load at `RAM:fcb730` (`RAM:000b0e`),
the length load at `RAM:fcb73c` (`RAM:000b12`), the transfer call at
`RAM:fcb832`, and the source-byte read at `RAM:fddd8e`. The captured
instruction listing is reproducible under `out/agent-tests/f5_static/`.
The reconstruction source and `vendor_f5_tests` agree with that contract.
This read-only comparison was executed on 2026-09-25 against a Windows-enumerated
`PLEXTOR CD-R PX-W5224A` revision 1.04 at `H:`. Both CPU data reads returned the
immutable stock bytes `8a fd ce a9 a9 d3 d1 ca 2b ca 4b ca 5b ca 6b ca`,
establishing the normal-command data-read alias on that drive. It does not
establish flash write routing or the command byte lane.
The stock configuration-programming caller at `RAM:fd643b` also checks
configuration word `0x000f`; when nonzero it clears `BITS:0580` and sets
`BITS:0581` before the `RAM:ff4500` monitor call (`RAM:fd647b..fd649d`).
After a successful call it sets `BITS:0580` and sets or clears `BITS:0581`
according to `RAM:000fae` (`RAM:fd64b7..fd64c9`). The failure path returns
before those restoration writes. This is confirmed by read-only canonical
instructions in `out/agent-tests/configuration_p3_headless.log`. The P3 pins
may influence external address or chip selection, but their board wiring has
not been established. The stock WRITE BUFFER handler also clears both
`BITS:0580` and
`BITS:0581` at `RAM:fd2b79..fd2b7b`, then calls the runtime programmer
with operation 3 or 5 through `RAM:ff4000` (`RAM:fd2b83..fd2bbb`;
`out/agent-tests/write_buffer_p3_headless.log`). Consequently a
normal-command F5 data-read alias, even if measured, would not by itself
prove the mapping at either programming handoff. The board wiring and
physical effect of these P3 states still need independent evidence.
The same selector-zero path permits a candidate two-byte DATA IN read from
`RAM:0000f8` with CDB `f5 00 00 00 00 00 00 f8 00 02`. The captured flash-ID loader at
`flash_identification_relocated::000800..000869` reads the device word and
stores it at `RAM:0000f8` (`::000833`/`::000836`), then compares that slot
with `0xbf00` and `0x6200` and writes normalized `0x2000` for other values
(`::00084b..000863`). Read-only canonical instructions are retained in
`out/agent-tests/flash_id_selector_headless.log`. The live drive returned
`bf00`, which selects the firmware's alternate SST-style programmer branch.
The manufacturer's SST39LF400A table gives the software manufacturer-ID word
as `0x00bf`; the live CPU-visible `0xbf00` is therefore the byte-swapped form of
that exact word. The relocated programmer writes CPU words `0xaa00`, `0x5500`,
and `0x8000` at `RAM:f0aaaa`, `RAM:f05554`, and `RAM:f0aaaa`. Dividing those CPU
byte offsets by two gives the SST x16 word addresses `0x5555`, `0x2aaa`, and
`0x5555`, while the observed ID byte order places each CPU high-byte command on
the SST command byte `DQ7..DQ0`. The live ID, exact stock CPU stores, and
manufacturer command table thus strongly establish the command address and
byte ordering; they are no longer free assumptions in the intended stock
programmer path. The remaining unobserved fact is the physical effect of a
write/erase cycle itself. The P3.0/P3.1 board wiring also remains unnamed, but
their stock transitions are already part of the factory WRITE BUFFER path.

Record the device path, INQUIRY vendor/product/revision, physical drive identity,
CDB, transfer direction, requested/returned lengths, status, sense, and response
bytes for each command. Only use a device identified as the intended PX-W5224A
running stock firmware 1.04. With `sg_raw` and a verified `/dev/sgX`, the
equivalent Linux sequence is:

```sh
sg_raw --readonly --request=96 --outfile=inquiry.bin --timeout=10 /dev/sgX 12 00 00 00 60 00
sg_raw --readonly --request=16 --outfile=f80050.bin --timeout=10 /dev/sgX f5 00 00 00 00 f8 00 50 00 10
sg_raw --readonly --request=16 --outfile=f00050.bin --timeout=10 /dev/sgX f5 00 00 00 00 f0 00 50 00 10
```

All three CDBs request DATA IN and have no data-out payload. Check the
`RAM:f80050` response against the immutable stock 1.04 bytes
`8a fd ce a9 a9 d3 d1 ca 2b ca 4b ca 5b ca 6b ca`. A mismatch, short
transfer, or non-good status means the alias comparison is inconclusive. If
both 16-byte reads equal that reference, it supports a *CPU data-read* alias
between these windows on that drive. It still cannot prove that writes to
`RAM:f00050` reach the same flash sector, nor that the first erase follows the
assumed SST high-byte command lane. Those require independent bus evidence
before a flash write. On Windows,
`python tools/probe_readonly_w5224a.py --device H: --output <new-directory>`
uses native SCSI pass-through for the same fixed DATA-IN-only CDB table. Its
identity-gated capture and opt-in full backup are retained under
`out/agent-tests/windows-readonly-w5224a-full-backup-20260925/`; all 44
commands completed with SCSI GOOD status and the declared transfer length.
Representative reads further establish `RAM:f70000 == RAM:ff0000`,
`RAM:f72a3e == RAM:ff2a3e`, and `RAM:f78000 == RAM:ff8000` in normal command
mode. The high-window bytes match the immutable runtime-monitor capture at its
entry and F5 handler, and the immutable stock fixed data at `RAM:ff8000`.
The full `RAM:f00000..f7ffff` and `RAM:f80000..ffffff` captures are
byte-identical (SHA-256
`c3f6dff082319e1e3ac29554c3954be4a9a05d6b2345a5114fee8a65c9f5a65c`),
and the latter is exactly stock 1.04 through `RAM:feffff` plus the protected
runtime capture at `RAM:ff0000..ffffff`.

The separate captured recovery monitor has its own F5 route: dispatch-table
entry `RAM:ff7dd4` is `00 ff 2a 3e`, pointing to
`HandleMonitorReadMemoryF5Command` at `RAM:ff2a3e`. That handler reads selector
CDB[3] from `RAM:000f05`, source CDB[4..7] from `RAM:000f06`, and length
CDB[8..9] from `RAM:000f0a`; its transfer worker reads CPU source bytes at
`RAM:ff32ed` and `RAM:ff32fa`. These are checked against the immutable runtime
capture in the read-only Ghidra inspection at
`out/agent-tests/f5_static/monitor-f5-final.log`.

If an independently identified drive is **already in monitor/recovery mode**,
a separate read-only checkpoint is to request 16 bytes from the monitor's own
`RAM:ff2a3e` entry:

```sh
sg_raw --readonly --request=16 --outfile=monitor-ff2a3e.bin --timeout=10 /dev/sgX f5 00 00 00 00 ff 2a 3e 00 10
```

The immutable capture predicts `ca 3b 0b fe 6d 66 7e b3 0f 05 24 00 78 12 7e 1f`.
Record the same identity, status, sense, transfer length, and response fields as
above. A match would show the monitor's command and DATA IN read path works on
that drive in that mode. It would not establish monitor-mode *writing* or
recovery after an interrupted erase. Do not issue a reset or force recovery as
part of this read-only check.

Exact accesses, state machines, symbols, and comments belong in canonical
Ghidra. This document retains only platform evidence and modeling boundaries.

## SFR and MMIO naming

The stock Ghidra 80251 specification includes peripheral aliases inherited
from Intel parts. They are not an LC898095 register map. The fixed language
uses neutral labels such as `SFR_A9`, `SFR_D8` through `SFR_DB`, `SFR_E1`,
`SFR_E4`, and `SFR_F1`; these identify addresses only.

Extended bit instructions address internal RAM below byte address `0x80` and
SFR space at or above `0x80`. Architectural names such as `ACC`, `PSW`, `SP`,
and `DPTR` are safe where the MCS-251 specification defines them. Controller
ports, event bits, measurements, pins, and signals must retain neutral names
until firmware data flow, runtime tests, or authoritative hardware material
establishes a physical role. Preserve their volatile access order in C.

## Missing hardware evidence

- LC898095 CPU-visible SFR and `RAM:018000+` register definitions.
- Physical partition of the external 2-MiB buffer.
- Analog front-end identity and serial-command protocol.
- Servo-position units.
- Exact paths from channel/CIRC data into main, C2, and subcode buffers.
