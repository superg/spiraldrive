# Controller enable and setup reconstruction

Reviewed through live Ghidra MCP on 2026-09-12 in firmware 1.04,
`/validated/MONT_104.bin`, using `80251-fixed:BE:24:default`.

| Source body | C symbol | Original interface |
|---|---|---|
| `RAM:fbd495..fbd4f4` | `SetWritePipelineControllerEnable` | ACC byte selector, LCALL/RET, no scalar result |
| `RAM:fbe586..fbe5ab` | `UpdateControllerPortSetupFlag6FromBoolean` | ACC byte, ECALL/ERET, no scalar result |

Both implementations are in `src/platform/leaf_workers.c`. Internal calls use
the C251 ABI; these are not original-address or original-call-frame replacements.

## Instruction evidence

`RAM:fbd495` decrements ACC and branches on zero, then increments and rejects
remaining nonzero inputs. Only 1 enables and only 0 disables. The caller
`ExecuteWriteCommandRange` loads exactly those values at `RAM:fbbdf1` and
`RAM:fbbe00`, respectively. For either valid input the observable sequence is:

1. Read `RAM:000fca`, OR with `0x08`, write the shadow then `RAM:018002`.
2. Write `RAM:018003` with `0x80` for enable or zero for disable.
3. Reread `RAM:000fca`, AND with `0xe7`, write the shadow then `RAM:018002`.
4. Read/modify/write `RAM:000c87` bit zero to the selected state.

There are no SFR accesses or interrupt masking in this function. Reusing the
interrupt-masking port helper would change its contract. Ghidra's optimized
decompiler output omits the initial shadow handshake and reorders stores;
the instruction listing is the evidence for volatile ordering.

`RAM:fbe586` starts with `ANL A,#1`. Odd values clear `RAM:00fae0` bit six;
even values set it and call `RAM:fb2214`. The caller loads the entire
`RAM:000b0c` byte at `RAM:fcb3a1`. Both routes call `SetSenseData` at
`RAM:faff63` with ACC and WR6 zero. The historical Boolean name does not mean
that every nonzero byte takes the odd route.

Both Ghidra parameter types were corrected from `bool` to `byte`, retaining
their verified `ACC:1` storage. Existing names were preserved, and comments
were refined and saved to the canonical main program.

## Dependency correction

The existing C for `RAM:fb2214..fb22b9` now preserves its initial
`RAM:00fbf6=0x43` store, conditional shadow RMW, shadow read before port 809D,
second `RAM:00fae0` read after that port write, and `RAM:00fbf7` read before
port 809F. Earlier C cached the flags and omitted some of these accesses.
Physical meanings of the flags and ports remain unassigned.

## Validation

The permanent CTest target `reconstruction_reviewed_tests`
keeps assertions enabled even under Release. It checks 65,536 input/shadow
pairs for the first function and 65,536 input/setup-flag pairs for the second,
including the full ordered shadow, port, and sense-write traces. Two simulated
mid-transaction changes verify the shadow reread after port 8003 and the
setup-flag reread after port 809D. Debug and Release focused runs pass.

```powershell
cmake --build --preset firmware
cmake --build out/windows --config Release --target reconstruction_reviewed_tests
ctest --test-dir out/windows -C Release -R 'reconstruction_reviewed_tests|firmware_image_validation' --output-on-failure
```

Enabling assertions also exposed absent pre-existing test readback helpers.
The harness now has 24-bit-addressed, untraced big-endian readback helpers,
and its one invalid call to a private production helper uses test readback.
The older general Release host target still disables assertions; its success
alone must not be treated as behavioral validation. The full `verify` run did
not pass: its general host suite was manually stopped after 190.41 seconds
without completion. The Python image tests and focused asserted tests passed.
The retired disabled-idle image checks are no longer part of the build. No
full-suite behavioral pass is claimed, and the current complete firmware link
is not yet clean. C251 listings retain the two shadow transactions and the
ACC-bit-zero branch; these results establish neither function-byte equivalence
nor operational drive behavior.
