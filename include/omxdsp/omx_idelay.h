// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * @file omx_idelay.h
 * @brief The integer delay line: a whole-sample delay over a caller-owned ring whose re-point
 *        cross-fades from the old tap to the new one across one block.
 *
 * The write-then-read cursor law of omx_lookahead.h, `out[n] = in[n − D]`, applied to a block in
 * place: D = 0 reads the sample just written, an exact passthrough. When the delay moves, the
 * block reads both the old and the new tap and blends them linearly across its length, so the
 * move never clicks; after that block the applied delay is the new one. Never a fractional read:
 * a delay between samples is omx_fdelay.h's.
 *
 * The console aligns a loudspeaker with it: the output delay of a route leg or a bus, per
 * destination, after the trim and fold. The ring's size, the ceiling and the rate it is sized
 * for are the caller's; a NULL ring is a passthrough, so a ring allocated lazily on the control
 * thread needs no other gate on the audio thread.
 *
 * Contract: NO allocation, NO lock, NO libc.
 */
#ifndef OMX_IDELAY_H
#define OMX_IDELAY_H

#include <stddef.h>
#include <stdint.h>

#include <omxcontract/omx_vocabulary.h>

#include "omx_contract.h"
#include "omx_kernel_desc.h"
#include "omx_lookahead.h"
#include "omxdsp.h"

/** @brief The line's ceiling, ms: the contract's output delay travel top (OUTPUT_DELAY_RANGE.max). */
#define OMX_IDELAY_MAX_MS OMX_OUTPUT_DELAY_RANGE_MAX
/** @brief The highest graph sample rate the line's ring is sized for, Hz: the contract's hard target rate.
 *  The ring is sized for this rate so OMX_IDELAY_MAX_MS is reachable at every declared rate alike; at a lower
 *  rate the reachable ms is unchanged (fewer samples per ms) and the ring is just not full. */
#define OMX_IDELAY_MAX_RATE OMX_RT_HARD_TARGET_RATE
/** @brief The ring's capacity, samples, per leg: OMX_IDELAY_MAX_MS at OMX_IDELAY_MAX_RATE, plus one so the
 *  full ceiling is a tap strictly inside the ring (a read at exactly `cap` would alias the write cursor). */
#define OMX_IDELAY_CAP (((OMX_IDELAY_MAX_RATE / 1000) * OMX_IDELAY_MAX_MS) + 1)
_Static_assert(OMX_IDELAY_CAP == (OMX_IDELAY_MAX_RATE / 1000) * OMX_IDELAY_MAX_MS + 1,
               "the ring holds the ceiling at the highest rate, plus one");

#undef OMX_CONTRACT_STAGE
#define OMX_CONTRACT_STAGE "idelay/samples"
/**
 * @brief The whole-sample delay for `ms` at rate `sr`, rounded to the nearest sample and
 *        clamped to `[0, cap − 1]`.
 *
 * A negative or zero `ms` is no delay (a leg can only be pushed later); a huge one saturates at
 * the ring's length. The one ms → samples conversion of the line, so the one place a dialled
 * delay could become a different delay.
 * @param ms The delay, ms; finite.
 * @param sr The graph rate, Hz; a declared rate, or 0 or below for no delay.
 * @param cap The ring's length in samples; 0 for no delay.
 * @return The delay, samples, in `[0, cap)`; 0 when `cap` is 0.
 * @pre `rate-is-declared`, `ms-is-finite`.
 * @post `delay-inside-the-ring`.
 * @note Thread-safe: arithmetic.
 * @thread audio
 */
static inline uint32_t omx_idelay_samples(float ms, float sr, uint32_t cap) {
  OMX_PRE(sr <= 0.0f || OMX_RATE_IS_DECLARED(sr), "rate-is-declared");
  OMX_PRE(ms - ms == 0.0f, "ms-is-finite");
  if (ms <= 0.0f || cap == 0 || sr <= 0.0f) { OMX_POST(1, "delay-inside-the-ring"); return 0; }
  float s = ms * sr * 0.001f + 0.5f; /* round to nearest sample */
  if (s <= 0.0f) { OMX_POST(1, "delay-inside-the-ring"); return 0; }
  uint32_t d = s >= (float)cap ? cap : (uint32_t)s; /* no cast of a float past uint32_t's range */
  d = d >= cap ? cap - 1 : d;
  OMX_POST(d < cap, "delay-inside-the-ring");
  return d;
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "idelay/apply"
/**
 * @brief Delay `buf` in place by `tgt` samples through `ring`, cross-fading from the applied
 *        delay `*cur` when it differs.
 *
 * Each sample is written at `*wpos` and read back `tgt` writes behind. At a steady delay
 * (`*cur == tgt`) the block is `out[i] = in[i − tgt]` bit for bit; otherwise every sample reads
 * both taps and blends old → new with weight `(i + 1) / n`, and `*cur` becomes `tgt`. A NULL
 * ring, `cap` 0 or `n` 0 leaves the block and the cursors untouched. A `tgt` past the ring is
 * clamped to `cap − 1` here too.
 * @param buf The block, `n` samples, delayed in place; finite.
 * @param n Samples in the block.
 * @param ring `cap` floats, owned by the caller; NULL: a passthrough.
 * @param cap The ring's length in samples.
 * @param wpos The write cursor, in `[0, cap)`; advanced by `n`.
 * @param cur The applied delay, samples; set to `tgt` by a cross-fade.
 * @param tgt The delay wanted, samples, in `[0, cap)` (omx_idelay_samples()).
 * @pre `finite-in`, `target-clamped-by-caller`.
 * @post `finite-out`.
 * @note No allocation, fixed work per sample, one branch per block. Thread-safe on
 *       distinct state.
 * @thread audio
 */
static inline void omx_idelay_apply(float *buf, uint32_t n, float *ring, uint32_t cap, uint32_t *wpos,
                                    uint32_t *cur, uint32_t tgt) {
  OMX_PRE(omx_block_finite(buf, n), "finite-in");
  OMX_PRE(tgt < cap || cap == 0u, "target-clamped-by-caller");
  if (n == 0 || ring == NULL || cap == 0) return;
  if (tgt >= cap) tgt = cap - 1;
  uint32_t w = *wpos;
  uint32_t d0 = *cur;
  if (d0 >= cap) d0 = cap - 1;
  if (d0 == tgt) {
    for (uint32_t i = 0; i < n; i++) {
      ring[w] = buf[i];
      uint32_t r = omx_lookahead_back(w, tgt, cap);
      buf[i] = ring[r];
      w = omx_lookahead_fwd(w, 1u, cap);
    }
  } else {
    for (uint32_t i = 0; i < n; i++) {
      ring[w] = buf[i];
      uint32_t r0 = omx_lookahead_back(w, d0, cap);
      uint32_t r1 = omx_lookahead_back(w, tgt, cap);
      float t = (float)(i + 1) / (float)n; /* 0 → 1 across the block */
      buf[i] = ring[r0] * (1.0f - t) + ring[r1] * t;
      w = omx_lookahead_fwd(w, 1u, cap);
    }
    *cur = tgt;
  }
  *wpos = w;
  OMX_POST(omx_block_finite(buf, n), "finite-out");
}
#undef OMX_CONTRACT_STAGE

/**
 * @brief A stereo pair of integer delay lines: two caller-owned rings of one length and the
 *        cursors omx_idelay_apply() keeps for each leg.
 *
 * `cap` 0 and NULL rings until the caller allocates them; until then the pair is a passthrough.
 */
struct omx_idelay_pair {
  float *ring_l;           /**< The left ring, `cap` floats; NULL: a passthrough. */
  float *ring_r;           /**< The right ring, `cap` floats. */
  uint32_t cap;            /**< Each ring's length in samples; 0 until allocated. */
  uint32_t wpos_l;         /**< The left write cursor. */
  uint32_t wpos_r;         /**< The right write cursor. */
  uint32_t cur_l;          /**< The left applied delay, for the cross-fade. */
  uint32_t cur_r;          /**< The right applied delay, for the cross-fade. */
};

/** @thread audio */
OMXDSP_STATE_LAYOUT(idelay_pair, struct omx_idelay_pair)

#define OMX_CONTRACT_STAGE "idelay/pair"
/**
 * @brief omx_idelay_apply() on each present leg, each at its own delay.
 *
 * Equal `tgt_l` and `tgt_r` delay both legs alike. `r` NULL is a mono pair: the right ring and
 * cursors are left alone. A NULL `d` is a passthrough.
 * @param l The left block, `n` samples, delayed in place; NULL: left alone.
 * @param r The right block; NULL for a mono pair.
 * @param n Samples per leg.
 * @param d The pair.
 * @param tgt_l The left delay, samples, in `[0, d->cap)`.
 * @param tgt_r The right delay, samples, in `[0, d->cap)`.
 * @pre `finite-in`, `both-targets-inside-the-shared-ring`.
 * @post `finite-out`.
 * @note No allocation, fixed work per sample. Thread-safe on distinct state.
 * @thread audio
 */
static inline void omx_idelay_pair_apply(float *l, float *r, uint32_t n, struct omx_idelay_pair *d,
                                         uint32_t tgt_l, uint32_t tgt_r) {
  OMX_PRE(omx_block_pair_finite(l, r, n), "finite-in");
  OMX_PRE(d == NULL || d->cap == 0u || (tgt_l < d->cap && tgt_r < d->cap),
          "both-targets-inside-the-shared-ring");
  if (n == 0 || d == NULL) return;
  if (l != NULL) omx_idelay_apply(l, n, d->ring_l, d->cap, &d->wpos_l, &d->cur_l, tgt_l);
  if (r != NULL) omx_idelay_apply(r, n, d->ring_r, d->cap, &d->wpos_r, &d->cur_r, tgt_r);
  OMX_POST((l == NULL || omx_block_finite(l, n)) && (r == NULL || omx_block_finite(r, n)),
           "finite-out");
}
#undef OMX_CONTRACT_STAGE

/** @brief The output delay as the generic face (fx/omx_fx_face.h) serves it: a pair over two rings of its own,
 * inline, OMX_IDELAY_CAP floats each, so the full travel is reachable at every oracle rate and nothing is
 * allocated. The pair points into its own rings: the state is bound where it lives and never copied after. */
struct omx_idelay_kernel {
  float sr;                      /**< The sample rate init bound the state to, Hz. */
  struct omx_idelay_pair pair;   /**< The pair over `ring_l` and `ring_r`: the cursors and the applied delays. */
  uint32_t tgt;                  /**< The delay, whole samples, omx_idelay_samples() of `ms`. */
  float ms;                      /**< outputDelay as the last set took it, ms. */
  int enabled;                   /**< Non-zero while engaged, as the last set left it. */
  float ring_l[OMX_IDELAY_CAP];  /**< The left ring. */
  float ring_r[OMX_IDELAY_CAP];  /**< The right ring. */
};

/**
 * @brief Bind a zeroed state to its rate, its pair over its own rings, at no delay. Allocates nothing.
 * @param state A struct omx_idelay_kernel, zeroed.
 * @param sr The sample rate, Hz: one of the nine oracle rates.
 * @return 1 when usable, 0 when the rate is not one of them.
 * @thread control
 */
static inline int omx_idelay_kernel_init(void *state, float sr) {
  struct omx_idelay_kernel *k = state;
  if (!omx_rate_is_oracle_full(sr)) return 0;
  k->sr = sr;
  k->pair.ring_l = k->ring_l;
  k->pair.ring_r = k->ring_r;
  k->pair.cap = (uint32_t)OMX_IDELAY_CAP;
  k->enabled = 1;
  return 1;
}

#define OMX_CONTRACT_STAGE "idelay/clear"
/**
 * @brief Silence the pair, keeping the delay: both rings back to 0, the write cursors and the applied delays back
 *        at 0, as init left them. Bounded work, no allocation.
 * @param state A struct omx_idelay_kernel.
 * @post `cleared-pair-is-fresh`.
 * @thread audio
 */
static inline void omx_idelay_kernel_clear(void *state) {
  struct omx_idelay_kernel *k = state;
  for (uint32_t i = 0; i < (uint32_t)OMX_IDELAY_CAP; i++) k->ring_l[i] = 0.0f, k->ring_r[i] = 0.0f;
  k->pair.wpos_l = k->pair.wpos_r = 0u;
  k->pair.cur_l = k->pair.cur_r = 0u;
  OMX_POST(k->ring_l[0] == 0.0f && k->ring_r[OMX_IDELAY_CAP - 1] == 0.0f && k->pair.wpos_l == 0u && k->pair.cur_r == 0u,
           "cleared-pair-is-fresh");
}
#undef OMX_CONTRACT_STAGE

/**
 * @brief Take the resolved outputDelay: whole samples, omx_idelay_samples()'s, the line's own conversion; both legs
 *        take the one delay. A bypassed -> engaged edge clears the pair, so a re-engaged delay is a fresh one.
 * @param state A struct omx_idelay_kernel.
 * @param values outputDelay, ms, inside OUTPUT_DELAY_RANGE.
 * @param bypass Non-zero while bypassed.
 * @thread audio
 */
static inline void omx_idelay_kernel_set(void *state, const float *values, int bypass) {
  struct omx_idelay_kernel *k = state;
  if (!bypass && !k->enabled) omx_idelay_kernel_clear(k);
  k->enabled = bypass ? 0 : 1;
  k->ms = values[0];
  k->tgt = omx_idelay_samples(k->ms, k->sr, k->pair.cap);
}

/**
 * @brief One engaged block, both legs in place, at the delay: omx_idelay_pair_apply(), a move cross-faded across
 *        the block.
 * @param state A struct omx_idelay_kernel.
 * @param l The left leg, `n` frames.
 * @param r The right leg, `n` frames.
 * @param n The block length.
 * @thread audio
 */
static inline void omx_idelay_kernel_process(void *state, float *l, float *r, uint32_t n) {
  struct omx_idelay_kernel *k = state;
  omx_idelay_pair_apply(l, r, n, &k->pair, k->tgt, k->tgt);
}

/**
 * @brief The frames of latency the output delay reports: none. The delay IS the effect; a host that compensated it
 *        would undo the alignment.
 * @param state A struct omx_idelay_kernel, unread.
 * @return 0.
 * @thread audio
 */
static inline uint32_t omx_idelay_kernel_latency(const void *state) {
  (void)state;
  return 0u;
}

/**
 * @brief The output delay's descriptor: data/kernels/idelay.json's one control through the render. The travel
 *        declares no default; its floor, no delay, is the come-up value and what a non-finite word reads as.
 * @return The descriptor, a constant.
 * @thread control
 */
static inline const struct omx_kernel_desc *omx_idelay_desc(void) {
  static const struct omx_param_desc params[] = {
      {"outputDelay", OMX_PARAM_TRAVEL, (float)OMX_OUTPUT_DELAY_RANGE_MIN, (float)OMX_OUTPUT_DELAY_RANGE_MAX,
       (float)OMX_OUTPUT_DELAY_RANGE_MIN, OMX_PARAM_UNITS_MS},
  };
  static const struct omx_kernel_desc desc = {
      "idelay", sizeof(struct omx_idelay_kernel), 1u, params, 0u, NULL, omx_idelay_kernel_init, omx_idelay_kernel_set,
      omx_idelay_kernel_process, omx_idelay_kernel_clear, omx_idelay_kernel_latency, NULL,
  };
  return &desc;
}

#endif /* OMX_IDELAY_H */
