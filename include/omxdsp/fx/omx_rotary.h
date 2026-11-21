// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * @file omx_rotary.h
 * @brief The ROTARY SPEAKER stage: a drum rotor on the low band and a horn rotor on the high band
 *        of one complementary split, weighed by the balance law, mixed with the dry signal.
 *
 * Composed of mix_rotor.h (twice),
 * `omx_biquad` over `omx_eq_design_f`'s low-pass for the split (`hi = x − lo`, exact), and
 * `omx_balance_law`. The rings are inline: nothing is allocated. Latency (L1, for a modulated delay):
 * the sweep's shortest read, ROTARY_BASE_MS at the rate. At full mix every output sample comes
 * through a Doppler read that is never shorter, so that much delay is always present; the rest of
 * the sweep is the effect.
 */
#ifndef OMX_MIX_ROTARY_H
#define OMX_MIX_ROTARY_H

#include <stddef.h>
#include <stdint.h>

#include <omxdsp/omx_balance_law.h>
#include "omx_rotor.h"
#include <omxdsp/omx_biquad.h>
#include <omxdsp/omx_contract.h>
#include <omxdsp/omx_denormal.h>
#include <omxdsp/omx_eq_design.h>
#include <omxdsp/omx_wetdry.h>

/** @brief The `speed` member set, by index: the row's enum order. */
enum omx_rotary_speed { OMX_ROTARY_STOP = 0, OMX_ROTARY_SLOW = 1, OMX_ROTARY_FAST = 2 };

/** @brief The resolved atom, built once per block by omx_rotary_resolve(). */
struct omx_rotary {
  int enabled;            /**< 0 → passthrough before a sample or a state word is touched. */
  struct omx_rotor drum;  /**< The low band's rotor. */
  struct omx_rotor horn;  /**< The high band's rotor. */
  float xo[5];            /**< The split's low-pass section. */
  float g_lo;             /**< The drum's balance gain, in [0, 1]. */
  float g_hi;             /**< The horn's balance gain, in [0, 1]. */
  float mix;              /**< Wet share, in [0, 1]; 0 is bit-identical dry. */
  float dry;              /**< 1 − mix, resolved once per block. */
};

/** @brief The stage's state: the split's history per leg, two rotors, four inline rings. */
struct omx_rotary_state {
  float xs_l[4];                                /**< The left leg's split section history. */
  float xs_r[4];                                /**< The right leg's split section history. */
  struct omx_rotor_state drum;                  /**< The drum's phase and rate. */
  struct omx_rotor_state horn;                  /**< The horn's phase and rate. */
  struct omx_fdelay lo_l, lo_r, hi_l, hi_r;     /**< The four lines over the rings below. */
  float ring[4][OMX_ROTARY_RING_FLOATS];        /**< Inline rings: drum L/R, horn L/R. */
};

#undef OMX_CONTRACT_STAGE
#define OMX_CONTRACT_STAGE "rotary/init"
/**
 * @brief Zero the state and arm the four lines over the inline rings.
 * @param s The state, written.
 * @return The line's own code; OMX_FDELAY_OK when armed.
 * @post `four-lines-armed-at-the-rotor-order`.
 * @note A `memset` of the state, done on insert. Thread-safe on distinct state.
 * @thread audio
 */
static inline enum omx_fdelay_code omx_rotary_init(struct omx_rotary_state *s) {
  memset(s, 0, sizeof *s);
  struct omx_fdelay *lines[4] = {&s->lo_l, &s->lo_r, &s->hi_l, &s->hi_r};
  for (int k = 0; k < 4; k++) {
    const enum omx_fdelay_code c =
        omx_fdelay_init(lines[k], s->ring[k], OMX_ROTARY_RING_FLOATS, OMX_ROTOR_ORDER);
    if (c != OMX_FDELAY_OK) return c;
  }
  OMX_POST(s->hi_r.order == OMX_ROTOR_ORDER, "four-lines-armed-at-the-rotor-order");
  return OMX_FDELAY_OK;
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "rotary/resolve"
/**
 * @brief Resolve the controls into the atom for the next block.
 * @param p The atom, written.
 * @param s The state, read for the rotors' present rates (the accel/decel choice).
 * @param enabled The row's `on`.
 * @param speed The `speed` member index.
 * @param horn_slow_hz The horn's slow rate, Hz.
 * @param horn_fast_hz The horn's fast rate, Hz.
 * @param drum_slow_hz The drum's slow rate, Hz.
 * @param drum_fast_hz The drum's fast rate, Hz.
 * @param accel The scale on every speed-change time, positive.
 * @param balance The balance, in [−1, 1]: −1 drum only, +1 horn only.
 * @param mix The wet share, in [0, 1].
 * @param sr The live rate, a declared rate.
 * @pre `a-speed-is-a-member`, `mix-in-unit-range`.
 * @post `balance-gains-never-boost`; `the-sweep-ends-at-the-declared-far-end`: the farther rotor's
 *       base plus its travel is ROTARY_LONGEST_MS at the rate (to a float's rounding), the far end a
 *       descriptor's latency sweep refers to.
 * @note Per block: two `expf` and the split's design. Thread-safe on distinct atoms.
 * @thread audio
 */
static inline void omx_rotary_resolve(struct omx_rotary *p, const struct omx_rotary_state *s,
                                      int enabled, int speed, float horn_slow_hz,
                                      float horn_fast_hz, float drum_slow_hz, float drum_fast_hz,
                                      float accel, float balance, float mix, float sr) {
  OMX_PRE(speed >= OMX_ROTARY_STOP && speed <= OMX_ROTARY_FAST, "a-speed-is-a-member");
  OMX_PRE(mix >= 0.0f && mix <= 1.0f, "mix-in-unit-range");
  const float horn_t = speed == OMX_ROTARY_FAST ? horn_fast_hz : speed == OMX_ROTARY_SLOW ? horn_slow_hz : 0.0f;
  const float drum_t = speed == OMX_ROTARY_FAST ? drum_fast_hz : speed == OMX_ROTARY_SLOW ? drum_slow_hz : 0.0f;
  p->enabled = enabled;
  omx_rotor_resolve(&p->horn, &s->horn, horn_t, OMX_ROTARY_HORN_ACCEL_MS, OMX_ROTARY_HORN_DECEL_MS,
                    accel, OMX_ROTARY_HORN_DOPPLER_MS, OMX_ROTARY_HORN_AM, sr);
  omx_rotor_resolve(&p->drum, &s->drum, drum_t, OMX_ROTARY_DRUM_ACCEL_MS, OMX_ROTARY_DRUM_DECEL_MS,
                    accel, OMX_ROTARY_DRUM_DOPPLER_MS, OMX_ROTARY_DRUM_AM, sr);
  double q[2];
  omx_eq_butterworth_qs(0, q);
  omx_eq_design_f(OMX_EQ_LOWPASS, OMX_ROTARY_CROSSOVER_HZ, q[0], 0.0, sr, p->xo);
  omx_balance_law(balance, &p->g_lo, &p->g_hi);
  p->mix = mix;
  p->dry = 1.0f - mix;
  OMX_POST(p->g_lo >= 0.0f && p->g_lo <= 1.0f && p->g_hi >= 0.0f && p->g_hi <= 1.0f,
           "balance-gains-never-boost");
#ifdef OMX_CONTRACTS
  const float horn_end = p->horn.base + p->horn.depth, drum_end = p->drum.base + p->drum.depth;
  const float far_end = horn_end > drum_end ? horn_end : drum_end;
  const float declared = OMX_ROTARY_LONGEST_MS * 0.001f * sr, off = far_end - declared;
  OMX_POST((off < 0.0f ? -off : off) <= 1e-5f * declared, "the-sweep-ends-at-the-declared-far-end");
#endif
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "rotary"
/**
 * @brief Process one block in place: split each leg, run the drum on the low band and the horn on
 *        the high band (one phase each, both legs), weigh, mix.
 * @param l The left leg.
 * @param r The right leg.
 * @param n Frames.
 * @param p The atom.
 * @param s The state.
 * @pre `finite-in` (the atom's `mix` range is the resolve's PRE).
 * @post `finite-out`, per sample, by omx_wetdry_mix().
 * @note Per sample two sections, two rotor ticks (four four-tap reads), no libm, no
 *       allocation. `enabled == 0` returns before a word is touched; `mix == 0` returns the
 *       block bit-identical after advancing the rotors (a muted mic does not stop the cabinet).
 *       Thread-safe on distinct state.
 * @thread audio
 */
static inline void omx_rotary_process(float *l, float *r, uint32_t n, const struct omx_rotary *p,
                                      struct omx_rotary_state *s) {
  if (!p->enabled || n == 0u) return;
  OMX_PRE(omx_block_pair_finite(l, r, n), "finite-in");
  if (p->mix <= 0.0f) {
    float d = 0.0f, a = 0.0f;
    for (uint32_t i = 0; i < n; i++) {
      omx_rotor_step(&p->drum, &s->drum, &d, &a);
      omx_rotor_step(&p->horn, &s->horn, &d, &a);
    }
    return;
  }
  for (uint32_t i = 0; i < n; i++) {
    const float lo_l = omx_flush(omx_biquad(l[i], p->xo, s->xs_l));
    const float lo_r = omx_flush(omx_biquad(r[i], p->xo, s->xs_r));
    float dl = 0.0f, dr = 0.0f, hl = 0.0f, hr = 0.0f;
    omx_rotor_tick(&p->drum, &s->drum, &s->lo_l, &s->lo_r, lo_l, lo_r, &dl, &dr);
    omx_rotor_tick(&p->horn, &s->horn, &s->hi_l, &s->hi_r, l[i] - lo_l, r[i] - lo_r, &hl, &hr);
    const float wet_l = p->g_lo * dl + p->g_hi * hl, wet_r = p->g_lo * dr + p->g_hi * hr;
    omx_wetdry_mix(&l[i], &r[i], l[i], r[i], wet_l, wet_r, p->dry, p->mix);
  }
}
#undef OMX_CONTRACT_STAGE

/**
 * @brief The sweep's shortest Doppler read at a rate, samples: ROTARY_BASE_MS at `sr`, the same
 *        figure omx_rotor_resolve() sets as each rotor's `base` (both rotors share it).
 * @param sr The live rate, Hz.
 * @return `ROTARY_BASE_MS · 1e-3 · sr`.
 * @note Thread-safe: arithmetic.
 * @thread audio
 */
static inline float omx_rotary_shortest_delay(float sr) { return OMX_ROTARY_BASE_MS * 0.001f * sr; }

/**
 * @brief The sweep's longest Doppler read at a rate, samples: ROTARY_LONGEST_MS at `sr`, the base
 *        plus the larger rotor's whole travel (the horn's, 2·HORN_DOPPLER_MS against the drum's
 *        2·DRUM_DOPPLER_MS); the kernel oracle holds the scalar to that sum.
 * @param sr The live rate, Hz.
 * @return `ROTARY_LONGEST_MS · 1e-3 · sr`.
 * @note Thread-safe: arithmetic.
 * @thread audio
 */
static inline float omx_rotary_longest_delay(float sr) { return OMX_ROTARY_LONGEST_MS * 0.001f * sr; }

/**
 * @brief The stage's latency at a rate, frames (L1 for a modulated delay): the sweep's shortest
 *        read, rounded to the nearest frame — where an impulse peaks with both rotors at the
 *        sweep's shortest phase. It holds while the wet path is the whole output; a caller with a
 *        direct dry share reports zero (omx_rotary_instance_latency()).
 * @param sr The live rate, Hz, positive.
 * @return `round(omx_rotary_shortest_delay(sr))`.
 * @note Thread-safe: arithmetic, no libm.
 * @thread audio
 */
static inline uint32_t omx_rotary_latency(float sr) { return (uint32_t)(omx_rotary_shortest_delay(sr) + 0.5f); }

/**
 * @brief The sweep at a rate, frames: the shortest read (the latency, omx_rotary_latency()) and
 *        the longest, each rounded to the nearest frame. Every lag the stage's wet path shows lies
 *        inside it; a host declares it and checks a measured lag against the range, not one figure.
 * @param sr The live rate, Hz, positive.
 * @param shortest Written: `round(omx_rotary_shortest_delay(sr))`.
 * @param longest Written: `round(omx_rotary_longest_delay(sr))`.
 * @pre `outs-present`: both pointers name a word.
 * @note Thread-safe: arithmetic, no libm.
 * @thread audio
 */
#define OMX_CONTRACT_STAGE "rotary/sweep"
static inline void omx_rotary_sweep(float sr, uint32_t *shortest, uint32_t *longest) {
  OMX_PRE(shortest != NULL && longest != NULL, "outs-present");
  *shortest = omx_rotary_latency(sr);
  *longest = (uint32_t)(omx_rotary_longest_delay(sr) + 0.5f);
}
#undef OMX_CONTRACT_STAGE

#endif /* OMX_MIX_ROTARY_H */
