/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * hrp_place.test.c — the HRP placement and stepping in C (omx_hrp_place.h) held to the shared vector file,
 * test/analysis/fixtures/hrp_place.vectors, which is its one source and which a consumer's binding is held to. Every value is judged exactly; test/analysis/hrp-place-sabotage.sh breaks
 * each rule once in a copy of src/omx_hrp_place.c and requires this program to fail on that rule's line.
 *
 *   build/analysis_hrp_place [<vectors>]
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <omxdsp/analysis/omx_hrp_place.h>

#define VECTORS "test/analysis/fixtures/hrp_place.vectors"
#define MAXW 96

static int checks, failures, lines;

static double num(const char *w) {
  if (strcmp(w, "nan") == 0) return NAN;
  if (strcmp(w, "inf") == 0) return INFINITY;
  if (strcmp(w, "-inf") == 0) return -INFINITY;
  return strtod(w, NULL);
}
/* Exact; a NaN is matched only by a NaN. */
static int same(double a, double b) { return isnan(b) ? isnan(a) : a == b; }

static void check(int ok, const char *name, const char *line, const char *got) {
  checks++;
  if (ok) return;
  failures++;
  printf("FAIL %s: %s: read %s\n", name, line, got);
}

int main(int argc, char **argv) {
  const char *path = argc > 1 ? argv[1] : VECTORS;
  FILE *f = fopen(path, "r");
  if (!f) {
    printf("FAIL hrp_place: no vector file at %s\n", path);
    return 1;
  }
  char raw[4096], line[4096];
  while (fgets(raw, sizeof(raw), f)) {
    char *hash = strchr(raw, '#');
    if (hash) *hash = '\0';
    raw[strcspn(raw, "\r\n")] = '\0';
    strcpy(line, raw);
    char *w[MAXW];
    int n = 0;
    for (char *t = strtok(raw, " \t"); t && n < MAXW; t = strtok(NULL, " \t")) w[n++] = t;
    if (n == 0) continue;
    char got[2048] = "";
    if (strcmp(w[0], "step") == 0 && n == 5) {
      lines++;
      const double r = omx_hrp_step_toward(num(w[2]), num(w[3]));
      snprintf(got, sizeof(got), "%.17g", r);
      check(same(r, num(w[4])), w[1], line, got);
    } else if (strcmp(w[0], "place") == 0 && n >= 9 && strcmp(w[7], "g") == 0) {
      lines++;
      int arrow = -1;
      for (int i = 8; i < n; i++)
        if (strcmp(w[i], "=>") == 0) { arrow = i; break; }
      if (arrow < 0 || (n - arrow - 1) % 4 != 0) {
        check(0, w[1], line, "a malformed place line");
        continue;
      }
      double gains[MAXW];
      const uint32_t ng = (uint32_t)(arrow - 8);
      for (uint32_t k = 0; k < ng; k++) gains[k] = num(w[8 + k]);
      const OmxHrpPlaceLimits lim = {num(w[4]), num(w[5]), num(w[6])};
      OmxHrpPlacedBand out[MAXW];
      const uint32_t m = omx_hrp_place_bands(num(w[2]), gains, ng, num(w[3]), &lim, out);
      const uint32_t want = (uint32_t)((n - arrow - 1) / 4);
      int ok = m == want;
      size_t at = (size_t)snprintf(got, sizeof(got), "%u band(s):", m);
      for (uint32_t b = 0; b < m && at < sizeof(got); b++)
        at += (size_t)snprintf(got + at, sizeof(got) - at, " %u %.17g %.17g %.17g", out[b].harmonic, out[b].freq_hz,
                               out[b].gain_db, out[b].q);
      for (uint32_t b = 0; ok && b < want; b++) {
        char **e = &w[arrow + 1 + 4 * b];
        ok = out[b].harmonic == (uint32_t)strtoul(e[0], NULL, 10) && same(out[b].freq_hz, num(e[1])) &&
             same(out[b].gain_db, num(e[2])) && same(out[b].q, num(e[3]));
      }
      check(ok, w[1], line, got);
    } else {
      check(0, "hrp_place", line, "a line this test does not read");
    }
  }
  fclose(f);
  /* The range predicate on its own: the two bounds kept, one ulp outside each dropped, NaN dropped. */
  check(omx_hrp_place_in_range((double)OMX_EQ_FREQ_RANGE_MIN), "in-range", "the lower bound is kept", "dropped");
  check(omx_hrp_place_in_range((double)OMX_EQ_FREQ_RANGE_MAX), "in-range", "the upper bound is kept", "dropped");
  check(!omx_hrp_place_in_range(nextafter((double)OMX_EQ_FREQ_RANGE_MIN, 0.0)), "in-range", "below is dropped", "kept");
  check(!omx_hrp_place_in_range(nextafter((double)OMX_EQ_FREQ_RANGE_MAX, INFINITY)), "in-range", "above is dropped",
        "kept");
  check(!omx_hrp_place_in_range(NAN), "in-range", "NaN is dropped", "kept");
  if (lines == 0) check(0, "hrp_place", path, "no vector line");
  printf("hrp_place: %d checks over %d lines, %d failures\n", checks, lines, failures);
  return failures ? 1 : 0;
}
