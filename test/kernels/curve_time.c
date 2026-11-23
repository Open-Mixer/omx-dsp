// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>

/* ---- the time_response (omx_curve.h, families `delay` and `reverb`): what the kernels do on an impulse ----------
 *
 * The picture omx-panel renders (its time_response: stems from the floor to a level at a time, an envelope polyline,
 * one axis in ms from 0 and dB with the floor at CURVE_EXTENTS timeFloorDb) is answered here, and the oracle is the
 * kernel itself. The DELAY: an impulse through omx_delay_instance at 48 kHz, and every stem the twin answers must be
 * the kernel's own output at that sample, in dB, on the leg the stem names — the dry tap, each echo through the ring,
 * ping-pong on a left-only impulse (the kernel cross-feeds the legs, so the echoes alternate), the tone's shade the
 * kernel's own pole. The REVERB: an impulse through omx_reverb_instance, its tail's RMS envelope in dB fitted over the
 * middle of its RT60, and the slope must be the twin's closed form (REVERB_DECAY_LOOPS, the longest loop's gain and
 * length) within the bound a sum of eight combs leaves; the onset must be the predelay's. The sheet the twin draws
 * from is held to the kernel's own tunings, so the picture and the tail cannot drift apart. */

#include <omxdsp/fx/omx_delay_instance.h>
#include <omxdsp/fx/omx_reverb_instance.h>
#include <omxdsp/omx_curve.h>

#define CTM_RATE 48000.0
#define CTM_TERM(role, value, unit) \
  { OMX_CURVE_ROLES_##role, 0, (value), OMX_CURVE_UNITS_##unit, OMX_CURVE_NO_SHAPE, OMX_CURVE_ORIGINS_USER }

static uint32_t ctm_ask(enum omx_curve_families family, const struct omx_curve_term *t, uint32_t nt, double tempo,
                        uint32_t points, double *x, double *y, struct omx_curve_stem *stems, uint32_t *nstems, int *code) {
  struct omx_curve_request rq = {OMX_CURVE_DISPLAYS_TIME_RESPONSE, family, CTM_RATE, points, 0, 0.0, 0.0, t, nt, 0, tempo};
  struct omx_curve_extra ex = {NULL, NULL, 0u, 0u, stems, OMX_CURVE_TAPS_MAX, 0u};
  *code = -1;
  const uint32_t n = omx_curve_points_marked(&rq, x, y, OMX_CURVE_POINTS_MAX, stems ? &ex : NULL, code);
  *nstems = ex.nstems;
  return n;
}

/* The delay kernel on an impulse (`left` 1 on the left leg only, else on both), one tap, no colour, no duck. */
static void ctm_delay_kernel(float time_ms, float feedback, float tone, float mix, int pingpong, int left, float *l,
                             float *r, uint32_t n) {
  static OmxDelayInstance s;
  memset(&s, 0, sizeof s);
  omx_delay_instance_init(&s, (float)CTM_RATE);
  omx_delay_instance_resolve(&s, 0, time_ms, feedback, tone, mix, pingpong, (float)OMX_FX_DELAY_TAPS_RANGE_DEFAULT, 1.0f,
                             0.0f, (int)OMX_DELAY_TAP1_FACTORS_DEFAULT, 0.0f, 0.0f, (int)OMX_DELAY_TAP2_FACTORS_DEFAULT,
                             0.0f, 0.0f, (int)OMX_DELAY_TAP3_FACTORS_DEFAULT, 0.0f, 0.0f, 0, (int)OMX_DELAY_DIVISIONS_DEFAULT,
                             (int)OMX_DELAY_MODES_DEFAULT, (float)OMX_DELAY_DUCK_DB_RANGE_DEFAULT,
                             (float)OMX_DELAY_DUCK_THRESHOLD_DB_RANGE_DEFAULT, (float)OMX_DELAY_DUCK_RELEASE_MS_RANGE_DEFAULT);
  memset(l, 0, n * sizeof *l);
  memset(r, 0, n * sizeof *r);
  l[0] = 1.0f;
  r[0] = left ? 0.0f : 1.0f;
  for (uint32_t off = 0; off < n; off += 256u) {
    const uint32_t k = n - off < 256u ? n - off : 256u;
    omx_delay_instance_run(&s, l + off, r + off, l + off, r + off, k);
  }
}

static double ctm_db(double lin) { return lin > 0.0 ? 20.0 * log10(lin) : -INFINITY; }

/* The reverb kernel on an impulse: `n` samples of its wet tail (mix 1), no early taps, no duck, no de-ess. */
static void ctm_reverb_kernel(int algorithm, float size, float predelay_ms, float *l, float *r, uint32_t n) {
  static OmxReverbInstance s;
  memset(&s, 0, sizeof s);
  omx_reverb_instance_init(&s, (float)CTM_RATE);
  omx_reverb_instance_resolve(&s, 0, (float)OMX_REVERB_PLATE_MOD_DEPTH_RANGE_DEFAULT, 1.0f, size, 0.0f, 1.0f, predelay_ms,
                              0.0f, (float)OMX_REVERB_HIGHCUT_RANGE_MAX, OMX_REVERB_INSTANCE_REVERSE_DEFAULT,
                              OMX_REVERB_INSTANCE_HOLD_DEFAULT, OMX_REVERB_INSTANCE_RELEASE_DEFAULT,
                              OMX_REVERB_INSTANCE_GATE_THRESHOLD_DEFAULT, algorithm, (float)OMX_REVERB_EARLY_DB_RANGE_MIN,
                              (float)OMX_REVERB_TAIL_DB_RANGE_DEFAULT, (float)OMX_REVERB_EARLY_SIZE_PCT_RANGE_DEFAULT,
                              (float)OMX_REVERB_DUCK_DB_RANGE_DEFAULT, (float)OMX_REVERB_DUCK_THRESHOLD_DB_RANGE_DEFAULT,
                              (float)OMX_REVERB_DUCK_RELEASE_MS_RANGE_DEFAULT, 0, (float)OMX_REVERB_DEESS_FREQ_HZ_RANGE_DEFAULT,
                              (float)OMX_REVERB_DEESS_THRESHOLD_DB_RANGE_DEFAULT);
  memset(l, 0, n * sizeof *l);
  memset(r, 0, n * sizeof *r);
  l[0] = r[0] = 1.0f;
  for (uint32_t off = 0; off < n; off += 256u) {
    const uint32_t k = n - off < 256u ? n - off : 256u;
    omx_reverb_instance_run(&s, l + off, r + off, l + off, r + off, k);
  }
}

/* The RMS of a tail in dB over windows of `w` samples, from sample `from`: the envelope the eye reads. */
static uint32_t ctm_envelope(const float *l, const float *r, uint32_t n, uint32_t from, uint32_t w, double *env_db,
                             uint32_t room) {
  uint32_t k = 0;
  for (uint32_t at = from; at + w <= n && k < room; at += w, k++) {
    double e = 0.0;
    for (uint32_t i = 0; i < w; i++) e += 0.5 * ((double)l[at + i] * l[at + i] + (double)r[at + i] * r[at + i]);
    env_db[k] = ctm_db(sqrt(e / (double)w));
  }
  return k;
}

/* The slope of a line through (i, env[i]) for i in [a, b), dB per window. */
static double ctm_slope(const double *env, uint32_t a, uint32_t b) {
  double sx = 0, sy = 0, sxx = 0, sxy = 0;
  const double n = (double)(b - a);
  for (uint32_t i = a; i < b; i++) sx += i, sy += env[i], sxx += (double)i * i, sxy += (double)i * env[i];
  return (n * sxy - sx * sy) / (n * sxx - sx * sx);
}

static void arm_curve_time(void) {
  g_arm = "curve-time";
  static double x[OMX_CURVE_POINTS_MAX], y[OMX_CURVE_POINTS_MAX], env[512];
  static float l[1 << 20], r[1 << 20];
  struct omx_curve_stem st[OMX_CURVE_TAPS_MAX];
  uint32_t ns = 0;
  int code;

  /* The leg words read back; the time words are in the tables the curve arm reads. */
  for (uint32_t i = 0; i < OMX_CURVE_LEGS_COUNT; i++)
    ok(omx_curve_word(OMX_CURVE_LEG_WORDS, OMX_CURVE_LEGS_COUNT, OMX_CURVE_LEG_WORDS[i]) == (int)i, "a leg word reads back", i, i);
  ok(omx_curve_word(OMX_CURVE_DISPLAY_WORDS, OMX_CURVE_DISPLAYS_COUNT, "time_response") == OMX_CURVE_DISPLAYS_TIME_RESPONSE &&
     omx_curve_word(OMX_CURVE_FAMILY_WORDS, OMX_CURVE_FAMILIES_COUNT, "reverb") == OMX_CURVE_FAMILIES_REVERB &&
     omx_curve_word(OMX_CURVE_ROLE_WORDS, OMX_CURVE_ROLES_COUNT, "low_cut") == OMX_CURVE_ROLES_LOW_CUT &&
     omx_curve_word(OMX_CURVE_UNIT_WORDS, OMX_CURVE_UNITS_COUNT, "ms") == OMX_CURVE_UNITS_MS, "the time words are the panel's", 0, 0);

  /* THE SHEET IS THE KERNEL'S: the loop the twin draws from is the longest line the kernel lays out. */
  {
    const uint32_t room_c[] = {OMX_REVERB_ROOM_COMB_TUNINGS}, hall_c[] = {OMX_REVERB_HALL_COMB_TUNINGS},
                   plate_t[] = {OMX_REVERB_PLATE_TANK_TUNINGS};
    uint32_t rm = 0, hm = 0;
    for (uint32_t i = 0; i < OMX_FV_NCOMB; i++) rm = room_c[i] > rm ? room_c[i] : rm, hm = hall_c[i] > hm ? hall_c[i] : hm;
    ok(rm == OMX_REVERB_DECAY_LOOPS_ROOM_LOOP_SAMPLES, "the room's loop is its longest comb", rm, OMX_REVERB_DECAY_LOOPS_ROOM_LOOP_SAMPLES);
    ok(hm == OMX_REVERB_DECAY_LOOPS_HALL_LOOP_SAMPLES, "the hall's loop is its longest comb", hm, OMX_REVERB_DECAY_LOOPS_HALL_LOOP_SAMPLES);
    ok(plate_t[2] + plate_t[3] == OMX_REVERB_DECAY_LOOPS_PLATE_LOOP_SAMPLES, "the plate's loop is its longer tank half",
       plate_t[2] + plate_t[3], OMX_REVERB_DECAY_LOOPS_PLATE_LOOP_SAMPLES);
    ok(OMX_REVERB_PLATE_DECAY_FLOOR == OMX_REVERB_DECAY_LOOPS_PLATE_GAIN_FLOOR, "the plate's decay floor is the sheet's", 0, 0);
  }

  /* 1. THE DELAY IS THE KERNEL ON AN IMPULSE: 120 ms, feedback 0.5, mix 0.5, no tone, both legs. */
  {
    const struct omx_curve_term t[] = {CTM_TERM(TIME, 120.0, MS), CTM_TERM(FEEDBACK, 0.5, NONE), CTM_TERM(MIX, 50.0, PERCENT),
                                       CTM_TERM(TONE, 0.0, NONE), CTM_TERM(PINGPONG, 0.0, NONE)};
    const uint32_t n = ctm_ask(OMX_CURVE_FAMILIES_DELAY, t, 5u, 0.0, 16u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_OK && n == 0u, "a delay answers no point", code, n);
    ok(ns == 10u, "the dry tap and nine echoes before the floor (0.5 x 0.5^8 = -54 dB, the tenth under -60)", ns, 10u);
    const uint32_t d = omx_fxdelay_ms_to_samples(120.0f, (float)CTM_RATE), len = d * 12u + 8u;
    ctm_delay_kernel(120.0f, 0.5f, 0.0f, 0.5f, 0, 0, l, r, len);
    double worst = 0.0;
    for (uint32_t k = 0; k < ns; k++) {
      const uint32_t at = (uint32_t)lrint(st[k].t_ms * CTM_RATE / 1000.0);
      ok(at == k * d, "a stem falls on the kernel's own tap sample", at, k * d);
      const double gap = fabs(st[k].level_db - ctm_db(l[at]));
      worst = gap > worst ? gap : worst;
      ok(gap <= 1e-4, "a stem's level is the kernel's output there, in dB", st[k].level_db, ctm_db(l[at]));
      ok(st[k].leg == OMX_CURVE_LEGS_BOTH && st[k].shade == 1.0, "no tone, no ping-pong: a bright stem on both legs", st[k].leg, 0);
      /* and nothing between taps */
      if (k + 1u < ns) ok(fabs(l[at + d / 2u]) < 1e-9, "nothing stands between taps", fabs(l[at + d / 2u]), 1e-9);
    }
    ok(st[0].t_ms == 0.0 && st[1].t_ms == 120.0 && st[9].t_ms == 1080.0, "the taps sit at n x time", st[9].t_ms, 1080.0);
    printf("  curve-time: delay stems vs the kernel, worst gap %.3g dB over %u taps\n", worst, ns);
  }

  /* 2. PING-PONG on a left-only impulse: the echoes alternate, each on its own leg at the train's level. */
  {
    const struct omx_curve_term t[] = {CTM_TERM(TIME, 0.1, S), CTM_TERM(FEEDBACK, 60.0, PERCENT), CTM_TERM(MIX, 1.0, COEF),
                                       CTM_TERM(PINGPONG, 1.0, NONE)};
    ctm_ask(OMX_CURVE_FAMILIES_DELAY, t, 4u, 0.0, 16u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_OK && ns >= 6u, "a ping-pong train at mix 1 (no dry tap: 1 - mix is silence)", code, ns);
    ok(st[0].t_ms == 100.0 && st[0].leg == OMX_CURVE_LEGS_LEFT && st[1].leg == OMX_CURVE_LEGS_RIGHT && st[2].leg == OMX_CURVE_LEGS_LEFT,
       "the first echo lands left, then right, then left", st[0].leg, OMX_CURVE_LEGS_LEFT);
    const uint32_t d = omx_fxdelay_ms_to_samples(100.0f, (float)CTM_RATE), len = d * (ns + 1u) + 8u;
    ctm_delay_kernel(100.0f, 0.6f, 0.0f, 1.0f, 1, 1, l, r, len);
    for (uint32_t k = 0; k < ns; k++) {
      const uint32_t at = (uint32_t)lrint(st[k].t_ms * CTM_RATE / 1000.0);
      const float on = st[k].leg == OMX_CURVE_LEGS_LEFT ? l[at] : r[at], off = st[k].leg == OMX_CURVE_LEGS_LEFT ? r[at] : l[at];
      ok(fabs(st[k].level_db - ctm_db(on)) <= 1e-4, "a ping-pong stem is the kernel's level on its leg", st[k].level_db, ctm_db(on));
      ok(fabs(off) < 1e-9, "and silence on the other leg", fabs(off), 1e-9);
    }
  }

  /* 3. THE TONE IS THE SHADE, the kernel's own pole at this rate; the levels do not move. */
  {
    const struct omx_curve_term t[] = {CTM_TERM(TIME, 120.0, MS), CTM_TERM(FEEDBACK, 0.5, NONE), CTM_TERM(MIX, 0.5, NONE),
                                       CTM_TERM(TONE, 0.6, NONE)};
    ctm_ask(OMX_CURVE_FAMILIES_DELAY, t, 4u, 0.0, 16u, x, y, st, &ns, &code);
    const double p = (double)omx_fxdelay_tone_pole(0.6f, (float)CTM_RATE), bright = (1.0 - p) / (1.0 + p);
    ok(code == OMX_CURVE_OK && ns == 10u, "the tone changes no level and ends no tap", ns, 10u);
    ok(st[0].shade == 1.0 && st[1].shade == 1.0 && fabs(st[2].shade - bright) < 1e-12 && fabs(st[4].shade - bright * bright * bright) < 1e-12,
       "echo n carries the one-pole's Nyquist gain raised to n - 1", st[2].shade, bright);
    ok(st[1].level_db == ctm_db(0.5) && st[2].level_db == ctm_db(0.25), "the broadband levels are the tone's business not at all", st[2].level_db, ctm_db(0.25));
  }

  /* 4. A SYNCED TIME is the division's beats at the request's tempo, never a guess at either. */
  {
    const struct omx_curve_term t[] = {CTM_TERM(TIME, 300.0, MS), CTM_TERM(FEEDBACK, 0.3, NONE), CTM_TERM(SYNC, 1.0, NONE),
                                       CTM_TERM(DIVISION, (double)OMX_DELAY_DIVISIONS_QUARTER, NONE)};
    ctm_ask(OMX_CURVE_FAMILIES_DELAY, t, 4u, 0.0, 16u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_TERM_MISSING, "synced with no tempo in the request: termMissing", code, OMX_CURVE_TERM_MISSING);
    ctm_ask(OMX_CURVE_FAMILIES_DELAY, t, 4u, 120.0, 16u, x, y, st, &ns, &code);
    /* no mix term: mix 1, so the dry tap is silent and the first stem is the first echo */
    ok(code == OMX_CURVE_OK && ns >= 2u && st[0].t_ms == (double)omx_bpm_division_ms(120.0f, 1.0f) && st[1].t_ms == 1000.0,
       "a quarter at 120 BPM is 500 ms, the kernel's own derivation", st[0].t_ms, 500.0);
    ctm_ask(OMX_CURVE_FAMILIES_DELAY, t, 3u, 120.0, 16u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_TERM_MISSING, "synced with no division: termMissing", code, OMX_CURVE_TERM_MISSING);
    const struct omx_curve_term bad[] = {CTM_TERM(FEEDBACK, 0.3, NONE), CTM_TERM(SYNC, 1.0, NONE), CTM_TERM(DIVISION, 99.0, NONE)};
    ctm_ask(OMX_CURVE_FAMILIES_DELAY, bad, 3u, 120.0, 16u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_KIND_UNKNOWN, "a division past the table: kindUnknown", code, OMX_CURVE_KIND_UNKNOWN);
  }

  /* 5. THE DRY TAP ALONE at mix 0, the floor and the cap, the room. */
  {
    const struct omx_curve_term t[] = {CTM_TERM(TIME, 120.0, MS), CTM_TERM(FEEDBACK, 0.999, NONE), CTM_TERM(MIX, 0.0, NONE)};
    ctm_ask(OMX_CURVE_FAMILIES_DELAY, t, 3u, 0.0, 16u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_OK && ns == 1u && st[0].t_ms == 0.0 && st[0].level_db == 0.0, "mix 0: the dry tap at 0 dB and nothing else", ns, 1u);
    const struct omx_curve_term run[] = {CTM_TERM(TIME, 10.0, MS), CTM_TERM(FEEDBACK, 0.999, NONE), CTM_TERM(MIX, 1.0, NONE)};
    ctm_ask(OMX_CURVE_FAMILIES_DELAY, run, 3u, 0.0, 16u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_OK && ns == OMX_CURVE_TAPS_MAX, "a runaway feedback stops at CURVE_TAPS_MAX", ns, OMX_CURVE_TAPS_MAX);
    const struct omx_curve_term none[] = {CTM_TERM(TIME, 10.0, MS), CTM_TERM(MIX, 1.0, NONE)};
    ctm_ask(OMX_CURVE_FAMILIES_DELAY, none, 2u, 0.0, 16u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_TERM_MISSING, "no feedback: termMissing", code, OMX_CURVE_TERM_MISSING);
    ctm_ask(OMX_CURVE_FAMILIES_DELAY, t, 3u, 0.0, 16u, x, y, NULL, &ns, &code);
    ok(code == OMX_CURVE_ROOM, "a delay with no room for its stems: room", code, OMX_CURVE_ROOM);
    const struct omx_curve_term db[] = {CTM_TERM(TIME, 120.0, MS), CTM_TERM(FEEDBACK, 0.5, D_B)};
    ctm_ask(OMX_CURVE_FAMILIES_DELAY, db, 2u, 0.0, 16u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_UNIT_REFUSED, "a feedback in dB is no share: unitRefused", code, OMX_CURVE_UNIT_REFUSED);
    struct omx_curve_request wrong = {OMX_CURVE_DISPLAYS_TRANSFER_CURVE, OMX_CURVE_FAMILIES_DELAY, CTM_RATE, 16u, 0, 0.0, 0.0, t, 3u, 0, 0.0};
    ok(omx_curve_points(&wrong, x, y, OMX_CURVE_POINTS_MAX, &code) == 0u && code == OMX_CURVE_KIND_UNKNOWN,
       "a delay has no transfer curve: kindUnknown", code, OMX_CURVE_KIND_UNKNOWN);
    wrong.kind = OMX_CURVE_DISPLAYS_TIME_RESPONSE, wrong.family = OMX_CURVE_FAMILIES_EQ;
    ok(omx_curve_points(&wrong, x, y, OMX_CURVE_POINTS_MAX, &code) == 0u && code == OMX_CURVE_KIND_UNKNOWN,
       "an eq has no time response: kindUnknown", code, OMX_CURVE_KIND_UNKNOWN);
  }

  /* 6. THE REVERB'S ENVELOPE IS THE KERNEL'S TAIL: room, hall and plate at size 0.5, the slope over the middle of
   *    the RT60 within a quarter of the closed form (eight combs of unequal length share the start of the tail; the
   *    longest sets the end). The onset is the predelay. */
  {
    const int algos[] = {OMX_REVERB_ROOM, OMX_REVERB_HALL, OMX_REVERB_PLATE};
    const char *names[] = {"room", "hall", "plate"};
    for (uint32_t a = 0; a < 3u; a++) {
      const struct omx_curve_term t[] = {CTM_TERM(SIZE, 0.5, NONE), CTM_TERM(ALGORITHM, (double)algos[a], NONE), CTM_TERM(MIX, 1.0, NONE)};
      const uint32_t n = ctm_ask(OMX_CURVE_FAMILIES_REVERB, t, 3u, 0.0, 64u, x, y, st, &ns, &code);
      ok(code == OMX_CURVE_OK && n == 64u && ns == 0u, "a reverb answers its grid and no stem", code, n);
      ok(x[0] == 0.0, "the axis starts at 0 ms", x[0], 0.0);
      ok(y[0] == 0.0, "at mix 1 the envelope starts at 0 dB", y[0], 0.0);
      ok(y[n - 1u] == OMX_CURVE_EXTENTS_TIME_FLOOR_DB, "and ends on the floor at the RT60", y[n - 1u], -60.0);
      double gain = 0.0, loop_ms = 1.0;
      ok(omx_curve_reverb_loop(algos[a], 0.5, &gain, &loop_ms), "the configuration has a loop", algos[a], 1);
      const double rt60 = loop_ms * log(0.001) / log(gain);
      ok(fabs(x[n - 1u] - rt60) < 1e-9, "the envelope ends at loop x ln(0.001) / ln(gain)", x[n - 1u], rt60);
      const double twin_slope = -60.0 / rt60; /* dB per ms */
      const uint32_t total = (uint32_t)(rt60 * CTM_RATE / 1000.0) + 4096u;
      ok(total < (1u << 20), "the tail fits the probe", total, 1u << 20);
      ctm_reverb_kernel(algos[a], 0.5f, 0.0f, l, r, total);
      const uint32_t w = (uint32_t)(loop_ms * CTM_RATE / 1000.0);
      const uint32_t nw = ctm_envelope(l, r, total, 0u, w, env, 512u);
      const uint32_t from = nw / 5u, to = nw * 4u / 5u;
      const double k_slope = ctm_slope(env, from, to) / loop_ms; /* dB per window -> dB per ms */
      const double share = k_slope / twin_slope;
      ok(share > 0.75 && share < 1.25, "the kernel's tail falls at the closed form's slope", share, 1.0);
      printf("  curve-time: %s size 0.5: twin RT60 %.0f ms (loop %.1f ms, gain %.3f), kernel slope %.2f of it over %u windows\n",
             names[a], rt60, loop_ms, gain, share, to - from);
    }
    /* the onset: a 100 ms predelay is a gap, and the kernel is silent until then */
    const struct omx_curve_term t[] = {CTM_TERM(SIZE, 0.5, NONE), CTM_TERM(PREDELAY, 100.0, MS), CTM_TERM(MIX, 0.5, NONE)};
    const uint32_t n = ctm_ask(OMX_CURVE_FAMILIES_REVERB, t, 3u, 0.0, 32u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_OK && n == 34u, "the onset adds its two anchors to the grid", n, 34u);
    uint32_t at = 0;
    while (at < n && x[at] < 100.0) at++;
    ok(at > 0 && x[at] == 100.0 && y[at] == OMX_CURVE_EXTENTS_TIME_FLOOR_DB && x[at + 1u] == 100.0 && fabs(y[at + 1u] - ctm_db(0.5)) < 1e-12,
       "the gap: the floor up to the predelay, the mix's level from it", y[at + 1u], ctm_db(0.5));
    for (uint32_t i = 0; i < at; i++) ok(y[i] == OMX_CURVE_EXTENTS_TIME_FLOOR_DB, "nothing before the predelay", y[i], -60.0);
    const uint32_t pre = (uint32_t)(0.1 * CTM_RATE);
    ctm_reverb_kernel(OMX_REVERB_ROOM, 0.5f, 100.0f, l, r, pre + 4800u);
    double before = 0.0, after = 0.0;
    for (uint32_t i = 0; i < pre; i++) before += fabs(l[i]);
    for (uint32_t i = pre; i < pre + 4800u; i++) after += fabs(l[i]);
    ok(before == 0.0 && after > 0.0, "the kernel is silent before the predelay and speaks from it", before, after);
  }

  /* 7. A DECAY SAID OUTRIGHT, the mix's level, mix 0, and the absences by name. */
  {
    const struct omx_curve_term t[] = {CTM_TERM(DECAY, 2.0, S), CTM_TERM(MIX, 25.0, PERCENT)};
    const uint32_t n = ctm_ask(OMX_CURVE_FAMILIES_REVERB, t, 2u, 0.0, 5u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_OK && n == 5u && x[4] == 2000.0 && fabs(y[0] - ctm_db(0.25)) < 1e-12 && fabs(y[2] - (ctm_db(0.25) - 30.0)) < 1e-9,
       "a decay of 2 s: -12 dB at 0 (mix 0.25), 30 dB further at 1 s, the floor at 2 s", y[2], ctm_db(0.25) - 30.0);
    const struct omx_curve_term silent[] = {CTM_TERM(DECAY, 2.0, S), CTM_TERM(MIX, 0.0, NONE)};
    ok(ctm_ask(OMX_CURVE_FAMILIES_REVERB, silent, 2u, 0.0, 5u, x, y, st, &ns, &code) == 0u && code == OMX_CURVE_OK,
       "mix 0: no point at all, the kernel's bit-identical dry", code, OMX_CURVE_OK);
    const struct omx_curve_term nomix[] = {CTM_TERM(SIZE, 0.5, NONE)};
    ctm_ask(OMX_CURVE_FAMILIES_REVERB, nomix, 1u, 0.0, 5u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_TERM_MISSING, "no mix: termMissing", code, OMX_CURVE_TERM_MISSING);
    const struct omx_curve_term nosize[] = {CTM_TERM(MIX, 1.0, NONE), CTM_TERM(PREDELAY, 10.0, MS)};
    ctm_ask(OMX_CURVE_FAMILIES_REVERB, nosize, 2u, 0.0, 5u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_TERM_MISSING, "neither size nor decay: termMissing", code, OMX_CURVE_TERM_MISSING);
    const struct omx_curve_term rev[] = {CTM_TERM(SIZE, 0.5, NONE), CTM_TERM(ALGORITHM, (double)OMX_REVERB_REVERSE, NONE), CTM_TERM(MIX, 1.0, NONE)};
    ctm_ask(OMX_CURVE_FAMILIES_REVERB, rev, 3u, 0.0, 5u, x, y, st, &ns, &code);
    ok(code == OMX_CURVE_KIND_UNKNOWN, "a reverse reverb has no loop here: kindUnknown, a named absence", code, OMX_CURVE_KIND_UNKNOWN);
    /* the extents, when given, bound the grid: a 0..500 ms window over a 2 s decay */
    struct omx_curve_request rq = {OMX_CURVE_DISPLAYS_TIME_RESPONSE, OMX_CURVE_FAMILIES_REVERB, CTM_RATE, 3u, 1, 0.0, 500.0, t, 2u, 0, 0.0};
    ok(omx_curve_points(&rq, x, y, OMX_CURVE_POINTS_MAX, &code) == 3u && x[2] == 500.0 && fabs(y[2] - (ctm_db(0.25) - 15.0)) < 1e-9,
       "the extents bound the grid, the slope unchanged", y[2], ctm_db(0.25) - 15.0);
    rq.x_min = -1.0;
    ok(omx_curve_points(&rq, x, y, OMX_CURVE_POINTS_MAX, &code) == 0u && code == OMX_CURVE_POINTS_RANGE, "a time axis starts at 0 or later", code, OMX_CURVE_POINTS_RANGE);
  }
}
