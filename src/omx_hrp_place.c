/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * omx_hrp_place.c — the HRP placement and stepping (omx_hrp_place.h). NOT YET: places nothing and jumps to the
 * target, so test/analysis/hrp_place.test.c reads the shared vectors red before the rules are written.
 */
#include <omxdsp/analysis/omx_hrp_place.h>

uint32_t omx_hrp_place_bands(double f0_hz, const double *gains_db, uint32_t n_gains, double q,
                             const OmxHrpPlaceLimits *limits, OmxHrpPlacedBand *out) {
  (void)f0_hz, (void)gains_db, (void)n_gains, (void)q, (void)limits, (void)out;
  return 0u;
}

int omx_hrp_place_in_range(double freq_hz) {
  (void)freq_hz;
  return 1;
}

double omx_hrp_step_toward(double current, double target) {
  (void)current;
  return target;
}
