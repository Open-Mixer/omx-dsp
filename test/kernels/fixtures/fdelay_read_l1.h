// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * The oracle of the fractional read's L1 norm Λ (FDELAY_READ_L1_NORM, data/primitives/fdelay-read-l1-norm.json):
 * the worst Σ|c| of omx_fdelay_lagrange's taps at `order`, over 1024 fractions of one sample, rounded to 1e-9. One
 * body for the suite's fdelay arm (test/kernels/fdelay.c) and the declaration test that moves the value
 * (test/contract/fdelay-read-l1-ownership.test.mjs): the value is this library's, held by its own kernel.
 */
#ifndef OMXDSP_TEST_FDELAY_READ_L1_H
#define OMXDSP_TEST_FDELAY_READ_L1_H

#include <math.h>

#include <omxdsp/omx_fdelay.h>

/** The worst L1 norm of the order-`order` read over 1024 fractions, rounded to 1e-9. */
static inline double omx_test_fdelay_read_l1(int order) {
  double worst = 0.0;
  for (int i = 0; i < 1024; i++) {
    float c[OMX_FDELAY_MAX_TAPS];
    omx_fdelay_lagrange(order, (float)i / 1024.0f, c);
    double l1 = 0.0;
    for (int k = 0; k <= order; k++) l1 += fabs((double)c[k]);
    if (l1 > worst) worst = l1;
  }
  return round(worst * 1e9) / 1e9;
}

#endif
