# D8 negative-LBA, lead-in, and C2 analysis

This report applies to PX-W5224A firmware 1.04. Addresses refer to the main
program based at `RAM:f80000` and use the fixed 80251 language.

## D8 command contract

`HandleScsiOpcodeD8` at `RAM:fd6bfe` accepts only these CDB forms:

- `(CDB[1] & 0x1f) == 0`;
- `(CDB[11] & 0xfc) == 0`;
- CDB byte 10 is one of `0`, `1`, `2`, `3`, or `8`.

It reads the big-endian start LBA from CDB bytes 2–5, adds 150, and passes the
biased start in `DR4`. It adds the big-endian sector count from bytes 6–9 to
form the exclusive end in `DR0`. CDB byte 10 supplies the format in `ACC` to
`ExecuteRawSectorReadRange` at `RAM:fd6cf2`.

The special negative interval is tested after the 150-sector bias:

```text
-20000 < biased_start <= -1000
```

Therefore the corresponding original D8 LBA interval is:

```text
-20150 < CDB_start_lba <= -1150
```

The accepted negative path sets `BITS:0013` (`SpecialRawReadMode`). Other
routines can also set this mode, so its name describes verified behavior rather
than assigning it exclusively to negative LBAs. It does not convert the
negative LBA into an ordinary physical seek target.

## Related standard READ CD path

The standard READ CD (`0xbe`) and READ CD MSF (`0xb9`) handlers converge on
`ExecuteReadCdCommand` at `RAM:fd3442`. READ CD copies the signed CDB LBA into
the shared start field without the D8 handler's 150-sector bias; READ CD MSF
converts its start tuple and subtracts 150 before entering the same body. The
shared body classifies its signed start against exact thresholds -1000,
-20000, and -30000, initializes raw-read mode state, and coordinates
`ExecuteRawSectorReadRange` and completion/sense reporting. These checks are
distinct from D8's `-20000 < biased_start <= -1000` admission interval.

The associated `UpdateRawReadProgressCountersAndPhaseFlags` at `RAM:fc6721`
and `ResetRawReadProgressCountersAndPhaseFlags` at `RAM:fc67ff` maintain the
neutral counter/phase state at `RAM:0015e0..0015e4` and
`BITS:003d/003e/0040`; neither exposes a session selector.

## Transfer formats

The internal format index selects three verified tables:

| Internal index | Base bytes/sector | Auxiliary bytes | Pipeline start bias |
|---:|---:|---:|---:|
| 0 | 2352 | 0 | -1 |
| 1 | 2368 | 16 | -3 |
| 2 | 2448 | 96 | -4 |
| 3 | 96 | 0 | -4 |
| 4 | 2448 | 96 | -4 |

The tables are `RawReadBytesPerSectorByFormat` at `RAM:ff95ba`,
`RawReadAuxBytesByFormat` at `RAM:ff95c4`, and
`RawReadStartBiasByFormat` at `RAM:ff95ce`.

D8 CDB format 8 selects internal index 4 and sets bit 1 of
`RawReadOptionFlags` at `RAM:000f0b`. That option adds `0x126` (294) bytes, so
the host transfer length is 2448 + 294 = **2742 bytes per sector**.

## What physically starts negative acquisition

The negative D8 path does use the `0x10000000` acquisition sentinel.

At `RAM:f92a7d`, `SpecialRawReadMode` selects:

```text
signed_adjustment = 0
base_target       = 0x10000000
RequestReadAcquisitionAndWait(base_target, signed_adjustment)
```

The instruction sequence is `SUB DR0,DR0; XRL WR6,WR6; MOV WR4,#0x1000`.
Because `DR4` is the paired value `WR4:WR6`, this constructs `0x10000000`,
not `0x00001000`.

The ordinary path instead uses signed delta `-10` and the requested pipeline
target. `RequestReadAcquisitionAndWait` at `RAM:fdccc1` publishes
`base_target + signed_delta`, waits for the acquisition busy byte to clear,
and returns its result.

Consequently every accepted negative D8 request enters physical acquisition
through the same sentinel, independent of the requested negative number and
the pickup's preceding user-visible LBA. The requested negative
value remains in `ReadPipelineTargetAddress` at `RAM:001598` for virtual stream
progression and range checks.

The lower path recognizes the sentinel at `RAM:fc858f`, clears its pending
state bit, and calls `MovePickupByScaledRadialDelta` at `RAM:fd8380` with a
nonzero inward-direction selector and magnitude `0x00c8` (200). Returning zero
keeps the surrounding acquisition/retry path active.

The enclosing loop has now been traced through `RAM:f890e8`, `RAM:fc858f`,
and `RAM:fcfc06`. For this sentinel, it behaves as follows:

```text
repeat:
    move inward by 200 scaled-radial units
    acquire and parse the controller Q-like packet
until packet byte 1 (TNO) == 0

move inward by 200 scaled-radial units once more
acquire and parse another packet
accept when packet byte 1 (TNO) == 0
```

Other controller-error and retry exits remain possible, but the successful
lead-in predicate is exact. `ReadAcquisitionControlFlags` bit 0 at
`RAM:001305` records whether the mandatory extra move is still required.

## Position-field acceptance and session selection

`DecodeControllerPositionFields` at `RAM:fee8f0` normally validates and decodes
the controller bytes at `RAM:018004..018006` as packed BCD. Negative mode
relaxes this validation and permits selected controller sentinel values.
Additional negative-mode branches at `RAM:f92c27` and `RAM:f92d0b` bypass the
ordinary address/Q synchronization path.

Separately, `ParseControllerQPacket` at `RAM:fcfc06` parses the ten-byte packet
at `RAM:00124f`: byte 0 contains control/ADR fields, byte 1 is TNO, byte 2 is
index, and bytes 3–5 and 7–9 are packed-BCD time tuples. It clears
`ParsedQStatusFlags` at `RAM:00125a`, then sets bit 3 exactly when packet TNO
is zero. `PositionPickupForAcquisition` uses that flag for the stop predicate
above.

The surrounding packet handshake is now explicit. The controller-packet
interrupt marks a ten-byte packet captured and disables external interrupt 1.
`ProcessCapturedControllerQPacket` at `RAM:fcf36f` consumes it according to the
active acquisition mode and ADR. `RearmControllerQPacketCaptureWithTimeout` at
`RAM:fcf345` clears the captured flag, reloads the byte countdown at
`RAM:001205` to 30, acknowledges `SFR:91` bit 1, and re-enables external
interrupt 1. Thus `0xfe` from the processor means pending/rearmed rather than a
successfully accepted Q packet; 0 is a terminal precondition/timeout result and
1 is accepted content.

No field in the decoded D8 handler selects a session number, and the sentinel
search compares TNO only with zero. It does not compare a requested session.

The firmware evidence supports this conclusion:

- D8 negative mode asks the lower acquisition machinery to process the
  `0x10000000` sentinel, which causes repeated 200-unit inward movements.
- The first parsed Q packet with TNO zero triggers one more inward movement;
  a second TNO-zero packet is required for successful acceptance.
- It tolerates position information that the normal read path rejects.
- The first lead-in that survives the step size and confirmation logic wins.
  A narrow later-session lead-in can be stepped over, after which the search
  continues inward toward an earlier session.

No deterministic stock-firmware session selector has yet been found in this
path. Changing the magnitude of the negative D8 LBA changes virtual progression
and bounds, not the initial sentinel supplied to physical acquisition.

## Why negative D8 with C2 returns all zeros

The all-zero main, subchannel, and C2 transfer has a concrete firmware-level
cause in `ProcessControllerReadEvent` at `RAM:f90d22`:

1. With C2 requested, `RAM:f912e7..f91306` reads the controller cursor at
   `RAM:018019..01801b` and stores it as `AuxiliaryRingCursor` at `RAM:00126a`.
2. At `RAM:f9130a`, negative mode takes the jump at `RAM:f9130d` to
   `RAM:f913a4`. This skips the normal main-channel cursor read from
   `RAM:01800b..01800d`.
3. At `RAM:f913a4`, formats other than internal index 4 can read an alternate
   main cursor from `RAM:018008..01800a`.
4. D8 format 8 maps to internal index 4, so it skips that alternate read too.
5. The value still present on the stack—the C2 cursor—is then stored as
   `MainDataRingCursor` at `RAM:00126e`.

Negative + C2 + D8 format 8 therefore points the main/subchannel gather at the
C2 ring. If that ring contains zeros, the complete returned transfer is zero;
there is no need to posit a host-output `memset` in the D8 handler.

Separate buffered code pre-clears C2 slots with `0x126` zero bytes. Static
firmware evidence does not establish whether the ASIC subsequently produces
real C2 data while acquiring lead-in. Fixing the main cursor alias may recover
main/subchannel data while the 294-byte C2 tail remains zero.

## Minimal patch hypothesis

No firmware patch has been applied. The smallest identified experiment is:

```text
RAM:f9130d  02 13 a4    LJMP RAM:f913a4
         -> 02 13 ad    LJMP RAM:f913ad
```

`RAM:f913ad` is the existing alternate main-channel cursor read from
`RAM:018008..01800a`. Negative formats 0–3 already reach this code through
`RAM:f913a4`, so the changed jump is behavior-identical for them. Only negative
internal format 4 changes: it loads the ordinary main cursor instead of
retaining the stale C2 cursor. This preserves the intentional negative-mode
bypass of normal alignment/position validation at `RAM:f91327..f913a1` and is
therefore narrower than redirecting to `RAM:f91310`.

It remains a hypothesis, not a confirmed fix. It must be built as a separate
checksummed image and tested only with an independently verified recovery path
and explicit authorization. Even if main/subchannel data is restored, static
analysis cannot prove that the ASIC supplies useful lead-in C2 bytes.

## Reconstruction implications

Preserve these concepts independently in C:

- requested/virtual raw-read position (`RAM:001598`);
- physical acquisition target (`RAM:000c8e` / `RAM:000e3b`);
- negative-LBA compatibility mode (`BITS:0013`);
- raw format index and option flags (`RAM:000f06` / `RAM:000f0b`);
- C2 and main-channel ring cursors (`RAM:00126a` / `RAM:00126e`);
- read-side MMIO cursor interpretations, which overlap write-side controller
  address slots.

Collapsing those values into one `lba` or one buffer pointer would reproduce
neither the lead-in virtualization nor the C2 cursor bug.
