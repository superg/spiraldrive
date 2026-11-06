# PX-W4824A York 1.07: repeated negative-D8 acquisition

## Result and scope

York 1.07 has a confirmed signed-position defect in its raw-read buffer reuse
routine. With a nonempty descriptor queue, it interprets stored negative
positions as large positive values and resets acquisition even when the
requested negative position is already buffered. Mont 1.04 stores and compares
full 32-bit positions and does not have this particular truncation defect.

This is directly relevant to **D8 format 2, 2352 main + 96 subchannel bytes**.
The user's clarified scope excludes C2. No C2 change is proposed here.

Actual York machine code reproduces the erroneous branch in Keil S251.
Replacing two two-byte instructions restores buffered negative-position reuse
in the tested cases. This establishes the firmware defect and a small offline
correction, not a measured physical speedup or a complete drive-validation result.
No live-drive commands, flashing, firmware activation, or reset were performed.

## Input and analysis identity

- Input: `data/York_107.bin`, 524,288 bytes.
- SHA-256: `79553d4e8d8143c5d0d00f72e8ec274985fd292a151cc569d7ea5569b251c978`.
- Header identifies `PX-W4824A1.07`, `03/24/06  14:00`, `PLEXTOR YORK`.
- Stored checksum `0xf0ab` matches
  `(0x8000 + sum(image[:0x79ffe])) & 0xffff`.
- New Ghidra analysis: `/york/York_107.bin` in `ghidra/gplextor`, using
  `80251-fixed:BE:24:default`, `srcMode=1`, ROM at `RAM:f80000`.
- Comparison: canonical `/validated/MONT_104.bin`, official 5224 firmware 1.04.

York was not already imported in the project. Its new analysis contains scoped
function recovery and comments; it is not a fully validated whole-program import.
Its ROM bytes were hash-checked before saving and remain identical to the input.
The file header/checksum do not independently establish official provenance.
Shared instruction set and controller accesses do not prove identical silicon.

## What is already the same

| Behavior | York 1.07 | Mont 1.04 |
|---|---|---|
| D8 CDB parsing and +150 bias | `RAM:fd4f3a` | `RAM:fd6bfe` |
| Raw range executor | `RAM:fd5026` | `RAM:fd6cf2` |
| Negative admission | `-20000 < biased_start <= -1000` | Same |
| Original CDB LBA interval | `-20150 < LBA <= -1150` | Same |
| Special mode bit | `BITS:0013` | Same |
| Special acquisition target | `0x10000000` at `RAM:f8d7ef..f8d7fa` | Same at `RAM:f92a7d..f92a88` |
| Inward search | 200-unit steps; TNO-zero detection plus one extra move/confirmation | Same logical predicate |
| Negative raw-read speed cap | 4 at `RAM:fd6d30` | 4 at `RAM:fbf67c` |
| Format-2 bytes and pipeline bias | 2448 and -4 | Same |

York's format tables are at `RAM:ff9e99`, `RAM:ff9ea3`, and `RAM:ff9ead`.
Their five entries match Mont's respective tables at `RAM:ff95ba`,
`RAM:ff95c4`, and `RAM:ff95ce`.

York's positioner `RAM:fdf545` uses parsed TNO-zero flag `RAM:001108` bit 3,
the extra-move flag `RAM:0011b7` bit 0, and the sentinel at `RAM:000de7`.
The sentinel move at `RAM:fdf6ce..fdf6e1` supplies ACC=1 and WR6=200 to
`RAM:fd8cc6`. Its return is **ERET**, unlike Mont's near-returning positioner
at `RAM:fc858f`; copying either function's bytes across models is inappropriate.

These common mechanisms do not explain a model-specific restart between reads.

## The important difference: descriptor position width

York builds a 20-byte descriptor at `RAM:001122`. At
`RAM:fe0006..fe003b`, it copies only the low three bytes of the four-byte
pipeline position `RAM:00153f` into descriptor offsets 0..2.
The main cursor at offsets 3..5 is also three bytes.

Mont's builder at `RAM:fe0004..fe0022` writes a full four-byte position at
`RAM:0012a2` and a full four-byte cursor at offset 4. Its descriptor is 22 bytes.

York's active-mode/matching-format branch in the range executor reaches
`RAM:fec1b2` through `RAM:fd51d8..fd51da`. This routine checks whether the new
signed DR4 target can be satisfied from the existing descriptor queue.
It reads both newest and oldest descriptor positions with:

```text
RAM:fec212  49 03 ff ff   MOV WR0,@WR6-1
RAM:fec216  49 13 00 01   MOV WR2,@WR6+1
RAM:fec21a  9c 00         SUB R0,R0
RAM:fec21c  bf 03         CMP DR0,DR12

RAM:fec23a  49 03 ff ff   MOV WR0,@WR6-1
RAM:fec23e  49 13 00 01   MOV WR2,@WR6+1
RAM:fec242  9c 00         SUB R0,R0
RAM:fec244  bf 03         CMP DR0,DR12
```

The two word loads put the preceding descriptor byte in R0 and the stored
24-bit position in R1:R2:R3. Clearing R0 **zero-extends**, rather than
sign-extends, that position. For example:

```text
requested / stored signed position: -20003
descriptor bytes:                  ff b1 dd
York comparison value:             00 ff b1 dd = +16757213
correct signed comparison value:   ff ff b1 dd = -20003
```

The newest descriptor appears greater than the negative request. The oldest
descriptor also appears greater, so the branch at `RAM:fec246..fec248` reaches
`RAM:fec28c` even for a valid buffer hit. That path calls queue reset
`RAM:f8d523`, reinstalls coroutine 2 (`RAM:f8d71d` for mode 3), publishes the
new pipeline target at `RAM:00153f`, and yields. The alternate producer again
requests sentinel acquisition when `BITS:0013` is set.

Thus sequential host commands can repeatedly restart lead-in acquisition
instead of draining the existing stream. The initial command and individual
frames within a single command are distinct cases. An empty queue at exactly
the active producer position takes a separate path and avoids this bad
comparison; do not claim that literally every command must restart.

Mont's equivalent `RAM:fc5af5` compares full 32-bit descriptor positions at
`RAM:fc5b5d..fc5b79`, preserving their negative sign.

## Narrow correction, simulator only

| York address | File offset | Original | Candidate |
|---|---|---|---|
| `RAM:fec21a` | `0x6c21a` | `9c 00` — `SUB R0,R0` | `1a 01` — `MOVS WR0,R1` |
| `RAM:fec242` | `0x6c242` | `9c 00` — `SUB R0,R0` | `1a 01` — `MOVS WR0,R1` |

`MOVS WR0,R1` retains R1 and fills R0 from R1's sign. The following comparison
sets its own flags. No call, return, stack use, or instruction boundary changes;
no payload space is required. Both complete original ten-byte load sequences
are guarded in the probe, and A251 MODSRC independently confirms the encodings.

This interprets descriptor positions as signed 24-bit values, consistent with
their negative use. Positive CD positions remain unchanged; positive values
at or above `0x800000` would change interpretation and are outside this tested
CD-position domain. A production patch still requires an inventory of relevant
callers/position domains, a separately named image with corrected checksum,
and physical validation with an authorized recovery procedure. No flashable
patched image was generated in this task.

## Execution evidence and limitations

`tools/verify_york_d8_buffer_reuse.py` links the original York routine into an
S251 code-space application and executes eight cases each for stock and the
two-instruction correction. The queue is synthetic; execution stops before
the reset or backlog helper. It verifies actual branch PC, secondary cursor,
and expected stack growth from the entry PUSH DR12, and records the loaded position.

All **16/16** cases passed:

- Buffered positive positions near 100 and 450000: both variants select cursor 1.
- Buffered negative positions near -20003 and -1005: stock reaches reset;
  corrected code selects cursor 1.
- Negative request exactly at the active producer cursor: stock reaches reset;
  corrected code selects producer cursor 4.
- Requests before the buffer or beyond the producer: both correctly reset.
- Empty queue exactly at the active producer: both select cursor 0.

Successful report: `out/agent-tests/york-d8/s251-cases/report.json`.
Run on Windows with installed Keil C251/uVision:

```powershell
python tools/verify_york_d8_buffer_reuse.py
```

The runner starts uVision hidden. This session's private-desktop launches
failed; early multi-case initialization scripts also produced incomplete
observations. The accepted run uses a linked code-space application and one
process per case. Earlier scratch reports are superseded, not passing evidence.

No timing model, optical controller, real subchannel CRC, delivered-sector
continuity, or host transport is simulated. The finding explains a concrete
source of repeated acquisition overhead, but does not measure how much of the
reported physical slowness it causes. First-acquisition slowness could have
additional causes. Next physical evidence should distinguish startup delay
from per-command delay and check returned Q continuity using data+subchannel.

## Decoder caveats discovered during this comparison

The currently installed fixed language still reverses the register roles of
opcode `0x49` (`MOV WRj,@WRj+dis16`). For example, it displays `49 03 ff ff`
as `MOV WR6,@WR0+0xffff`; A251 and S251 confirm the correct instruction shown
above. Consequently the current York decompiler's pointer flow is unreliable
for this routine. Also, opcode `0x1a` is rendered `MOVZ` even though its P-code
uses sign extension and A251 names it `MOVS`.

These language defects were not changed globally or applied to canonical
Mont analysis. Any repair needs an isolated language regression first.
