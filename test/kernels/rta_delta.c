// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * The difference of two taps (analysis/omx_rta_bands.h omx_rta_band_delta), held by its closed form: hand-made band
 * levels whose differences are known exactly, a boost, a cut, a pair of opposite signs, the deadband from both sides,
 * a silent band, a strength past the EQ's gain limit, and too few bands for a pair.
 */
#define RD_N 8u

static struct omx_rta_band rd_pre[RD_N], rd_post[RD_N];
static double rd_diff[RD_N];
static struct omx_rta_wedge rd_w[RD_N];

/* pre at `base` dB in every band, post = pre + d[i] (d may be -INFINITY for a silent post band) */
static void rd_set(double base, const double *d) {
  for (uint32_t i = 0; i < RD_N; i++) {
    rd_pre[i] = (struct omx_rta_band){100.0 * (i + 1), base, base};
    rd_post[i] = (struct omx_rta_band){100.0 * (i + 1), base + d[i], base + d[i]};
  }
}

static void arm_rta_delta(void) {
  g_arm = "rta_delta";
  const double full = (double)OMX_EQ_GAIN_RANGE_MAX, dead = OMX_RTA_DELTA_DEADBAND_DB_DOUBLE;

  /* a boost of 6 dB on bands 3..5: diff exact, the four pairs that touch it are boosts at 6/15 */
  const double boost[RD_N] = {0, 0, 0, 6, 6, 6, 0, 0};
  rd_set(-30.0, boost);
  uint32_t k = omx_rta_band_delta(rd_pre, rd_post, RD_N, rd_diff, rd_w);
  uint32_t off = 0;
  for (uint32_t i = 0; i < RD_N; i++) off += rd_diff[i] != boost[i];
  ok(off == 0, "diff is POST - PRE in every band, exactly", off, 0);
  ok(k == 4u, "a 6 dB boost on three bands draws the four pairs that touch it", k, 4);
  uint32_t wrong = 0;
  for (uint32_t j = 0; j < k && j < 4u; j++)
    wrong += rd_w[j].band != 2u + j || !rd_w[j].boost || rd_w[j].strength != 6.0 / full;
  ok(k == 4u && wrong == 0, "each is a boost at the larger difference over the EQ's gain limit, in band order", wrong, 0);

  /* a cut of 9 dB: cuts at 9/15 */
  const double cut[RD_N] = {0, 0, 0, 0, -9, 0, 0, 0};
  rd_set(-30.0, cut);
  k = omx_rta_band_delta(rd_pre, rd_post, RD_N, rd_diff, rd_w);
  ok(k == 2u && !rd_w[0].boost && !rd_w[1].boost && rd_w[0].band == 3u && rd_w[1].band == 4u,
     "a 9 dB cut draws two cuts, at bands 3 and 4", k, 2);
  ok(k == 2u && rd_w[0].strength == 9.0 / full && rd_w[1].strength == 9.0 / full, "a cut's strength is 9 dB over the limit",
     k ? rd_w[0].strength : -1, 9.0 / full);

  /* opposite signs in one pair: the sign of their sum, the strength of the larger */
  const double mixed[RD_N] = {0, 0, 0, 3, -5, 0, 0, 0};
  rd_set(-30.0, mixed);
  k = omx_rta_band_delta(rd_pre, rd_post, RD_N, rd_diff, rd_w);
  int found = 0;
  for (uint32_t j = 0; j < k; j++)
    if (rd_w[j].band == 3u) found = !rd_w[j].boost && rd_w[j].strength == 5.0 / full ? 1 : -1;
  ok(found == 1, "a pair of +3 and -5 dB is a cut (their sum) at 5 dB (the larger)", found, 1);

  /* the deadband: a difference of exactly the deadband draws, one just under does not */
  const double at[RD_N] = {0, 0, 0, 0, dead, 0, 0, 0};
  rd_set(-1.0, at);
  ok(rd_post[4].db - rd_pre[4].db == dead, "the fixture's difference is the deadband exactly", rd_post[4].db - rd_pre[4].db,
     dead);
  k = omx_rta_band_delta(rd_pre, rd_post, RD_N, rd_diff, rd_w);
  ok(k == 2u, "a difference of exactly the deadband draws its two pairs", k, 2);
  const double under[RD_N] = {0, 0, 0, 0, dead * 0.99, 0, 0, 0};
  rd_set(-1.0, under);
  k = omx_rta_band_delta(rd_pre, rd_post, RD_N, rd_diff, rd_w);
  ok(k == 0u, "a difference under the deadband draws nothing", k, 0);
  const double flat[RD_N] = {0};
  rd_set(-30.0, flat);
  ok(omx_rta_band_delta(rd_pre, rd_post, RD_N, rd_diff, rd_w) == 0u, "two equal taps draw nothing", 1, 0);

  /* a silent band: its diff is NAN, and no pair across it draws, however large its neighbour's difference */
  const double silent[RD_N] = {0, 0, 12, -INFINITY, 12, 0, 0, 0};
  rd_set(-30.0, silent);
  k = omx_rta_band_delta(rd_pre, rd_post, RD_N, rd_diff, rd_w);
  ok(isnan(rd_diff[3]) && rd_diff[2] == 12.0, "a silent band's diff is NAN, its neighbours' are numbers", rd_diff[3], NAN);
  uint32_t across = 0;
  for (uint32_t j = 0; j < k; j++) across += rd_w[j].band == 2u || rd_w[j].band == 3u;
  ok(across == 0 && k == 3u, "no pair across a silent band draws; the others do", k, 3);
  rd_set(-30.0, boost);
  rd_pre[0].db = -INFINITY;
  omx_rta_band_delta(rd_pre, rd_post, RD_N, rd_diff, rd_w);
  ok(isnan(rd_diff[0]), "a silent PRE band's diff is NAN too", rd_diff[0], NAN);

  /* past the EQ's gain limit the strength is 1 */
  const double loud[RD_N] = {0, 0, 0, 30, 0, 0, 0, 0};
  rd_set(-40.0, loud);
  k = omx_rta_band_delta(rd_pre, rd_post, RD_N, rd_diff, rd_w);
  ok(k == 2u && rd_w[0].strength == 1.0, "a 30 dB boost is at full strength, clamped to 1", k ? rd_w[0].strength : -1, 1);

  /* too few bands for a pair */
  ok(omx_rta_band_delta(rd_pre, rd_post, 1u, rd_diff, rd_w) == 0u && rd_diff[0] == 30.0 - 30.0 + rd_post[0].db - rd_pre[0].db,
     "one band: its diff, no wedge", 1, 0);
  ok(omx_rta_band_delta(rd_pre, rd_post, 0u, rd_diff, rd_w) == 0u, "no band: nothing", 1, 0);
}
