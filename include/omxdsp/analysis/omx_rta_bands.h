/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * omx_rta_bands.h — a magnitude spectrum reduced to log-spaced display bands: the reduction every
 * surface that draws a spectrum shares. Its TypeScript twin is ts/src/rta.ts aggregateSpectrumBands,
 * held to it by vectors/rta-bands.json. The numbers are data/kernels/rta.json's: the band edges
 * (OMX_RTA_BAND_F_MIN, OMX_RTA_BAND_F_MAX_CAP), the tier crossover and the quiet gate.
 *
 * Not yet implemented: the declarations land first, the reduction after its tests.
 */
#ifndef OMX_RTA_BANDS_H
#define OMX_RTA_BANDS_H

#include <math.h>
#include <stdint.h>

#include <omxdsp/omxdsp_definitions.h>

/** One tier of a reading: linear magnitudes (bin 0 = DC) and the transform that produced them. */
struct omx_rta_tier {
  const float *mags;
  uint32_t bins;
  double rate;
  uint32_t fft_size;
};

/** One band: its geometric centre (Hz), the RMS across its bins and its loudest bin (dBFS; silence is -INFINITY). */
struct omx_rta_band {
  double hz;
  double db;
  double peak_db;
};

/** The highest frequency a band covers at `rate`: OMX_RTA_BAND_F_MAX_CAP, or the Nyquist where that is lower. */
static inline double omx_rta_band_f_max(double rate) {
  (void)rate;
  return 0.0;
}

/** The dB that level-matches a low-tier band to the high tier: the bin-density ratio; 0 for a degenerate tier. */
static inline double omx_rta_low_gain_db(const struct omx_rta_tier *high, const struct omx_rta_tier *low) {
  (void)high, (void)low;
  return 0.0;
}

/** Reduce `high` (and `low` under the crossover, when not NULL) to `n` bands into `out`; the count written. */
static inline uint32_t omx_rta_bands(const struct omx_rta_tier *high, const struct omx_rta_tier *low, uint32_t n,
                                     struct omx_rta_band *out) {
  (void)high, (void)low, (void)n, (void)out;
  return 0u;
}

#endif /* OMX_RTA_BANDS_H */
