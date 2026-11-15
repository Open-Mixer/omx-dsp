// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * The difference of two taps (analysis/omx_rta_bands.h omx_rta_band_delta), held by its closed form: hand-made band
 * levels whose differences are known exactly, a boost, a cut, a pair of opposite signs, the deadband from both sides,
 * a silent band, a strength past the EQ's gain limit, and too few bands for a pair; then vectors/rta-delta.json, the
 * file the TypeScript twin (ts/src/rta.ts bandDelta) is held to, each case run again from its own inputs. It reads the
 * file with rta_bands.c's helpers (rb_load, rbj_num, rbj_arr), which the suite includes first.
 */
#ifndef OMX_RTA_DELTA_VECTORS
#define OMX_RTA_DELTA_VECTORS "vectors/rta-delta.json"
#endif

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

/* The wedges after `"wedges": [` as [band, boost, strength] triples into `out` (room `max`); their count, or -1. */
static int rdj_wedges(const char *line, struct omx_rta_wedge *out, int max) {
  const char *p = strstr(line, "\"wedges\": [");
  if (!p) return -1;
  p += 11;
  int n = 0;
  while (*p == '[') {
    char *end;
    if (n >= max) return -1;
    const double band = strtod(p + 1, &end);
    if (*end != ',') return -1;
    const double boost = strtod(end + 1, &end);
    if (*end != ',') return -1;
    const double strength = strtod(end + 1, &end);
    if (*end != ']') return -1;
    out[n++] = (struct omx_rta_wedge){(uint32_t)band, (int)boost, strength};
    p = end + 1;
    while (*p == ',' || *p == ' ') p++;
  }
  return *p == ']' ? n : -1;
}

/* One line of the file run again from its own inputs; 1 when it was a case. A diff is null where either tap is
 * silent (rbj_arr reads null as -INFINITY). */
static int rd_file_case(const char *line, uint32_t *bad) {
  if (!strstr(line, "\"case\": \"")) return 0;
  static double pre_db[RB_MAX_BANDS], post_db[RB_MAX_BANDS], want[RB_MAX_BANDS], got[RB_MAX_BANDS];
  static struct omx_rta_band pre[RB_MAX_BANDS], post[RB_MAX_BANDS];
  static struct omx_rta_wedge want_w[RB_MAX_BANDS], got_w[RB_MAX_BANDS];
  const double nb = rbj_num(line, "n");
  const int np = rbj_arr(line, "preDb", pre_db, RB_MAX_BANDS), nq = rbj_arr(line, "postDb", post_db, RB_MAX_BANDS),
            nd = rbj_arr(line, "diff", want, RB_MAX_BANDS), nw = rdj_wedges(line, want_w, RB_MAX_BANDS);
  if (!(nb >= 1 && nb <= RB_MAX_BANDS) || np != (int)nb || nq != np || nd != np || nw < 0) {
    (*bad)++;
    return 1;
  }
  for (int i = 0; i < np; i++) pre[i] = (struct omx_rta_band){0.0, pre_db[i], pre_db[i]};
  for (int i = 0; i < np; i++) post[i] = (struct omx_rta_band){0.0, post_db[i], post_db[i]};
  const uint32_t k = omx_rta_band_delta(pre, post, (uint32_t)np, got, got_w);
  int wrong = k != (uint32_t)nw;
  for (int i = 0; i < np && !wrong; i++)
    wrong = isinf(want[i]) ? !isnan(got[i]) : !(fabs(got[i] - want[i]) <= 1e-9);
  for (uint32_t j = 0; j < k && !wrong; j++)
    wrong = got_w[j].band != want_w[j].band || got_w[j].boost != want_w[j].boost ||
            !(fabs(got_w[j].strength - want_w[j].strength) <= 1e-12);
  *bad += (uint32_t)wrong;
  return 1;
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
  ok(across == 0 && k == 2u && rd_w[0].band == 1u && rd_w[1].band == 4u,
     "no pair across a silent band draws; the pairs beside it do (bands 1 and 4)", k, 2);
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
  ok(omx_rta_band_delta(rd_pre, rd_post, 1u, rd_diff, rd_w) == 0u && rd_diff[0] == 0.0, "one band: its diff, no wedge",
     rd_diff[0], 0);
  ok(omx_rta_band_delta(rd_pre, rd_post, 0u, rd_diff, rd_w) == 0u, "no band: nothing", 1, 0);

  /* the file: every case run again from its own inputs, the same difference and wedges */
  char *s = rb_load(OMX_RTA_DELTA_VECTORS);
  ok(s != NULL, "the vector file " OMX_RTA_DELTA_VECTORS " reads", 0, 1);
  if (!s) return;
  ok(rbj_num(s, "deadbandDb") == dead && rbj_num(s, "fullScaleDb") == full, "the file's law is the declared one",
     rbj_num(s, "deadbandDb"), dead);
  uint32_t cases = 0, bad = 0;
  for (char *line = strtok(s, "\n"); line; line = strtok(NULL, "\n")) cases += (uint32_t)rd_file_case(line, &bad);
  free(s);
  ok(cases == RB_FILE_RATES_COUNT * 7u * 4u + 1u, "the file holds every pair at every file rate and band count, and the edges",
     cases, RB_FILE_RATES_COUNT * 7u * 4u + 1u);
  ok(bad == 0, "every case of the file is what omx_rta_band_delta answers", bad, 0);
}
