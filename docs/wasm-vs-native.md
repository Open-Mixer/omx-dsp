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
