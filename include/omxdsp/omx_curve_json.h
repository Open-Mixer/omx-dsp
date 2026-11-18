// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * @file omx_curve_json.h
 * @brief omx_curve_points_marked() with JSON text on both sides: the call every runtime that cannot hold a C
 *        struct makes (the WebAssembly build, a binding over a shared library), one call shape, no struct across.
 *
 * The request is the document of `curve-io.schema.json` (`CurveRequest`): the JSON form of struct
 * omx_curve_request, its words the OMX_CURVE_*_WORDS tables' and its keys the TypeScript twin's, so a request
 * the twin took is a request this takes. The answer is the same schema's `CurveReply` (or, when the curve is refused, `CurveRefusal`): the points, marks and, when
 * the request asks, the user-only curve and a delay's stems, or the code of what was refused. Both are plain text:
 * numbers are written `%.17g` (read back exactly) and a locale's decimal comma never reaches the wire.
 *
 * A request is parsed whole before it is answered, and a document that is no `CurveRequest` (a key the shape does
 * not name, a key given twice, a missing required key, a value of the wrong kind, trailing text, more terms than
 * OMX_CURVE_JSON_TERMS_MAX) is refused with OMX_CURVE_JSON_MALFORMED, never read as far as it goes. A word of a
 * term's role, unit or origin that no table holds is answered as the struct call answers a value out of range
 * (`unitRefused`); a kind or family word of none, or a shape word of none, as `kindUnknown`.
 *
 * Not on the audio path: a control thread, a page or a host's verb calls it. Design: one behaviour in C
 * (typed-contracts-shared-behaviour.md §4).
 */
#ifndef OMX_CURVE_JSON_H
#define OMX_CURVE_JSON_H

#ifndef __wasi__
#include <locale.h> /* wasi-libc has no locale: its numbers are the C locale's, always */
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "omx_curve.h"

/** @brief The most terms one request carries: 62 bands of five roles, the head filters and the time family's. */
#define OMX_CURVE_JSON_TERMS_MAX 384u
/** @brief The room an answer to any request fits in: 256 points twice (the points and the user-only curve), a band's
 *         marks and a delay's stems. */
#define OMX_CURVE_JSON_OUT_MAX 65536u
/** @brief omx_curve_points_json()'s return for a request that is not a `CurveRequest`; `out` holds an empty string. */
#define OMX_CURVE_JSON_MALFORMED (-1)
/** @brief omx_curve_points_json()'s return for an `out` too small for the answer; `out` holds an empty string. */
#define OMX_CURVE_JSON_CAP (-2)

/** @brief The wire word of each enum omx_curve_code, by value: the TypeScript twin's `CurveCode` ids. */
static const char *const OMX_CURVE_CODE_WORDS[OMX_CURVE_CODES_COUNT] = {"ok",          "kindUnknown",   "termMissing",
                                                                        "unitRefused", "valueNotFinite", "pointsRange",
                                                                        "room"};

/** @brief The reader's state: the text, the place in it and whether it has failed. Internal to this header. */
struct omx_curve_json_cursor {
  const char *p;   /**< The next byte. */
  const char *end; /**< One past the last. */
  int bad;         /**< Non-zero once the text is not a `CurveRequest`; every later read then answers nothing. */
  char point;      /**< The locale's decimal point, swapped for `.` when a number is read. */
};

/**
 * @brief The byte that separates a locale's decimal digits, as a one-byte string, so a number is read and written
 *        with `.` whatever LC_NUMERIC holds.
 * @return The locale's decimal point (`.` in the C locale), a single byte, or `.` when the locale's is longer.
 * @note Thread-safe as localeconv() is: the caller does not change the locale during the call.
 * @thread control
 */
static inline char omx_curve_json_point(void) {
#ifdef __wasi__
  return '.';
#else
  const struct lconv *lc = localeconv();
  return lc && lc->decimal_point && lc->decimal_point[0] && !lc->decimal_point[1] ? lc->decimal_point[0] : '.';
#endif
}

/**
 * @brief Skip white space (RFC 8259: space, tab, line feed, carriage return).
 * @param c The cursor.
 * @note Thread-safe on distinct cursors.
 * @thread control
 */
static inline void omx_curve_json_ws(struct omx_curve_json_cursor *c) {
  while (c->p < c->end && (*c->p == ' ' || *c->p == '\t' || *c->p == '\n' || *c->p == '\r')) c->p++;
}

/**
 * @brief Consume `ch` after white space, or fail the cursor.
 * @param c The cursor.
 * @param ch The byte.
 * @return 1 when it was there, else 0 with the cursor failed.
 * @note Thread-safe on distinct cursors.
 * @thread control
 */
static inline int omx_curve_json_eat(struct omx_curve_json_cursor *c, char ch) {
  omx_curve_json_ws(c);
  if (c->bad || c->p >= c->end || *c->p != ch) return (c->bad = 1), 0;
  c->p++;
  return 1;
}

/**
 * @brief Consume `ch` after white space when it is next, without failing the cursor otherwise.
 * @param c The cursor.
 * @param ch The byte.
 * @return 1 when it was there and is consumed, else 0.
 * @note Thread-safe on distinct cursors.
 * @thread control
 */
static inline int omx_curve_json_maybe(struct omx_curve_json_cursor *c, char ch) {
  omx_curve_json_ws(c);
  if (c->bad || c->p >= c->end || *c->p != ch) return 0;
  c->p++;
  return 1;
}

/**
 * @brief A string with no escape in `buf` (every word of the shape is plain ASCII), NUL-terminated.
 * @param c The cursor.
 * @param buf Where the string goes.
 * @param cap Room in `buf`, with the NUL.
 * @return 1 on a string that fits, else 0 with the cursor failed.
 * @note Thread-safe on distinct cursors.
 * @thread control
 */
static inline int omx_curve_json_string(struct omx_curve_json_cursor *c, char *buf, size_t cap) {
  if (!omx_curve_json_eat(c, '"')) return 0;
  size_t n = 0;
  while (c->p < c->end && *c->p != '"') {
    if (*c->p == '\\' || (unsigned char)*c->p < 0x20 || n + 1u >= cap) return (c->bad = 1), 0;
    buf[n++] = *c->p++;
  }
  if (c->p >= c->end) return (c->bad = 1), 0;
  c->p++;
  buf[n] = '\0';
  return 1;
}

/**
 * @brief A JSON number (RFC 8259's grammar, not strtod's: no `+`, no leading zero, no hex, no `inf`, no `nan`).
 * @param c The cursor.
 * @param out The value.
 * @return 1 on a number, else 0 with the cursor failed.
 * @note Thread-safe on distinct cursors; the locale's decimal point is swapped in for the read.
 * @thread control
 */
static inline int omx_curve_json_number(struct omx_curve_json_cursor *c, double *out) {
  omx_curve_json_ws(c);
  const char *s = c->p;
  char tmp[64];
  size_t n = 0;
  const char point = c->point;
  if (c->p < c->end && *c->p == '-') c->p++;
  if (c->p >= c->end || *c->p < '0' || *c->p > '9') return (c->bad = 1), 0;
  if (*c->p == '0') {
    c->p++;
  } else {
    while (c->p < c->end && *c->p >= '0' && *c->p <= '9') c->p++;
  }
  if (c->p < c->end && *c->p == '.') {
    c->p++;
    if (c->p >= c->end || *c->p < '0' || *c->p > '9') return (c->bad = 1), 0;
    while (c->p < c->end && *c->p >= '0' && *c->p <= '9') c->p++;
  }
  if (c->p < c->end && (*c->p == 'e' || *c->p == 'E')) {
    c->p++;
    if (c->p < c->end && (*c->p == '+' || *c->p == '-')) c->p++;
    if (c->p >= c->end || *c->p < '0' || *c->p > '9') return (c->bad = 1), 0;
    while (c->p < c->end && *c->p >= '0' && *c->p <= '9') c->p++;
  }
  if ((size_t)(c->p - s) >= sizeof tmp) return (c->bad = 1), 0;
  for (const char *q = s; q < c->p; q++) tmp[n++] = *q == '.' ? point : *q;
  tmp[n] = '\0';
  *out = strtod(tmp, NULL);
  return 1;
}

/**
 * @brief A non-negative integer number held to `max`, as a JSON number with no fraction or exponent.
 * @param c The cursor.
 * @param max The largest value accepted.
 * @param out The value.
 * @return 1 on an integer in 0..`max`, else 0 with the cursor failed.
 * @note Thread-safe on distinct cursors.
 * @thread control
 */
static inline int omx_curve_json_uint(struct omx_curve_json_cursor *c, double max, uint32_t *out) {
  double v;
  if (!omx_curve_json_number(c, &v)) return 0;
  if (!(v >= 0.0 && v <= max && v == floor(v))) return (c->bad = 1), 0;
  *out = (uint32_t)v;
  return 1;
}

/**
 * @brief `true` or `false`.
 * @param c The cursor.
 * @param out 1 or 0.
 * @return 1 on a boolean, else 0 with the cursor failed.
 * @note Thread-safe on distinct cursors.
 * @thread control
 */
static inline int omx_curve_json_bool(struct omx_curve_json_cursor *c, int *out) {
  omx_curve_json_ws(c);
  const size_t left = (size_t)(c->end - c->p);
  if (!c->bad && left >= 4u && memcmp(c->p, "true", 4) == 0) return (c->p += 4), (*out = 1), 1;
  if (!c->bad && left >= 5u && memcmp(c->p, "false", 5) == 0) return (c->p += 5), (*out = 0), 1;
  return (c->bad = 1), 0;
}

/** @brief A value out of every word table's range: the struct call's own checks then refuse it as a value out of range. */
#define OMX_CURVE_JSON_NO_WORD 0x7fff

/**
 * @brief One member name of an object: `"key":`.
 * @param c The cursor.
 * @param key Where the key goes.
 * @param cap Room in `key`.
 * @return 1 on a key and its colon, else 0 with the cursor failed.
 * @note Thread-safe on distinct cursors.
 * @thread control
 */
static inline int omx_curve_json_key(struct omx_curve_json_cursor *c, char *key, size_t cap) {
  return omx_curve_json_string(c, key, cap) && omx_curve_json_eat(c, ':');
}

/**
 * @brief A string member read as a word of one table.
 * @param c The cursor.
 * @param words The table.
 * @param n The table's count.
 * @return The word's value, or OMX_CURVE_JSON_NO_WORD for a string no row holds; the cursor failed for a non-string.
 * @note Thread-safe on distinct cursors.
 * @thread control
 */
static inline int omx_curve_json_word(struct omx_curve_json_cursor *c, const char *const *words, uint32_t n) {
  char w[32];
  if (!omx_curve_json_string(c, w, sizeof w)) return OMX_CURVE_JSON_NO_WORD;
  const int v = omx_curve_word(words, n, w);
  return v < 0 ? OMX_CURVE_JSON_NO_WORD : v;
}

/**
 * @brief One term object into `t`; a key the shape does not name fails the cursor.
 * @param c The cursor.
 * @param t The term, set whole.
 * @param bad_shape Set to 1 when the term's shape word is in no table (left alone otherwise).
 * @return 1 when the term has its three required keys (role, value, unit), else 0.
 * @note Thread-safe on distinct cursors.
 * @thread control
 */
static inline int omx_curve_json_term(struct omx_curve_json_cursor *c, struct omx_curve_term *t, int *bad_shape) {
  char key[16];
  int have_role = 0, have_value = 0, have_unit = 0, have_index = 0, have_shape = 0, have_origin = 0;
  memset(t, 0, sizeof *t);
  t->shape = OMX_CURVE_NO_SHAPE;
  t->origin = OMX_CURVE_ORIGINS_USER;
  if (!omx_curve_json_eat(c, '{')) return 0;
  if (!omx_curve_json_maybe(c, '}')) {
    do {
      if (!omx_curve_json_key(c, key, sizeof key)) return 0;
      if (!strcmp(key, "role")) {
        if (have_role) return (c->bad = 1), 0;
        t->role = (enum omx_curve_roles)omx_curve_json_word(c, OMX_CURVE_ROLE_WORDS, OMX_CURVE_ROLES_COUNT);
        have_role = 1;
      } else if (!strcmp(key, "index")) {
        if (have_index++) return (c->bad = 1), 0;
        if (!omx_curve_json_uint(c, 4294967295.0, &t->index)) return 0;
      } else if (!strcmp(key, "value")) {
        if (have_value) return (c->bad = 1), 0;
        if (!omx_curve_json_number(c, &t->value)) return 0;
        have_value = 1;
      } else if (!strcmp(key, "unit")) {
        if (have_unit) return (c->bad = 1), 0;
        t->unit = (enum omx_curve_units)omx_curve_json_word(c, OMX_CURVE_UNIT_WORDS, OMX_CURVE_UNITS_COUNT);
        have_unit = 1;
      } else if (!strcmp(key, "shape")) {
        if (have_shape++) return (c->bad = 1), 0;
        t->shape = omx_curve_json_word(c, OMX_CURVE_SHAPE_WORDS, OMX_CURVE_SHAPES_COUNT);
        if (t->shape == OMX_CURVE_JSON_NO_WORD) *bad_shape = 1;
      } else if (!strcmp(key, "origin")) {
        if (have_origin++) return (c->bad = 1), 0;
        t->origin = (enum omx_curve_origins)omx_curve_json_word(c, OMX_CURVE_ORIGIN_WORDS, OMX_CURVE_ORIGINS_COUNT);
      } else {
        return (c->bad = 1), 0;
      }
      if (c->bad) return 0;
    } while (omx_curve_json_maybe(c, ','));
    if (!omx_curve_json_eat(c, '}')) return 0;
  }
  return have_role && have_value && have_unit ? 1 : ((c->bad = 1), 0);
}

/**
 * @brief A request document into the struct: the shape of `curve-io.schema.json`'s `CurveRequest`.
 * @param c The cursor over the document.
 * @param rq The request, set whole; its terms point into `terms`.
 * @param terms Room for OMX_CURVE_JSON_TERMS_MAX terms.
 * @param bad_shape Set to 1 when a term's shape word is in no table.
 * @return 1 when the document is a `CurveRequest` and nothing follows it, else 0.
 * @note Thread-safe on distinct cursors and buffers.
 * @thread control
 */
static inline int omx_curve_json_request(struct omx_curve_json_cursor *c, struct omx_curve_request *rq,
                                         struct omx_curve_term *terms, int *bad_shape) {
  char key[16];
  int have_kind = 0, have_family = 0, have_rate = 0, have_points = 0, have_terms = 0, have_extents = 0, have_user = 0,
      have_tempo = 0;
  memset(rq, 0, sizeof *rq);
  rq->terms = terms;
  if (!omx_curve_json_eat(c, '{')) return 0;
  if (!omx_curve_json_maybe(c, '}')) {
    do {
      if (!omx_curve_json_key(c, key, sizeof key)) return 0;
      if (!strcmp(key, "kind")) {
        if (have_kind) return (c->bad = 1), 0;
        rq->kind = (enum omx_curve_displays)omx_curve_json_word(c, OMX_CURVE_DISPLAY_WORDS, OMX_CURVE_DISPLAYS_COUNT);
        have_kind = 1;
      } else if (!strcmp(key, "family")) {
        if (have_family) return (c->bad = 1), 0;
        rq->family = (enum omx_curve_families)omx_curve_json_word(c, OMX_CURVE_FAMILY_WORDS, OMX_CURVE_FAMILIES_COUNT);
        have_family = 1;
      } else if (!strcmp(key, "sampleRate")) {
        if (have_rate) return (c->bad = 1), 0;
        if (!omx_curve_json_number(c, &rq->sample_rate)) return 0;
        have_rate = 1;
      } else if (!strcmp(key, "points")) {
        if (have_points) return (c->bad = 1), 0;
        if (!omx_curve_json_uint(c, 4294967295.0, &rq->points)) return 0;
        have_points = 1;
      } else if (!strcmp(key, "extents")) {
        char k2[8];
        int have_min = 0, have_max = 0;
        if (have_extents++) return (c->bad = 1), 0;
        if (!omx_curve_json_eat(c, '{')) return 0;
        do {
          if (!omx_curve_json_key(c, k2, sizeof k2)) return 0;
          if (!strcmp(k2, "min") && !have_min && omx_curve_json_number(c, &rq->x_min)) have_min = 1;
          else if (!strcmp(k2, "max") && !have_max && omx_curve_json_number(c, &rq->x_max)) have_max = 1;
          else return (c->bad = 1), 0;
        } while (omx_curve_json_maybe(c, ','));
        if (!omx_curve_json_eat(c, '}') || !have_min || !have_max) return (c->bad = 1), 0;
        rq->has_extents = 1;
      } else if (!strcmp(key, "terms")) {
        if (have_terms) return (c->bad = 1), 0;
        if (!omx_curve_json_eat(c, '[')) return 0;
        if (!omx_curve_json_maybe(c, ']')) {
          do {
            if (rq->nterms >= OMX_CURVE_JSON_TERMS_MAX) return (c->bad = 1), 0;
            if (!omx_curve_json_term(c, &terms[rq->nterms], bad_shape)) return 0;
            rq->nterms++;
          } while (omx_curve_json_maybe(c, ','));
          if (!omx_curve_json_eat(c, ']')) return 0;
        }
        have_terms = 1;
      } else if (!strcmp(key, "userOnly")) {
        if (have_user++) return (c->bad = 1), 0;
        if (!omx_curve_json_bool(c, &rq->user_only)) return 0;
      } else if (!strcmp(key, "tempoBpm")) {
        if (have_tempo++) return (c->bad = 1), 0;
        if (!omx_curve_json_number(c, &rq->tempo_bpm)) return 0;
      } else {
        return (c->bad = 1), 0;
      }
      if (c->bad) return 0;
    } while (omx_curve_json_maybe(c, ','));
    if (!omx_curve_json_eat(c, '}')) return 0;
  }
  omx_curve_json_ws(c);
  if (c->p != c->end) return (c->bad = 1), 0;
  return have_kind && have_family && have_rate && have_points && have_terms ? 1 : ((c->bad = 1), 0);
}

/** @brief The answer writer's place: the buffer, how much of it is used, whether it ran out, and the locale's decimal
 *         point its numbers are formatted with. Internal to this header. */
struct omx_curve_json_out {
  char *buf;  /**< The caller's buffer. */
  size_t cap; /**< Room in it. */
  size_t n;   /**< Bytes written, never more than `cap` - 1. */
  int full;   /**< Non-zero once a write did not fit. */
  char point; /**< The byte the locale's formatting puts between a number's digits, put back to `.` on the wire. */
};

/**
 * @brief Append a literal piece of the document: a key, a word, a bracket or a comma, never a number.
 * @param o The writer.
 * @param s The text, NUL-terminated; copied byte for byte, whatever the locale holds.
 * @note Thread-safe on distinct writers; `o->full` is set when the piece does not fit.
 * @thread control
 */
static inline void omx_curve_json_lit(struct omx_curve_json_out *o, const char *s) {
  const size_t len = strlen(s);
  if (o->full || len >= o->cap - o->n) {
    o->full = 1;
    return;
  }
  memcpy(o->buf + o->n, s, len);
  o->n += len;
  o->buf[o->n] = '\0';
}

/**
 * @brief Append one number, `%.17g` (read back exactly), its decimal point `.` whatever the locale holds: the only
 *        bytes this swaps are the ones of the number itself, so a comma-decimal locale cannot reach a separator.
 * @param o The writer.
 * @param v The value; a value that is not finite is written `null` (a curve never holds one).
 * @note Thread-safe on distinct writers; `o->full` is set when the piece does not fit.
 * @thread control
 */
static inline void omx_curve_json_num(struct omx_curve_json_out *o, double v) {
  char tmp[40];
  if (!isfinite(v)) {
    omx_curve_json_lit(o, "null");
    return;
  }
  const int w = snprintf(tmp, sizeof tmp, "%.17g", v);
  for (int i = 0; i < w; i++)
    if (tmp[i] == o->point) tmp[i] = '.';
  omx_curve_json_lit(o, tmp);
}

/**
 * @brief The `"key":[{"x":..,"y":..},..]` member of two parallel arrays.
 * @param o The writer.
 * @param key The member name.
 * @param x The x of each point.
 * @param y The y of each point.
 * @param n The count.
 * @note Thread-safe on distinct writers.
 * @thread control
 */
static inline void omx_curve_json_points(struct omx_curve_json_out *o, const char *key, const double *x, const double *y,
                                         uint32_t n) {
  omx_curve_json_lit(o, "\"");
  omx_curve_json_lit(o, key);
  omx_curve_json_lit(o, "\":[");
  for (uint32_t i = 0; i < n; i++) {
    omx_curve_json_lit(o, i ? ",{\"x\":" : "{\"x\":");
    omx_curve_json_num(o, x[i]);
    omx_curve_json_lit(o, ",\"y\":");
    omx_curve_json_num(o, y[i]);
    omx_curve_json_lit(o, "}");
  }
  omx_curve_json_lit(o, "]");
}

/**
 * @brief omx_curve_points_json() with the locale's decimal point named: the one body, so a test can name `,` and
 *        prove no separator of the document is touched by it.
 * @param request The request document, NUL-terminated.
 * @param out The answer, NUL-terminated; an empty string when the return is negative.
 * @param cap Room in `out`.
 * @param point The decimal point the locale's number formatting and parsing use.
 * @return As omx_curve_points_json().
 * @note Thread-safe on distinct output, reentrant.
 * @thread control
 */
static inline int omx_curve_points_json_point(const char *request, char *out, size_t cap, char point) {
  if (!out || cap == 0u) return OMX_CURVE_JSON_CAP;
  out[0] = '\0';
  if (!request) return OMX_CURVE_JSON_MALFORMED;
  struct omx_curve_json_cursor cur = {request, request + strlen(request), 0, point};
  struct omx_curve_request rq;
  struct omx_curve_term terms[OMX_CURVE_JSON_TERMS_MAX];
  int bad_shape = 0;
  if (!omx_curve_json_request(&cur, &rq, terms, &bad_shape)) return OMX_CURVE_JSON_MALFORMED;
  double x[OMX_CURVE_POINTS_MAX], y[OMX_CURVE_POINTS_MAX], yu[OMX_CURVE_POINTS_MAX];
  struct omx_curve_mark marks[OMX_CURVE_BANDS_MAX];
  struct omx_curve_stem stems[OMX_CURVE_TAPS_MAX];
  struct omx_curve_extra ex = {yu, marks, OMX_CURVE_BANDS_MAX, 0u, stems, OMX_CURVE_TAPS_MAX, 0u};
  int code = OMX_CURVE_OK;
  const uint32_t n = omx_curve_points_marked(&rq, x, y, OMX_CURVE_POINTS_MAX, &ex, &code);
  if (code == OMX_CURVE_OK && bad_shape) code = OMX_CURVE_KIND_UNKNOWN;
  struct omx_curve_json_out o = {out, cap, 0u, 0, point};
  if (code != OMX_CURVE_OK) {
    omx_curve_json_lit(&o, "{\"code\":\"");
    omx_curve_json_lit(&o, OMX_CURVE_CODE_WORDS[(unsigned)code < OMX_CURVE_CODES_COUNT ? code : 0]);
    omx_curve_json_lit(&o, "\"}");
  } else {
    omx_curve_json_lit(&o, "{");
    omx_curve_json_points(&o, "points", x, y, n);
    omx_curve_json_lit(&o, ",\"marks\":[");
    for (uint32_t i = 0; i < ex.nmarks; i++) {
      omx_curve_json_lit(&o, i ? ",{\"freq\":" : "{\"freq\":");
      omx_curve_json_num(&o, marks[i].freq_hz);
      omx_curve_json_lit(&o, ",\"depthDb\":");
      omx_curve_json_num(&o, marks[i].depth_db);
      omx_curve_json_lit(&o, ",\"q\":");
      omx_curve_json_num(&o, marks[i].q);
      omx_curve_json_lit(&o, ",\"origin\":\"");
      omx_curve_json_lit(&o, OMX_CURVE_ORIGIN_WORDS[marks[i].origin]);
      omx_curve_json_lit(&o, "\"}");
    }
    omx_curve_json_lit(&o, "]");
    if (rq.user_only) {
      omx_curve_json_lit(&o, ",");
      omx_curve_json_points(&o, "userPoints", x, yu, n);
    }
    if (ex.nstems) {
      omx_curve_json_lit(&o, ",\"stems\":[");
      for (uint32_t i = 0; i < ex.nstems; i++) {
        omx_curve_json_lit(&o, i ? ",{\"x\":" : "{\"x\":");
        omx_curve_json_num(&o, stems[i].t_ms);
        omx_curve_json_lit(&o, ",\"y\":");
        omx_curve_json_num(&o, stems[i].level_db);
        omx_curve_json_lit(&o, ",\"shade\":");
        omx_curve_json_num(&o, stems[i].shade);
        omx_curve_json_lit(&o, ",\"leg\":\"");
        omx_curve_json_lit(&o, OMX_CURVE_LEG_WORDS[stems[i].leg]);
        omx_curve_json_lit(&o, "\"}");
      }
      omx_curve_json_lit(&o, "]");
    }
    omx_curve_json_lit(&o, "}");
  }
  if (o.full) return (out[0] = '\0'), OMX_CURVE_JSON_CAP;
  return (int)o.n;
}

/**
 * @brief Answer a `CurveRequest` document with a `CurveReply` or a `CurveRefusal` document (curve-io.schema.json), one
 *        call, no struct across.
 *
 * The request is read whole (see the file's comment), then answered by omx_curve_points_marked(): the points and the
 * marks always, `userPoints` when the request set `userOnly`, `stems` when a delay's time response carries them; or,
 * when the curve is refused, `{"code":"<id>"}` with the id of CURVE_CODES (a refusal is an answer: the return is its
 * length). Every omx_curve_points_marked() precondition is its own refusal here.
 * @param request The request document, NUL-terminated.
 * @param out The answer, NUL-terminated; an empty string when the return is negative.
 * @param cap Room in `out`; OMX_CURVE_JSON_OUT_MAX always suffices.
 * @return The answer's length without the NUL; OMX_CURVE_JSON_MALFORMED for a document that is no `CurveRequest`;
 *         OMX_CURVE_JSON_CAP when `out` is too small.
 * @note Thread-safe on distinct output, reentrant, as long as no thread changes the locale during the call; the stack
 *       holds the terms and the points (about 40 kB).
 * @twin curvePoints
 * @thread control
 */
static inline int omx_curve_points_json(const char *request, char *out, size_t cap) {
  return omx_curve_points_json_point(request, out, cap, omx_curve_json_point());
}

#endif /* OMX_CURVE_JSON_H */
