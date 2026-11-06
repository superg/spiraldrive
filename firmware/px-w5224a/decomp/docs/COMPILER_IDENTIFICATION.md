# Compiler and linker identification

## Current conclusion

The original PX-W5224A firmware 1.04 compiler and linker are **not
identified**. Keil C251/L251 is the leading toolchain candidate because its
documented 251 source-mode, memory-model, register-allocation, and locator
features fit the emitted program. Those properties are compatibility evidence,
not provenance: other 80251 toolchains or a mixed C/assembly build could emit
the same architectural features.

No Keil name, compiler version, linker version, source path, object-module
name, or other identifying toolchain string has been found in
`data/W5224A104/MONT_104.bin` or the 64-KiB recovery capture
`data/read_memory_ff0000.debug`. The image header identifies Plextor's product,
revision, build date, and `MONT` payload only.

## Firmware evidence

Firmware 1.04 establishes these requirements for any candidate build system:

- Native MCS-251 source-mode code, including extended registers and 24-bit
  control transfers.
- A linked code/data image larger than 64 KiB, based at `RAM:f80000`, with
  executable code through `RAM:fef193` and fixed high-address data.
- Unprefixed native/source-mode extended instructions, including `EJMP`,
  `ECALL`, and `ERET`. A 251 binary-mode encoding would use the `A5` escape
  prefix instead.
- A deliberate mixture of `ECALL`/`ERET` and `LCALL`/`RET`, plus tail jumps,
  interrupt wrappers, and private coroutine stacks.
- Function-specific register parameter and return assignments. The prepared
  corpus records 1,350 proven implementation roles across 1,364 function
  records; this evidence does not form one global ABI.
- Near, bank-preserving far, and full-width address operations in the same
  program.
- Shared tails, duplicated library-like helpers, non-contiguous Ghidra bodies,
  and code whose behavior is partly expressed through condition flags.
- Absolute firmware tables, dispatch pointers, MMIO addresses, overlay/ROM
  entry points, and checksum placement.

The downloadable image alone cannot reveal whether individual routines came
from C, hand-written assembly, compiler runtime libraries, or separately
located modules.

The clean firmware 1.04 listing contains these decoded control-flow counts:

| Instruction | Count | Detail |
|---|---:|---|
| `ECALL` | 8,445 | 2,483 same-64-KiB-page, 5,941 cross-page, 21 indirect |
| `LCALL` | 1,590 | near calls |
| `ERET` | 988 | extended returns |
| `RET` | 705 | near returns |
| `LJMP` | 4,442 | long jumps |
| `EJMP` | 2 | extended jumps |
| `ACALL` / `AJMP` | 0 | absent |

Representative matched procedure classes are visible at instruction level:
`RAM:f84c6a` uses `LCALL RAM:f85583`, whose implementation returns with
`RET`; `RAM:f8493e` uses `ECALL RAM:f849cc`, whose implementation returns at
`RAM:f849de` with `ERET`.

## Why Keil C251 is a candidate

Keil's official C251 documentation describes native 251 source mode, code
spaces up to 16 MiB, near/far/huge object types, register parameters, multiple
optimization levels, global register coloring, reentrant functions, and the
L251 linker/locator's absolute placement and overlay facilities.

Application Note 116 is the strongest specific compatibility fingerprint:
under ROM(HUGE), ordinary procedures use `ECALL`/`ERET`, while eligible
`static` or explicitly `near` procedures use `LCALL`/`RET`. It also documents
`USERCLASS`/`CLASSES` placement as a way to permit near calls across modules.
That precisely accommodates the two procedure classes observed above, but it
still does not uniquely identify Keil.

Primary references:

- [Keil C251 product overview](https://www.keil.com/products/c251/c251.asp)
- [uVision C251 target options](https://www.keil.com/support/man/docs/uv4cl/uv4cl_dg_target251.htm)
- [uVision C251 compiler options](https://www.keil.com/support/man/docs/uv4cl/uv4cl_dg_c251.htm)
- [L251 linker/locator manual](https://www.keil.com/support/man/docs/l251/default.htm)
- [Keil application note 116: using near functions in ROM(HUGE)](https://www.keil.com/appnotes/files/apnt_116.pdf)
- [Keil application note 106: Writing Relocatable 8xC251Sx and 8x930Ax/USB Programs](https://www.keil.com/appnotes/files/apnt_106.pdf)

None of those compatible features is unique enough to identify Keil or a
particular release. Until emitted bytes from a period-correct compiler match a
distinctive set of firmware helpers, record the toolchain as unknown.

## Release chronology and toolchain prerequisite

The image date is 2006-04-10. The header uses MM/DD/YY: firmware 1.03 contains
the unambiguous date `12/17/03`, so 1.04's `04/10/06` cannot mean October 4.
Keil documented C251 v3.5 in 2002, while C251 v4 was publicly announced on
2006-05-03, 23 days after this firmware stamp. A late v3.x compiler is therefore
chronologically likelier, but v4 or pre-release access cannot be excluded.

The selected practical reconstruction toolchain is installed externally at
`C:/Keil_v5/C251`: C251 5.60.0.0, A251 4.69.13.0, L251 4.66.93.0, and OH251
1.47.0.0. The CMake build records executable hashes, versions, commands, objects,
maps, and listings under the build directory. No tool binaries or libraries are
committed. Successful reconstruction builds identify only the selected modern
toolchain; they do not identify the original compiler or demonstrate
byte-identical C generation. See `BUILD_MILESTONE.md`.
Legacy v3.x release material, if pursued later, requires a valid license key.

Chronology references:

- [Keil C251 v3.5 newsletter, 2002](https://www.keil.com/product/newsletters/2002sp_keil.pdf)
- [Keil C251 v4 announcement, 2006-05-03](https://www.keil.com/pr/article/1092.htm)
- [Keil legacy C251 v3.x release/license note](https://www.keil.com/support/docs/1931.htm)

## Exact-match experiment

Exact matching should begin with small, fully understood functions rather than
the complete firmware. Prioritize `MinU8`, `MinU16`, `MinU32`,
`AbsoluteValueI16`, `MultiplyU32Low`, both packed-BCD byte conversions,
`ReadBigEndianU16`, `WriteBigEndianU16`, `ZeroMemoryWords`, `CopyBytes16`,
`CopyBytesWith8BitCount`, `CopyThreeBytes16`, `CopySixBytesFar`,
`SwapByteOrderInWords`, `MapControllerRegisterIndexSkippingGap`,
`IsBinaryMsfTupleValid`, `DecodeAsciiHexNibbleOrZero`, and
`MapThreeFieldStatusCode`. Add `MapOperatingCodeToProfileIndex` as a compact
control-flow fixture whose exact `ACC` input/output contract is validated;
test all 256 possible input values in the compiler experiment.
`CopyThreeBytes16` is a useful early stability
fixture because its 27-byte, 11-instruction body is identical at the same
address in firmware 1.01 through 1.04. Each fixture must preserve its verified
register ABI and integer widths.

`DivideS16WithRemainder` is also a distinctive target fixture: it consumes a
signed dividend in `WR6` and divisor in `WR4`, and returns quotient in `WR6`
plus a dividend-signed remainder in `WR4`. Its behavior has been recovered for
every nonzero divisor, including the two's-complement `INT16_MIN / -1` bit
result. Division by zero has no established source-level contract, so the
portable C harness deliberately excludes this routine and exact-match source
experiments must not invent a zero-divisor result.

For every available period-correct C251 release, build a reproducible matrix
covering:

1. Candidate C251/A251/L251 and runtime-library revisions, beginning with
   licensed v3.11, v3.5 or the latest obtainable v3.x, then v4.
2. SOURCE plus ROM(HUGE) as the primary configuration; BINARY and ROM(LARGE)
   as negative controls.
3. Every data model supported by the chosen release where the fixture permits
   it.
4. Every optimizer level, SIZE versus SPEED preference, and global-register
   coloring setting.
5. Reentrant mode, alias checking, variable ordering, plain-character
   signedness, and the interrupt-frame/INTR2 option.
6. Source forms including ternary versus `if`, explicit temporaries, parameter
   order, expression grouping, and `volatile`/`const`/near/far/huge qualifiers.
7. `static` versus external and address-taken functions, explicit `near`,
   same-module versus separate-module builds, and `USERCLASS` placement.
8. Library-runtime inclusion versus locally compiled source for duplicated
   helper bodies.

Do not use assembly wrappers to force the observed ABI during compiler
identification. A wrapper can demonstrate eventual link compatibility, but it
cannot establish that a compiler naturally emitted the original body and
calling graph.

Capture compiler and library hashes, full command lines, environment, object
bytes, fixups, map files, and emitted listings. First compare normalized object
structure for triage; then link each fixture at its original address and
require exact bytes. Require multiple independent body/ABI matches and the
correct near/far caller graph before claiming a compiler-version match.
Same-address body bytes from the retained older images can provide stability
clues, but equality does not prove equal older-version boundaries, semantics,
or compiler provenance.

Only after several distinctive helpers match should the experiment expand to
whole modules. A full image match additionally requires the original or
equivalent:

- startup and interrupt assembly;
- compiler runtime library revision;
- object-module ordering;
- section, segment, and absolute-address layout;
- linker/locator options and overlay decisions;
- alignment, fill, and padding policy;
- dispatch-table and constant placement;
- final Plextor header and checksum generation.

### RAM:fe19de fixture result

Firmware 1.04 `RAM:fe19de..fe1a15` is now a reproducible medium-complexity
fixture. Its 56-byte body has three byte inputs in `ACC`, `R7`, and `R6`, two
nested selector groups, no calls or data references, and a near `RET`.
`tools/run_fe19de_compiler_matrix.py` compiles three equivalent source shapes,
extracts the function bytes from C251 listings, and ranks them against the
immutable image.

C251 5.60 produced no exact match across default O0-O8 or O3-O8 with both SIZE
and SPEED preferences. At O3-O6 the nested-switch form reproduced the
original destructive `DEC R7` selector sequence and shared-return structure,
but the modern optimizer chose a different order for the inner `R6` switch.
O7-O8 emitted multiple early returns. The switch-expression variant had the
best measured overlap: 60 bytes versus 56, with a 27-byte identical contiguous
block. These results strengthen the case for testing licensed C251 v3.x/v4
releases and preserve the exact fixture needed to do so, but they do not prove
which older release generated the firmware.

### Practical C251 5.60 reconstruction baseline

The reconstruction now uses this explicit default compiler configuration:

```text
MODSRC ROM(HUGE) LARGE REGPARMS PARM251 OPTIMIZE(5,SIZE) CODE
```

This is a pragmatic modern-toolchain default, not a claim that the original
build used the same spelling or optimization level. `MODSRC` and `ROM(HUGE)`
match the native source-mode instructions, 24-bit code space, and mixed near
and extended calls. `LARGE` remains the best-supported data model because the
firmware routinely treats low-address application globals as external data;
the current 19-function corpus also made its emitted total closest to the
original among C251 5.60's five data models. `REGPARMS` and `PARM251` make the
compiler's documented defaults explicit and agree with the recovered
register-argument conventions.

`tools/run_c251_optimizer_corpus.py` compares 19 independently bounded pure or
leaf-like functions against firmware 1.04. With the current source forms and
`LARGE`, the useful optimizer results were:

| C251 5.60 level | Candidate/original bytes | Mean similarity | Exact bodies |
|---|---:|---:|---:|
| O3 | 632 / 738 | 0.5564 | 2 / 19 |
| O4 | 628 / 738 | 0.5564 | 2 / 19 |
| O5 | 628 / 738 | 0.5564 | 2 / 19 |
| O6 | 611 / 738 | 0.5190 | 0 / 19 |
| O7 | 573 / 738 | 0.4290 | 0 / 19 |

O0-O2 were much larger than the original corpus (938, 820, and 820 bytes
respectively, versus 466 bytes in the initial 14-function run). O3 is therefore
a credible lower bound, not the best default. Keil's documented O4 switch and
common-subexpression passes fit the control-flow evidence, and O5 is the
highest level that preserves the two exact bodies:
`MapOperatingCodeToProfileIndex` (`RAM:f8f47a`, 111 bytes) and
`MapOperatingCodeToTransitionClass` (`RAM:f8f4e9`, 49 bytes). O6 changes both;
O7's tail merging and simplification also changes the characteristic shared
return shapes. O4 and O5 are indistinguishable on all 19 fixtures; relative to
O3, O4 shortens the current `IntegerSquareRootU32` source form by four bytes.
Consequently O5 is the uniform space-conscious reconstruction setting. The
build deliberately avoids per-translation-unit optimizer exceptions: source
and low-level adapters must express required behavior without changing the
global optimization level.

C251 5.60 documents no current code-generation difference between SIZE and
SPEED for this optimizer, and both produced the same fixture results. SIZE is
selected to record the reconstruction's firmware-size intent. Explicit
`REGPARMS PARM251` produced the same 628 function bytes as their implicit
defaults. Do not enable global `NOALIAS`, `REENTRANT`, O8/O9 linker packing, or
global register coloring without separate evidence; each changes semantics,
ABI, or whole-program layout beyond what this corpus establishes. Translation
units requiring native binary64 continue to opt into `FLOAT64` individually.
The complete `status_mapping.c` unit also compiles at this baseline with zero
diagnostics, so its former O1 build exception has been removed. All seven units
that formerly selected O8 also compile at O5/SIZE with zero diagnostics; those
overrides have been removed.

## Matching and safety criteria

Use three separate claims:

- **Behavioral match:** tests show equivalent outputs and state transitions.
- **Function-byte match:** one or more bounded functions emit identical bytes.
- **Image match:** every covered byte, address, table, padding region, and
  checksum matches the original image.

A matching additive firmware checksum is not a behavioral or byte-identical
match. Conversely, a behaviorally useful reconstruction need not reproduce the
original compiler output. Keep those tracks separate so source cleanup cannot
silently weaken the exact-baseline work.

Do not flash a reconstructed image merely because it compiles or has a valid
checksum. Flash testing needs an independently verified recovery path and an
explicitly authorized hardware experiment.
