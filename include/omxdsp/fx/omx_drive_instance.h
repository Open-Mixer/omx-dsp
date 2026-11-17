/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * omx_drive_instance.h — the DRIVE stage as a live insert for other
 * hosts: the shell's core, with no format in it.
 *
 * Under the HRP shells rule (one C core, several shells — what
 * differs is who calls it) and the drive stage's own law: drive is NATIVE
 * inside the console; this shell does not reopen that — it is the same kernel offered to a
 * host that is not openmixer.
 *
 * ## The face: init, resolve, run, latency
 *
 * Everything below is plain C over `omx_drive.h` and knows nothing about a plugin format. Its shape
 * is every instance face's: {@link omx_drive_instance_init} binds an instance to a declared rate,
 * {@link omx_drive_instance_resolve} takes the drive kernel's contract controls in their declared
 * order and user units (`amount` dB, `character`, `band_freq` Hz, `mix` %, `trim` dB, then the
 * `curve` and `band` choice indices, the `auto_gain` and `stereo_link` switches, the
 * `hf_rolloff` id and the `oversample` factor), {@link omx_drive_instance_run} runs the
 * kernel and
 * {@link omx_drive_instance_latency} publishes its latency. A plugin binding generated from the
 * contract calls them with no hand-written map.
 *
 * ## The oversampling factor
 *
 * `oversample` is a contract control (omx-contract 2.8.0): the shaper runs inside the shared
 * oversampler at 1x, 2x or 4x, the id being the factor, and comes up at 4X, the oversampler's
 * largest ({@link OMX_DRIVE_INSTANCE_FACTOR}). A factor change rebuilds the kernel's state for the
 * new factor inside resolve, at the block boundary, so no oversampler history designed for another
 * rate is ever run. The coefficient bank runs at the base rate, before the oversampler and after it,
 * so a factor change moves no coefficient and redesigns nothing.
 *
 * ## ZERO DUPLICATED DSP
 *
 * {@link omx_drive_instance_run} copies the host's input to its output and calls
 * `omx_drive_process` — the SAME inline the console's lane walk calls (mix_lane.h) — on it.
 * There is no second waveshaper, no second oversampler, no second auto-gain. What this file
 * owns is the RESOLVER: the console builds its `struct omx_drive` from word-atomic controls
 * plus TypeScript-designed coefficients (mixer_rt.c's resolve_fx_drive over console-rig.ts's
 * driveStateToNativeDrive); a foreign host hands us its controls and a rate, so
 * {@link omx_drive_instance_resolve} does the same job from those, with the coefficient design in C
 * (mix_drive_design.h — a pinned twin of the TypeScript, not a second opinion) and the time
 * constants from the kernel's own `omx_drive_time_constants`.
 *
 * ## NO WORKER, and why (the one place this shell is simpler than HRP's)
 *
 * HRP's live shell needs `lv2:worker` because its analysis is an FFT chain that must never run
 * in an audio callback. The drive has no analysis: its `run()` is a resolve (a dozen loads and
 * clamps, a coefficient redesign ONLY when a frequency port moved — a few trig calls, no
 * allocation) and the kernel, which is fixed work per sample with every buffer inline in
 * `struct omx_drive_state`. Nothing here needs a second thread, so none is asked for.
 *
 * ## LATENCY IS PUBLISHED, and it is the ELEMENT's number
 *
 * {@link omx_drive_instance_latency} is `omx_drive_latency`: the shared oversampler's round-trip
 * at the face's factor (72 base samples at 4x — "declared by the element,
 * derived by everyone else"; 48 at 2x, 0 at 1x), and 0 while bypassed, exactly as the console's `latencySamples`
 * row derives it. A plugin publishes it every cycle, so a host can compensate the dry paths
 * beside it.
 *
 * ## STEREO, 2 in / 2 out
 *
 * The kernel is a stereo pair with an optional right leg, and `stereoLink` (§4e: ONE auto-gain
 * path from both legs' summed energy) is a fact about a PAIR. A mono plugin instantiated twice
 * by a host cannot link, so the plugin is 2x2 and a host racking it on a mono strip feeds one
 * leg; the other runs on silence at no audible cost.
 */
/**
 * @file omx_drive_instance.h
 * @brief the DRIVE stage as a live insert for other hosts: the shell's core, with no format in it.
 */
#ifndef OMX_DRIVE_INSTANCE_H
#define OMX_DRIVE_INSTANCE_H

#include <stdint.h>
#include <string.h>

#include <omxdsp/fx/omx_drive.h>
#include <omxdsp/fx/omx_drive_design.h>
#include <omxdsp/omxdsp_definitions.h> /* the drive kernel's controls: every travel below */
#include <omxdsp/fx/omx_instance_io.h>
#include <omxdsp/omx_param.h>

/** The face's channel count: a stereo pair over one atom. */
#define OMX_DRIVE_INSTANCE_CHANNELS 2

/** The shared oversampler's largest factor, the `oversample` control's come-up. */
#define OMX_DRIVE_INSTANCE_FACTOR ((int)OMX_OVS_MAX_FACTOR)
OMXDSP_STATIC_ASSERT(OMX_DRIVE_OVERSAMPLES_DEFAULT == OMX_DRIVE_INSTANCE_FACTOR,
                     "the contract's oversample come-up is the oversampler's largest factor");
OMXDSP_STATIC_ASSERT(OMX_DRIVE_OVERSAMPLES_COUNT == 3u && OMX_DRIVE_OVERSAMPLES_1 == 1 && OMX_DRIVE_OVERSAMPLES_2 == 2 &&
                         OMX_DRIVE_OVERSAMPLES_4 == 4,
                     "the contract's oversample factors are the ones the drive's oversampler offers");
/** What the face designs the HF roll-off section at with the roll-off off (console-rig.ts:
 * `hfHz = ... ? 20000 : ...`). */
#define OMX_DRIVE_INSTANCE_HF_OFF_DESIGN_HZ 20000.0f

/* Control defaults: the contract's (DRIVE_*_RANGE.default); a choice comes up at its first id. */
/** The amount a non-finite word reads as, dB: DRIVE_AMOUNT_RANGE's default. */
#define OMX_DRIVE_INSTANCE_AMOUNT_DEFAULT ((float)OMX_DRIVE_AMOUNT_RANGE_DEFAULT)
/** The character a non-finite word reads as, -1..+1: DRIVE_CHARACTER_RANGE's default. */
#define OMX_DRIVE_INSTANCE_CHARACTER_DEFAULT ((float)OMX_DRIVE_CHARACTER_RANGE_DEFAULT)
/** The band corner a non-finite word reads as, Hz: DRIVE_BAND_FREQ_RANGE's default. */
#define OMX_DRIVE_INSTANCE_BAND_FREQ_DEFAULT ((float)OMX_DRIVE_BAND_FREQ_RANGE_DEFAULT)
/** The mix a non-finite word reads as, percent: DRIVE_MIX_RANGE's default. */
#define OMX_DRIVE_INSTANCE_MIX_DEFAULT ((float)OMX_DRIVE_MIX_RANGE_DEFAULT)
/** The trim a non-finite word reads as, dB: DRIVE_TRIM_RANGE's default. */
#define OMX_DRIVE_INSTANCE_TRIM_DEFAULT ((float)OMX_DRIVE_TRIM_RANGE_DEFAULT)
/** The curve an out-of-range word reads as: the soft shaper, DRIVE_CURVES' first id. */
#define OMX_DRIVE_INSTANCE_CURVE_DEFAULT OMX_DRIVE_SOFT
/** The band an out-of-range word reads as: the full band, DRIVE_BANDS' first id. */
#define OMX_DRIVE_INSTANCE_BAND_DEFAULT OMX_DRIVE_BAND_FULL
/** The auto-gain switch an out-of-range word reads as: DRIVE_AUTO_GAINS' default id. */
#define OMX_DRIVE_INSTANCE_AUTO_GAIN_DEFAULT ((int)OMX_DRIVE_AUTO_GAINS_DEFAULT)
/** The stereo-link switch an out-of-range word reads as: DRIVE_STEREO_LINKS' default id. */
#define OMX_DRIVE_INSTANCE_STEREO_LINK_DEFAULT ((int)OMX_DRIVE_STEREO_LINKS_DEFAULT)
/** The HF roll-off an out-of-range word reads as: DRIVE_HF_ROLLOFFS' default id. */
#define OMX_DRIVE_INSTANCE_HF_ROLLOFF_DEFAULT ((int)OMX_DRIVE_HF_ROLLOFFS_DEFAULT)
/** The oversampling an out-of-range word reads as: DRIVE_OVERSAMPLES' default id (the factor 4). */
#define OMX_DRIVE_INSTANCE_OVERSAMPLE_DEFAULT ((int)OMX_DRIVE_OVERSAMPLES_DEFAULT)

/** One instance: the atom, the kernel's state (inline: the oversampler histories and the
 * compensation line), and what the coefficient bank was last designed for. Nothing is caller-owned. */
typedef struct {
  uint32_t rate;                /**< The sample rate the instance is bound to, Hz. */
  struct omx_drive atom;        /**< The kernel's atom as last resolved. */
  struct omx_drive_state state; /**< The kernel's state: the oversampler histories and the compensation line, inline. */
  float designed_band_hz;       /**< The band corner, Hz, the coefficient bank was last designed for: resolve() redesigns only on a change. */
  float designed_hf_hz;         /**< The HF roll-off corner, Hz, the bank was last designed for. */
  int ready;                    /**< 1 after a successful omx_drive_instance_init(); 0 makes run() the identity. */
} OmxDriveInstance;

/**
 * @brief The atom's static controls from the host's words, each clamped into its declared travel, a
 * non-finite word reading as the declared default. The one mapping:
 * {@link omx_drive_instance_resolve} applies it to the atom it runs, and omx_curve_points() draws the drive's
 * transfer curve through it, so the curve is what the kernel runs at those words. Pure.
 * @param o The atom written.
 * @param amount The pre-shaper gain, dB, inside DRIVE_AMOUNT_RANGE.
 * @param character The bias weight, -1..+1, folded to (character + 1) / 2.
 * @param mix The wet share, percent, inside DRIVE_MIX_RANGE.
 * @param trim The make-up, dB, inside DRIVE_TRIM_RANGE.
 * @param curve The shaper, a DRIVE_CURVES index; any other value reads as the declared default.
 * @thread audio
 */
static inline void omx_drive_instance_controls(struct omx_drive *o, float amount, float character, float mix,
                                               float trim, int curve) {
  o->curve = curve >= OMX_DRIVE_SOFT && curve <= OMX_DRIVE_EXCITER ? curve : OMX_DRIVE_INSTANCE_CURVE_DEFAULT;
  o->drive_lin = omx_db_to_lin(omx_clamp_or(amount, (float)OMX_DRIVE_AMOUNT_RANGE_MIN, (float)OMX_DRIVE_AMOUNT_RANGE_MAX,
                                            OMX_DRIVE_INSTANCE_AMOUNT_DEFAULT));
  /* even_w = (character + 1) / 2, the same fold resolve_fx_drive does. */
  o->even_w = 0.5f * (omx_clamp_or(character, (float)OMX_DRIVE_CHARACTER_RANGE_MIN, (float)OMX_DRIVE_CHARACTER_RANGE_MAX,
                                   OMX_DRIVE_INSTANCE_CHARACTER_DEFAULT) +
                      1.0f);
  o->mix = 0.01f * omx_clamp_or(mix, (float)OMX_DRIVE_MIX_RANGE_MIN, (float)OMX_DRIVE_MIX_RANGE_MAX,
                                OMX_DRIVE_INSTANCE_MIX_DEFAULT);
  o->trim_lin = omx_db_to_lin(omx_clamp_or(trim, (float)OMX_DRIVE_TRIM_RANGE_MIN, (float)OMX_DRIVE_TRIM_RANGE_MAX,
                                           OMX_DRIVE_INSTANCE_TRIM_DEFAULT));
}

#define OMX_CONTRACT_STAGE "drive-instance/resolve"
/**
 * @brief Resolve the host's control values into the atom for this cycle. The arguments are the drive
 * kernel's contract controls, in their declared order and user units. The clamps are the row's, applied
 * for the same reason the console's controller applies them: the kernel's contract requires mix in
 * [0,1], and a host is free to write anything to a port. Coefficients are redesigned only when
 * `band_freq` or the roll-off corner moved; the state is rebuilt for the new factor only when
 * `oversample` moved, and the latency follows the factor. Allocation-free. A null or not-ready
 * instance is refused and nothing changes.
 * @param s The instance.
 * @param bypass Non-zero disables the atom: run() is then the identity.
 * @param amount The pre-shaper gain, dB, clamped into DRIVE_AMOUNT_RANGE; non-finite reads the declared default.
 * @param character The bias weight, -1..+1, clamped into DRIVE_CHARACTER_RANGE; non-finite reads the declared default.
 * @param band_freq The band corner, Hz, clamped into DRIVE_BAND_FREQ_RANGE; non-finite reads the declared default.
 * @param mix The wet share, percent, clamped into DRIVE_MIX_RANGE; non-finite reads the declared default.
 * @param trim The make-up, dB, clamped into DRIVE_TRIM_RANGE; non-finite reads the declared default.
 * @param curve The shaper, a DRIVE_CURVES index (`enum omx_drive_curve`); any other value reads as the first.
 * @param band The band driven, a DRIVE_BANDS index (`enum omx_drive_band`); any other value reads as the first.
 * @param auto_gain The DRIVE_AUTO_GAINS index, 0 off, 1 on; any other value reads as the declared default.
 * @param stereo_link The DRIVE_STEREO_LINKS index, 0 off, 1 on; any other value reads as the declared default.
 * @param hf_rolloff A DRIVE_HF_ROLLOFFS id (`enum omx_drive_hf_rolloffs`: 0 off, 1, 2); any other value reads as
 *        the declared default. The corner in Hz comes from the contract's `omx_drive_hf_rolloffs_value`, so a
 *        corner the low-pass design cannot take is never designed.
 * @param oversample A DRIVE_OVERSAMPLES id, the factor (1, 2 or 4); any other value reads as the declared default (4).
 * @post The atom satisfies every precondition of the drive kernel.
 * @thread audio
 */
static inline void omx_drive_instance_resolve(OmxDriveInstance *s, int bypass, float amount, float character,
                                              float band_freq, float mix, float trim, int curve, int band,
                                              int auto_gain, int stereo_link, int hf_rolloff, int oversample) {
  if (!s || !s->ready) return;
  /* CONTRACT (omx_contract.h). Whatever a host wrote to the ports, the atom leaves here inside
   * omx_drive_process's own PRE: mix in [0,1], the factor the state is built for, the bias weight
   * in [0,1], and the two gains finite and positive. */
  struct omx_drive *o = &s->atom;
  o->enabled = bypass ? 0 : 1;
  omx_drive_instance_controls(o, amount, character, mix, trim, curve);
  o->band = band >= OMX_DRIVE_BAND_FULL && band <= OMX_DRIVE_BAND_TILT ? band : OMX_DRIVE_INSTANCE_BAND_DEFAULT;
  o->auto_gain = auto_gain == 0 || auto_gain == 1 ? auto_gain : OMX_DRIVE_INSTANCE_AUTO_GAIN_DEFAULT;
  o->stereo_link = stereo_link == 0 || stereo_link == 1 ? stereo_link : OMX_DRIVE_INSTANCE_STEREO_LINK_DEFAULT;
  const int hf = hf_rolloff == OMX_DRIVE_HF_ROLLOFFS_OFF || hf_rolloff == OMX_DRIVE_HF_ROLLOFFS_12000 ||
                         hf_rolloff == OMX_DRIVE_HF_ROLLOFFS_16000
                     ? hf_rolloff
                     : OMX_DRIVE_INSTANCE_HF_ROLLOFF_DEFAULT;
  float corner_hz = 0.0f;
  o->hf_on = omx_drive_hf_rolloffs_value(hf, &corner_hz) ? 1 : 0;
  const int factor = oversample == OMX_DRIVE_OVERSAMPLES_1 || oversample == OMX_DRIVE_OVERSAMPLES_2 ||
                             oversample == OMX_DRIVE_OVERSAMPLES_4
                         ? oversample
                         : OMX_DRIVE_INSTANCE_OVERSAMPLE_DEFAULT;
  if (factor != o->os_factor) {
    o->os_factor = factor;
    omx_drive_state_init(&s->state, factor);
  }
  const float hf_hz = o->hf_on ? corner_hz : OMX_DRIVE_INSTANCE_HF_OFF_DESIGN_HZ;
  const float band_hz = omx_clamp_or(band_freq, (float)OMX_DRIVE_BAND_FREQ_RANGE_MIN,
                                     (float)OMX_DRIVE_BAND_FREQ_RANGE_MAX, OMX_DRIVE_INSTANCE_BAND_FREQ_DEFAULT);
  if (band_hz != s->designed_band_hz || hf_hz != s->designed_hf_hz) {
    omx_drive_design_bank(o, band_hz, hf_hz, s->rate);
    s->designed_band_hz = band_hz;
    s->designed_hf_hz = hf_hz;
  }
  OMX_POST(o->mix >= 0.0f && o->mix <= 1.0f, "mix-in-unit-range");
  OMX_POST(o->even_w >= 0.0f && o->even_w <= 1.0f, "bias-weight-in-unit-range");
  OMX_POST(s->state.factor == o->os_factor, "state-built-for-the-factor");
  OMX_POST(o->drive_lin >= 1.0f && o->drive_lin - o->drive_lin == 0.0f, "drive-gain-finite-and-at-least-unity");
  OMX_POST(o->trim_lin > 0.0f && o->trim_lin - o->trim_lin == 0.0f, "trim-gain-finite-and-positive");
}
#undef OMX_CONTRACT_STAGE

/**
 * @brief Bind an instance to its rate: the time constants, the state built for the default factor, the
 * contract's defaults resolved and the bank designed, engaged. Allocates nothing — the caller owns
 * the struct.
 * @param s The instance; every word of it is written.
 * @param sr The sample rate, Hz, one the console declares.
 * @return 1 when usable, 0 when not: a rate the console does not declare is refused, leaving `ready`
 *         clear, and run() is then the identity.
 * @thread control
 */
static inline int omx_drive_instance_init(OmxDriveInstance *s, float sr) {
  if (!omx_instance_bind(s, sizeof(*s), sr)) return 0;
  s->rate = (uint32_t)sr;
  omx_drive_time_constants(&s->atom, sr);
  s->atom.os_factor = OMX_DRIVE_INSTANCE_OVERSAMPLE_DEFAULT;
  omx_drive_state_init(&s->state, s->atom.os_factor);
  s->designed_band_hz = -1.0f; /* forces the first design */
  s->designed_hf_hz = -1.0f;
  s->ready = 1;
  omx_drive_instance_resolve(s, 0, OMX_DRIVE_INSTANCE_AMOUNT_DEFAULT, OMX_DRIVE_INSTANCE_CHARACTER_DEFAULT,
                             OMX_DRIVE_INSTANCE_BAND_FREQ_DEFAULT, OMX_DRIVE_INSTANCE_MIX_DEFAULT,
                             OMX_DRIVE_INSTANCE_TRIM_DEFAULT, OMX_DRIVE_INSTANCE_CURVE_DEFAULT,
                             OMX_DRIVE_INSTANCE_BAND_DEFAULT, OMX_DRIVE_INSTANCE_AUTO_GAIN_DEFAULT,
                             OMX_DRIVE_INSTANCE_STEREO_LINK_DEFAULT, OMX_DRIVE_INSTANCE_HF_ROLLOFF_DEFAULT,
                             OMX_DRIVE_INSTANCE_OVERSAMPLE_DEFAULT);
  return 1;
}

#define OMX_CONTRACT_STAGE "drive-instance/run"
/**
 * @brief One cycle: `out = drive(in)` for both legs, `n` frames. `in_*` and `out_*` may alias. Not ready,
 * or bypassed, is a copy and nothing else — the kernel's own "zero cost when disengaged" (§4g),
 * reached through the same `enabled` word.
 * @param s The instance; null, not ready or bypassed copies in to out.
 * @param in_l The left input, `n` frames.
 * @param in_r The right input, `n` frames.
 * @param out_l The left output, `n` frames, may be `in_l`.
 * @param out_r The right output, `n` frames, may be `in_r`.
 * @param n The block length, frames; a null port or 0 touches nothing.
 * @post Finite in, finite out, on both legs.
 * @thread audio
 */
static inline void omx_drive_instance_run(OmxDriveInstance *s, const float *in_l, const float *in_r, float *out_l,
                                          float *out_r, uint32_t n) {
  /* CONTRACT (omx_contract.h). Finite in, finite out, on both legs — the kernel's own law,
   * restated at the host boundary because the copy is this function's. */
  if (!omx_instance_block_ok(in_l, in_r, out_l, out_r, n)) return;
  OMX_PRE(omx_block_pair_finite(in_l, in_r, n), "finite-in");
  omx_instance_copy(in_l, in_r, out_l, out_r, n);
  if (!s || !s->ready) return;
  omx_drive_process(out_l, out_r, n, &s->atom, &s->state);
  OMX_POST(omx_block_pair_finite(out_l, out_r, n), "finite-out");
}
#undef OMX_CONTRACT_STAGE

/** @brief The stage's declared latency in frames at the instance rate.
 * @param s The instance.
 * @return The element's latency at the resolved factor (0, 48 or 72 frames at 1x, 2x, 4x); 0 while
 *         bypassed, not ready or null.
 * @thread audio */
static inline uint32_t omx_drive_instance_latency(const OmxDriveInstance *s) {
  return s && s->ready ? (uint32_t)omx_drive_latency(&s->atom) : 0u;
}

#endif /* OMX_DRIVE_INSTANCE_H */
