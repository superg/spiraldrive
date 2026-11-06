# C251 optimizer corpus

`corpus.c` contains small, independently bounded source reconstructions used to
choose a reasonable compiler baseline. Run it from the repository root:

```sh
python3 tools/run_c251_optimizer_corpus.py \
  --keil-root /path/to/C251 \
  --levels 3,4,5,6,7 \
  --models LARGE \
  --preference SIZE
```

The runner extracts generated function bytes from the C251 listing and compares
them with immutable firmware 1.04 ranges. Its results rank source/compiler
similarity only; they do not establish compiler provenance or behavioral
correctness. Output belongs under `out/agent-tests/`.
