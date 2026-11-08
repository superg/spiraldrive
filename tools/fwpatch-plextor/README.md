# fwpatch-plextor

Generic Rust firmware patcher for Sanyo-based Plextor drives. Applies JSON-defined
byte patches and compiled payloads, updates the product revision, and recalculates
the checksum.

Build from the repository root (Rust edition 2024):

```sh
cargo build --release --locked --manifest-path tools/fwpatch-plextor/Cargo.toml
```

In VS Code, use **Terminal → Run Build Task** (`Ctrl+Shift+B`).

GitHub Actions checks formatting, Clippy, and tests on pushes to `main` and pull
requests. Push a tag matching the Cargo version, such as `fwpatch-plextor-v0.1.0`,
to publish Windows and Linux x86/x64 and macOS Intel/Apple Silicon ZIP archives.

The executable is `tools/fwpatch-plextor/target/release/fwpatch-plextor`
(`.exe` on Windows). Run it with:

```text
fwpatch-plextor <original-firmware> <description.json>
```

Writes `<original-stem>_patched.<extension>` in the current directory, replacing
any existing output while protecting the firmware, JSON, and payload inputs.

The JSON description contains:

- `sha1`: 40-digit SHA-1 of the original image.
- `revision`: four printable ASCII characters.
- `patches`: objects with a hexadecimal file `offset` (such as `"0x6c21a"`) and
  a `value` of whitespace-separated hexadecimal byte pairs (such as `"1a 01"`).

For example:

```json
{
  "offset": "0x6c21a",
  "value": "1a 01"
}
```

Both the description and each patch accept an optional string `comment`.
See [york-107.json](../../firmware/px-w4824a/york-107.json) for an example.

Validates the input hash, checksum, patch bounds, and overlaps.
Current image validation accepts 512-KiB PX-S88T/OTTAWA, PX-W8432T/MUNCHEN,
PX-W1610A/SEATTLE, PX-W2410A/WINCHES, PX-W4824A/YORK, and PX-W5224A/MONT images.
Revision and checksum fields are managed automatically. Recovery and padding
regions are preserved.

An optional `payload` object supplies paths to compiled Intel HEX and Keil map
files. Paths resolve relative to the JSON file. HEX addresses map into the known
PX-W4824A/PX-W5224A image layout beginning at `0xf80000`. Sparse records preserve
the stock bytes between payload sections. Payload destinations must contain `ff`.

```json
"payload": {
  "hex": "../../build/px-w5224a/spiraldrive/payload/c4.hex",
  "map": "../../build/px-w5224a/spiraldrive/payload/c4.map"
}
```

The HEX reader accepts data, EOF, and extended-linear-address records, validating
their checksums, lengths, addresses, and uniqueness. Named ECODE symbols resolve
from the Keil map using C names without trailing `?` characters. Replace `value`
with `symbol` and `size` to write an address as three or four big-endian bytes:

```json
{
  "offset": "0x4207",
  "symbol": "handle_scsi_opcode_c4",
  "size": 4
}
```

Each patch requires either `value` or `symbol`. Symbol patches require a payload
map and a target address within the loaded payload. All writes participate in
bounds, overlap, and protected-region checks.

Optional `vendor_specific` text replaces the 20-byte inquiry field in layouts with
a known field location. It accepts up to 20 printable ASCII bytes, padded with
spaces. See [mont-104.json](../../firmware/px-w5224a/mont-104.json) for a complete
payload definition.
