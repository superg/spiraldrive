# Reconstructed firmware physical-memory layout

This document defines the physical ownership contract used by L251 and the
image packer for Plextor PX-W5224A firmware 1.04. Canonical behavior and symbol
identity remain authoritative in `/validated/MONT_104.bin`.

## Flash image ownership

| RAM range | Size | Owner | Policy |
|---|---:|---|---|
| `f80000..f8004f` | `0x50` | packer | immutable 1.04 identification header |
| `f80050..feb4ff` | `0x6b4b0` | L251 `ECODE` | reconstructed executable code and vector veneers |
| `feb500..fefeff` | `0x4a00` | L251 `HCONST` | relocatable reconstructed constants |
| `feff00..feffff` | `0x100` | L251 `CODE` | C251 runtime initialization table/code |
| `ff0000..ff7fff` | `0x8000` | recovery overlay | erased in the downloadable image; never linker-owned |
| `ff8000..ff9ffd` | `0x1ffe` | packer | canonical fixed-data copy from firmware 1.04 |
| `ff9ffe..ff9fff` | `0x2` | packer | final big-endian image checksum |
| `ffa000..ffffff` | `0x6000` | runtime overlay | erased in the downloadable image; never linker-owned |

The three linker-owned flash classes are disjoint and together cover exactly
`RAM:f80050..feffff`. The ECODE/HCONST boundary at `RAM:feb500` balances the
current O5/SIZE executable and relocatable-constant requirements while the
standard C251 `CODE` class retains a dedicated final page. The fixed main-loop
segment remains at `RAM:fdcea9`.

The immutable header and `RAM:ff8000..ff9ffd` fixed-data range are packer-owned
and are not duplicated as linker inputs. Target code addresses the fixed range
at its canonical address. The packer copies it from the hash-pinned 1.04
reference before calculating the new image checksum. Tables in
`RAM:f82320..f848ff` and the 73 code-as-data bytes at `RAM:f894e3..f8952b`
remain relocatable HCONST because reconstructed code overwrites their canonical
main-payload addresses.

## Volatile data ownership

| RAM range | Owner | Policy |
|---|---|---|
| `000000..0000ff` | CPU/direct data | register banks, direct RAM, bit aliases, and stacks |
| `010000..017fff` | L251 `XDATA` | compiler-generated temporaries and static data |
| `018000+` | LC898095 MMIO | fixed volatile hardware contract; excluded from XDATA |
| `100000+` | controller buffer window | fixed volatile hardware contract; excluded from XDATA |

Other low external-RAM addresses used explicitly by firmware remain fixed
runtime-state contracts and are not allocated by L251.

## Capacity accounting

The O5/SIZE baseline with the selected C251 runtime currently uses:

| Class | Used | Physical capacity |
|---|---:|---:|
| `ECODE` | `0x6b35b` (439,131) | `0x6b4b0` (439,472) |
| `HCONST` | `0x484e` (18,510) | `0x4a00` (18,944) |
| `CODE` | `0x31` (49) | `0x100` (256) |
| Combined main payload | `0x6fbda` (457,690) | `0x6ffb0` (458,672) |

The combined payload has `0x3d6` (982) bytes of headroom. Class-local
headroom is intentionally narrower: `0x155` (341) bytes in ECODE and `0x1b2`
(434) bytes in HCONST before locator gaps and alignment. The physical link and
image validators remain the authority on whether a particular build fits.

`PLEXTOR_FIRMWARE_DIAGNOSTIC_LAYOUT=ON` is a locator-only troubleshooting
mode. It moves HCONST and CODE outside the downloadable-image window and lets
ECODE occupy `RAM:f80050..ffffff`, including reserved recovery/fixed regions.
It must never feed the image packer or a flash operation; its sole purpose is
to reveal which fixups remain after every relocatable segment receives an
address.
