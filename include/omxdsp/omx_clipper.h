// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * @file omx_clipper.h
 * @brief The clipper kernel: a drive, a ceiling and a softness, per sample, no memory, and its descriptor for
 *        the generic face.
 *
 * `u = drive·x`. Below the knee `a = c·(1 − span·softness)` a sample is `u`; above it
 * `y = a + (c − a)·d / (d + c − a)`, `d = |u| − a`, signed as `u`, a hyperbola that leaves the straight line
 * at slope 1 and approaches the ceiling `c` from below. At a softness of 0 `a` is `c`, the hard clip. Each leg
 * is its own; no latency. The readout `gainReductionDb` is the reduction of the block's loudest driven sample.
 */
#ifndef OMX_CLIPPER_H
#define OMX_CLIPPER_H

#include <math.h>
#include <stdint.h>

#include <omxcontract/omx_vocabulary.h>
#include <omxdsp/omx_contract.h>
#include <omxdsp/omx_kernel_desc.h>
#include <omxdsp/omx_units.h>
#include <omxdsp/omxdsp_definitions.h>

/**
 * @brief One sample through the clip.
 * @param u The sample after the drive.
 * @param c The ceiling, linear, positive.
 * @param a The knee, linear, in (0, c].
 * @return `u` under the knee, else the bend, signed as `u`, never past `c`.
 * @note One divide above the knee. Thread-safe: pure.
 * @thread audio
 */
static inline float omx_clipper_sample(float u, float c, float a) {
  const float m = fabsf(u);
  if (m <= a) return u;
  const float room = c - a, d = m - a;
  const float y = room > 0.0f ? a + room * d / (d + room) : c;
  return u < 0.0f ? -y : y;
}

/** @brief The clipper's state: the resolved drive, ceiling and knee, and the last block's reduction. */
struct omx_clipper_kernel {
  float drive_lin;   /**< The gain before the clip. */
  float ceiling_lin; /**< The ceiling. */
  float knee_lin;    /**< Where the bend starts. */
  float gr_db;       /**< The last block's reduction, dB, negative or 0. */
};

/**
 * @brief Bind a zeroed state to its rate: the rate must be one of the oracle set, as the clipper face takes it.
 * @param state A struct omx_clipper_kernel.
 * @param sr The sample rate, Hz.
 * @return 1 when usable, 0 when the rate is outside the oracle set (32 to 192 kHz).
 * @thread control
 */
static inline int omx_clipper_kernel_init(void *state, float sr) {
  (void)state;
  return omx_rate_is_oracle_full(sr);
}

#define OMX_CONTRACT_STAGE "clipper/kernel-set"
/**
 * @brief Take the resolved driveDb, ceilingDb and softnessPct. A bypass rests the readout at 0.
 * @param state A struct omx_clipper_kernel.
 * @param values driveDb (dB), ceilingDb (dB) and softnessPct (%), each inside its travel.
 * @param bypass Non-zero: the readout reads 0, as a bypassed clipper takes nothing off.
 * @post `knee-under-the-ceiling`.
 * @thread audio
 */
static inline void omx_clipper_kernel_set(void *state, const float *values, int bypass) {
  struct omx_clipper_kernel *k = state;
  k->drive_lin = omx_db_to_lin(values[0]);
  k->ceiling_lin = omx_db_to_lin(values[1]);
  k->knee_lin = k->ceiling_lin * (1.0f - (float)OMX_CLIPPER_KNEE_SPAN * values[2] * 0.01f);
  if (bypass) k->gr_db = 0.0f;
  OMX_POST(k->knee_lin > 0.0f && k->knee_lin <= k->ceiling_lin, "knee-under-the-ceiling");
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "clipper/kernel-process"
/**
 * @brief One engaged block, in place: the clip over both legs, and the block's reduction.
 * @param state A struct omx_clipper_kernel.
 * @param l The left leg, `n` frames.
 * @param r The right leg, `n` frames.
 * @param n The block length.
 * @pre `finite-in`.
 * @post `finite-out`.
 * @thread audio
 */
static inline void omx_clipper_kernel_process(void *state, float *l, float *r, uint32_t n) {
  struct omx_clipper_kernel *k = state;
  OMX_PRE(omx_block_pair_finite(l, r, n), "finite-in");
  const float g = k->drive_lin, c = k->ceiling_lin, a = k->knee_lin;
  float over = 0.0f, kept = 0.0f;
  for (uint32_t i = 0; i < n; i++) {
    const float ul = g * l[i], ur = g * r[i];
    const float yl = omx_clipper_sample(ul, c, a), yr = omx_clipper_sample(ur, c, a);
    const float ml = fabsf(ul), mr = fabsf(ur);
    if (ml > over) { over = ml; kept = fabsf(yl); }
    if (mr > over) { over = mr; kept = fabsf(yr); }
    l[i] = yl;
    r[i] = yr;
  }
  k->gr_db = over > 0.0f && kept < over ? omx_lin_to_db(kept / over) : 0.0f;
  OMX_POST(omx_block_pair_finite(l, r, n), "finite-out");
}
#undef OMX_CONTRACT_STAGE

/**
 * @brief Rest the readout: a clipper has no other memory.
 * @param state A struct omx_clipper_kernel.
 * @thread audio
 */
static inline void omx_clipper_kernel_clear(void *state) {
  struct omx_clipper_kernel *k = state;
  k->gr_db = 0.0f;
}

/**
 * @brief The gainReductionDb readout.
 * @param state A struct omx_clipper_kernel.
 * @param i The readout's index: 0, gainReductionDb.
 * @return The last block's reduction, dB, 0 or negative.
 * @thread audio
 */
static inline float omx_clipper_kernel_meter(const void *state, uint32_t i) {
  const struct omx_clipper_kernel *k = state;
  (void)i;
  return k->gr_db;
}

/**
 * @brief The frames of latency the clipper introduces: none.
 * @param state A struct omx_clipper_kernel.
 * @return 0.
 * @thread audio
 */
static inline uint32_t omx_clipper_kernel_latency(const void *state) {
  (void)state;
  return 0u;
}

/**
 * @brief The clipper kernel's descriptor: data/kernels/clipper.json's controls and readout through the render.
 * @return The descriptor, a constant.
 * @thread control
 */
static inline const struct omx_kernel_desc *omx_clipper_desc(void) {
  static const struct omx_param_desc params[] = {
      {"driveDb", OMX_PARAM_TRAVEL, (float)OMX_CLIPPER_DRIVE_DB_RANGE_MIN, (float)OMX_CLIPPER_DRIVE_DB_RANGE_MAX, (float)OMX_CLIPPER_DRIVE_DB_RANGE_DEFAULT, OMX_PARAM_UNITS_DB},
      {"ceilingDb", OMX_PARAM_TRAVEL, (float)OMX_CLIPPER_CEILING_DB_RANGE_MIN, (float)OMX_CLIPPER_CEILING_DB_RANGE_MAX, (float)OMX_CLIPPER_CEILING_DB_RANGE_DEFAULT, OMX_PARAM_UNITS_DB},
      {"softnessPct", OMX_PARAM_TRAVEL, (float)OMX_CLIPPER_SOFTNESS_PCT_RANGE_MIN, (float)OMX_CLIPPER_SOFTNESS_PCT_RANGE_MAX, (float)OMX_CLIPPER_SOFTNESS_PCT_RANGE_DEFAULT, OMX_PARAM_UNITS_PERCENT},
  };
  static const struct omx_meter_desc meters[] = {
      {"gainReductionDb", OMX_PARAM_UNITS_DB, OMX_CLIPPER_READOUT_GAIN_REDUCTION_DB_MIN, OMX_CLIPPER_READOUT_GAIN_REDUCTION_DB_MAX},
  };
  static const struct omx_kernel_desc desc = {
      "clipper", sizeof(struct omx_clipper_kernel), 3u, params, 1u, meters, omx_clipper_kernel_init, omx_clipper_kernel_set,
      omx_clipper_kernel_process, omx_clipper_kernel_clear, omx_clipper_kernel_latency, omx_clipper_kernel_meter,
  };
  return &desc;
}

#endif /* OMX_CLIPPER_H */
