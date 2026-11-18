<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> -->
# The curve function as WebAssembly against the native build

One C source (`omx_curve_json.h`) is compiled twice: natively against glibc, and to WebAssembly with clang and wasi-libc
(`tools/wasm-build.sh`), whose libm is musl's. Both are held to the same closed-form vectors
(`vectors/curve-points.json`, `make curve-vectors-wasm`) within each row's own tolerance, and both pass. Where the two
doors are put against each other the doubles are not always the same: the difference is REPORTED here and by
`node test/curve-vectors.mjs --native ... --wasm ... --report`, and no tolerance was widened to absorb it.

## What differs (measured 2026-11-17, clang 22.1.8, wasi-libc 34, glibc of Fedora 44)

- The transfer curves (dynamics, de-esser, limiter, gate), the delay's x, the notch floor and every refusal: identical,
  bit for bit. They use no libm call, or one (`log10`, `pow` in the delay's levels: 1 ulp on one stem).
- The EQ rows (head filters, bell, notch): 68 of 504 numbers differ, by at most 3.5e-14 dB in absolute terms. The ulp
  counts are large (up to 3e11) only where the answer is a cancellation near 0 dB; the same gap is 1 ulp where the answer is
  around 1 dB. The cause is `cos`, `sin`, `exp` and `pow` of musl against glibc inside the biquad design and the magnitude.
- The wide agreement grid of `ts/test/agreement.test.mjs` (the module against the native oracle at the nine oracle rates)
  measures the largest gaps: EQ 2.6e-12 dB, the drive's shaper 3.8e-5 dB (float `tanhf`/`expf` of musl against glibc,
  within the float step the drive is held to), the time response 9e-13. Each is inside the bound its row declares in
  `ts/test/tolerances.mjs`, which are the bounds the TypeScript twin was held to; none was changed.

A change of the toolchain that moves these numbers is a finding to read here, not a tolerance to widen.

## What it costs, against the TypeScript twin it replaced

`tools/curve-bench.sh` builds the twin from the commit before its deletion and puts the shared vectors' 22 requests to each
door 1000 times, every door in its own process (Node 22, one core, Fedora 44, the module built in a debian trixie container):

| | median per call | p99 per call | load | peak resident set over a bare Node's |
|---|---|---|---|---|
| WebAssembly `curvePoints` (JSON in, JSON out) | 4.2 us | 22.5 us | 0.15 ms (first 1.0 ms), 92 080 bytes | +30.5 MB |
| WebAssembly `pointsJson` (the C call on text) | 2.3 us | 25.9 us | | |
| the TypeScript twin | 0.41 us | 16.3 us | 6.7 ms (first import) | +21.4 MB |

The module answers a curve about ten times slower at the median than the twin did, and costs about 9 MB more of resident
memory: each call is a JSON document in and out, and the twin was a JIT-compiled loop. A curve is drawn on a control change,
a few microseconds against a frame of 16 ms; it loads in a fifth of the time. Against the twin its answers are 21.7 ulp of
max(|v|, 1) apart at the largest, 3.6e-12 dB absolute, over 488 numbers (the twin's own bounds are in
`ts/test/tolerances.mjs`). Run to run the medians move by a few percent. `node tools/curve-bench.mjs --json` prints the
same as data.
