# fwpatch-plextor

Generic Rust firmware patcher for Sanyo-based Plextor drives. Applies JSON-defined
byte patches, updates the product revision, and recalculates the checksum.

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
any existing output while protecting both inputs.

The JSON description contains:

- `sha1`: 40-digit SHA-1 of the original image.
- `revision`: four printable ASCII characters.
- `patches`: objects with a hexadecimal file `offset` (such as `"0x6c21a"`) and
  equal-length `old`/`new` byte strings (such as `"9c 00"`/`"1a 01"`).

Both the description and each patch accept an optional string `comment`.
See [york-107.json](../../firmware/px-w4824a/york-107.json) for an example.

Validates the input hash, checksum, original bytes, patch bounds, and overlaps.
Current image validation accepts 512-KiB PX-S88T/OTTAWA, PX-W8432T/MUNCHEN,
PX-W1610A/SEATTLE, PX-W2410A/WINCHES, PX-W4824A/YORK, and PX-W5224A/MONT images.
Revision and checksum fields are managed automatically. Recovery and padding
regions are preserved.
