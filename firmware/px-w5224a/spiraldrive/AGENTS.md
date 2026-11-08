# SpiralDrive PX-W5224A firmware

Root instructions apply. Keep firmware-specific preferences in this file.

## Code style

- Name new or renamed functions in `snake_case`, for example `read_sector`.
- Name variables in lowercase `snake_case`, including globals, locals, parameters, structure fields, and hardware register/bit mappings.
- Name constants, macros, and enum values in uppercase `SNAKE_CASE`, including `constexpr` constants.
- Name new or renamed structure, enum, and other types in `PascalCase`, for example `DriveState` and `ReadMode`.
- Keep abbreviations and acronyms fully uppercase in PascalCase identifiers, for example `SpiralDriveCDB` instead of `SpiralDriveCdb`.
- Use the repository's `.clang-format` as the source of truth for C/C++ formatting. Read it before editing and run `clang-format` on changed code where available.
- Leave three empty lines after an include block before the rest of the code.
- Leave two empty lines between function implementations, including before any comments attached to the next function. This spacing does not apply to function declarations.
- Make headers self-contained and keep compiler calling conventions explicit on functions so include order does not affect compilation or behavior.
- Use structure fields directly instead of copying them into local aliases unless a snapshot or transformation is needed.
- Omit integer constant suffixes such as `u`, `l`, and `ul` unless required for correct arithmetic, signedness, or compiler behavior. Write `0` and `4` instead of `0u` and `4u`.
- Start comments with lowercase text, use one sentence whenever possible, and omit the final period. Preserve the casing of names, identifiers, and acronyms.
- In CMake files, follow the same column limit where practical and use the available width instead of wrapping arguments onto many short lines.
- In CMake files, enclose string literals in double quotes, for example `"string"`.
- Keep CMake files minimal and readable. Avoid unnecessary settings, helper variables, and checks.
- Place `add_subdirectory()` calls at the end of CMakeLists.txt files.
- Apply these rules while editing code. Do not rename existing identifiers solely to change their style.

## Project rules

- SpiralDrive names the custom firmware and its C4 extension. For custom firmware identifiers and artifacts, use `spiraldrive` in lowercase and `SPIRALDRIVE` in uppercase macros and CMake settings.
- Use source and header filenames without the `spiraldrive_` prefix.
- Prefix stock firmware variables, memory locations, registers, bits, and shared buffers with `firmware_`. Use descriptive names supported by the sibling `../decomp` analysis project, keeping unresolved roles explicitly unknown. Use neutral names for stock firmware functions, without `spiraldrive`, `SPIRALDRIVE`, or `SpiralDrive` prefixes.
- The static `../mont-104.json` is the source of truth for the `SpiralDrive vX.Y.Z` SCSI INQUIRY text and compiled HEX/map paths.
- C4 SEEK accepts a signed raw controller displacement from -32767 through 32767 and executes one movement. Redumper owns step selection, repetition, and positioning policy.
- Successful C4 SEEK returns one DATA IN byte with GOOD: NONE (0) for completion (including SEEK(0)), or INNER_LIMIT (1) for sensor-confirmed inward home detection. Failures return CHECK CONDITION with meaningful sense, without a result byte. Only interpret the result after GOOD.
- Report C4 command errors with meaningful standard SCSI sense keys and ASC/ASCQ values supported by the detected failure. Preserve specific stock-helper errors instead of replacing them with a generic error. Keep named status and sense constants, leave information invalid when no disc address is known, and never turn raw Q/C2 errors into command failures.
- Keep SEEK(0) as stop-capture/discard-stream without movement. Do not add separate STOP or HOME operations. Redumper owns homing policy. Inward SEEK must report sensor-confirmed inner-limit detection separately from other movement failures and allow outward recovery.
- Validate C4 movement with C4 SEEK/READ and raw-sector sub-Q. Do not mix stock D8 reads into the measurement sequence.
- Use `s32` for the shared C4 CDB value. SEEK interprets it as displacement and READ requires a positive frame count.
- C4 READ captures raw main/C2/subchannel frames at the current pickup position. Do not gate it on decoded Q, disc addresses, track type, or lead-in/lead-out boundaries, and do not reposition during READ.
- C4 READ returns DATA/C2/SUB/SUBQ per frame. SUBQ contains only the 12-byte Q packet including CRC, excluding the four trailing bytes of the controller's 16-byte slot.
- SpiralDrive and redumper aim to preserve all controller-delivered disc data as raw as possible, including lead-in, lead-out, C2 information, invalid subcode, and damaged data. When the controller reports no completed subcode slot (0xffff), preserve the captured main/C2 frame and return 96 zero bytes for its unavailable subcode. Otherwise preserve captured subcode unchanged. Do not introduce a readable-start quality gate or discard, repair, or replace returned frames based on Q validity or read-quality indicators.
- When a full C4 buffer stops capture, drain its frames and report continuity lost once before returning any post-gap frames. The following READ may start a new stream without an explicit recovery operation.
- C4 READ is a raw variant of D8: preserve normal servo and idle behavior, but never use subcode or read errors to select, retry, or reposition a read.
- Buffer-full handling must stop raw acquisition while retaining queued frames and normal servo control. SEEK discards the queue and pending gap. Do not add an inactivity timer for this behavior.
- After C4 capture stops, reuse the stock D8 idle speed policy and countdown while preserving queued frames and avoiding stock position acquisition.
- Cap C4 movement and capture at the stock D8 lead-in 4x ceiling, preserving lower native limits and the saved host speed request. Apply and settle speed before nonzero SEEK and new READ acquisition. SET CD SPEED occurs only before C4 startup, not during a dump.
- Run hardware reading tests at 4x speed and reapply the speed setting after flashing.
- Investigate controller faults using firmware analysis and software diagnostics. External hardware probing and logic analyzers are out of scope.
- For ringed discs, prioritize continuous forward capture through unreadable areas without repositioning. Valid data inside the rings is not required.
- This is a development project. Do not add backward compatibility, migration, or cleanup code for old interfaces, settings, caches, or behavior unless the user explicitly asks for it.
- The default CMake build includes the Keil payload, Rust `fwpatch-plextor`, and final patched firmware image.
- Firmware packaging depends on the original firmware, static JSON definition, compiled HEX/map, and Rust patcher. Build in repository-root `build/px-w5224a/spiraldrive` to match the paths in the JSON.
- Keep Keil discovery and build mechanics in a reusable, project-independent CMake module. Declare payload sources and settings in `src/CMakeLists.txt`.
- Minimize explicit payload build settings. Prefer tool defaults and linker-resolved symbol addresses over fixed function placements.
- Prefer named volatile absolute variables for fixed firmware memory locations, with explicit memory qualifiers and one definition in the memory map. Avoid memory-access macros and stored pointers for new mappings.
- Prioritize clean, readable C code when refactoring memory accesses. Preserve behavior and verify builds. Byte-for-byte firmware equality is not required.
- Keep memory mapping simple. Do not add linker settings or scripts for these refactors.
- Keep the memory map as the source of truth for named absolute variables. Do not duplicate a per-variable address list elsewhere.
- Order memory-map variables by ascending address and keep header declarations in the same order as their definitions.
- Consolidate target types and stock firmware mappings in `src/firmware_memory_map.h` and `src/firmware_memory_map.c`. Do not use `src/target/` or separate bit headers. Omit header comments that only repeat addresses from definitions. Keep Keil-required SFR declarations in the header.
- Use `tools/fwpatch-plextor` from the repository root for firmware packaging. It identifies stock firmware by SHA-1, loads sparse HEX and Keil map symbols, applies JSON patches, and updates the checksum.
- Ask the user before adding or expanding permanent stock-firmware patches required for custom firmware operation. Temporary diagnostic hooks do not require separate approval.
- Do not add firmware source tests unless the user explicitly requests them. The Rust patcher follows the root test policy.
- Flash H: with `C:\Users\G\projects\redumper\build\Release\redumper.exe flash::plextor --drive=H: --firmware <image>`. Wait about ten seconds before checking readiness, without unplugging the drive.
