// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>

/* ---- omx_curve_points_json (omx_curve_json.h): the curve function with JSON text on both sides ---------------- */

#include <omxdsp/omx_curve_json.h>

#include "fixtures/curve_requests.h"

#define CJ_DYN                                                                                                      \
  "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,"                          \
  "\"extents\":{\"min\":-40,\"max\":0},\"terms\":[{\"role\":\"threshold\",\"value\":-20,\"unit\":\"dB\"},"           \
  "{\"role\":\"ratio\",\"value\":4,\"unit\":\"ratio\"}]}"

/* A request whose terms are `terms` (a JSON array body) on the dynamics family, the code or the answer of it. */
static int cj_call(const char *json, char *out, size_t cap) { return omx_curve_points_json(json, out, cap); }

static int cj_is(const char *json, const char *want) {
  static char out[OMX_CURVE_JSON_OUT_MAX];
  const int w = cj_call(json, out, sizeof out);
  return w == (int)strlen(want) && strcmp(out, want) == 0;
}

/* The numbers after each `"x":` or `"y":` of a members list, in order, read back with strtod. */
static uint32_t cj_numbers(const char *text, const char *key, double *v, uint32_t room) {
  uint32_t n = 0;
  for (const char *p = text; (p = strstr(p, key)) != NULL && n < room; p += strlen(key)) v[n++] = strtod(p + strlen(key), NULL);
  return n;
}

static void arm_curve_json(void) {
  g_arm = "curve-json";
  static char out[OMX_CURVE_JSON_OUT_MAX];

  /* The answer, byte for byte: the points of a hard-knee compressor at -40, -20 and 0 dB. */
  ok(cj_is(CJ_DYN, "{\"points\":[{\"x\":-40,\"y\":-40},{\"x\":-20,\"y\":-20},{\"x\":0,\"y\":-15}],\"marks\":[]}"),
     "a transfer curve answers its points as text", 0, 0);

  /* Each code is a refusal answered as the code's id, never an error return. */
  ok(cj_is("{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":[]}",
           "{\"code\":\"termMissing\"}"), "no threshold: termMissing", 0, 0);
  ok(cj_is("{\"kind\":\"eq_response\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":[]}",
           "{\"code\":\"kindUnknown\"}"), "an eq display on the dynamics family: kindUnknown", 0, 0);
  ok(cj_is("{\"kind\":\"loudness\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":[]}",
           "{\"code\":\"kindUnknown\"}"), "a kind no table holds: kindUnknown", 0, 0);
  ok(cj_is("{\"kind\":\"transfer_curve\",\"family\":\"compressor\",\"sampleRate\":48000,\"points\":3,\"terms\":[]}",
           "{\"code\":\"kindUnknown\"}"), "a family no table holds: kindUnknown", 0, 0);
  ok(cj_is("{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":["
           "{\"role\":\"attack\",\"value\":1,\"unit\":\"ms\"}]}", "{\"code\":\"unitRefused\"}"),
     "a role no table holds: unitRefused", 0, 0);
  ok(cj_is("{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":["
           "{\"role\":\"threshold\",\"value\":-20,\"unit\":\"furlong\"}]}", "{\"code\":\"unitRefused\"}"),
     "a unit no table holds: unitRefused", 0, 0);
  ok(cj_is("{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":["
           "{\"role\":\"threshold\",\"value\":-20,\"unit\":\"db\"},{\"role\":\"ratio\",\"value\":4,\"unit\":\"ratio\"}]}",
           "{\"code\":\"unitRefused\"}"),
     "a unit is its PARAM_UNITS id: the lowercase `db` is no unit, unitRefused", 0, 0);
  ok(cj_is("{\"kind\":\"eq_response\",\"family\":\"eq\",\"sampleRate\":48000,\"points\":8,\"terms\":["
           "{\"role\":\"band_freq\",\"value\":1000,\"unit\":\"Hz\",\"shape\":\"bump\"}]}", "{\"code\":\"kindUnknown\"}"),
     "a shape no table holds: kindUnknown", 0, 0);
  ok(cj_is("{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":0,\"points\":3,\"terms\":[]}",
           "{\"code\":\"valueNotFinite\"}"), "a rate of zero: valueNotFinite", 0, 0);
  ok(cj_is("{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":1e999,\"points\":3,\"terms\":[]}",
           "{\"code\":\"valueNotFinite\"}"), "a number that overflows a double (1e999) is infinity: valueNotFinite", 0, 0);
  ok(cj_is("{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":257,\"terms\":[]}",
           "{\"code\":\"pointsRange\"}"), "257 grid points: pointsRange", 0, 0);

  /* A document that is no request is refused whole, whatever it was read as far as: -1, an empty answer. */
  const char *const bad[] = {
      "", "[]", "{", CJ_DYN "x", CJ_DYN " {}",                                                       /* shape of the text */
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3}",     /* no terms */
      "{\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":[]}",                   /* no kind */
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":[],\"x\":1}", /* a key of none */
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":048000,\"points\":3,\"terms\":[]}", /* a leading zero */
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":nan,\"points\":3,\"terms\":[]}",   /* not JSON */
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3.5,\"terms\":[]}", /* a fraction */
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":-3,\"terms\":[]}",  /* negative */
      "{\"kind\":3,\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":[]}",         /* a number for a word */
      "{\"kind\":\"transfer\\u005fcurve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":[]}", /* an escape */
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":["
      "{\"role\":\"threshold\",\"unit\":\"dB\"}]}",                                                   /* a term with no value */
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":[,]}",
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"extents\":{\"min\":0},\"terms\":[]}",
      /* a key given twice is refused, never the last or the first of them */
      "{\"kind\":\"transfer_curve\",\"kind\":\"eq_response\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":[]}",
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":[],\"terms\":[]}",
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":["
      "{\"role\":\"threshold\",\"value\":-20,\"value\":-30,\"unit\":\"dB\"}]}",
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,"
      "\"extents\":{\"min\":-40,\"min\":-50,\"max\":0},\"terms\":[]}",
      "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"sampleRate\":44100,\"points\":3,\"terms\":[]}"};
  for (uint32_t i = 0; i < sizeof bad / sizeof bad[0]; i++) {
    out[0] = 'x';
    ok(cj_call(bad[i], out, sizeof out) == OMX_CURVE_JSON_MALFORMED && out[0] == '\0', "a document that is no request: malformed", i, 0);
  }
  ok(omx_curve_points_json(NULL, out, sizeof out) == OMX_CURVE_JSON_MALFORMED, "no document: malformed", 0, 0);
  ok(omx_curve_points_json(CJ_DYN, NULL, 10u) == OMX_CURVE_JSON_CAP && omx_curve_points_json(CJ_DYN, out, 0u) == OMX_CURVE_JSON_CAP,
     "no room at all: cap", 0, 0);
  {
    /* more terms than the reader holds: refused whole, not cut at the limit; exactly the limit is read */
    static char big[OMX_CURVE_JSON_OUT_MAX];
    for (uint32_t count = OMX_CURVE_JSON_TERMS_MAX; count <= OMX_CURVE_JSON_TERMS_MAX + 1u; count++) {
      size_t n = (size_t)snprintf(big, sizeof big, "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,\"terms\":[");
      for (uint32_t i = 0; i < count; i++)
        n += (size_t)snprintf(big + n, sizeof big - n, "%s{\"role\":\"threshold\",\"value\":-20,\"unit\":\"dB\"}", i ? "," : "");
      snprintf(big + n, sizeof big - n, "]}");
      const int w = cj_call(big, out, sizeof out);
      ok(count == OMX_CURVE_JSON_TERMS_MAX ? w > 0 : w == OMX_CURVE_JSON_MALFORMED,
         "the limit's own terms are read, one term more is malformed", count, w);
    }
  }

  /* A comma-decimal locale reaches no separator of the document. The writer is run with `,` named as the locale's
   * point (the one body omx_curve_points_json() calls): a number's own bytes are the only ones it may swap, and every
   * comma of the document stays a comma. */
  {
    static const char rq[] =
        "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":5,"
        "\"extents\":{\"min\":-60,\"max\":0},\"userOnly\":true,\"terms\":[{\"role\":\"threshold\",\"value\":-20,\"unit\":\"dB\"},"
        "{\"role\":\"ratio\",\"value\":4,\"unit\":\"ratio\"}]}";
    static char dot[OMX_CURVE_JSON_OUT_MAX], comma[OMX_CURVE_JSON_OUT_MAX];
    const int a = omx_curve_points_json_point(rq, dot, sizeof dot, '.'), b = omx_curve_points_json_point(rq, comma, sizeof comma, ',');
    ok(a > 0 && a == b && strcmp(dot, comma) == 0 && strstr(comma, "{\"x\":-60,\"y\":-60},{\"x\":-45,") != NULL &&
           strstr(comma, "\"y\":-18.75}") != NULL,
       "with a comma named as the locale's point every comma of the document is still a comma", a, b);
    /* and the real thing, where the host has a comma-decimal locale installed */
    const char *const names[] = {"de_DE.UTF-8", "de_DE.utf8", "de_DE", "fr_FR.UTF-8", "fr_FR.utf8", "es_ES.UTF-8", "ca_ES.UTF-8"};
    const char *had = setlocale(LC_NUMERIC, NULL);
    char saved[64];
    snprintf(saved, sizeof saved, "%s", had ? had : "C");
    int tried = 0;
    for (uint32_t i = 0; i < sizeof names / sizeof names[0] && !tried; i++) {
      if (!setlocale(LC_NUMERIC, names[i]) || omx_curve_json_point() != ',') continue;
      tried = 1;
      const int r = omx_curve_points_json(rq, comma, sizeof comma);
      ok(r == a && strcmp(dot, comma) == 0, "under a comma-decimal locale the answer is the C locale's, byte for byte", r, a);
      char dec[OMX_CURVE_JSON_OUT_MAX];
      snprintf(dec, sizeof dec, "%s", "{\"kind\":\"transfer_curve\",\"family\":\"dynamics\",\"sampleRate\":48000,\"points\":3,"
               "\"extents\":{\"min\":-60.5,\"max\":0},\"terms\":[{\"role\":\"threshold\",\"value\":-20.25,\"unit\":\"dB\"},"
               "{\"role\":\"ratio\",\"value\":4,\"unit\":\"ratio\"}]}");
      ok(omx_curve_points_json(dec, comma, sizeof comma) > 0 && strstr(comma, "\"x\":-60.5,") != NULL,
         "under a comma-decimal locale a decimal number of the request is read as written", 0, 0);
    }
    setlocale(LC_NUMERIC, saved);
    if (!tried) printf("curve-json: no comma-decimal locale is installed here; the real-locale arm was not exercised\n");
  }

  /* The room: exactly the answer and its NUL draws, one byte less is cap with an empty answer. */
  const int len = cj_call(CJ_DYN, out, sizeof out);
  ok(len > 0 && cj_call(CJ_DYN, out, (size_t)len + 1u) == len, "the exact room draws", len, 0);
  out[0] = 'x';
  ok(cj_call(CJ_DYN, out, (size_t)len) == OMX_CURVE_JSON_CAP && out[0] == '\0', "one byte less: cap, empty", len, 0);

  /* A grid the struct call answers, answered as text with the same bits: every number read back equals the double. */
  {
    static const char eq[] =
        "{\"kind\":\"eq_response\",\"family\":\"eq\",\"sampleRate\":48000,\"points\":16,\"userOnly\":true,\"terms\":["
        "{\"role\":\"band_freq\",\"index\":0,\"value\":1000,\"unit\":\"Hz\",\"shape\":\"bell\"},"
        "{\"role\":\"band_gain\",\"index\":0,\"value\":6,\"unit\":\"dB\"},"
        "{\"role\":\"band_q\",\"index\":0,\"value\":1,\"unit\":\"oct\"},"
        "{\"role\":\"band_freq\",\"index\":1,\"value\":0.25,\"unit\":\"kHz\",\"shape\":\"notch\",\"origin\":\"fbs-live\"},"
        "{\"role\":\"band_q\",\"index\":1,\"value\":8,\"unit\":\"none\"},"
        "{\"role\":\"hpf_freq\",\"value\":40,\"unit\":\"Hz\"},{\"role\":\"hpf_slope\",\"value\":24,\"unit\":\"dB\"}]}";
    const struct omx_curve_term t[] = {
        CT(BAND_FREQ, 0, 1000.0, HZ, OMX_CURVE_SHAPES_BELL), CT(BAND_GAIN, 0, 6.0, DB, CT_NS), CT(BAND_Q, 0, 1.0, OCT, CT_NS),
        CTO(BAND_FREQ, 1, 0.25, KHZ, OMX_CURVE_SHAPES_NOTCH, FBS_LIVE), CT(BAND_Q, 1, 8.0, NONE, CT_NS),
        CT(HPF_FREQ, 0, 40.0, HZ, CT_NS), CT(HPF_SLOPE, 0, 24.0, DB, CT_NS)};
    const struct omx_curve_request rq = {OMX_CURVE_DISPLAYS_EQ_RESPONSE, OMX_CURVE_FAMILIES_EQ, 48000.0, 16u, 0, 0.0, 0.0, t, 7u, 1, 0.0};
    double x[OMX_CURVE_POINTS_MAX], y[OMX_CURVE_POINTS_MAX], yu[OMX_CURVE_POINTS_MAX];
    struct omx_curve_mark mk[4];
    struct omx_curve_extra ex = {yu, mk, 4u, 0u, NULL, 0u, 0u};
    int code = -1;
    const uint32_t n = omx_curve_points_marked(&rq, x, y, OMX_CURVE_POINTS_MAX, &ex, &code);
    const int w = cj_call(eq, out, sizeof out);
    double gx[OMX_CURVE_POINTS_MAX], gv[2u * OMX_CURVE_POINTS_MAX];
    ok(code == OMX_CURVE_OK && w > 0 && n > 16u, "an eq with a planted band answers", code, w);
    ok(cj_numbers(out, "\"x\":", gx, OMX_CURVE_POINTS_MAX) == 2u * n, "the points and the user-only curve are n each", n, 0);
    const uint32_t ny = cj_numbers(out, "\"y\":", gv, 2u * OMX_CURVE_POINTS_MAX);
    ok(ny == 2u * n, "a y for each x", ny, 2u * n);
    int same = ny == 2u * n;
    for (uint32_t i = 0; same && i < n; i++) same = gx[i] == x[i] && gv[i] == y[i] && gx[n + i] == x[i] && gv[n + i] == yu[i];
    ok(same, "every number reads back as the double the struct call answered, bit for bit", 0, 0);
    ok(strstr(out, "],\"userPoints\":[{\"x\":") != NULL, "the user-only curve is a member of the answer", 0, 0);
    ok(ex.nmarks == 1u && strstr(out, "\"marks\":[{\"freq\":250,") != NULL && strstr(out, "\"origin\":\"fbs-live\"}]") != NULL,
       "the planted band is a mark with its origin word", ex.nmarks, 1);
  }
  {
    /* A head filter's slope in the unit the served description gives it, PARAM_UNITS `dBPerOct` (x-symbol dB/oct, the
     * one slope unit), is read as dB per octave: the same answer, byte for byte, as the slope sent in `dB`. Before,
     * CURVE_UNITS had no `dBPerOct`, so omx-eq8's curve, asked with the description's word, answered unitRefused and
     * the panel drew an empty frame. Both head filters, both slopes. */
    static const char *const head[] = {"hpf", "lpf"};
    static const double corner[] = {80.0, 9000.0};
    static const int slope[] = {12, 24};
    static char a_db[OMX_CURVE_JSON_OUT_MAX], a_oct[OMX_CURVE_JSON_OUT_MAX];
    for (int h = 0; h < 2; h++)
      for (int s = 0; s < 2; s++) {
        char rq_db[512], rq_oct[512];
        static const char fmt[] =
            "{\"kind\":\"eq_response\",\"family\":\"eq\",\"sampleRate\":48000,\"points\":32,\"terms\":["
            "{\"role\":\"%s_freq\",\"value\":%g,\"unit\":\"Hz\"},{\"role\":\"%s_slope\",\"value\":%d,\"unit\":\"%s\"}]}";
        snprintf(rq_db, sizeof rq_db, fmt, head[h], corner[h], head[h], slope[s], "dB");
        snprintf(rq_oct, sizeof rq_oct, fmt, head[h], corner[h], head[h], slope[s], "dBPerOct");
        const int n_db = cj_call(rq_db, a_db, sizeof a_db), n_oct = cj_call(rq_oct, a_oct, sizeof a_oct);
        ok(n_db > 0 && n_oct == n_db && memcmp(a_db, a_oct, (size_t)n_db) == 0,
           "a head filter's slope in dBPerOct draws what the same slope in dB draws", n_oct, n_db);
      }
  }
  {
    /* a delay answers stems and no point */
    ok(cj_call("{\"kind\":\"time_response\",\"family\":\"delay\",\"sampleRate\":48000,\"points\":8,\"terms\":["
               "{\"role\":\"time\",\"value\":100,\"unit\":\"ms\"},{\"role\":\"feedback\",\"value\":0.5,\"unit\":\"none\"},"
               "{\"role\":\"mix\",\"value\":0.5,\"unit\":\"none\"},{\"role\":\"pingpong\",\"value\":1,\"unit\":\"none\"}]}",
               out, sizeof out) > 0 && strncmp(out, "{\"points\":[],\"marks\":[],\"stems\":[{\"x\":0,", 40) == 0 &&
           strstr(out, "\"leg\":\"left\"") != NULL && strstr(out, "\"leg\":\"right\"") != NULL,
       "a delay answers its stems, the ping-pong's legs by word", 0, 0);
  }
  {
    /* the answer to the dearest request fits OMX_CURVE_JSON_OUT_MAX: 256 points and the user-only curve, planted bands as marks */
    static char big[OMX_CURVE_JSON_OUT_MAX];
    size_t n = (size_t)snprintf(big, sizeof big,
        "{\"kind\":\"eq_response\",\"family\":\"eq\",\"sampleRate\":192000,\"points\":160,\"userOnly\":true,"
        "\"extents\":{\"min\":3.3333333333333e-3,\"max\":7.7777777777777e4},\"terms\":[");
    for (uint32_t b = 0; b < 32u; b++)
      n += (size_t)snprintf(big + n, sizeof big - n,
          "%s{\"role\":\"band_freq\",\"index\":%u,\"value\":%.17g,\"unit\":\"Hz\",\"shape\":\"bell\",\"origin\":\"hrp\"},"
          "{\"role\":\"band_gain\",\"index\":%u,\"value\":-1.2345678901234567,\"unit\":\"dB\"},"
          "{\"role\":\"band_q\",\"index\":%u,\"value\":1.2345678901234567,\"unit\":\"none\"}",
          b ? "," : "", b, 100.123456789012345 * (b + 1u), b, b);
    snprintf(big + n, sizeof big - n, "]}");
    const int w = cj_call(big, out, sizeof out);
    ok(w > 0 && (size_t)w < OMX_CURVE_JSON_OUT_MAX - 1u, "a request with 160 points, 32 planted bands and the user-only curve fits", w, OMX_CURVE_JSON_OUT_MAX);
  }
  expect_clean();
}
