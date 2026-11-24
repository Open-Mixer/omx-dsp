// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * @file omx_trim.h
 * @brief The trim kernel: the lane's head gain, a de-zippered ramp from the gain the last block ended on to
 *        the one resolved now, both legs from the same start, and its descriptor for the generic face.
 *
 * The arithmetic is omx_ramp.h's: `out[i] = in[i] * omx_ramp_at(r, i)`, run once per leg from the same
 * start gain, as the engine's trim-gain fill. The one control is `trimDb` (TRIM_RANGE), turned into a
 * linear gain with omx_db_to_lin. A fresh state's gain is unity; a bypassed block holds the ramp where it
 * was, so re-engaging glides from the gain the last engaged block ended on. No latency, no allocation.
 */
#ifndef OMX_TRIM_H
#define OMX_TRIM_H

#include <stdint.h>

#include <omxcontract/omx_vocabulary.h>
#include <omxdsp/omx_kernel_desc.h>
#include <omxdsp/omx_ramp.h>
#include <omxdsp/omx_units.h>
#include <omxdsp/omxdsp_definitions.h>

/** @brief The trim's state: the linear gain the last engaged block ended on and the one the next block ramps to. */
struct omx_trim_kernel {
  float cur; /**< The gain the last engaged block ended on; init sets it to 1. */
  float tgt; /**< The gain the last set resolved from trimDb. */
};

/**
 * @brief Bind a zeroed state to its rate: unity gain. The gain does not depend on the rate; a rate that is not
 *        positive and finite is refused, as the trim face refuses it.
 * @param state A struct omx_trim_kernel.
 * @param sr The sample rate, Hz.
 * @return 1 when usable, 0 when the rate is not positive and finite.
 * @thread control
 */
static inline int omx_trim_kernel_init(void *state, float sr) {
  struct omx_trim_kernel *k = state;
  if (!(sr > 0.0f) || sr - sr != 0.0f) return 0;
  k->cur = k->tgt = 1.0f;
  return 1;
}

/**
 * @brief Take the resolved trimDb: the gain the next engaged block ramps to. A bypass changes nothing here.
 * @param state A struct omx_trim_kernel.
 * @param values trimDb, dB, inside TRIM_RANGE.
 * @param bypass Unused: a bypassed trim holds its ramp, and the face runs nothing.
 * @thread audio
 */
static inline void omx_trim_kernel_set(void *state, const float *values, int bypass) {
  struct omx_trim_kernel *k = state;
  (void)bypass;
  k->tgt = omx_db_to_lin(values[0]);
}

/**
 * @brief One engaged block, in place: both legs ramped from the same start gain to the resolved one.
 * @param state A struct omx_trim_kernel.
 * @param l The left leg, `n` frames.
 * @param r The right leg, `n` frames.
 * @param n The block length, at least 1.
 * @thread audio
 */
static inline void omx_trim_kernel_process(void *state, float *l, float *r, uint32_t n) {
  struct omx_trim_kernel *k = state;
  const struct omx_ramp g = omx_ramp_begin(&k->cur, k->tgt, n);
  for (uint32_t i = 0; i < n; i++) l[i] *= omx_ramp_at(g, i), r[i] *= omx_ramp_at(g, i);
  omx_ramp_end(g, &k->cur);
}

/**
 * @brief Land the ramp: the next block runs at the resolved gain from its first sample.
 * @param state A struct omx_trim_kernel.
 * @thread audio
 */
static inline void omx_trim_kernel_clear(void *state) {
  struct omx_trim_kernel *k = state;
  k->cur = k->tgt;
}

/**
 * @brief The frames of latency the trim introduces: none.
 * @param state A struct omx_trim_kernel.
 * @return 0.
 * @thread audio
 */
static inline uint32_t omx_trim_kernel_latency(const void *state) {
  (void)state;
  return 0u;
}

/**
 * @brief The trim kernel's descriptor: data/kernels/trim.json's one control through the render.
 * @return The descriptor, a constant.
 * @thread control
 */
static inline const struct omx_kernel_desc *omx_trim_desc(void) {
  static const struct omx_param_desc params[] = {
      {"trimDb", OMX_PARAM_TRAVEL, (float)OMX_TRIM_RANGE_MIN, (float)OMX_TRIM_RANGE_MAX, (float)OMX_TRIM_RANGE_DEFAULT, OMX_PARAM_UNITS_DB},
  };
  static const struct omx_kernel_desc desc = {
      "trim", sizeof(struct omx_trim_kernel), 1u, params, 0u, NULL, omx_trim_kernel_init, omx_trim_kernel_set,
      omx_trim_kernel_process, omx_trim_kernel_clear, omx_trim_kernel_latency, NULL,
  };
  return &desc;
}

#endif /* OMX_TRIM_H */
