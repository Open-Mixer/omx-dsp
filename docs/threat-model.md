<!-- SPDX-License-Identifier: GPL-3.0-or-later
     Copyright (C) 2026 Pau Aliagas -->
# omx-dsp: threat model

## What it is and where it runs

omx-dsp is a C11 library (`-lm` only) of DSP primitives and effect kernels. It is not a daemon and
opens no socket, file or device. It ships as headers under `include/omxdsp/` and three static
archives (`libomxdsp.a`, `libomxdsp-contracts.a`, `libomxdsp-tsan.a`, see `docs/overview.md`); most
code is `static inline` in the headers, and `src/` holds the analysis units and the oversampler.
Consumers link it at build time: the OpenMixer engine and the omx LV2/CLAP plugins (`README.md`).

Most of the code runs on a host's audio thread. Functions carry a `@thread audio`, `control` or
`worker` tag. The library does no allocation, takes no lock and makes no system call: every buffer
and state is owned by the caller (`omx_instance_bind()` in `include/omxdsp/fx/omx_instance_io.h`,
"Allocates nothing"; the rings of `omx_fdelay.h` are "allocated on insert, never here"). The
analysis shells (`include/omxdsp/analysis/omx_hrp_lv2.h`, `omx_fbs_lv2.h`) split their work so FFTs
and detectors run in the host's worker, never in `run()`. A grep of `include/` and `src/` finds no
`malloc`, `free`, `fopen` or `pthread_` call.

The devel package also installs two developer tools (`packaging/omx-dsp.spec`):
`omx-dsp-copy-check` (`tools/omx-dsp-copy-check.sh`) and, in its own package, `omx-rt-gate`
(`tools/rt-gate/omx-rt-gate.c`). Neither runs inside an audio program.

## Inputs

| input | who can write it | trust |
|---|---|---|
| Control/port values (float, int, choice words, incl. NaN, ±Inf, denormals) | any host or graph client driving a plugin or the engine | untrusted |
| Audio sample blocks (finite or any float word) | any graph client or upstream plugin | untrusted |
| Sample rate passed to init or design | the host | untrusted |
| Block length `n` and the buffer pointers | the host, through the consumer's own glue | trusted (caller contract) |
| MIDI 1.0 channel messages (`omx_note_map_*`) | any MIDI source the host routes in | untrusted |
| Note-map leg policy (key range, transpose, velocity curve) | the controller or user of the consumer | untrusted |
| SRC rate pair (`omx_src_plan_of`) | the host or the engine's configuration | untrusted |
| Delay order, ring size and delay word (`omx_fdelay.h`, `omx_idelay.h`) | the consumer (ring) and any port (delay) | ring: trusted; delay: untrusted |
| EQ bank (up to 24 bands, kinds, gains, Q, frequencies) | port values from a host or controller | untrusted |
| Mix-matrix entries (`omx_mixmatrix_sparse`, strip/output indices) | the engine's routing code | trusted (caller contract) |
| Analysis inputs (sweeps, room captures, RTA frames, FBS/HRP shells) | audio from the graph; parameters from a controller | untrusted |
| Compiled tables and definitions (`data/*.json` rendered to `omxdsp_definitions.h`) | the maintainer and the packager, at build time | trusted |
| Presets, saved state, data files | none: the library has no parser of any serialised format (`fuzz/README.md`) | n/a |
| Source trees read by `omx-dsp-copy-check` and `omx-rt-gate` | the local user running the tool on their own checkout | trusted |

## Trust boundary

The boundary is the function call. Anything a plugin host, a graph client, a MIDI source or a
remote controller can set reaches the library as a port word, a sample, a rate or a MIDI message,
and the library must give a defined, bounded answer for every value of those. The caller (engine or
plugin glue) is trusted for memory: buffer pointers, block lengths, ring sizes and matrix indices
must describe memory it owns. Build-time data comes from this repository and the packager.

## What the code assumes and enforces

- Port words: `omx_clampf()` in `include/omxdsp/omx_param.h` maps NaN and -Inf to the floor and
  +Inf to the ceiling; `omx_clamp_or()` maps every non-finite word to the declared default.
- Rates: `omx_instance_bind()` (`include/omxdsp/fx/omx_instance_io.h`) refuses a rate outside the
  oracle set and leaves the face unready, so `run()` is the identity; `omx_instance_bind_positive()`
  refuses a non-positive or NaN rate. `omx_sweep_length()` (`src/omx_sweep.c`) returns 0 for a
  non-positive or over-large length.
- Blocks: `omx_instance_block_ok()` refuses a NULL pointer or `n == 0`; in and out may alias.
- MIDI: `omx_note_map` (`include/omxdsp/omx_note_map.h`) drops status bytes outside 0x80..0xEF and
  data bytes above 127, validates a leg (`omx_note_leg_valid()`, transpose within
  ±`OMX_NOTE_TRANSPOSE_MAX`), and bounds the held table to `OMX_NOTE_MAP_HELD_MAX` entries.
- SRC: `omx_src_plan_of()` (`include/omxdsp/omx_src.h`) returns -1 for a zero rate, a NULL plan, or
  an interpolation factor above `OMX_SRC_MAX_L` (1024), before anything is sized.
- Delay: `omx_fdelay_init()` (`include/omxdsp/omx_fdelay.h`) returns `OMX_FDELAY_BAD_ORDER` or `OMX_FDELAY_RING_TOO_SMALL`
  and leaves the line unarmed; an unarmed line leaves the block untouched.
- EQ design: Q is held in [`OMX_EQ_Q_MIN`, `OMX_EQ_Q_MAX`] (`include/omxdsp/omx_eq_design.h`).
- Contracts: every function states `OMX_PRE`/`OMX_POST` laws (`include/omxdsp/omx_contract.h`). They
  are compiled in only with `-DOMX_CONTRACTS`; in a release build they are `((void)0)` and
  not enforced. A release caller that breaks a PRE gets no check.
- Real time: `omx-rt-gate` checks a consumer's audio roots for allocation, locks, blocking calls
  and I/O against its tables in `tools/rt-gate/`; it is a build-time check, not a runtime one.

## Fuzz coverage

libFuzzer harnesses, built with ASan, UBSan and `-DOMX_CONTRACTS` (`fuzz/README.md`); CI runs each
for 60 s (`.github/workflows/ci.yml`, job `fuzz`).

| untrusted input | harness |
|---|---|
| Port words, sample blocks (finite and hostile) and rates of all 31 instance faces in `include/omxdsp/fx/*_instance.h` | `fuzz/fuzz_fx.c` |
| Port words through the scalar laws (clamps, dB/linear, pan, balance, fader, ramp, slew, wet/dry, one-pole) | `fuzz/fuzz_laws.c` |
| EQ bank (up to 24 bands), design rate, blocks through the cascade | `fuzz/fuzz_eq.c` |
| SRC rate pair, polyphase table, blocks | `fuzz/fuzz_src.c` |
| Delay order, ring size, delay word, blocks; integer delay and cross-fade | `fuzz/fuzz_delay.c` |
| MIDI channel messages and note-map leg policy | `fuzz/fuzz_note_map.c` |

### Gaps

- The analysis code is not fuzzed: `src/omx_room.c`, `src/omx_sweep.c`, `src/omx_align_fit.c`,
  `src/omx_fbs_detect.c`, `src/omx_fbs_place.c`, and the headers under `include/omxdsp/analysis/`
  (HRP, FBS and RTA shells, `omx_live_shell.h`). These take audio from the graph.
- `src/omx_oversampler.c` and most of `include/omxdsp/mix/` (bus stage and sum, meter, route gain,
  mix coefficients, pan law, key) have no harness of their own; some are reached only through the
  instance faces.
- `omx_mixmatrix_sparse()` checks entry indices against `n_strips`/`n_out` only under
  `OMX_CONTRACTS`; a release build indexes `out[]` and `in[]` unchecked. Not fuzzed.
- Release builds enforce no PRE; the harnesses only run the contracts flavour, so a path that is
  safe only because a PRE is held by the caller is not exercised as a release build.
- Block lengths are fuzzed only in 1..512 frames (`fuzz_fx.c`); larger blocks are not driven.
- In hostile mode (non-finite samples) only the sanitizers are the oracle; no output property is
  checked.
- No harness checks a real-time property (allocation, time per block); libFuzzer's 25 s timeout is
  the only bound.
- The installed tools `omx-dsp-copy-check` and `omx-rt-gate` parse source trees and are not fuzzed;
  their input is the local user's own checkout.

## Reporting

Report vulnerabilities privately through GitHub: the repository's Security tab, "Report a
vulnerability".
The policy is the organisation's `SECURITY.md`.
