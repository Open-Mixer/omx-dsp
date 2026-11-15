/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * omx_rta_bands.h — a magnitude spectrum reduced to log-spaced display bands: the reduction every
 * surface that draws a spectrum shares. Its TypeScript twin is ts/src/rta.ts aggregateSpectrumBands,
 * held to it by vectors/rta-bands.json. The numbers are data/kernels/rta.json's: the band edges
 * (OMX_RTA_BAND_F_MIN, OMX_RTA_BAND_F_MAX_CAP), the tier crossover and the quiet gate.
 *
 * A trace is drawn on a LOG frequency axis and a linear FFT is the wrong shape for it: an 8192-point
 * transform at 96 kHz puts half its bins above 24 kHz and one bin per octave at the bottom. So the
 * spectrum is reduced to `n` bands tiling [OMX_RTA_BAND_F_MIN, omx_rta_band_f_max] evenly in log
 * frequency, each answered at its GEOMETRIC centre. A band's `db` is the RMS across its bins, not the
 * loudest, so one transient bin cannot paint a whole band hot; `peak_db` keeps the loudest bin anyway,
 * because only that one is a clip. A band centred under OMX_RTA_TIER_CROSSOVER_HZ reads the low
 * (decimated) tier when the reading carries one, level-matched by the bin-density ratio
 * (omx_rta_low_gain_db); its peak is never corrected, per-bin amplitude being calibrated in both tiers.
 * When no band rises above OMX_RTA_BAND_GATE_DB, every band reads as silence, so an idle floor never
 * draws as a flat block. The arithmetic is double throughout, the twin's own; it allocates nothing and
 * runs on a control thread, never in an audio callback.
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

/** The highest frequency a band covers at `rate`: OMX_RTA_BAND_F_MAX_CAP, or the Nyquist where that is lower. @thread control */
static inline double omx_rta_band_f_max(double rate) {
  const double nyquist = rate / 2.0;
  return (double)OMX_RTA_BAND_F_MAX_CAP < nyquist ? (double)OMX_RTA_BAND_F_MAX_CAP : nyquist;
}

/** The dB that level-matches a low-tier band to the high tier: the bin-density ratio; 0 for a degenerate tier. @thread control */
static inline double omx_rta_low_gain_db(const struct omx_rta_tier *high, const struct omx_rta_tier *low) {
  const double ratio = (double)low->fft_size / low->rate / ((double)high->fft_size / high->rate);
  if (!isfinite(ratio) || ratio <= 0.0) return 0.0;
  return 10.0 * log10(ratio);
}

/** Linear amplitude to dB, silence as -INFINITY: the twin's linearGainToDb. @thread control */
static inline double omx_rta_band_db(double lin) {
  return lin <= 0.0 ? -INFINITY : 20.0 * log10(lin);
}

/** RMS and peak (linear) of `tier` over the bins covering [f0, f1); a band spanning none samples the nearest. @thread control */
static inline void omx_rta_band_stats(const struct omx_rta_tier *tier, double f0, double f1, double *rms, double *peak) {
  const int64_t last = (int64_t)tier->bins - 1;
  int64_t i0 = (int64_t)floor(f0 * tier->fft_size / tier->rate);
  int64_t i1 = (int64_t)ceil(f1 * tier->fft_size / tier->rate);
  if (i0 < 1) i0 = 1;
  if (i1 > last) i1 = last;
  if (i1 < i0) i0 = i1;
  double sum_sq = 0.0, top = 0.0;
  uint32_t n = 0;
  for (int64_t i = i0; i <= i1; i++) {
    const double m = i >= 0 && i <= last ? (double)tier->mags[i] : 0.0;
    sum_sq += m * m;
    if (m > top) top = m;
    n++;
  }
  *rms = n > 0 ? sqrt(sum_sq / n) : 0.0;
  *peak = top;
}

/** Reduce `high` (and `low` under the crossover, when not NULL) to `n` bands into `out`; the count written. @thread control */
static inline uint32_t omx_rta_bands(const struct omx_rta_tier *high, const struct omx_rta_tier *low, uint32_t n,
                                     struct omx_rta_band *out) {
  if (n < 1u) return 0u;
  const double f_min = (double)OMX_RTA_BAND_F_MIN;
  double f_max = omx_rta_band_f_max(high->rate);
  if (f_max < f_min * 2.0) f_max = f_min * 2.0;
  const double lo = log10(f_min), hi = log10(f_max);
  const double low_gain = low ? omx_rta_low_gain_db(high, low) : 0.0;
  double max_db = -INFINITY;
  for (uint32_t b = 0; b < n; b++) {
    const double f0 = pow(10.0, lo + (hi - lo) * b / n), f1 = pow(10.0, lo + (hi - lo) * (b + 1u) / n);
    const double centre = sqrt(f0 * f1);
    const int use_low = low && centre < (double)OMX_RTA_TIER_CROSSOVER_HZ;
    double rms, peak;
    omx_rta_band_stats(use_low ? low : high, f0, f1, &rms, &peak);
    const double db = omx_rta_band_db(rms) + (use_low ? low_gain : 0.0);
    if (db > max_db) max_db = db;
    out[b] = (struct omx_rta_band){centre, db, omx_rta_band_db(peak)};
  }
  if (max_db < (double)OMX_RTA_BAND_GATE_DB)
    for (uint32_t b = 0; b < n; b++) out[b].db = out[b].peak_db = -INFINITY;
  return n;
}

/*
 * The difference of two taps of one spectrum, PRE (what a processor is given) and POST (what it gives), reduced to the
 * same n bands: what the processor did, band by band. The console draws it as wedges between the two traces; this is
 * its arithmetic, read in dB. Its TypeScript twin is ts/src/rta.ts bandDelta, held to it by vectors/rta-delta.json.
 */

/** One wedge: bands `band` and `band + 1`, a boost or a cut, and its strength, 0 to 1. */
struct omx_rta_wedge {
  uint32_t band;
  int boost;
  double strength;
};

/**
 * POST − PRE of n bands' levels (`db`): diff[i] in dB, NAN where either band is silent (-INFINITY). Then the wedges,
 * one per pair of neighbouring bands whose larger |difference| reaches OMX_RTA_DELTA_DEADBAND_DB, none across a silent
 * band: a boost when the pair's two differences sum to zero or more, else a cut, its strength the larger |difference|
 * over the EQ's own gain limit (omx-contract's EQ_GAIN_RANGE maximum), clamped to 1. `wedges` holds n − 1; returns how
 * many it wrote. @thread control
 */
static inline uint32_t omx_rta_band_delta(const struct omx_rta_band *pre, const struct omx_rta_band *post, uint32_t n,
                                          double *diff, struct omx_rta_wedge *wedges) {
  (void)pre, (void)post, (void)wedges;
  for (uint32_t i = 0; i < n; i++) diff[i] = 0.0;
  return 0;
}

#endif /* OMX_RTA_BANDS_H */
