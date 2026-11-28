<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> -->
# One generic face from a kernel descriptor

Every effect kernel used to reach a host through a hand-written instance face,
`include/omxdsp/fx/omx_<k>_instance.h`: its own struct, its own `resolve()` taking the kernel's
controls one argument each, its own `run()`, `latency()` and readouts. The faces repeat one pattern,
and each one needs its name in hand lists (the Makefile, the spec, debian, face-conformance, the
docs). This page specifies what replaces them: each kernel publishes a **descriptor**, and **one
face**, a single family of C functions, serves every kernel from it. No code is generated.

## The descriptor

`struct omx_kernel_desc` (`include/omxdsp/omx_kernel_desc.h`) is data. A kernel's header returns
its own descriptor (`omx_trim_desc()`, `omx_width_desc()`, `omx_clipper_desc()`), and
`include/omxdsp/fx/omx_fx_kernels.h` names every kernel the face serves, the one list there is.

| Field | What it holds | Where it comes from |
|---|---|---|
| `name` | the kernel's name, `width` | the kernel file's name, `data/kernels/width.json` |
| `params[]`, `n_params` | one row per control, in the declared order | the kernel file's `controls` |
| `params[i].name` | the control's contract name, `pseudoMs` | `controls[i].name` |
| `params[i].kind` | `OMX_PARAM_TRAVEL` or `OMX_PARAM_CHOICE` | `controls[i].kind` |
| `params[i].min`, `max`, `def` | a travel's range and default in its user unit; a choice's first index (0), last index and default index | the render of the kernel file, `omxdsp_definitions.h` (`OMX_WIDTH_PSEUDO_MS_RANGE_MIN`, `OMX_WIDTH_PSEUDOS_COUNT`, `OMX_WIDTH_PSEUDOS_DEFAULT`) |
| `params[i].unit` | the PARAM_UNITS id, `"percent"`; `"none"` for a choice | omx-contract's `OMX_PARAM_UNITS_*` (`omxcontract/omx_vocabulary.h`), the id whose symbol is the travel's `unit` |
| `meters[]`, `n_meters` | one row per readout: name, PARAM_UNITS id, range | the kernel file's `readouts` |
| `state_size` | the bytes of the kernel's state, owned by the caller | `sizeof` the kernel's state struct |
| `init` | binds the state to a rate; 1 when usable | the kernel |
| `set` | takes the resolved values, in the declared order, and the bypass | the kernel |
| `process` | one block, both legs in place | the kernel |
| `clear` | silences the kernel's memory, keeping its values (may be null) | the kernel |
| `latency` | the frames the kernel delays at its resolved values | the kernel |
| `meter` | reads one readout (null: no readouts) | the kernel |

The ranges are the render's macros, never typed again. The check that the descriptor is its
kernel file is `tools/face-descriptor-check.sh` (in `make checks`): every row's name, order, kind,
range, default and unit, and every meter, against `data/kernels/<k>.json`, travels resolved through
their `ref`s and units through omx-contract's PARAM_UNITS. A descriptor that drifts (two rows swapped,
a default moved, a unit changed) is named, and its `--self-test` plants each of those.

## The face's five calls

`include/omxdsp/fx/omx_fx_face.h`, over a `struct omx_fx_face` the caller owns and a kernel state the
caller owns (`state_size` bytes):

| Call | Thread | What it does |
|---|---|---|
| `omx_fx_face_init(f, desc, state, sr)` | control | zeroes the state, `init`s it at the rate, resolves every default, engaged; 0 when the kernel refuses the rate |
| `omx_fx_face_resolve(f, bypass, values)` | audio | each travel clamped into its range, a non-finite value reading as its default; each choice rounded to its index, one outside the set reading as its default; then `set` |
| `omx_fx_face_run(f, in_l, in_r, out_l, out_r, n)` | audio | copies in to out where they differ, then, engaged, `process` in place on out |
| `omx_fx_face_clear(f)` | audio | `clear` |
| `omx_fx_face_latency(f)` | audio | `latency` |

`omx_fx_face_meter(f, i)` reads readout `i`. A null face, a face that is not ready and an empty block
are refused the way every hand-written face refuses them: nothing is touched, or out holds in.

**The cost** is one call through a function pointer per block (`process`), and one per resolve
(`set`): never one per sample. The face allocates nothing and takes no lock; its run is `@thread
audio` and the real-time gate walks the kernels' functions through the descriptor's fields as a table
(`make rt-gate-selfrun`).

## What stays hand-written

The faces whose state is more than their parameters keep their own state I/O: **hrp**, **fbs** and
**convolution**. An analysis engine's learned bands, a feedback suppressor's notch pool and a
convolver's impulse response are state a parameter list cannot express, so a descriptor of values
cannot carry them.

## Migration

Phase 1 (this change): the descriptor, the face, their tests, and the first batch, **trim**,
**width** and **clipper**, the three smallest kernels whose state is their parameters. Each is
proven twice: its existing golden digests (`test/golden/<k>_instance.sha256`) pass unchanged through
the generic face (`test/fx/face_<k>_golden.test.c`), and the generic face gives the hand-written
face's output bit for bit, meters and latency included, over hostile values, bypass edges and the
nine oracle rates (`test/fx/face.test.c`).

A hand-written face is removed, with every hand list that names it, once no consumer outside this
library includes it. The first batch's three headers are still included by omx-plugins (its trim,
width, clipper and channel-strip plugins and their oracles), so they stay, unchanged, until those
plugins call the generic face; no header is kept as a forwarding wrapper. While both faces stand, each
body lives once, in the kernel's header, and both faces call it: the clip's block and knee
(`omx_clipper_block`, `omx_clipper_knee`) and the width's redesign (`omx_width_redesign`); the trim's
one ramp is omx_ramp.h's.

The later batches, smallest first, each a kernel whose state is its parameters:

1. tremolo, upward, glue, limiter, phaser
2. chorus, flanger, rotary, transient, deesser, pitch
3. gate, spring, tone, loudlevel, leveler, mbcomp, balancer
4. doubler, suboct, drive, plosive, delay, reverb
5. dyneq, comp, geq, eq (the counted controls: a band's row taken once per band)
6. align and idelay, written straight against the descriptor (no hand-written face first). Done
   (omx-dsp#185): `omx_align_desc()` and `omx_idelay_desc()`, served by `omx_fx_kernels.h`. With no
   hand-written twin, `test/fx/face.test.c` holds the face to the kernel itself, driven from its own
   calls, and `test/fx/face_align.test.c` and `test/fx/face_idelay.test.c` carry the kernels' closed
   forms through the face (a unit impulse exactly D samples late at exactly 1, at the nine oracle
   rates). `tools/face-conformance.sh` requires both kernels to have a face, of either kind.

Each batch removes its faces and their hand-list rows once their outside consumers call the
generic face.
