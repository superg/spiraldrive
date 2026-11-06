# Ghidra 80251 fixed language

This extension is based on the 80251 processor module shipped with Ghidra
12.1.2. It corrects the displacement width used by the four MCS-251
`@DRk+dis` MOV encodings. It also corrects reversed P-code semantics in the
four register-destination displacement MOV constructors and swapped register
fields in the affected displacement encodings.

Intel's *8XC251Sx User's Manual*, Table A-14, defines these instructions with
a 16-bit signed displacement. Ghidra 12.1.2's preliminary `80251.sinc` uses a
24-bit token, causing the decoder to consume one byte from the next
instruction.

The memory-destination word form `MOV @WRjd+dis16,WRjs` also has its two
register fields reversed in the preliminary module. Intel Table A-14 and
Atmel's TSC80251 programmer guide encode the source register in bits 7:4 and
the indirect base register in bits 3:0. This matters in the firmware's
coroutine initializers: `59 12 00 02` is `MOV @WR4+0x2,WR2`, completing the
four-byte entry point stored at the private stack base.

The extension also corrects the valid-register mask for indirect `@DRk`
operands. The four-bit encoding permits 0 through 7 (`DR0` through `DR28`)
and 14 through 15 (`DR56`/`DPX` and `DR60`/`SPX`); encodings 8 through 13 are
reserved. The preliminary module's indirect-only mask admits 8 through 9 and
rejects `DPX`/`SPX`, causing valid instructions such as `MOV R15,@DPX` to be
treated as bad data.

The inherited MCS-51 specification also assigned `CPL A` (`F4`) and `CPL CY`
(`B3`) to the source-mode escape group even though both opcodes are in columns
0 through 5 of the opcode map. Those columns are unescaped in both source and
binary mode. The fixed language decodes both instructions directly, matching
Intel User's Manual Tables A-6 and A-7.

MCS-251 extended bit operations are also corrected. Their encoded byte address
selects internal RAM below `0x80` and SFR space at `0x80` and above; Ghidra's
preliminary implementation sent both classes to RAM. The fixed language gives
the `BITS` space enough address width for the full `(byte << 3) + bit` range and
routes each operation to the architectural byte space. For example,
`A9 C7 E1` at firmware 1.04 `RAM:fd10df` now clears bit 7 of `SFR:e1`, rather
than unrelated `RAM:0000e1`. Validation scans every extended bit instruction in
the image and checks the selected byte space.

The inherited legacy `INC` and `DEC` byte constructors and `INC DPTR` now
update the MCS-251 N and Z result flags. Unlike the original MCS-51, the
MCS-251 architecture defines those flags for all accumulator, register,
direct, indirect-byte, and data-pointer forms. Omitting them made flag-based
`JE`/`JNE` sequences depend on stale state; firmware 1.04 uses that pattern
extensively. The validator checks every single-operand `INC`/`DEC` instruction
in the recovered code corpus for a PSW write.

The byte multiply/divide destination-pair table also corrects an inherited
R5 entry that incorrectly selected R3 as the product/remainder high byte.
MCS-251 odd byte destinations use the containing even/odd register pair, so
`MUL R5,Rm` and `DIV R5,Rm` write R4:R5. The incorrect mapping corrupted 79
firmware 1.04 operations and caused Ghidra to discard valid packed-BCD
seconds/minutes rollover paths as unreachable.

The arithmetic and result-flag semantics now follow Intel's *8XC251Sx User's
Manual*, Table 4-4. `ADD`, `ADDC`, `SUB`, `SUBB`, and `CMP` update carry and
overflow, byte operations also update auxiliary carry, and their results
update N and Z. Logical, rotate, shift, increment, decrement, decimal-adjust,
multiply, divide, `CJNE`, and `DJNZ` forms now update the exact documented
flag subsets. Multiply and divide clear carry, set overflow according to the
documented result or zero-divisor condition, and derive N/Z from the specified
result width. The validator scans the complete firmware arithmetic corpus for
these P-code effects.

Several extended data/bit operations are corrected independently of decoding:
`MOV bit,CY` now both sets and clears its destination, `CPL bit` toggles only
the selected bit, and `ORL CY,/bit` complements the source bit rather than the
carry input. Direct-byte transfers to and from a `DRk` use the full 32-bit RAM
object while retaining the architecture's byte-only SFR behavior.

Explicit `PUSH` and `POP` operations now preserve their architectural operand
width: word registers transfer two bytes and dword registers transfer four.
These data-stack operations remain present even though implicit call/return
address traffic is intentionally omitted for decompilation. The firmware-wide
validator checks the actual word and dword stack corpus. The previously absent
`MOV DRk,PC` encoding is also implemented and records the address of the
following instruction.

The processor specification retains architectural MCS-251 names, but replaces
unverified peripheral aliases inherited from Intel parts with neutral labels
such as `SFR_A9`, `SFR_D8`, and `SFR_E1`. The LC898095 peripheral identity of
those addresses must be established from firmware or hardware evidence.

The extension exposes a separate language ID so it does not replace Ghidra's
built-in language:

    80251-fixed:BE:24:default

The original firmware bytes are not modified.

For clearer decompilation, the language omits the physical two- or three-byte
return-address stack traffic generated by CALL/RET and ECALL/ERET. Explicit
firmware PUSH/POP instructions and stack-frame adjustments remain modeled.
The compiler specification limits inferred local stack frames to 256 bytes to
prevent wrapped 24-bit stack offsets from becoming enormous fake arrays. Some
pointer-like locals still render as 245–255-byte arrays; these are known
decompiler artifacts and must not be copied literally into reconstructed C/C++.

The firmware uses optimized, function-specific register assignments that are
not safe to infer as one universal ABI. Verified custom storage is retained in
canonical Ghidra program `/validated/MONT_104.bin`; new signatures must be
applied only after checking every caller. The current set covers the
buffer-transfer and sense-data paths, memory/range/checksum helpers, delay
helper, coroutine context constructors, controller-port and controller-buffer
helpers, and selected initialization/configuration routines. It is
intentionally not a global ABI model.

`ValidatePlextor80251Language.java` checks representative decoder probes and
the complete firmware corpus for extended-bit address spaces, arithmetic flag
effects, byte multiply/divide register pairs, and explicit stack widths.
`ValidatePlextorCanonicalProject.java` separately checks source identity,
memory layout, function/coverage metrics, tags, checksum, runtime overlays,
loader instructions, and interrupt routing. Both scripts are read-only and are
run by `tools/validate_canonical_ghidra_project.sh` against a working or stored
project.
