// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * The band reduction's scenarios, run by tools/rta-bands-golden.c to write vectors/rta-bands.json and by the suite's
 * rta_bands arm to check omx_rta_bands against that file. One place, so the file and the check run the same
 * spectra; ts/test/rta-bands.test.mjs builds the same ones with the same float arithmetic (Math.fround).
 *
 * Each scenario is a magnitude spectrum at a rate's own window (omx_rta_fft_size_for), bin 0 = DC, linear:
 *   slope    every bin k >= 1 at 0.1 / sqrt(k): a falling broadband floor, loud enough to pass the gate;
 *   tones    40 Hz at 0.5, 1 kHz at 1.0, 12 kHz at 0.25, each its nearest bin with its two neighbours at half;
 *   tiers    the slope plus a 50 Hz tone at 1.0, AND the low zoom's tier (OMX_RTA_LOW_FFT_SIZE points at rate / 8)
 *            carrying the same: the bands below the crossover read it, with its gain correction;
 *   quiet    every bin at 0.0002 (-74 dBFS), under the gate: every band reads as silence;
 *   silence  every bin zero.
 * Band counts 1, 64 and 640 (the finest panel density), at 48 and 96 kHz in the file; the arm's closed forms run at
 * every oracle rate.
 */
#ifndef OMXDSP_TEST_RTA_BANDS_SCENARIOS_H
#define OMXDSP_TEST_RTA_BANDS_SCENARIOS_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include <omxdsp/analysis/omx_rta.h>
#include <omxdsp/analysis/omx_rta_bands.h>
#include <omxdsp/omx_contract.h>

enum rb_kind { RB_SLOPE, RB_TONES, RB_TIERS, RB_QUIET, RB_SILENCE, RB_KINDS };
static const char *const RB_KIND_NAMES[RB_KINDS] = {"slope", "tones", "tiers", "quiet", "silence"};
static const float RB_FILE_RATES[] = {48000.0f, 96000.0f};
#define RB_FILE_RATES_COUNT 2u
static const uint32_t RB_BAND_COUNTS[] = {1u, 64u, 640u};
#define RB_BAND_COUNTS_COUNT 3u
#define RB_MAX_BANDS 640u

/** One scenario's spectra and the tiers that point at them. */
struct rb_case {
  float high[OMX_RTA_FFT_SIZE_MAX / 2];
  float low[OMX_RTA_LOW_FFT_SIZE / 2];
  struct omx_rta_tier h, l;
  bool has_low;
};

/** A tone at `hz` of amplitude `amp` on its nearest bin of `m` (bins wide, `bin_hz` apart), half on each neighbour. */
static inline void rb_tone(float *m, uint32_t bins, float bin_hz, float hz, float amp) {
  const long b = lroundf(hz / bin_hz);
  if (b < 1 || (uint32_t)b + 1u >= bins) return;
  m[b] += amp;
  m[b - 1] += amp * 0.5f;
  m[b + 1] += amp * 0.5f;
}

static inline void rb_slope(float *m, uint32_t bins) {
  m[0] = 0.0f;
  for (uint32_t k = 1; k < bins; k++) m[k] = 0.1f / sqrtf((float)k);
}

/** Build scenario `kind` at `rate` into `c`. */
static inline void rb_build(enum rb_kind kind, float rate, struct rb_case *c) {
  const uint32_t fft = omx_rta_fft_size_for((uint32_t)rate), bins = fft / 2u;
  const float bin_hz = rate / (float)fft;
  const uint32_t lfft = OMX_RTA_LOW_FFT_SIZE, lbins = lfft / 2u;
  const float lrate = rate / (float)OMX_RTA_LOW_DECIM_FACTOR, lbin_hz = lrate / (float)lfft;
  for (uint32_t k = 0; k < bins; k++) c->high[k] = 0.0f;
  for (uint32_t k = 0; k < lbins; k++) c->low[k] = 0.0f;
  c->has_low = false;
  switch (kind) {
  case RB_SLOPE: rb_slope(c->high, bins); break;
  case RB_TONES:
    rb_tone(c->high, bins, bin_hz, 40.0f, 0.5f);
    rb_tone(c->high, bins, bin_hz, 1000.0f, 1.0f);
    rb_tone(c->high, bins, bin_hz, 12000.0f, 0.25f);
    break;
  case RB_TIERS:
    rb_slope(c->high, bins);
    rb_tone(c->high, bins, bin_hz, 50.0f, 1.0f);
    rb_slope(c->low, lbins);
    rb_tone(c->low, lbins, lbin_hz, 50.0f, 1.0f);
    c->has_low = true;
    break;
  case RB_QUIET:
    for (uint32_t k = 0; k < bins; k++) c->high[k] = 0.0002f;
    break;
  case RB_SILENCE:
  case RB_KINDS: break;
  }
  c->h = (struct omx_rta_tier){c->high, bins, (double)rate, fft};
  c->l = (struct omx_rta_tier){c->low, lbins, (double)lrate, lfft};
}

#endif /* OMXDSP_TEST_RTA_BANDS_SCENARIOS_H */
