/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * omx_hrp_place.c — the HRP placement and stepping (omx_hrp_place.h), one function per rule so
 * test/analysis/hrp-place-sabotage.sh can break each once. The console's harmonicBands and stepToward, term for
 * term: the same comparisons in the same order, in doubles, so the shared vectors hold both to the bit.
 */
#include <math.h>

#include <omxdsp/analysis/omx_hrp_place.h>

/* A number that may take part in the arithmetic: the console's Number.isFinite. */
static int usable(double v) { return isfinite(v); }

/* The console's clamp: below lo is lo, above hi is hi, anything else (a NaN too) passes through. */
static double clamp(double v, double lo, double hi) { return v < lo ? lo : v > hi ? hi : v; }

/* REFUSED WHOLE: an input the placement cannot honour places nothing. */
static int refused(double f0_hz, double q, const OmxHrpPlaceLimits *l) {
  if (!l) return 1;
  if (!usable(f0_hz) || f0_hz <= 0.0) return 1;
  if (!usable(q)) return 1;
  if (!usable(l->max_cut_db) || l->max_cut_db < 0.0) return 1;
  if (!usable(l->max_boost_db) || l->max_boost_db < 0.0) return 1;
  if (!usable(l->max_bands) || l->max_bands < 1.0) return 1;
  return 0;
}

int omx_hrp_place_in_range(double freq_hz) {
  /* DROP, NEVER CLAMP: both bounds kept; written as the console's refusal so a NaN is dropped too */
  return !(freq_hz < (double)OMX_EQ_FREQ_RANGE_MIN || freq_hz > (double)OMX_EQ_FREQ_RANGE_MAX || isnan(freq_hz));
}

/* THE BELL'S Q: the console's clampEqBandQ('bell', q). */
static double bell_q(double q) { return clamp(q, (double)OMX_EQ_Q_RANGE_MIN, (double)OMX_EQ_Q_RANGE_MAX); }

/* THE CAP's order: a before b when its |gain| is larger, ties to the lower harmonic. */
static int stronger(const OmxHrpPlacedBand *a, const OmxHrpPlacedBand *b) {
  const double ga = fabs(a->gain_db), gb = fabs(b->gain_db);
  if (ga != gb) return ga > gb;
  return a->harmonic < b->harmonic;
}

/* THE CAP: over `keep` bands, the weakest leaves until `keep` are left; the rest keep their (harmonic) order. */
static uint32_t cap(OmxHrpPlacedBand *out, uint32_t m, double max_bands) {
  if ((double)m <= max_bands) return m;
  const uint32_t keep = (uint32_t)max_bands; /* a free-slot count: a fraction keeps its whole part */
  while (m > keep) {
    uint32_t weakest = 0u;
    for (uint32_t i = 1u; i < m; i++)
      if (stronger(&out[weakest], &out[i])) weakest = i;
    for (uint32_t i = weakest; i + 1u < m; i++) out[i] = out[i + 1u];
    m--;
  }
  return m;
}

uint32_t omx_hrp_place_bands(double f0_hz, const double *gains_db, uint32_t n_gains, double q,
                             const OmxHrpPlaceLimits *limits, OmxHrpPlacedBand *out) {
  if (!out || (n_gains > 0u && !gains_db)) return 0u;
  if (refused(f0_hz, q, limits)) return 0u;
  const double band_q = bell_q(q);
  uint32_t m = 0u;
  for (uint32_t k = 0u; k < n_gains; k++) {
    const double requested = gains_db[k];
    if (!usable(requested)) continue;
    const uint32_t harmonic = k + 1u;
    const double freq_hz = (double)harmonic * f0_hz;
    if (!omx_hrp_place_in_range(freq_hz)) continue;
    const double gain_db = clamp(requested, -limits->max_cut_db, limits->max_boost_db);
    if (gain_db == 0.0) continue; /* SPARSE: -0 is the same fact */
    out[m].harmonic = harmonic;
    out[m].freq_hz = freq_hz;
    out[m].gain_db = gain_db;
    out[m].q = band_q;
    m++;
  }
  return cap(out, m, limits->max_bands);
}

double omx_hrp_step_toward(double current, double target) {
  const double delta = target - current;
  if (fabs(delta) <= (double)OMX_HRP_GAIN_STEP_DB) return target;
  /* the console's Math.sign: a NaN delta stays NaN, so a NaN in gives a NaN out */
  const double sign = delta > 0.0 ? 1.0 : delta < 0.0 ? -1.0 : delta;
  return current + sign * (double)OMX_HRP_GAIN_STEP_DB;
}
