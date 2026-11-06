# C251 exact-match fixture for RAM:fe19de

`MapThreeFieldStatusCode` is a bounded compiler-identification fixture from
firmware 1.04. It occupies 56 bytes at `RAM:fe19de..fe1a15`, has no data
references or calls, receives its three inputs in `ACC`, `R7`, and `R6`, and
returns through near `RET`.

Run the reproducible matrix against an installed C251 release:

```sh
python tools/run_fe19de_compiler_matrix.py \
  --keil-root /path/to/C251
```

The runner compiles each source form under the requested optimizer levels and
SIZE/SPEED preferences, extracts only the function bytes from the compiler
listing, and ranks them against immutable `MONT_104.bin`. Outputs and Keil
provenance records go under `out/agent-tests/` by default. Exit status zero
means an exact match was found; status one means the matrix completed without
one.

The C251 5.60 default O0-O8 and O3-O8 SIZE/SPEED sweeps produced no exact
match. The closest structural result was `switch_expression.c` at O3-O6: 60
bytes, including a 27-byte common block. `nested_switch.c` at O3-O6 reproduced
the original destructive `R7` switch sequence and shared-return topology, but
C251 5.60 selected a different inner-switch comparison order. O7-O8 instead
emitted multiple early returns. This is evidence for testing an older C251
optimizer; it is not yet identification of a particular release.
