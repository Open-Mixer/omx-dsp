#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# curve-json-sabotage.sh <dir> <libomxdsp-contracts.a> — the curve-json arm (test/kernels/curve_json.c) is not blind.
# The suite is built against a SABOTAGED COPY of omx_curve_json.h, placed ahead of the real include directory, once per
# sabotage, and each build must fail the curve-json arm:
#   trailing      text after the document ignored;
#   leading-zero  a number with a leading zero read (JSON has none);
#   unknown-key   a key the shape does not name skipped, not refused;
#   required      a request with no `terms` answered, not refused;
#   shape-word    a shape word of no table read as "not drawn", not kindUnknown;
#   term-limit    the term limit cut at, not refused, when one more arrives;
#   cap           the room check one byte too loose (an answer written with no room for its NUL);
#   user-comma    the user-only curve joined with no comma;
#   locale-lit    the decimal point swapped in every piece written, structural commas too (the reviewed bug);
#   dup-key       a request key given twice (terms) accepted;
#   dup-value     a term's value given twice accepted;
#   fraction      a grid-point count with a fraction read as its floor.
# The tree is never edited; a sed that matches nothing is a failure, not a skip. The real suite, run by `make suite`,
# is the control.
# Usage: CC=… CFLAGS=… CONTRACT_INC=… curve-json-sabotage.sh <scratch-dir> <libomxdsp-contracts.a>
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
PKG="$(cd "$HERE/.." && pwd)"
OUT="${1:?scratch dir}"
LIBC="${2:?contracts archive}"
CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Wextra -Werror -O2}"
: "${CONTRACT_INC:=$(sh "$PKG/tools/contract-include.sh")}"   # make exports it
bash "$PKG/tools/kernels-gen.sh" "$OUT/gen" >/dev/null
H=omxdsp/omx_curve_json.h

run() { # $1 = name, $2 = sed expression
  local dir="$OUT/$1"
  rm -rf "$dir" && mkdir -p "$dir"
  cp -r "$PKG/include/omxdsp" "$dir/"
  sed -i -z "$2" "$dir/$H"   # -z: the whole file is one record, so a sabotage may span lines; each targets its first match
  if cmp -s "$PKG/include/$H" "$dir/$H"; then
    echo "curve-json-sabotage: FAIL — the $1 sabotage matched nothing; the line it targets is gone"
    exit 1
  fi
  $CC $CFLAGS -ffp-contract=off -I"$dir" -Iinclude -isystem "$CONTRACT_INC" -DOMX_CONTRACTS -I"$OUT/gen" \
    -o "$dir/suite" "$PKG/test/omxdsp_suite.c" "$LIBC" -lm
  local log="$dir/suite.log"
  if "$dir/suite" > "$log"; then
    echo "curve-json-sabotage: FAIL — the suite passed with the $1 sabotage"
    exit 1
  fi
  if ! grep -q '^FAIL \[curve-json\]' "$log"; then
    echo "curve-json-sabotage: FAIL — the $1 sabotage was not caught by the curve-json arm"
    exit 1
  fi
  echo "curve-json-sabotage: $1 — $(grep -c '^FAIL \[curve-json\]' "$log") curve-json checks fail, as they must"
}

run trailing 's|  if (c->p != c->end) return (c->bad = 1), 0;|  (void)0;|'
run leading-zero 's|  if (\*c->p == .0.) {|  if (0) {|'
run unknown-key 's|rq->tempo_bpm)) return 0;\n      } else {|rq->tempo_bpm)) return 0;\n      } else if (strcmp(key, "x") == 0) {\n        double skip;\n        if (!omx_curve_json_number(c, \&skip)) return 0;\n      } else {|'
run required 's|have_points \&\& have_terms ? 1|have_points \&\& (have_terms \| 1) ? 1|'
run shape-word 's|  if (code == OMX_CURVE_OK \&\& bad_shape) code = OMX_CURVE_KIND_UNKNOWN;|  (void)bad_shape;|'
run term-limit 's|            if (rq->nterms >= OMX_CURVE_JSON_TERMS_MAX) return (c->bad = 1), 0;|            if (rq->nterms >= OMX_CURVE_JSON_TERMS_MAX) { rq->nterms = OMX_CURVE_JSON_TERMS_MAX; return 1; }|'
run cap 's|len >= o->cap - o->n|len > o->cap - o->n|'
run user-comma 's|      omx_curve_json_lit(&o, ",");|      omx_curve_json_lit(\&o, "");|'
run fraction 's|  if (!(v >= 0.0 \&\& v <= max \&\& v == floor(v))) return (c->bad = 1), 0;|  if (!(v >= 0.0 \&\& v <= max)) return (c->bad = 1), 0;|'
run locale-lit 's|  memcpy(o->buf + o->n, s, len);|  memcpy(o->buf + o->n, s, len);\n  for (size_t i = 0; i < len; i++)\n    if (o->buf[o->n + i] == o->point) o->buf[o->n + i] = \x27.\x27;|'
run dup-key 's|        if (have_terms) return (c->bad = 1), 0;|        (void)0;|'
run dup-value 's|        if (have_value) return (c->bad = 1), 0;|        (void)0;|'
echo "curve-json-sabotage: all twelve sabotages are caught"
