/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * omx_delay_instance.h — the native FX delay as a plugin instance: the shell's core, with no format
 * in it. The core is `omx_delay.h`'s `omx_fx_delay_process` — THE SAME INLINE the console's delay
 * stage runs (mix_lane.h step OMX_STAGE_DELAY) — and this file adds no DSP to it. It adds what a
 * host's port model needs and the kernel's atom does not carry:
 *
 *   1. RINGS. `struct omx_fx_delay_state` reads two rings the kernel leaves to its caller; the
 *      instance OWNS them, inline, {@link OMX_FXDELAY_CAP} floats each (the declared 2 s ceiling at
 *      192 kHz), so the full travel is reachable at every declared rate and nothing is allocated, at
 *      init or after. An instance is initialised where it lives and is never copied after init (the
 *      state points into its own rings).
 *   2. RESOLVE. omx_delay_instance_resolve() takes the delay kernel's contract controls in
 *      their declared order and user units: the time (ms) through the kernel's own
 *      omx_fxdelay_ms_to_samples() — shared with the console's resolve, never a second
 *      conversion — the feedback, the tone and the mix (0..1) and the ping-pong switch. Every travel
 *      is clamped into its declared range (FX_DELAY_TIME_RANGE, FX_DELAY_FEEDBACK_RANGE, which stops
 *      short of the unity the kernel refuses, DELAY_TONE_RANGE, DELAY_MIX_RANGE), a non-finite word
 *      reading as the declared default: a foreign host's port is not the console's codec.
 *      Then the sync and the division (at tempo, the time is omx_bpm_division_ms of the host's tempo,
 *      omx_delay_instance_set_tempo(), and the division's DELAY_DIVISION_BEATS, held to the
 *      time travel's top; while the host hands no tempo, the time control, and
 *      omx_delay_instance_synced() says so), the mode (digital, tape, analog: the kernel's colour, its loop low pass
 *      at DELAY_TAPE_LP_HZ or DELAY_ANALOG_LP_HZ, tape's wow at DELAY_TAPE_WOW_HZ, DELAY_TAPE_WOW_MS
 *      deep) and the wet duck (depth, threshold, release; attack FX_DUCK_ATTACK_MS).
 *   3. IN -> OUT. The kernel works IN PLACE on the two legs; run() copies in to out where they
 *      differ (they may alias) and runs the kernel on out with its colour. Identity when bypassed or
 *      not ready. At free, digital and a 0 dB duck the instance is the delay that shipped, bit for bit.
 *
 * A time change moves the read tap and MAY CLICK, as mix_delay.h states for the console: the face
 * adds no cross-fade the console does not have. Re-engaging from bypass CLEARS the rings and the
 * per-leg damping, the console's own re-enable recipe (mixer_strip.c), so a re-enabled delay starts
 * silent instead of bursting a tail buffered before the bypass: a bounded memset over the inline
 * rings, no allocation.
 *
 * LATENCY IS ZERO, AND PUBLISHED: the delay IS the effect a host must not compensate away, and the
 * dry path is frame-aligned with the input.
 *
 * Moved from omx-plugins' plugins/omx-delay/omx_delay_instance.h (the caller-ring form), into the
 * instance face's shape.
 */
/**
 * @file omx_delay_instance.h
 * @brief the native FX delay as a plugin instance: the shell's core, with no format in it. The core is `omx_delay.h`'s `omx_fx_delay_process` — THE SAME INLINE the console's delay stage runs (mix_lane.h step OMX_STAGE_DELAY) — and this file adds no DSP to it.
 */
#ifndef OMX_DELAY_INSTANCE_H
#define OMX_DELAY_INSTANCE_H

#include <stdint.h>
#include <string.h>

#include <omxdsp/omxdsp_definitions.h>
#include <omxdsp/fx/omx_delay.h>
#include <omxdsp/fx/omx_instance_io.h>
#include <omxdsp/omx_param.h>

/** The face's channel count: a stereo pair over one atom. */
#define OMX_DELAY_INSTANCE_CHANNELS 2
/** The frames of latency the face introduces: none, the delay is the effect. */
#define OMX_DELAY_INSTANCE_LATENCY_FRAMES 0.0f

/* Control defaults: the contract's (FX_DELAY_*_RANGE.default, DELAY_*_RANGE.default, DELAY_PINGPONGS). */
/** The time a non-finite word reads as, ms: FX_DELAY_TIME_RANGE's default. */
#define OMX_DELAY_INSTANCE_TIME_DEFAULT ((float)OMX_FX_DELAY_TIME_RANGE_DEFAULT)
/** The feedback a non-finite word reads as, 0..1: FX_DELAY_FEEDBACK_RANGE's default. */
#define OMX_DELAY_INSTANCE_FEEDBACK_DEFAULT ((float)OMX_FX_DELAY_FEEDBACK_RANGE_DEFAULT)
/** The tone a non-finite word reads as, 0..1: DELAY_TONE_RANGE's default. */
#define OMX_DELAY_INSTANCE_TONE_DEFAULT ((float)OMX_DELAY_TONE_RANGE_DEFAULT)
/** The mix a non-finite word reads as, 0..1: DELAY_MIX_RANGE's default. */
#define OMX_DELAY_INSTANCE_MIX_DEFAULT ((float)OMX_DELAY_MIX_RANGE_DEFAULT)
/** The ping-pong switch an out-of-range word reads as: DELAY_PINGPONGS' default id. */
#define OMX_DELAY_INSTANCE_PINGPONG_DEFAULT ((int)OMX_DELAY_PINGPONGS_DEFAULT)

/** The beats of each division, DELAY_DIVISION_BEATS, in the order of its ids. */
static const float OMX_DELAY_INSTANCE_DIVISION_BEATS[OMX_DELAY_DIVISION_BEATS_COUNT] = OMX_DELAY_DIVISION_BEATS_INIT;
/** The tap factors' numerators, DELAY_TAP_FACTOR_NUMS, member by member: tap1..3Factor's ids. */
static const uint32_t OMX_DELAY_INSTANCE_TAP_NUMS[OMX_DELAY_TAP_FACTOR_NUMS_COUNT] = OMX_DELAY_TAP_FACTOR_NUMS_INIT;
/** The tap factors' denominators, DELAY_TAP_FACTOR_DENS, in the same order as the numerators. */
static const uint32_t OMX_DELAY_INSTANCE_TAP_DENS[OMX_DELAY_TAP_FACTOR_DENS_COUNT] = OMX_DELAY_TAP_FACTOR_DENS_INIT;
_Static_assert(OMX_DELAY_TAP_FACTOR_NUMS_COUNT == OMX_DELAY_TAP1_FACTORS_COUNT &&
                   OMX_DELAY_TAP_FACTOR_DENS_COUNT == OMX_DELAY_TAP1_FACTORS_COUNT &&
                   OMX_DELAY_TAP2_FACTORS_COUNT == OMX_DELAY_TAP1_FACTORS_COUNT &&
                   OMX_DELAY_TAP3_FACTORS_COUNT == OMX_DELAY_TAP1_FACTORS_COUNT,
               "one factor per member of each tap's set");
_Static_assert(OMX_DELAY_DIVISION_BEATS_COUNT == OMX_DELAY_DIVISIONS_COUNT, "one beat count per division");
_Static_assert((int)OMX_DELAY_MODES_DIGITAL == (int)OMX_FX_DELAY_DIGITAL && (int)OMX_DELAY_MODES_TAPE == (int)OMX_FX_DELAY_TAPE &&
                   (int)OMX_DELAY_MODES_ANALOG == (int)OMX_FX_DELAY_ANALOG,
               "the mode's ids are the kernel's colours, in order");
/* The colour's declared constants: FX_DUCK_ATTACK_MS, DELAY_TAPE_LP_HZ, DELAY_TAPE_WOW_HZ, DELAY_TAPE_WOW_MS,
 * DELAY_ANALOG_LP_HZ. */
/** The wet duck's attack, ms: FX_DUCK_ATTACK_MS. */
#define OMX_DELAY_INSTANCE_DUCK_ATTACK_MS ((float)OMX_FX_DUCK_ATTACK_MS)
/** The tape mode's loop low-pass corner, Hz: DELAY_TAPE_LP_HZ. */
#define OMX_DELAY_INSTANCE_TAPE_LP_HZ ((float)OMX_DELAY_TAPE_LP_HZ)
/** The tape mode's wow rate, Hz: DELAY_TAPE_WOW_HZ. */
#define OMX_DELAY_INSTANCE_TAPE_WOW_HZ ((float)OMX_DELAY_TAPE_WOW_HZ)
/** The tape mode's wow depth, ms: DELAY_TAPE_WOW_MS. */
#define OMX_DELAY_INSTANCE_TAPE_WOW_MS ((float)OMX_DELAY_TAPE_WOW_MS)
/** The analog mode's loop low-pass corner, Hz: DELAY_ANALOG_LP_HZ. */
#define OMX_DELAY_INSTANCE_ANALOG_LP_HZ ((float)OMX_DELAY_ANALOG_LP_HZ)

/** One instance: the atom, the kernel's state over the instance's own rings. */
typedef struct {
  float sr;                               /**< The sample rate the instance is bound to, Hz. */
  struct omx_fx_delay atom;               /**< The kernel's atom as last resolved. */
  struct omx_fx_delay_state state;        /**< The kernel's state over the instance's rings. */
  struct omx_fx_delay_color color;        /**< the mode and the wet duck */
  struct omx_fx_delay_color_state cstate; /**< the loop low pass, the wow and the duck's follower */
  struct omx_fx_delay_taps taps;          /**< the read taps, from fxDelayTaps and tap0..3 */
  float tap_ms[OMX_FXDELAY_MAX_TAPS];     /**< each tap's time as delivered, ms */
  int tap_clamped[OMX_FXDELAY_MAX_TAPS];  /**< each tap's time met the time travel's top */
  float bpm;                              /**< the host's tempo, BPM; 0 while it hands none */
  int synced;                             /**< the last resolve's time followed the tempo */
  float ring_l[OMX_FXDELAY_CAP];          /**< The left delay line, the instance's own. */
  float ring_r[OMX_FXDELAY_CAP];          /**< The right delay line, the instance's own. */
  /** The previous cycle's engaged flag, so a bypass->engaged edge can clear the rings. */
  int was_engaged;
  int ready;                              /**< 1 after a successful omx_delay_instance_init(); 0 makes run() the identity. */
} OmxDelayInstance;

/**
 * @brief Bind an instance to its rate, over its own rings. Allocates nothing.
 * @param s The instance; every word of it is written.
 * @param sr The sample rate, Hz, one the console declares.
 * @return 1 when usable, 0 when not: a rate the console does not declare is refused, `ready` stays
 *         clear and run() is the identity.
 * @thread control
 */
static inline int omx_delay_instance_init(OmxDelayInstance *s, float sr) {
  if (!omx_instance_bind(s, sizeof(*s), sr)) return 0;
  s->sr = sr;
  s->state.ring_l = s->ring_l;
  s->state.ring_r = s->ring_r;
  s->state.cap = (uint32_t)OMX_FXDELAY_CAP;
  s->bpm = 0.0f;
  omx_lfo_start(&s->cstate.wow);
  s->ready = 1;
  return 1;
}

/** @brief The host's tempo, BPM, for a delay at tempo sync. A value that is not a positive finite tempo (0 is
 * how a face says its host handed none this block) is NO tempo: a delay at tempo then keeps its time
 * control, as free. Takes effect at the next resolve.
 * @param s The instance; null is ignored.
 * @param bpm The tempo, beats per minute, above 0; anything else is no tempo.
 * @thread audio */
static inline void omx_delay_instance_set_tempo(OmxDelayInstance *s, float bpm) {
  if (!s) return;
  s->bpm = bpm > 0.0f && bpm - bpm == 0.0f ? bpm : 0.0f;
}

#define OMX_CONTRACT_STAGE "fx-delay/instance-clear"
/** @brief The engage-edge clear: both rings and the per-leg damping back to silence. RT-safe: a bounded
 * memset over the inline rings.
 * @param s The instance; null or not ready is ignored.
 * @thread audio */
static inline void omx_delay_instance_clear(OmxDelayInstance *s) {
  if (!s || !s->ready) return;
  /* CONTRACT (omx_contract.h). A cleared delay holds nothing: the next tap read, at any delay, is
   * silence until the ring has been written that far. */
  memset(s->ring_l, 0, sizeof s->ring_l);
  memset(s->ring_r, 0, sizeof s->ring_r);
  s->state.wpos = 0u;
  s->state.damp_l = s->state.damp_r = 0.0f;
  s->cstate.lp_l = s->cstate.lp_r = 0.0f;
  s->cstate.duck.level = 0.0f;
  OMX_POST(s->ring_l[0] == 0.0f && s->ring_r[OMX_FXDELAY_CAP - 1] == 0.0f && s->state.wpos == 0u,
           "cleared-ring-is-silent");
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "fx-delay/instance-resolve"
/**
 * @brief Resolve the host's control values into the kernel's atom for one cycle. The arguments are the
 * delay kernel's contract controls, in their declared order and user units; every travel is clamped
 * into its declared range, a non-finite word reading as the declared default, and a choice outside
 * its ids reads as its default. A null or not-ready instance is refused and nothing changes.
 * @param s The instance.
 * @param bypass Non-zero disables the atom: run() is then the identity.
 * @param fx_delay_time The time, ms, both legs, inside FX_DELAY_TIME_RANGE.
 * @param fx_delay_feedback The feedback, 0..1, inside FX_DELAY_FEEDBACK_RANGE.
 * @param tone The loop tone, 0..1, inside DELAY_TONE_RANGE.
 * @param mix The wet share, 0..1, inside DELAY_MIX_RANGE.
 * @param pingpong The DELAY_PINGPONGS index: 0 off, 1 on.
 * @param fx_delay_taps How many read taps, 1 to OMX_FXDELAY_MAX_TAPS, the fxDelayTaps control.
 * @param tap0_gain Tap 0's gain, dB, inside its declared travel.
 * @param tap0_pan Tap 0's pan, -1 (left) to +1 (right).
 * @param tap1_factor Tap 1's time factor, a DELAY_TAP_FACTOR id (OMX_DELAY_INSTANCE_TAP_NUMS / _DENS).
 * @param tap1_gain Tap 1's gain, dB.
 * @param tap1_pan Tap 1's pan, -1 to +1.
 * @param tap2_factor Tap 2's time factor, a DELAY_TAP_FACTOR id.
 * @param tap2_gain Tap 2's gain, dB.
 * @param tap2_pan Tap 2's pan, -1 to +1.
 * @param tap3_factor Tap 3's time factor, a DELAY_TAP_FACTOR id.
 * @param tap3_gain Tap 3's gain, dB.
 * @param tap3_pan Tap 3's pan, -1 to +1.
 * @param sync The DELAY_SYNCS index: free, or tempo (the time follows the host's tempo and `division`).
 * @param division The DELAY_DIVISIONS index, the beats of OMX_DELAY_INSTANCE_DIVISION_BEATS.
 * @param mode The DELAY_MODES index: digital, tape, analog.
 * @param duck_db The wet duck's depth, dB, inside its declared travel.
 * @param duck_threshold_db The duck's threshold, dB.
 * @param duck_release_ms The duck's release, ms.
 * @post The atom satisfies every precondition of omx_fx_delay_process().
 * @thread audio
 */
static inline void omx_delay_instance_resolve(OmxDelayInstance *s, int bypass, float fx_delay_time,
                                              float fx_delay_feedback, float tone, float mix, int pingpong,
                                              float fx_delay_taps, float tap0_gain, float tap0_pan, int tap1_factor,
                                              float tap1_gain, float tap1_pan, int tap2_factor, float tap2_gain,
                                              float tap2_pan, int tap3_factor, float tap3_gain, float tap3_pan,
                                              int sync, int division, int mode, float duck_db,
                                              float duck_threshold_db, float duck_release_ms) {
  if (!s || !s->ready) return;
  /* CONTRACT (omx_contract.h). The atom this leaves behind satisfies every PRE omx_fx_delay_process
   * states — feedback strictly below unity, mix and tone in unit range, both taps inside the ring —
   * whatever the host's ports held. */
  const int engaged = bypass ? 0 : 1;
  if (engaged && !s->was_engaged) omx_delay_instance_clear(s);
  s->was_engaged = engaged;
  struct omx_fx_delay *o = &s->atom;
  o->enabled = engaged;
  const float ms = omx_clamp_or(fx_delay_time, (float)OMX_FX_DELAY_TIME_RANGE_MIN, (float)OMX_FX_DELAY_TIME_RANGE_MAX,
                                OMX_DELAY_INSTANCE_TIME_DEFAULT);
  /* At tempo with no tempo handed, the time control: the delay a user set, never an invented tempo. */
  const int sync_tempo = sync == OMX_DELAY_SYNCS_TEMPO && s->bpm > 0.0f ? 1 : 0;
  s->synced = sync_tempo;
  const float division_beats =
      OMX_DELAY_INSTANCE_DIVISION_BEATS[division >= 0 && division < (int)OMX_DELAY_DIVISIONS_COUNT
                                            ? division
                                            : (int)OMX_DELAY_DIVISIONS_DEFAULT];
  const float synced = omx_bpm_division_ms(s->bpm, division_beats);
  const float time_ms =
      sync_tempo ? (synced < (float)OMX_FX_DELAY_TIME_RANGE_MAX ? synced : (float)OMX_FX_DELAY_TIME_RANGE_MAX) : ms;
  o->d_l = o->d_r = omx_fxdelay_ms_to_samples(time_ms, s->sr);
  const int mode_id = mode >= 0 && mode < (int)OMX_DELAY_MODES_COUNT ? mode : (int)OMX_DELAY_MODES_DEFAULT;
  const float duck_depth = omx_clamp_or(duck_db, (float)OMX_DELAY_DUCK_DB_RANGE_MIN, (float)OMX_DELAY_DUCK_DB_RANGE_MAX,
                                        (float)OMX_DELAY_DUCK_DB_RANGE_DEFAULT);
  const float duck_thr = omx_clamp_or(duck_threshold_db, (float)OMX_DELAY_DUCK_THRESHOLD_DB_RANGE_MIN,
                                      (float)OMX_DELAY_DUCK_THRESHOLD_DB_RANGE_MAX,
                                      (float)OMX_DELAY_DUCK_THRESHOLD_DB_RANGE_DEFAULT);
  const float duck_rel = omx_clamp_or(duck_release_ms, (float)OMX_DELAY_DUCK_RELEASE_MS_RANGE_MIN,
                                      (float)OMX_DELAY_DUCK_RELEASE_MS_RANGE_MAX,
                                      (float)OMX_DELAY_DUCK_RELEASE_MS_RANGE_DEFAULT);
  struct omx_fx_delay_color *c = &s->color;
  c->mode = mode_id;
  c->loop_pole = mode_id == OMX_DELAY_MODES_TAPE     ? omx_pole_from_cutoff_hz(OMX_DELAY_INSTANCE_TAPE_LP_HZ, s->sr)
                 : mode_id == OMX_DELAY_MODES_ANALOG ? omx_pole_from_cutoff_hz(OMX_DELAY_INSTANCE_ANALOG_LP_HZ, s->sr)
                                                     : 0.0f;
  c->wow_inc = mode_id == OMX_DELAY_MODES_TAPE ? omx_lfo_inc(OMX_DELAY_INSTANCE_TAPE_WOW_HZ, s->sr) : 0.0f;
  c->wow_depth = mode_id == OMX_DELAY_MODES_TAPE ? OMX_DELAY_INSTANCE_TAPE_WOW_MS * 0.001f * s->sr : 0.0f;
  s->cstate.wow.inc = c->wow_inc;
  c->duck.thresh_db = duck_thr;
  c->duck.depth_db = duck_depth;
  c->duck.attack = omx_pole_from_time_ms(OMX_DELAY_INSTANCE_DUCK_ATTACK_MS, s->sr);
  c->duck.release = omx_pole_from_time_ms(duck_rel, s->sr);
  o->feedback = omx_clamp_or(fx_delay_feedback, (float)OMX_FX_DELAY_FEEDBACK_RANGE_MIN,
                             (float)OMX_FX_DELAY_FEEDBACK_RANGE_MAX, OMX_DELAY_INSTANCE_FEEDBACK_DEFAULT);
  o->tone = omx_clamp_or(tone, (float)OMX_DELAY_TONE_RANGE_MIN, (float)OMX_DELAY_TONE_RANGE_MAX,
                         OMX_DELAY_INSTANCE_TONE_DEFAULT);
  o->mix = omx_clamp_or(mix, (float)OMX_DELAY_MIX_RANGE_MIN, (float)OMX_DELAY_MIX_RANGE_MAX,
                        OMX_DELAY_INSTANCE_MIX_DEFAULT);
  o->pingpong = pingpong == 0 || pingpong == 1 ? pingpong : OMX_DELAY_INSTANCE_PINGPONG_DEFAULT;
  /* The read taps: tap 0 is the delay's own time, tap k >= 1 the time times its factor, formed in double and
   * bounded to the time travel's top before the one rounding; each tap's gain pair by the balance law. */
  const float nt = omx_clamp_or(fx_delay_taps, (float)OMX_FX_DELAY_TAPS_RANGE_MIN, (float)OMX_FX_DELAY_TAPS_RANGE_MAX,
                                (float)OMX_FX_DELAY_TAPS_RANGE_DEFAULT);
  const int f1 = tap1_factor >= 0 && tap1_factor < (int)OMX_DELAY_TAP1_FACTORS_COUNT ? tap1_factor : (int)OMX_DELAY_TAP1_FACTORS_DEFAULT;
  const int f2 = tap2_factor >= 0 && tap2_factor < (int)OMX_DELAY_TAP2_FACTORS_COUNT ? tap2_factor : (int)OMX_DELAY_TAP2_FACTORS_DEFAULT;
  const int f3 = tap3_factor >= 0 && tap3_factor < (int)OMX_DELAY_TAP3_FACTORS_COUNT ? tap3_factor : (int)OMX_DELAY_TAP3_FACTORS_DEFAULT;
  const int fk[OMX_FXDELAY_MAX_TAPS] = {0, f1, f2, f3};
  const float gk[OMX_FXDELAY_MAX_TAPS] = {tap0_gain, tap1_gain, tap2_gain, tap3_gain};
  const float pk[OMX_FXDELAY_MAX_TAPS] = {tap0_pan, tap1_pan, tap2_pan, tap3_pan};
  struct omx_fx_delay_taps *t = &s->taps;
  t->ntaps = (uint32_t)lrintf(nt);
  for (uint32_t k = 0; k < (uint32_t)OMX_FXDELAY_MAX_TAPS; k++) {
    omx_fxdelay_tap_legs(gk[k], pk[k], &t->gl[k], &t->gr[k]);
    s->tap_clamped[k] = 0;
    if (k == 0u) {
      t->d[0] = 0u;
      continue;
    }
    const float tms = omx_fxdelay_tap_ms(time_ms, OMX_DELAY_INSTANCE_TAP_NUMS[fk[k]], OMX_DELAY_INSTANCE_TAP_DENS[fk[k]], &s->tap_clamped[k]);
    t->d[k] = omx_fxdelay_ms_to_samples(tms, s->sr);
    s->tap_ms[k] = tms;
  }
  s->tap_ms[0] = time_ms;
  OMX_POST(o->feedback >= 0.0f && o->feedback < 1.0f && o->mix >= 0.0f && o->mix <= 1.0f && o->tone >= 0.0f &&
               o->tone <= 1.0f && o->d_l <= s->state.cap && o->d_r <= s->state.cap,
           "atom-meets-the-kernel-preconditions");
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "fx-delay/instance-run"
/**
 * @brief THE AUDIO CALLBACK'S WHOLE SHARE: copy in to out where they differ, run the console's kernel in
 * place on out. `in_*` and `out_*` may alias. Not ready, or bypassed, is the identity byte for byte.
 * @param s The instance; null, not ready or bypassed copies in to out.
 * @param in_l The left input, `n` frames.
 * @param in_r The right input, `n` frames.
 * @param out_l The left output, `n` frames, may be `in_l`.
 * @param out_r The right output, `n` frames, may be `in_r`.
 * @param n The block length, frames; a null port or 0 touches nothing.
 * @thread audio
 */
static inline void omx_delay_instance_run(OmxDelayInstance *s, const float *in_l, const float *in_r, float *out_l,
                                          float *out_r, uint32_t n) {
  OMX_INSTANCE_RUN_IN_PLACE(s, in_l, in_r, out_l, out_r, n, omx_fx_delay_process_color(out_l, out_r, n, &s->atom, &s->taps, &s->color, &s->cstate, &s->state, s->sr));
}
#undef OMX_CONTRACT_STAGE

/** @brief The frames of latency the instance introduces: none, at every rate (the delay is the effect).
 * @param s The instance, unread.
 * @return 0.
 * @thread audio */
static inline uint32_t omx_delay_instance_latency(const OmxDelayInstance *s) {
  (void)s;
  return 0u;
}

/** @brief Whether the last resolve's time followed the host's tempo. The plugin's state readout.
 * @param s The instance; null reads 0.
 * @return 1 when sync is at tempo and a tempo was handed, 0 when the time control rules: free, or at
 *         tempo with no tempo from the host.
 * @thread audio */
static inline int omx_delay_instance_synced(const OmxDelayInstance *s) { return s ? s->synced : 0; }

/** @brief Tap `k`'s time as the last resolve delivered it, ms, and whether it met the time travel's top: the
 * plugin's tapMs and tapClamped outputs.
 * @param s The instance; null reads 0.
 * @param k The tap, 0 to OMX_FXDELAY_MAX_TAPS - 1; outside the taps reads 0.
 * @param clamped When non-null, set to 1 when the tap's time met the time travel's top, else 0.
 * @return The tap's time, ms.
 * @thread audio */
static inline float omx_delay_instance_tap_ms(const OmxDelayInstance *s, uint32_t k, int *clamped) {
  const int in = s && k < (uint32_t)OMX_FXDELAY_MAX_TAPS;
  if (clamped) *clamped = in ? s->tap_clamped[k] : 0;
  return in ? s->tap_ms[k] : 0.0f;
}

#endif /* OMX_DELAY_INSTANCE_H */
