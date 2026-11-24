// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * @file omx_width.h
 * @brief The stereo width word: the M/S matrix at a width, a complementary comb or a Haas delay for a
 *        mono source, and the low end summed to mono through an LR4 split of the side.
 *
 * Per sample, `M = (L + R)/2`, `S = (L − R)/2`, then
 *
 *     S' = w·S + (comb ? g·M[n − d] : 0)                 the width, and the complementary comb's side
 *     bass mono:  M' = AP(M),  S'' = HP(S')              the LR4 crossover's all-pass and high band
 *     L = M' + S'',  R = M' − S''                        and with Haas, R = (M' − S'')[n − d]
 *
 * `g` is OMX_WIDTH_COMB_GAIN. The comb's legs are `M ± g·M[n − d]`, a complementary pair whose sum is
 * `2M`: the mono sum is the mid, whatever the comb. With bass mono, `M' + lo(S') + S'' = AP(L)`
 * (omx_xover's partition `lo + hi = AP`): each leg is its own all-pass less the side's low band, so
 * below the corner the side is gone and above it the image is the input's, in phase. Width 1, no
 * pseudo-stereo and no bass mono is not run: the caller's legs pass bit for bit.
 *
 * Contract: no allocation, no lock. The design runs on the control thread (one tan); the state
 * holds the delay as a ring of OMX_WIDTH_RING samples, the longest delay at the highest rate.
 */
#ifndef OMX_WIDTH_H
#define OMX_WIDTH_H

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "omx_contract.h"
#include "omx_kernel_desc.h"
#include "omx_xover.h"
#include <omxcontract/omx_vocabulary.h>
#include <omxdsp/omxdsp_definitions.h>

/** @brief The delay ring, samples: WIDTH_MAX_DELAY_MS at 192 kHz and one more, to a power of two. The
 *         comb's gain is OMX_WIDTH_COMB_GAIN, WIDTH_COMB_GAIN, from omxdsp_definitions.h. */
#define OMX_WIDTH_RING 8192u
_Static_assert(OMX_WIDTH_MAX_DELAY_MS * 192 + 1 <= OMX_WIDTH_RING, "the ring holds the longest delay at 192 kHz");

/** @brief The pseudo-stereo modes, WIDTH_PSEUDOS' order. */
enum omx_width_pseudo { OMX_WIDTH_OFF = 0, OMX_WIDTH_COMB = 1, OMX_WIDTH_HAAS = 2 };

/** @brief The designed word. */
struct omx_width {
  float w;              /**< The side's scale, widthPct/100. */
  int pseudo;           /**< An enum omx_width_pseudo. */
  uint32_t d;           /**< The pseudo-stereo's delay, samples. */
  int bass_mono;        /**< 1 sums the low end to mono. */
  struct omx_xover x;   /**< The bass mono crossover, LR4. */
  int active;           /**< 0 when the word is the identity and is not run. */
};

/** @brief The word's state. A zeroed state is silence. */
struct omx_width_state {
  struct omx_xover_state xs;    /**< The side's split. */
  struct omx_xover_ap_state ap; /**< The mid's all-pass. */
  float ring[OMX_WIDTH_RING];   /**< The delayed signal: the mid (comb) or the right leg (Haas). */
  uint32_t pos;                 /**< The ring's write index. */
};

#undef OMX_CONTRACT_STAGE
#define OMX_CONTRACT_STAGE "width/delay"
/**
 * @brief The pseudo-stereo's delay in samples: `pseudo_ms` at `sr`, rounded to the nearest sample.
 * @param pseudo_ms The delay, ms; held inside [0, WIDTH_MAX_DELAY_MS].
 * @param sr The rate, Hz.
 * @return The samples, below OMX_WIDTH_RING.
 * @post `inside-the-ring`.
 * @note Arithmetic. Pure.
 * @thread audio
 */
static inline uint32_t omx_width_delay(float pseudo_ms, float sr) {
  const float top = (float)OMX_WIDTH_MAX_DELAY_MS;
  const float ms = pseudo_ms > top ? top : pseudo_ms > 0.0f ? pseudo_ms : 0.0f;
  const uint32_t d = (uint32_t)lrintf(ms * 0.001f * sr);
  OMX_POST(d < OMX_WIDTH_RING, "inside-the-ring");
  return d < OMX_WIDTH_RING ? d : OMX_WIDTH_RING - 1u;
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "width/design"
/**
 * @brief Design the word.
 * @param p Receives the design.
 * @param w The side's scale, at least 0.
 * @param pseudo An enum omx_width_pseudo.
 * @param d The delay, samples, below OMX_WIDTH_RING.
 * @param bass_mono 1 to sum the low end to mono below `bass_hz`.
 * @param bass_hz The crossover's corner, Hz, inside (0, sr/2).
 * @param sr The rate, Hz.
 * @pre `width-not-negative`.
 * @note One tan when bass mono is on. Thread-safe: writes only `p`.
 * @thread audio
 */
static inline void omx_width_design(struct omx_width *p, float w, int pseudo, uint32_t d, int bass_mono, double bass_hz,
                                    double sr) {
  OMX_PRE(w >= 0.0f, "width-not-negative");
  p->w = w;
  p->pseudo = pseudo == OMX_WIDTH_COMB || pseudo == OMX_WIDTH_HAAS ? pseudo : OMX_WIDTH_OFF;
  p->d = d < OMX_WIDTH_RING ? d : OMX_WIDTH_RING - 1u;
  p->bass_mono = bass_mono ? 1 : 0;
  if (p->bass_mono) omx_xover_design(&p->x, 4u, bass_hz, sr);
  p->active = w != 1.0f || p->pseudo != OMX_WIDTH_OFF || p->bass_mono;
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "width/process"
/**
 * @brief One block through the word, both legs in place.
 * @param l The left leg.
 * @param r The right leg.
 * @param n Frames.
 * @param p The design.
 * @param st The state.
 * @pre `finite-in`.
 * @post `finite-out`.
 * @note Per sample a few multiplies, and with bass mono four double sections and one
 *       all-pass; no allocation. Thread-safe on distinct state.
 * @thread audio
 */
static inline void omx_width_process(float *l, float *r, uint32_t n, const struct omx_width *p, struct omx_width_state *st) {
  if (!p->active || n == 0u) return;
  OMX_PRE(omx_block_pair_finite(l, r, n), "finite-in");
  const uint32_t mask = OMX_WIDTH_RING - 1u;
  for (uint32_t i = 0; i < n; i++) {
    float m = 0.5f * (l[i] + r[i]);
    float s = p->w * (0.5f * (l[i] - r[i]));
    if (p->pseudo == OMX_WIDTH_COMB) {
      st->ring[st->pos] = m;
      s += OMX_WIDTH_COMB_GAIN * st->ring[(st->pos - p->d) & mask];
      st->pos = (st->pos + 1u) & mask;
    }
    if (p->bass_mono) {
      float lo, hi;
      omx_xover_process(&s, &lo, &hi, 1u, &p->x, &st->xs);
      omx_xover_allpass(&m, 1u, &p->x, &st->ap);
      s = hi;
    }
    l[i] = m + s;
    r[i] = m - s;
    if (p->pseudo == OMX_WIDTH_HAAS) {
      st->ring[st->pos] = r[i];
      r[i] = st->ring[(st->pos - p->d) & mask];
      st->pos = (st->pos + 1u) & mask;
    }
  }
  OMX_POST(omx_block_pair_finite(l, r, n), "finite-out");
}
#undef OMX_CONTRACT_STAGE

/** @brief The width kernel as the generic face serves it: the rate, the design, its state and whether it is engaged. */
struct omx_width_kernel {
  float sr;                   /**< The sample rate init bound the state to, Hz. */
  struct omx_width p;         /**< The design the last set made. */
  struct omx_width_state st;  /**< The design's running state, carried from block to block. */
  int enabled;                /**< Non-zero while engaged, as the last set left it. */
};

/**
 * @brief Bind a zeroed state to its rate: the rate must be one of the oracle set, as the width face takes it.
 * @param state A struct omx_width_kernel.
 * @param sr The sample rate, Hz.
 * @return 1 when usable, 0 when the rate is outside the oracle set (32 to 192 kHz).
 * @thread control
 */
static inline int omx_width_kernel_init(void *state, float sr) {
  struct omx_width_kernel *k = state;
  if (!omx_rate_is_oracle_full(sr)) return 0;
  k->sr = sr;
  return 1;
}

/**
 * @brief Take the resolved controls and design the word. The engage edge starts the word from silence; bass
 *        mono coming on starts its crossover from silence; a new pseudo-stereo mode or delay starts its ring
 *        from silence, so a ring left from minutes earlier, or the other mode's signal, is never read.
 * @param state A struct omx_width_kernel.
 * @param values widthPct (%), pseudo (a WIDTH_PSEUDOS index), pseudoMs (ms), bassMono (a WIDTH_BASS_MONOS
 *        index) and bassMonoHz (Hz), each resolved.
 * @param bypass Non-zero: disengaged.
 * @thread audio
 */
static inline void omx_width_kernel_set(void *state, const float *values, int bypass) {
  struct omx_width_kernel *k = state;
  const int was_bass = k->p.bass_mono, was_pseudo = k->p.pseudo, was_live = k->enabled && k->p.active;
  const uint32_t was_d = k->p.d;
  omx_width_design(&k->p, values[0] * 0.01f, (int)values[1], omx_width_delay(values[2], k->sr),
                   (int)values[3] == (int)OMX_WIDTH_BASS_MONOS_ON ? 1 : 0, (double)values[4], (double)k->sr);
  k->enabled = bypass ? 0 : 1;
  if (k->enabled && k->p.active && !was_live) {
    memset(&k->st, 0, sizeof k->st);
  } else {
    if (k->p.bass_mono && !was_bass) {
      memset(&k->st.xs, 0, sizeof k->st.xs);
      memset(&k->st.ap, 0, sizeof k->st.ap);
    }
    if (k->p.pseudo != was_pseudo || k->p.d != was_d) {
      memset(k->st.ring, 0, sizeof k->st.ring);
      k->st.pos = 0u;
    }
  }
}

/**
 * @brief One engaged block through the word, both legs in place; at the come-up values the identity.
 * @param state A struct omx_width_kernel.
 * @param l The left leg, `n` frames.
 * @param r The right leg, `n` frames.
 * @param n The block length.
 * @thread audio
 */
static inline void omx_width_kernel_process(void *state, float *l, float *r, uint32_t n) {
  struct omx_width_kernel *k = state;
  omx_width_process(l, r, n, &k->p, &k->st);
}

/**
 * @brief Silence the word's memory, its crossover and its ring, keeping the design.
 * @param state A struct omx_width_kernel.
 * @thread audio
 */
static inline void omx_width_kernel_clear(void *state) {
  struct omx_width_kernel *k = state;
  memset(&k->st, 0, sizeof k->st);
}

/**
 * @brief The frames of latency the width introduces: none.
 * @param state A struct omx_width_kernel.
 * @return 0.
 * @thread audio
 */
static inline uint32_t omx_width_kernel_latency(const void *state) {
  (void)state;
  return 0u;
}

/**
 * @brief The width kernel's descriptor: data/kernels/width.json's controls through the render.
 * @return The descriptor, a constant.
 * @thread control
 */
static inline const struct omx_kernel_desc *omx_width_desc(void) {
  static const struct omx_param_desc params[] = {
      {"widthPct", OMX_PARAM_TRAVEL, (float)OMX_WIDTH_WIDTH_PCT_RANGE_MIN, (float)OMX_WIDTH_WIDTH_PCT_RANGE_MAX, (float)OMX_WIDTH_WIDTH_PCT_RANGE_DEFAULT, OMX_PARAM_UNITS_PERCENT},
      {"pseudo", OMX_PARAM_CHOICE, 0.0f, (float)(OMX_WIDTH_PSEUDOS_COUNT - 1u), (float)OMX_WIDTH_PSEUDOS_DEFAULT, OMX_PARAM_UNITS_NONE},
      {"pseudoMs", OMX_PARAM_TRAVEL, (float)OMX_WIDTH_PSEUDO_MS_RANGE_MIN, (float)OMX_WIDTH_PSEUDO_MS_RANGE_MAX, (float)OMX_WIDTH_PSEUDO_MS_RANGE_DEFAULT, OMX_PARAM_UNITS_MS},
      {"bassMono", OMX_PARAM_CHOICE, 0.0f, (float)(OMX_WIDTH_BASS_MONOS_COUNT - 1u), (float)OMX_WIDTH_BASS_MONOS_DEFAULT, OMX_PARAM_UNITS_NONE},
      {"bassMonoHz", OMX_PARAM_TRAVEL, (float)OMX_WIDTH_BASS_MONO_HZ_RANGE_MIN, (float)OMX_WIDTH_BASS_MONO_HZ_RANGE_MAX, (float)OMX_WIDTH_BASS_MONO_HZ_RANGE_DEFAULT, OMX_PARAM_UNITS_HZ},
  };
  static const struct omx_kernel_desc desc = {
      "width", sizeof(struct omx_width_kernel), 5u, params, 0u, NULL, omx_width_kernel_init, omx_width_kernel_set,
      omx_width_kernel_process, omx_width_kernel_clear, omx_width_kernel_latency, NULL,
  };
  return &desc;
}

#endif /* OMX_WIDTH_H */
