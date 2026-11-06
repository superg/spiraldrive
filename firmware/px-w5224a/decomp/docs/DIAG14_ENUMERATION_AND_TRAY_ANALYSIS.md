# Diag14 enumeration capture and debounced-input correction

## Physical observation and capture identity

On 2026-09-26 the user reports that the drive, initially closed, empty and
untouched, ejects at power-on, closes after a few seconds, then opens again
and stays open. Windows exposes the optical drive as H:. This follows the
diag14 interrupt-return correction. Candidate identity is inferred from the
flashing conversation; no live firmware readback was performed.

The complete supplied Bus Hound text export is `e:\ISO\diag3.txt`, 69,977 bytes,
SHA-256 `b288fc0b439c617d091cf776b86a7b8e403260019a3ea3eb49596e209740a262`.
An identical archival copy, parsed transactions, and summary are under
`out/agent-tests/diag14-bushound-20260926/`. The read-only parser is
`out/agent-tests/summarize_diag3.py`. Original capture bytes are unchanged.

Device 27 is bridge `093b:0042`, serial `0042001D9E`; device 82 is the
`PLEXTOR CD-R PX-W5224A` revision `1.04` optical child. The export has sequence
positions and compressed repetition counts, but no timestamps. Counts below
are displayed groups, not expanded totals.

## Confirmed transport progress

- CBW 11 submits a 36-byte INQUIRY; record 12 returns the correct identity;
  CSW 13 matches its tag with status zero and residue zero. The next 96-byte
  INQUIRY also completes. This is actual drive payload/completion evidence.
- Of 81 displayed CBW groups, 80 have matching CSWs: 70 status-zero and ten
  status-one results. Every completed group's received length plus CSW residue
  equals its requested length. GET CONFIGURATION, MODE SENSE, GET PERFORMANCE,
  REQUEST SENSE and GET EVENT STATUS all produce payloads.
- The ten REQUEST SENSE replies comprise two `05/2400`, one `06/2900`, and
  seven `02/3a02` groups. The firmware uses `02/3a02` for its tray-open/no-media
  command response. The one power-on/reset unit attention is not evidence of
  repeated firmware resets.
- There is no START STOP UNIT (`0x1b`), LOAD/UNLOAD (`0xa6`), or other explicit
  tray movement command among the captured CBWs. The reported motion therefore
  is not explained by a captured host tray command.
- The export contains 26 endpoint-`27.1` RESET rows after endpoint stalls and
  21 optical `SSTS=12` entries labelled `data overrun`. These are not the
  unanswered/canceled-IN pattern from earlier images: CSWs follow, including
  status-zero short replies with accurate residue. For example, CBW 131 asks
  for `0xfff0` bytes of MODE SENSE data, receives 72 bytes, and CSW 135 reports
  residue `0xffa8`, status zero. A stock baseline would be needed to classify
  these transport details as regressions.
- The export ends at CBW 459, a GET EVENT STATUS request without a shown reply.
  Since capture ends there, this alone does not establish a timeout or stall.

## Confirmed input-address defect

Live canonical Ghidra instructions for firmware 1.04 show:

| Canonical predicate | Stock load | Mask | Return |
|---|---|---:|---|
| `RAM:fb01c2`, P1.5 | `MOV ACC,0x0011ed` | `0x20` | ERET |
| `RAM:fb0664`, P1.6 | `MOV ACC,0x0011ed` | `0x40` | near RET |
| `RAM:fb01d0`, P1.7 | `MOV ACC,0x0011ed` | `0x80` | ERET |

All three reconstructed helpers in `src/platform/status_mapping.c` incorrectly
read `RAM:001300`, the drive-control state. `TimerDispatchPhase0` maintains the
debounced sample at `RAM:0011ed`. The corrected helpers now read that sample.
This is an evidence-backed input bug; the individual physical switch roles
are not inferred from the P1 bit names.

The startup input sequence at stock `RAM:fb0000` calls these predicates to
choose its initial branch and its P1.5/P1.6 waits. The foreground transition
worker at `RAM:fdd374` also uses P1.7/P1.5. Reading the state instead of the
sample makes those decisions independent of the physical inputs. Whether this
fully explains the observed eject-close-eject sequence awaits the next drive
test.

The previous host test explicitly expected `RAM:001300`, so it duplicated the
bug. The generated simulator tests seeded both locations to zero, testing only
one indistinguishable input. Neither established correct address selection.

## Correction and validation

- The assertion-enabled host test checks all 65,536 sample/state pairs for
  all three helpers, exact reads from `RAM:0011ed`, Boolean outputs and no
  writes. It fails on old source and passes after correction. Run it with
  `ctest --test-dir out/windows -C Release -R '^debounced_p1_predicates_tests$' --output-on-failure`.
- `tests/simulator/cases/debounced-p1.json` supplies six cases with opposite
  sample/state bits. All six mismatch before the fix and pass afterward:
  `out/agent-tests/p1-before-fix/results.json` and
  `out/agent-tests/p1-after-fix/results.json`. The runner invocation is retained
  in `out/agent-tests/run_p1_audit.py`. Generated-case overrides now seed the
  input bit asserted and drive state zero, so ordinary catalog smoke cases
  cannot silently mask this particular address error again.
- The compiled listing reads `FirmwareRamWindow+4589` (`0x11ed`) in all three
  functions. Keil compilation/link and image checksum/fixed-data validation
  pass with zero diagnostics. No broad simulator suite was rerun. An initial
  host-test invocation entered the aggregate because its old environment
  selector occurs after the reviewed-suite return; it was stopped. The new
  dedicated `--debounced-p1` selector runs before the aggregate.
- Binary comparison against diag14 finds exactly eight changed bytes: operand
  bytes at reconstructed `RAM:f9630a..f9630b`, `RAM:f96317..f96318`, and
  `RAM:f96324..f96325` change `13 00` to `11 ed`; the checksum at
  `RAM:ff9ffe..ff9fff` changes `dc a7` to `df 68`. The interrupt-return fix and
  all other image bytes are identical to diag14.

Next candidate is the normal-running diag15 image, with LED diagnostics OFF:
`out/agent-tests/diag15-debounced-input-fix-20260926/PXW5224A_diag15_debounced_input_fix.bin`.
SHA-256 `4178646748630d77e986fc898959738050044ed54727fde1e2d3d2ab73dc880a`,
checksum `0xdf68`, 524,288 bytes. User-operated redumper
flashing and the established Eject-at-power-on stock restore remain the workflow.
No hardware commands were sent during this analysis.

## Diag15 physical follow-up: unresolved disappearance

The user reports that the tray now stays closed, but the drive seems absent
and there is no new-device sound. This does not establish successful startup
or a tray fix. Read-only Windows CIM/PnP queries on 2026-09-26 found neither
the PX-W5224A optical child nor its previous USB bridge identity 093b:0042.
The present USB list also showed no unidentified/error USB device. Snapshot:
`out/agent-tests/diag15-pnp-snapshot.json`, timestamp 12:47:51 -04:00.
The user subsequently confirmed the drive was powered on. A repeat query at
12:48:52 still found neither device and no USB error. The absence alone
cannot identify its cause.
No CDB, reset, activation, or flash command was sent.

A read-only `IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX` (`0x220448`)
query at 12:51:26 reports `NoDeviceConnected` on the bridge's last root-hub
port8 (`USB\ROOT_HUB30\9&2902e077&0&0`). A positive control on port2 of
that same hub correctly returns connected VID:PID `1e91:de2c`. The target
reply is all zero, including ConnectionIndex; validating only that field
would incorrectly reject it. Constants/layout were checked against local
Windows SDK 10.0.26100.0 `usbioctl.h`/`usbiodef.h`; the final query uses the
registered hub interface. Reproducer and raw headers are retained in
`out/agent-tests/read_diag15_usb_port.ps1` and
`out/agent-tests/diag15-usb-port-powered.json`.

PnP's last arrival/removal timestamps are 12:30:58/12:41:53 for revision
1.04, 12:41:58/12:42:43 for revision0.00, and 12:41:57/12:42:43 for the
bridge, all on 2026-09-26 at UTC-04:00. These do not identify the removal
cause. The user reconnected only USB, keeping drive power on. At 12:53:31
the target and USB-error lists remain empty; the 12:53:32 port query still
reports NoDeviceConnected with the same passing control. Device evidence:
`out/agent-tests/diag15-pnp-after-replug.json`. The port script reused its
output name, so `diag15-usb-port-powered.json` now contains this later sample;
the earlier sample's headers remain in tool history. Future script runs use
timestamped names to preserve each capture. Next check is user-operated
Eject-at-power-on recovery enumeration before choosing any flash.
No-device status is not proof of a cable defect: bridge startup/attachment
behavior under the rebuilt firmware remains an open possibility.

A bounded caller audit used live stock 1.04 instructions and three S251 cases
with RAM:0011ed bit7 asserted independently of drive state. Stock and rebuild
both pass the startup early-return path at RAM:fb0000, state9 successful
return at RAM:fb0226, and foreground transition at RAM:fdd374. The actual
input predicates execute; the existing declared output/yield stubs remain.
Explicit expected flags/state/result are checked, not just termination.
Reproduce with `python out/agent-tests/audit_diag15_branches.py`; evidence is
in `out/agent-tests/diag15-branches/`. These checks do not model physical
switch levels, integrated scheduler timing, or USB bridge behavior. They
identified no further defect, and no new image was produced for this report.

## Recovery and installed-image verification

At 12:54:23 on 2026-09-26 (UTC-04:00), after the user entered Eject-at-power-on
recovery, Windows reports H: as PX-W5224A revision0.00 and its original bridge
093b:0042, serial0042001D9E. The same hub port8 now returns DeviceConnected.
Recovery thus restores visibility; the associated power cycle prevents
attributing the earlier absence exclusively to a particular code branch.

`tools/verify_w5224a_flash.py --device H: --image out/agent-tests/diag15-debounced-input-fix-20260926/PXW5224A_diag15_debounced_input_fix.bin --output out/agent-tests/diag15-recovery-readback-20260926-1254 --capture --recovery`
performed only INQUIRY (`12 00 00 00 60 00`, 96 bytes) and 32 fixed F5 DATA-IN
commands. Each F5 reads 0x8000 bytes; the 16 chunks in each view cover
RAM:f00000..f7ffff and RAM:f80000..ffffff. Exact CDBs, lengths, status and
identity are retained in report.json. Every chunk returned GOOD/full length.
Both complete views have SHA-256
`bb65ae3fbe63beca498644bde550a18f12171e27c7fabd54adc59877c6ef6549`.

All programmed main/fixed-data/checksum bytes equal diag15, the protected
monitor equals its reference, and the stored and computed monitor checksum
are both 0xdf68. The verifier's overall pass=false is solely due to two
preserved configuration differences from its older backup: RAM:ffa077 00->03
and RAM:ffa07f 05->08, matching the pair observed in prior captures. See
comparison-details.json. Do not describe the complete composite as identical,
or mistake this result for failed programming.

Next proposed experiment: user-operated rollback to archived diag14 and
normal startup, checking whether the last responsive image restores command
response. It still has the known unsolicited tray behavior; this is a
diagnostic comparison, not an accepted functional firmware. Source retains
the proven input-address corrections. No agent flash/reset/activation was sent.

## Diag14 rollback and P1.7 isolation candidate

The user restored diag14 and reports the same opens-closes-opens-and-stays
tray sequence. Agent PnP query confirms the original bridge and optical child
revision1.04, both OK. The separate Win32_CDROMDrive query and normal-mode
readback verifier remained blocked for several minutes; their identified
processes were stopped. No completed readback report exists in
`out/agent-tests/diag14-rollback-readback-20260926/`. This establishes returned
PnP detection, not reliable command response. Earlier diag3 command evidence
must not be attributed to this new readback attempt.

Diag16 isolates the P1.7 correction because this predicate participates in
several startup branches. It is deliberately an incomplete diagnostic:
P1.5 and P1.6 keep their known diag14 address defects; maintained C retains
all three corrections. The offline generator pins both input image hashes,
checks the exact linked instruction sequence, patches only P1.7's operand,
recalculates the checksum, and verifies the precise output delta and format.
An initial incorrect opcode expectation was rejected before creating any
output; the guard now matches the compiled MOV/JNB sequence.

Reproduce with:

```powershell
python tools/make_w5224a_p1_bit7_diagnostic.py --diag14 out/agent-tests/diag14-interrupt-restart-fix-20260926/PXW5224A_diag14_interrupt_restart_fix.bin --diag15 out/agent-tests/diag15-debounced-input-fix-20260926/PXW5224A_diag15_debounced_input_fix.bin --output out/agent-tests/diag16-p1-bit7-only-20260926
```

The output directory must be new. Result:
`out/agent-tests/diag16-p1-bit7-only-20260926/PXW5224A_diag16_p1_bit7_only.bin`,
SHA-256 `989805a470f691cd49da9e7c41a9bc0f1398cb98a055da927a6135f82108273a`,
checksum dd92. The only differences from diag14 are RAM:f96324..f96325
(13 00->11 ed) and RAM:ff9ffe..ff9fff (dc a7->dd 92). Full P1.7 helper bytes
equal the simulator-tested diag15 helper; every other nonchecksum byte equals
diag14. Format, both checksum expressions and stock fixed data pass. No new
simulator run or physical result is claimed.

User test: redumper flash, then normal startup with tray initially closed and
empty; record detection and tray sequence. Recovery is the established
Eject-at-power-on stock restore. Failure to detect implicates P1.7 correction
or its downstream effects; successful detection leaves P1.5/P1.6 or combined
effects open. Neither result makes the intentionally incomplete image an
accepted functional reconstruction.
