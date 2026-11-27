<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> -->
# Building omx-dsp

Needs a C11 compiler, GNU make, binutils, awk, sed, grep and diffutils.

| command | does |
|---|---|
| `make` | `build/libomxdsp.a`, `build/libomxdsp-contracts.a` and `build/libomxdsp-tsan.a` |
| `make test` | the whole suite: every primitive's and kernel's oracle at every declared rate with contracts on, the negative, perturbation and thread arms, the writable-data and doc checks, and the golden digests |
| `make check-package` | what the deb's and the RPM's check run: the suite, the negative, consumer and thread arms, every oracle and golden digest, the flavours and the libmvec oracles; the sabotage and perturbation arms and the source checks run in `make test` (`make check-package` is derived from `make test`'s list in the Makefile, held by `tools/check-package-derived.sh`, part of `make checks`) |
| `make test-fx` | the effect kernels' oracles and golden digests alone |
| `make consumer` | the stateful stages built the way a consumer builds them (release flags, `-Werror`, states on the stack that only their init or settle writes) and run |
| `make test-analysis` | the feedback detector's, the RTA ring and spectrum's and HRP's oracles (the HRP render and live core among them) at the nine oracle rates (32 to 192 kHz), their no-allocation arm, golden digests and one sabotage per kernel; the room analysis, Farina sweep and alignment fit against closed forms at the nine rates and against core's TypeScript (the parity table), and one sabotage per kernel that both must catch; alone |
| `make test-engine-kernels` | the golden digests of the sample-rate converter, the generators and the analyser's readings at the nine oracle rates, which the engine's own copies wrote, and a break in each that must move its own digests and no other's |
| `make install PREFIX=/usr LIBDIR=/usr/lib64 DESTDIR=…` | headers, the three archives and `omxdsp.pc`, `omxdsp-contracts.pc`, `omxdsp-tsan.pc` |
| `make flavours` | each archive carries its flavour, and a contracts consumer reads a violation raised inside the compiled code only through the contracts archive |
| `make docs` | the API reference with doxygen, over every installed header (`include/omxdsp` and its `fx/`, `mix/` and `analysis/`) |
| `make docs-check` | the documentation gate (run on every pull request): doxygen's undocumented-declaration and undocumented-parameter warnings against `docs/doxygen-baseline.txt`, one row per public declaration still undocumented, compared as a set, which only shrinks: a warning not in it is red, a row no longer warned is red, a row the branch added against `DOCS_BASE=<remote>/<branch>` (required; the branch is fetched from the remote first, and an unset base, a failed fetch, an unresolvable ref or a base without a baseline is red by name — never a stale remote-tracking ref, never the head's own copy) is red, and what doxygen cannot see is red by name before it runs: a missing Doxyfile, baseline or warning log, a Doxyfile without the gate's lines or with a filter, a header without a doc-comment `@file`, a `@cond`/`@internal`/`@if`, an untracked header under `include/omxdsp`, a tree that is no git checkout, a baseline not sorted and unique; `make docs-baseline` rewrites the baseline from the tree and refuses to add a row; `make docs-sabotage` proves each red on a copy of the tree. The CI runner's doxygen (ubuntu-24.04, 1.9.8, pinned) is the authority; another doxygen may add doc-error rows |
| `make test-support-install` | the test support `make install` puts in `$(DATADIR)/omxdsp/` (`test-support/`: `log10f_cr.h`, `log10f_subst.h`; `tsan-gate.sh`), named by `omxdsp.pc` (`testsupportdir`, `tsangate`) for a consumer's native tests, read back from a scratch install |
| `make exports` | `build/omx-dsp-exports.json`: every public function and macro with its signature, its body as normalised tokens, its distinctive literals, maths calls, inline kernels and `@twin` names; `make install` puts it in `$(DATADIR)/omxdsp/` beside `omx-dsp-copy-check` in `$(BINDIR)` |
| `make copy-check` | `tools/omx-dsp-copy-check.sh --self-test`: planted copies in a scratch consumer (an exact copy, a renamed one, a TypeScript twin, a literal copy, inline kernels) must be found, a legitimate caller, boilerplate, tests and fakes must pass, and a stale or reasonless allowlist entry must fail; then each rule is broken in turn and the self-test must go red (part of `make test`, which runs it alone before everything else, so no parallel job runs beside it) |
| `make lint` | the doc check, the source scan, the log10f guard, tools/contract-single-source.sh (no name omx-contract defines is defined again here), tools/face-conformance.sh (every instance face's resolve takes its kernel's contract controls by name, order and kind), tools/travel-grid-check.sh (a travel's step reaches its maximum and default, a choice names its ids) and tools/clone-check.sh (a second copy of a body inside the library goes red); each checks its own sabotage |
| `make test-tsan` | the thread arm under ThreadSanitizer, where the toolchain has it |
| `make rtsan` | the per-block entry points under clang's RealtimeSanitizer, after a self-test that must see a planted blocking call (below); a dev tool, not part of `make test`, `CHECK_PACKAGE` or CI |
| `make rt-gate` | omx-rt-gate, the one static real-time gate (`omx-rt-gate(1)`): its class table measured by the externals arm, its planted fixtures read red at their place and path and their clean twins green, its load refusals, the `@thread` tag of every function of the shipped headers under three sets of flags, the self-run (every tagged audio and worker function a root, joined to the compiled units' graphs, nothing red), and the installed gate judging a consumer joined to the shipped graphs; `make rt-gate-sabotage` breaks each guard in turn (part of `make test`) |
| `make engine-kernels-identity OPENMIXER=<checkout>` | renders those digests again through the copies OpenMixer's engine still carries (`mix_src.h`, `mix_dsp.h`), and checks that each of its sabotages goes red; a desk check, not run in CI |
| `make test-mix` | the mix module's golden digests at the nine oracle rates (`test/golden/mix.sha256`, written by the engine's own copies), and a break in each kernel and in each of the bus stages' contract checks that must turn its arm red; alone |
| `make mix-engine-identity OPENMIXER=<checkout>` | renders the mix digests again through each set of copies OpenMixer's engine still carries (the bus stages' `omx_mix_strip`, `omx_bus_out_gain` and `omx_bus_out_stage` in `mix_dsp.h`, under the same names as `mix/omx_bus_stage.h`), and checks that each of its sabotages goes red; a desk check, not run in CI |
| `make strip-input-identity OMX_PLUGINS=<checkout>` | renders omx-plugins' channel strip's own input stage (`plugins/omx-strip/omx_strip.h`) and requires the trim face's golden digests, then its sabotage; a desk check, not run in CI |
| `make analysis-engine-identity OPENMIXER=<checkout>` | builds the analysis golden against the engine's own `mix_dsp.h`, `hrp_render.h` and `hrp_lv2.h` and requires the committed digests, then its sabotage; a desk check, not run in CI |
| `make eq-sabotage` | the EQ bank, head-filter and drawn-curve arms (`eq-curve`, `eq-design-ts`) against nine sabotaged copies of the headers; each must fail both arms (part of `make test`) |
| `make bench-mixmatrix` | the mix matrix's cost, then a 64 x 32 matrix held to MIXMATRIX_PERIOD_SHARE of the period at the nine rates, and a budget nothing can meet must go red; a desk check, not run in CI, whose shared runners time a core the console does not run on |
| `make golden-write` | rewrites every kernel's `test/golden/<kernel>.sha256`; only in a commit that bumps the minor version or adds a kernel |

`make -jN test` is supported. `tools/kernels-gen.sh` runs once, as the Make rule that produces
`build/kernels-suite.inc.c` and `build/kernels-perturb.inc.c`; every target that reads them lists
them as a prerequisite and no recipe or script calls the generator into `build/`. `make test-fx`
is one target per kernel, instance, oracle, golden and perturbation script (`fx-run-*`), so `-j`
runs them side by side; the set run and each assertion are the same as serially.
`make gen-once` (`test/gen-once.sh`, part of `make test`) counts the generations under `-j8` and
must see exactly one; it then re-adds the generator call to a recipe in a scratch copy of the
Makefile, which must turn it red.

Consumers compile the kernels through `pkg-config --cflags omxdsp`, which carries
`-ffp-contract=off`: the kernels are inline, and without it an architecture with fused
multiply-add would round differently from one without.

## Flavours

The compiled part of the library (the oversampler, the FBS detector, the room analysis, the Farina
sweep and the alignment fit; everything else is inline in the headers) is built once per flavour
into `build/`:

- `libomxdsp.a`: `CFLAGS` only, contracts compiled out.
- `libomxdsp-contracts.a`: objects in `build/contracts/`, with `-DOMX_CONTRACTS`.
- `libomxdsp-tsan.a`: objects in `build/tsan-lib/`, with `-DOMX_CONTRACTS -fsanitize=thread`.

Each has a pkg-config file of the same name (`omxdsp`, `omxdsp-contracts`, `omxdsp-tsan`) whose
cflags carry the flavour's flags, so the inline headers and the archive are always built the same
way. The suite, the thread arm and the contract-checked kernel tests link `libomxdsp-contracts.a`,
`make test-tsan` links `libomxdsp-tsan.a`, and the release-flag kernel tests link `libomxdsp.a`.
`make flavours` (`tools/flavour-check.sh`, part of `make test`) checks each archive's symbols and
builds `test/omxdsp_flavour.c`, a contracts consumer that feeds a NaN to the compiled oversampler:
against `libomxdsp-contracts.a` it must find the violation in its own ledger, and against
`libomxdsp.a` it must fail.

## GCC 12.2 on arm64

Debian bookworm's GCC 12.2 on arm64 (Raspberry Pi OS, Zynthian) crashes with an internal compiler
error in `vect_transform_reduction` on a loop that keeps a `double` running maximum or minimum with
`fmax`/`fmin` over values widened from `float`. Write such a reduction as a comparison instead,
`if (!(e <= worst)) worst = e;`, which also carries a NaN into the result. `make lint` refuses the
shape on any host (`tools/reduction-check.sh`), and CI builds the suite on bookworm arm64.

## Golden digests and libm

A golden digest covers the kernel's output bit for bit, so it also covers every libm call the
kernel makes per sample. A libm's `log10f` is not one function: glibc 2.36 (debian:bookworm)
misrounds 1.39% of the non-negative floats, 62 225 of them by 2 ulp, where glibc 2.41 (debian:trixie)
and 2.43 (Fedora 44) round every one correctly, since 2.41 took `log10f` from CORE-MATH. The limiter's
48 kHz digest (the render of FreeMixer/omx-dsp#21) read `5fe47250…` on bookworm against `69a449ce…`
on 2.41 and 2.43. The compiler played no part: the same binary flips with the glibc it runs on.

Production calls the libm: `omx_lin_to_db()` is `20·log10f(max(lin, 1e-9))`, about 2 ns per call
against 10 ns for a libm-free `log10f` on glibc 2.43. The goldens do not: `omx_log10f()` lives in
`test/support/log10f_cr.h`, a correctly rounded `log10` in IEEE double arithmetic (it needs
`FLT_EVAL_METHOD == 0` and `-ffp-contract=off`, which the Makefile's `FPFLAGS` carry), and every
effect-kernel golden TU is compiled with `-include test/support/log10f_subst.h`, which defines `log10f` as
`omx_log10f`. The kernels are static inline, so the substitution reaches them, and every kernel's
golden is an exact digest on every toolchain. Two guards hold it: `golden.h` refuses to compile a
golden without the substitution and the golden driver exits 2 when `log10f` there is not
`omx_log10f`, and `tools/log10f-guard.sh` (part of `make lint`) fails when `omx_log10f` appears
under `include/` or `src/`. `make check-log10f` compares `omx_log10f` with the host libm's `log10f`
on all 2³¹ non-negative floats; against glibc 2.43 none differ. `test/kernels/units.c` tests it
against fixed bit patterns, including `0x0efeee7a`, 7.8e-10 ulp from a float midpoint, which is
answered from its exact value. The analysis golden (`test/analysis/golden.test.c`), whose HRP
code calls `log10f` through the compiled headers, links with `-Wl,--wrap=log10f` and answers each
call with `omx_log10f`; it exits 2 if the wrap was never reached.

On glibc older than 2.41 a production build may therefore differ from the golden by up to 2 ulp of
a dB value in the units that call `log10f`. That is accepted.

The limiter's other per-sample call, `powf` in `omx_db_to_lin()`, is the same implementation in
every glibc since 2.28 and returned the same bits for equal arguments on 2.36, 2.41 and 2.43. A
kernel that adds a per-sample libm call adds it to the RT-safe allowlist named in its `@note`, and
checks that the goldens agree across glibc versions, or uses a libm-free word.

## libmvec

The drive's shaper (`omx_drive_shape_block`) and the dynamics gain curves (the loops of
`omx_dynamics_keyed` and `omx_dyn_flat_gain_block`) are stateless per sample, so their `tanhf`,
`log10f` and `powf` run four samples at a time through glibc's libmvec. Measured on a Pi 5 (glibc
2.41, gcc 14, 256-sample blocks), libm against libmvec in ns per sample: tanhf 9.34 to 2.52,
log10f 8.08 to 1.87, expf 6.22 to 1.80, powf 11.74 to 7.22. `include/omxdsp/omx_fastmath.h` makes the
`#pragma omp declare simd notinbranch` declarations, because glibc's own `<math.h>` makes them only under
`__FAST_MATH__`, which this library never uses (`-ffast-math` sets flush-to-zero at load).

The build decides, per function, by compiling and linking a probe (`tools/fastmath-probe.sh`): a function
counts only when the compiler turns a loop over it into a call of its `_ZGV` symbol and the program links with
`-lm -lmvec`. The Makefile then adds `-fopenmp-simd -fno-math-errno` and
`-DOMX_FASTMATH_MVEC_<F>=1` for each function found, and `-lmvec` to the link; `omxdsp.pc` carries the same in its
`Cflags` and `Libs`, so a consumer that links the static library takes libmvec on exactly the targets where the
library was built with it (`pkg-config --cflags --libs omxdsp`). Where libmvec is absent (Debian 12 on arm64;
glibc older than 2.35 on x86-64; glibc older than 2.40 for `tanhf` and `powf` on aarch64) the probe finds nothing, the
flags are empty and every kernel calls the libm as before. `make FASTMATH=off` forces that path. `-fopenmp-simd`
links no OpenMP runtime; `-fno-math-errno` lets the compiler treat the four functions as pure, and enables no
exception. The trapping-math default is kept: the loops select with `omx_fm_select()` rather than `?:` on floats, so
nothing has to be speculated for them to vectorise.

The output through libmvec is not bit-identical to the libm's. Held by:

- `make fastmath`: the oracle (`test/fx/fastmath.test.c`), each function against the double-precision libm over
  the range the library uses, with its specials, and a check (`tools/fastmath-vec-check.sh`) that every loop
  above really calls the vector symbols;
- `make fastmath-diff`: every kernel that uses them rendered on the libm and on libmvec at every declared rate,
  the largest sample difference in dB re full scale, failing above `FM_DIFF_LIMIT_DBFS` (-96);
- `make fastmath-sabotage`: the oracle red on a result moved by 5 ULP, the loop check red on a header without
  the pragma, the probe empty for a toolchain with no libmvec, no pragma or no vector symbol;
- `make fastmath-plain`: the oracle, the drive and the dynamics again with `FASTMATH=off`.

The golden digests stay on the libm: their translation units are built with the flags before the libmvec ones
(`CFLAGS_PLAIN`), and the stage's contracts build (`-DOMX_CONTRACTS`) keeps its per-sample calls, so a digest or a
contract ledger does not depend on the glibc's libmvec.

## Rates outside the declaration

The console declares six rates (`OMX_DECLARED_RATES`), and the kernels' contracts refuse any other
with a `rate-is-declared` precondition. The dynamics, balance, rotor and limiter oracles also run at
32, 64 and 128 kHz, the other rates of the oracle set (`OMX_FX_ORACLE_RATES` in
`test/fx/fx_rates.h`), with contracts still on: there the `rate-is-declared` precondition is
expected, and every other law must hold. Their golden digest files carry all nine rates.

## The engine's copies

The sample-rate converter (`omx_src.h`), the generators (`omx_gen.h`) and the analyser's spectrum
readings and half-band decimator (`analysis/omx_rta.h`) moved here from OpenMixer's engine unchanged in
behaviour. Their oracles are suite arms (`test/kernels/src.c`, `gen.c`,
`rta.c`); `test/engine_kernels/golden.test.c` hashes their output per kernel at the nine oracle rates,
and `test/golden/engine_kernels.sha256` holds the digests the engine's own copies gave, written by
`tools/engine-kernels-identity.sh --write`. `make engine-kernels-identity OPENMIXER=<checkout>`
compiles the program again with those copies in place of the three headers, against the
omx-contract release the engine pins, and requires the same digests (set
`ENGINE_IDENTITY_INCLUDE` to the include directory of the omx-dsp the engine pins, e.g. `git archive
<its commit> include`, when this tree's other headers need a newer contract than the engine's); its self-test changes one line
in each copy and requires a failure each time. It is a desk check and CI does not run it: the copies
live in OpenMixer's private repository. It ends when the engine builds from these headers instead
of its copies.

The engine spells the converter's design rolloff `OMX_SRC_ROLLOFF`, a double; omx-contract renders
it as the float `OMX_SRC_ROLLOFF` and the double `OMX_SRC_ROLLOFF_DOUBLE`. A caller that wants the
engine's coefficients passes `OMX_SRC_ROLLOFF_DOUBLE` to `omx_src_design`.

## The channel strip's input stage

omx-plugins' channel strip ran its input trim and the trim's ramp as arithmetic of its own. That
stage is now two faces: `fx/omx_trim_instance.h` (the trim kernel's one control, the console's
de-zippered head gain) and `fx/omx_eq_instance.h` with every band off (the HPF and the LPF, which
the console runs as sections of its channel EQ). `test/golden/trim_instance.sha256` was written by
`make strip-input-identity OMX_PLUGINS=<checkout>` (`tools/strip-input-identity.sh`) from the
plugin's own `omx_strip.h`, and `test/fx/trim_instance_golden.test.c` holds the two faces to it at
every declared rate. Run it again whenever either face or the strip's input stage changes; it ends
when the strip is generated from these faces and holds no arithmetic of its own.

## Room analysis and the alignment fit

OpenMixer's room analysis and Smart Alignment's fit moved here from TypeScript:
`analysis/omx_align_fit.h` (the cross-spectrum accumulator, the
delay and polarity fit, the correction), `analysis/omx_room.h` (the H1 transfer and coherence,
the impulse response and its early reflections, RT60 per octave band, the mode finder and the
mode-EQ proposal) and `analysis/omx_sweep.h` (the Farina sweep, its inverse and the
deconvolution), compiled in `src/omx_align_fit.c`, `src/omx_room.c` and `src/omx_sweep.c`. They
run on a control thread, never in the audio callback: every buffer and scratch array is the
caller's, sized as each function states, and nothing is allocated, locked or kept between calls.
A measurement that cannot conclude returns a refusal code (`enum omx_analysis_refusal`), one per
word of core's refusal lists.

The numbers core declares for these readers and omx-contract does not yet carry (the frame
floor, the default and mode bands, the trust coherence, the analysis floor, the early-reflection
horizon and floor, the T20 span, the octave centres, the mode margin and position floor, the
sweep's fade fraction) are the caller's to pass; this tree types none of them. The proposal's
bounds are omx-contract's (`OMX_FBS_MAX_NOTCH_DEPTH_DB`, `OMX_EQ_GAIN_RANGE_*`,
`OMX_EQ_Q_RANGE_*`).

Two kinds of oracle hold them. `test/analysis/align_fit.test.c`, `room.test.c` and
`sweep.test.c` check closed forms at the nine oracle rates (a known delay, gain, reflection,
decay, set of resonances, sweep law) and count allocations. `test/analysis/room_ts.test.c` holds
every result to what core's TypeScript computed on fixed inputs, from the parity table
`test/analysis/fixtures/room_ts.h`: integers exactly, reals within 1e-9 relative (measured gap
about 1e-12; V8's libm against the host's). Every input is built from an integer PRNG and exact
arithmetic, so the C test rebuilds the same bits. The table is frozen: core's TypeScript now calls
these kernels for every one of these results, so the C is the only source of these numbers and
nothing writes the table again. `test/analysis/room-sabotage.sh` breaks each kernel once in a
scratch copy and requires its closed-form oracle and the parity test to fail.
## The EQ maths openmixer's TypeScript computed

openmixer designed its console EQ and drew its curve in TypeScript. The C here computes
the same results, and openmixer now takes every one of them from this library (`@openmixer/omx-dsp`,
or these kernels through its DSP port), so the C is the only source of these numbers. Where each
TypeScript function went:

| TypeScript | C |
|---|---|
| core `rbjSection`, `rbjCoeffs` (core `eq.ts`) | `omx_eq_design`, `omx_eq_design_f` |
| core `bandwidthOctavesQ` | `omx_eq_bandwidth_q` |
| core `butterworthSlopeQs`, web `butterworthSectionQs` | `omx_eq_butterworth_qs` |
| core `passFilterMaxSections` | `OMX_EQ_PASS_MAX_SECTIONS` |
| core `complementSection` | `omx_eq_complement` |
| core `eqBandIsIdentity` | `omx_eq_band_is_identity` |
| core `eqCoeffs` | `omx_eq_bank_design` |
| core `passFilterCoeffs` | `omx_eq_pass_design` |
| core `sectionMagnitudeDb` | `omx_biquad_magnitude_db_d` |
| core `biquadMagnitudeDb` | `omx_eq_band_magnitude_db` |
| core `eqCurveDb` | `omx_eq_curve_db` (its frequencies: `omx_eq_curve_freqs`) |
| web `magnitudeSumDb`, `seatY`'s value (web `eqCurveGeom.ts`) | `omx_eq_magnitude_sum_db` |
| web `eqStateBands`, `filterBands`, `passFilterBands` | `omx_eq_curve_bands` |
| web `eqStateCurveDb`, `compositeCurveDb` | `omx_eq_curve_bands`, then `omx_eq_curve_db` |
| catalog `octavesToQ` (catalog `eq.ts`) | `omx_eq_octaves_q` |

`test/kernels/fixtures/eq_design_ts.h` is what that TypeScript computed on fixed inputs at the nine
oracle rates, frozen when the TypeScript went (nothing writes it again; likewise
`test/kernels/fixtures/eq_curve_ts.h`, core's `eqCurveDb` points). The `eq-design-ts` arm holds the
C to it: a coefficient, a Q or a frequency of the drawn grid within 4 ulp, a magnitude within
1e-9 dB, the bank's order, count and parked sections exactly. Two families of coefficient pass 4
ulp, both a cancellation that turns one ulp of V8's `exp` or `cos` into many (a matched bell or
shelf at a low corner or a wide Q, and the fitted low-pass's b2 far below Nyquist); they are held to
1e-13 absolute, and the arm prints the largest gap of each kind. The `eq-curve` arm holds the same
functions to closed forms.

## Loudness

`omx_loudness.h` measures EBU R 128 loudness the way ITU-R BS.1770-4 defines it
(FreeMixer/omx-dsp#64). The engine measures and accumulates; it reads the figures and publishes
them, and no other code computes them.

- **K-weighting.** Two sections designed for the rate at init, in double, from the analogue
  prototype of BS.1770's 48 kHz table (a high shelf at 1681.97 Hz, +4.0 dB, and the RLB high-pass
  at 38.135 Hz), by the bilinear transform with `tan` prewarping. At 48 kHz they reproduce the
  standard's printed coefficients; at the other rates they are the same filter. Each leg runs
  through omx_biquad_d(), double history.
- **Blocks.** The K-weighted squares, each leg weighted (1.0 for L, R, C; 1.41 for the surrounds;
  0 drops a leg, the LFE), are summed in 10 ms sub-blocks (`rate / 100` samples, a whole number at
  each of the nine oracle rates). Momentary loudness is the last 40 sub-blocks (400 ms), short-term
  the last 300 (3 s); a window that has not yet filled counts the missing time as silence.
- **Gating.** Every 100 ms a full 400 ms block joins the integrated measurement and, once 3 s have
  passed, the short-term value joins the range. Neither keeps the blocks: each keeps a histogram of
  0.1 LU bins from −70 LUFS up, every bin holding its count and the exact sum of its energies. The
  absolute gate (−70 LUFS) is a bin edge; the relative gate (−10 LU for the integrated loudness,
  −20 LU for the range) is placed in energy, and the bin it falls in is kept when its own mean is
  above the gate. So a programme of any length takes the same state, and the integrated value is
  exact except for blocks within 0.1 LU of the relative gate.
- **Loudness range.** EBU Tech 3342: the 10th to the 95th percentile of the gated short-term
  values, each percentile read as the energy mean of the bin that holds it.
- **True peak.** The largest reading of omx_truepeak_block() over every leg: the shared ×4
  half-band interpolator, BS.1770-4's four, not a second one.

Nothing on the process path allocates, locks or calls libm: the bin edges and coefficients are
computed at init, and a reading calls `pow` and `log10` at most twice. The state is one caller-owned struct
without pointers, exported with OMXDSP_STATE_LAYOUT.

The oracle (`test/fx/loudness.test.c`) generates EBU Tech 3341's and 3342's test signals from
their closed forms at each of the nine oracle rates and holds the readings to the tolerances the
documents give: momentary, short-term and integrated ±0.1 LU, true peak +0.2/−0.4 dB, loudness
range ±1 LU. Generated: 3341 signals 1 to 6, 9, 12 and 15 to 19, and 3342 signals 1 to 4. In
place of the burst signals 10 and 13, a burst of exactly one window (3 s, 400 ms) at −23 dBFS is
placed off the 10 ms grid and the largest reading must be −23.0 LUFS ±0.1. Not generated: the
programme recordings (3341 signals 7 and 8, 3342 signals 5 and 6) and the signals published only
as files (3341 signals 11, 14 and 20 to 23). The true-peak tones are faded in over 20 ms: a sine
switched on at full level is a step, and its reconstruction overshoots. The golden digests
(`test/golden/loudness.sha256`) hold the readings in energy, so no libm `log10` enters them;
`test/fx/loudness-perturb.sh` breaks the shelf, the relative gate, the range percentile, the true
peak and the momentary window one at a time, and each must turn the oracle red and move every
digest.

## The definitions and the limits

The kernels and DSP constants this library implements are defined in `data/`: one file per kernel
under `data/kernels/`, one per constant under `data/primitives/`, in omx-contract's schema. The C
code reads them through `<omxdsp/omxdsp_definitions.h>`, rendered from `data/` by omx-contract's tool
(`make definitions OMX_CONTRACT=<checkout>`, which runs `omx-contract render --target c --package
omx-dsp`) and committed, so a build needs no node. Never edit the header: change the definition and
render again; `make definitions-check` fails while the two differ, and `tools/contract-single-source.sh`
fails on a second definition of any of its names. `make install` installs `data/` into the contract directory
under its own prefix (`$(DATADIR)/omx-contract/contract.d`, the installed omx-contract's directory at `PREFIX=/usr`)
as the package `omx-dsp`, with a `MANIFEST.json`, where the other packages read it. Nothing is written or
removed outside `$(DESTDIR)$(PREFIX)` (`make install-confined`, in a throwaway container).

The facts no single package implements (the declared sample rates, the parameter scales) stay in
Open-Mixer/omx-contract, read through `<omxcontract/omx_contract_limits.h>`, which the definitions
header includes. That dependency is declared by name, with no version: `packaging/omx-dsp.spec`
(`BuildRequires`, `Requires`), `debian/control` (`Build-Depends`, `Depends`) and `omxdsp.pc` name the package
and nothing more. This level builds and installs against the omx-contract package the channel publishes; a
relation that must exist is generated by the packaging, never typed (owner ruling 2026-11-24). Install it from
the OpenMixer channel with the rest of the build dependencies:

    dnf builddep packaging/omx-dsp.spec     # Fedora, with the OpenMixer repository added
    apt-get build-dep ./                    # Debian, with the OpenMixer source list added

`tools/contract-include.sh` finds the installed `omx-contract` with `pkg-config` and prints its include
directory; with none installed the build stops, naming the package. Nothing is downloaded and no version is
pinned anywhere. `tools/contract-include-selftest.sh` (`make checks`) holds the script, and the spec,
`debian/control` and `omxdsp.pc.in` to no ceiling; `tools/no-floor-check.sh` (`make checks`, its self-test
planting six floors) refuses a version floor put back in the packaging, `package.json` or `data/`.

`OMX_CONTRACT_INC=<dir>` names another include directory (one that holds `omxcontract/`) in place of the
installed one. The perturbation arms use it, and so can you: render a contract with one value moved
into a scratch directory, `make test`, and the kernel test that reads that value must go red. A value
the kernels need is added to its definition in `data/` and the header rendered again; it is never typed
in the code.

## RealtimeSanitizer

`make rtsan` (`tools/rtsan/rtsan.sh`) asks clang's RealtimeSanitizer whether any per-block entry point allocates,
locks, sleeps or makes another unsafe libc or system call while the oracles run. It never edits the tree: it copies
the tree (without `build/`) under `build/rtsan/`, and `tools/rtsan/annotate.py` marks `__attribute__((nonblocking))`
on the copy's `static inline` definitions of `omx_*_{process*,run,run_keyed,block,render,render_window,render_network}`,
skipping init, reset, size and validity queries and the reading helpers (the script's docstring lists the names).
It needs clang with compiler-rt's rtsan runtime (Fedora: `dnf install clang compiler-rt llvm`); without it the run
stops with `rtsan: UNJUDGED: no clang rtsan runtime`, exit 2, never a pass.

1. Self-test: a second copy, marked and with a `usleep(1)` planted in `omx_gate_run`, runs the gate kernel test
   (`fx-run-kernel-gate`). RTSan must report `usleep` with `omx_gate_run` on its stack (the run keeps `-g`); otherwise `rtsan: FAIL the sanitizer saw nothing it
   should have`, and no clean result is claimed.
2. The run: the marked copy builds and runs `fx-oracles mix-golden analysis-oracles engine-kernels-golden consumer`
   and `suite` with `CC=clang CFLAGS="-Wall -O1 -fsanitize=realtime -g -fno-omit-frame-pointer -Wno-function-effects
   -Wno-error" FASTMATH=off`, `make -k`, under
   `RTSAN_OPTIONS=halt_on_error=false:abort_on_error=false:suppress_equal_stacks=true`. Each target prints its exit
   and its report count; any report, or a target that does not exit 0, is red. The suite's exit is printed but not
   judged: under clang -O1 four `clampf`/`clamp_or` checks fail on a signed zero, a separate item; its reports are
   judged, and its summary line must print. Logs and `RESULT.txt` stay in `build/rtsan/`; `RTSAN_JOBS` (default 4)
   sizes `make -j`, `RTSAN_CC` names the compiler.

What it cannot see:

- Only the paths the tests execute. Set-up in create and init is out of scope by design, and an allocation the
  compiler elides (a `malloc`/`free` pair optimised away at -O1) is not reported; one that survives optimisation is.
- `-Wno-function-effects` is set: clang's static effect analysis (a nonblocking function calling an unannotated one)
  is not triaged, so paths the tests do not reach are not found.
- `src/*.c` (room, sweep, oversampler, FBS and HRP placement, the alignment fit) is not marked, nor is an entry point
  the rule skips or does not name (`omx_hrp_render_feed`, for one).
- Fuzz, the thread arms, the perturbation, sabotage and install targets of `make test` are not run.

## Packages

`packaging/omx-dsp.spec` (RPM, `omx-dsp-devel`) and `debian/` (`libomxdsp-dev`). A `v<version>`
tag releases both through the OpenMixer release workflows; the version is
`OMXDSP_VERSION_*` in `include/omxdsp/omxdsp.h`, the spec's `Version:` and the top of
`debian/changelog`, and CI refuses them disagreeing.
