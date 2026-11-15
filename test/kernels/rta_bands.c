// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <omxdsp/analysis/omx_rta_bands.h>

#include "fixtures/rta_bands_scenarios.h"

/* ---- the band reduction (analysis/omx_rta_bands.h), against vectors/rta-bands.json, the file the TypeScript twin
 * (ts/src/rta.ts aggregateSpectrumBands) is held to. Each case of the file is run again from the same scenario
 * (fixtures/rta_bands_scenarios.h) and must answer the same bands. Beside the file, the reduction's own closed forms, so
 * a file written from a wrong kernel is red too: the tiling is log-even from RTA_BAND_F_MIN to the clamped cap with each
 * band at its geometric centre; the cap is the Nyquist where that is lower; the low tier's correction is the bin-density
 * ratio and reaches only the bands under the crossover; the loudest bin is a band's peak; a reading whose loudest band
 * is under the gate is silence everywhere. ---------------------------------------------------------------------------- */

#ifndef OMX_RTA_BANDS_VECTORS
#define OMX_RTA_BANDS_VECTORS "vectors/rta-bands.json"
#endif

static struct rb_case rb_c;
static struct omx_rta_band rb_out[RB_MAX_BANDS], rb_ref[RB_MAX_BANDS];

static char *rb_load(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) return NULL;
  fseek(f, 0, SEEK_END);
  const long len = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *s = len > 0 ? malloc((size_t)len + 1) : NULL;
  if (s && fread(s, 1, (size_t)len, f) != (size_t)len) {
    free(s);
    s = NULL;
  }
  if (s) s[len] = '\0';
  fclose(f);
  return s;
}

/** The number after `"key": ` on the line, or NAN. */
static double rbj_num(const char *line, const char *key) {
  char pat[64];
  snprintf(pat, sizeof pat, "\"%s\": ", key);
  const char *p = strstr(line, pat);
  return p ? strtod(p + strlen(pat), NULL) : NAN;
}

/** The array after `"key": [` into `out` (room `max`), `null` read as -INFINITY; its length, or -1. */
static int rbj_arr(const char *line, const char *key, double *out, int max) {
  char pat[64];
  snprintf(pat, sizeof pat, "\"%s\": [", key);
  const char *p = strstr(line, pat);
  if (!p) return -1;
  p += strlen(pat);
  int n = 0;
  while (*p && *p != ']') {
    if (n >= max) return -1;
    if (strncmp(p, "null", 4) == 0) {
      out[n++] = -INFINITY;
      p += 4;
    } else {
      char *end;
      const double x = strtod(p, &end);
      if (end == p) return -1;
      out[n++] = x;
      p = end;
    }
    while (*p == ',' || *p == ' ') p++;
  }
  return *p == ']' ? n : -1;
}

/** Two dB values agree: both silence, or both finite within `tol`. */
static int rb_same(double a, double b, double tol) {
  if (isinf(a) || isinf(b)) return isinf(a) && isinf(b) && a < 0 && b < 0;
  return fabs(a - b) <= tol;
}

/** One line of the file run again; 1 when it was a case. */
static int rb_file_case(const char *line, uint32_t *bad) {
  static char kind[16];
  const char *k = strstr(line, "\"kind\": \"");
  if (!k) return 0;
  k += 9;
  size_t len = 0;
  while (k[len] && k[len] != '"' && len + 1 < sizeof kind) len++;
  memcpy(kind, k, len);
  kind[len] = '\0';
  int ki = -1;
  for (int i = 0; i < RB_KINDS; i++)
    if (strcmp(kind, RB_KIND_NAMES[i]) == 0) ki = i;
  static double hz[RB_MAX_BANDS], db[RB_MAX_BANDS], pk[RB_MAX_BANDS];
  const double rate = rbj_num(line, "rate"), nb = rbj_num(line, "n");
  const int nh = rbj_arr(line, "hz", hz, RB_MAX_BANDS), nd = rbj_arr(line, "db", db, RB_MAX_BANDS),
            np = rbj_arr(line, "peak", pk, RB_MAX_BANDS);
  if (ki < 0 || !(rate > 0) || !(nb >= 1 && nb <= RB_MAX_BANDS) || nh != (int)nb || nd != nh || np != nh) {
    (*bad)++;
    return 1;
  }
  rb_build((enum rb_kind)ki, (float)rate, &rb_c);
  const uint32_t n = omx_rta_bands(&rb_c.h, rb_c.has_low ? &rb_c.l : NULL, (uint32_t)nb, rb_out);
  if (n != (uint32_t)nb) {
    (*bad)++;
    return 1;
  }
  for (uint32_t b = 0; b < n; b++)
    if (fabs(rb_out[b].hz - hz[b]) > 1e-6 * hz[b] || !rb_same(rb_out[b].db, db[b], 1e-6) ||
        !rb_same(rb_out[b].peak_db, pk[b], 1e-6)) {
      (*bad)++;
      break;
    }
  return 1;
}

static void arm_rta_bands(void) {
  static const float rates[OMX_ORACLE_FULL_RATES_COUNT] = OMX_ORACLE_FULL_RATES_INIT;

  ok(omx_rta_bands(&rb_c.h, NULL, 0u, rb_out) == 0u, "no band asked is no band answered", 1, 0);

  /* the cap and its Nyquist clamp */
  ok(omx_rta_band_f_max(48000.0) == (double)OMX_RTA_BAND_F_MAX_CAP, "the band cap at 48 kHz is the declared cap",
     omx_rta_band_f_max(48000.0), OMX_RTA_BAND_F_MAX_CAP);
  ok(omx_rta_band_f_max(32000.0) == 16000.0, "the band cap at 32 kHz is its Nyquist", omx_rta_band_f_max(32000.0), 16000);

  /* the tiling: log-even from the floor to the clamped cap, each band at its geometric centre, at every oracle rate */
  uint32_t untiled = 0, unsorted = 0;
  for (uint32_t ri = 0; ri < OMX_ORACLE_FULL_RATES_COUNT; ri++) {
    rb_build(RB_SLOPE, rates[ri], &rb_c);
    const uint32_t n = omx_rta_bands(&rb_c.h, NULL, 64u, rb_out);
    const double lo = log10((double)OMX_RTA_BAND_F_MIN), hi = log10(omx_rta_band_f_max((double)rates[ri]));
    for (uint32_t b = 0; b < n; b++) {
      const double f0 = pow(10.0, lo + (hi - lo) * b / 64.0), f1 = pow(10.0, lo + (hi - lo) * (b + 1) / 64.0);
      if (fabs(rb_out[b].hz - sqrt(f0 * f1)) > 1e-9 * rb_out[b].hz) untiled++;
      if (b && !(rb_out[b].hz > rb_out[b - 1].hz)) unsorted++;
    }
  }
  ok(untiled == 0, "each band sits at the geometric centre of its log-even edges, at every oracle rate", untiled, 0);
  ok(unsorted == 0, "the bands ascend", unsorted, 0);

  /* the tier correction is the bin-density ratio, at every oracle rate */
  uint32_t ungained = 0;
  for (uint32_t ri = 0; ri < OMX_ORACLE_FULL_RATES_COUNT; ri++) {
    rb_build(RB_TIERS, rates[ri], &rb_c);
    const double want = 10.0 * log10(((double)rb_c.l.fft_size / rb_c.l.rate) / ((double)rb_c.h.fft_size / rb_c.h.rate));
    if (fabs(omx_rta_low_gain_db(&rb_c.h, &rb_c.l) - want) > 1e-12) ungained++;
  }
  ok(ungained == 0, "the low tier's correction is 10 log10 of its bin-density ratio", ungained, 0);

  /* the low tier reaches only the bands under the crossover */
  rb_build(RB_TIERS, 48000.0f, &rb_c);
  omx_rta_bands(&rb_c.h, &rb_c.l, 640u, rb_out);
  omx_rta_bands(&rb_c.h, NULL, 640u, rb_ref);
  uint32_t above_moved = 0, below_moved = 0, below = 0;
  for (uint32_t b = 0; b < 640u; b++) {
    const int same = rb_same(rb_out[b].db, rb_ref[b].db, 0.0) && rb_same(rb_out[b].peak_db, rb_ref[b].peak_db, 0.0);
    if (rb_out[b].hz < (double)OMX_RTA_TIER_CROSSOVER_HZ) {
      below++;
      below_moved += !same;
    } else {
      above_moved += !same;
    }
  }
  ok(above_moved == 0, "a band at or above the crossover reads the high tier alone", above_moved, 0);
  ok(below > 0 && below_moved == below, "every band under the crossover reads the low tier", below_moved, below);

  /* the loudest bin is a band's peak: the 1 kHz tone at full scale peaks at 0 dBFS in the band that holds it */
  rb_build(RB_TONES, 48000.0f, &rb_c);
  omx_rta_bands(&rb_c.h, NULL, 640u, rb_out);
  double top = -INFINITY, top_hz = 0.0;
  for (uint32_t b = 0; b < 640u; b++)
    if (rb_out[b].peak_db > top) top = rb_out[b].peak_db, top_hz = rb_out[b].hz;
  ok(fabs(top) < 1e-9, "a full-scale bin peaks at 0 dBFS", top, 0);
  ok(top_hz > 980.0 && top_hz < 1020.0, "the full-scale peak sits in the band of the 1 kHz tone", top_hz, 1000);

  /* the gate: a reading whose loudest band is under it is silence everywhere; zeros are silence */
  uint32_t ungated = 0;
  for (int kind = RB_QUIET; kind <= RB_SILENCE; kind++) {
    rb_build((enum rb_kind)kind, 48000.0f, &rb_c);
    omx_rta_bands(&rb_c.h, NULL, 64u, rb_out);
    for (uint32_t b = 0; b < 64u; b++) ungated += !(isinf(rb_out[b].db) && isinf(rb_out[b].peak_db));
  }
  ok(ungated == 0, "a reading under the gate, and silence, read as silence in every band", ungated, 0);
  rb_build(RB_SLOPE, 48000.0f, &rb_c);
  omx_rta_bands(&rb_c.h, NULL, 64u, rb_out);
  uint32_t gated = 0;
  for (uint32_t b = 0; b < 64u; b++) gated += isinf(rb_out[b].db);
  ok(gated == 0, "a reading over the gate draws every band", gated, 0);

  /* the file: every case run again, the same bands */
  char *s = rb_load(OMX_RTA_BANDS_VECTORS);
  ok(s != NULL, "the vector file " OMX_RTA_BANDS_VECTORS " reads", 0, 1);
  if (!s) return;
  ok(rbj_num(s, "fMin") == OMX_RTA_BAND_F_MIN && rbj_num(s, "fMaxCap") == OMX_RTA_BAND_F_MAX_CAP &&
         rbj_num(s, "crossoverHz") == OMX_RTA_TIER_CROSSOVER_HZ && rbj_num(s, "gateDb") == OMX_RTA_BAND_GATE_DB,
     "the file's law is the declared one", rbj_num(s, "gateDb"), OMX_RTA_BAND_GATE_DB);
  uint32_t cases = 0, bad = 0;
  for (char *line = strtok(s, "\n"); line; line = strtok(NULL, "\n")) cases += (uint32_t)rb_file_case(line, &bad);
  free(s);
  ok(cases == RB_FILE_RATES_COUNT * RB_KINDS * RB_BAND_COUNTS_COUNT,
     "the file holds every scenario at every file rate and band count", cases,
     RB_FILE_RATES_COUNT * RB_KINDS * RB_BAND_COUNTS_COUNT);
  ok(bad == 0, "every case of the file is what omx_rta_bands answers", bad, 0);
}
