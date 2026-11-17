#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# curve-vectors-sabotage.sh <scratch-dir> — the shared vectors (vectors/curve-points.json) are not blind. The native
# door (tools/curve-json-cli.c) is built against a SABOTAGED COPY of one header, once per sabotage, and
# test/curve-vectors.mjs must go red on each; the control, the real headers, must go green:
#   makeup         the make-up left out of a transfer curve (omx_curve.h);
#   limiter-ratio  a limiter's missing ratio taken as 4, not infinity (omx_curve.h);
#   soft-knee      a soft knee's denominator 1 x width, not 2 x width (omx_gaincomp.h);
#   gate-range     a gate's floor at its range dropped (omx_gaincomp.h);
#   head-slope     a 24 dB/oct head filter drawn as 12 (omx_curve.h);
#   time-feedback  an echo's level feedback^k, not feedback^(k-1) (omx_curve.h);
#   pingpong-leg   the ping-pong's first echo on the right, not the left (omx_curve.h);
#   precision      the numbers of an answer written %.6g, not %.17g (omx_curve_json.h);
#   code-word      termMissing answered as kindUnknown (omx_curve_json.h).
# The tree is never edited; a sed that matches nothing is a failure, not a skip.
# Usage: CC=… CONTRACT_INC=… NODE=… curve-vectors-sabotage.sh <scratch-dir>
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
PKG="$(cd "$HERE/.." && pwd)"
OUT="${1:?scratch dir}"
CC="${CC:-cc}"
NODE="${NODE:-node}"
CFLAGS="${CFLAGS:--Wall -Wextra -Werror -O2}"
: "${CONTRACT_INC:=$(sh "$PKG/tools/contract-include.sh")}"

build() { # $1 = include root holding omxdsp/, $2 = output binary
  $CC $CFLAGS -ffp-contract=off -I"$1" -isystem "$CONTRACT_INC" -o "$2" "$PKG/tools/curve-json-cli.c" -lm
}

mkdir -p "$OUT"
build "$PKG/include" "$OUT/control"
( cd "$PKG" && "$NODE" test/curve-vectors.mjs --native "$OUT/control" ) >"$OUT/control.log" 2>&1 \
  || { echo "curve-vectors-sabotage: FAIL — the control (the real headers) is red"; cat "$OUT/control.log"; exit 1; }

run() { # $1 = name, $2 = header under include/omxdsp, $3 = sed expression
  local dir="$OUT/$1"
  rm -rf "$dir" && mkdir -p "$dir"
  cp -r "$PKG/include/omxdsp" "$dir/"
  sed -i "$3" "$dir/omxdsp/$2"
  if cmp -s "$PKG/include/omxdsp/$2" "$dir/omxdsp/$2"; then
    echo "curve-vectors-sabotage: FAIL — the $1 sabotage matched nothing; the line it targets is gone"
    exit 1
  fi
  build "$dir" "$dir/cli"
  if ( cd "$PKG" && "$NODE" test/curve-vectors.mjs --native "$dir/cli" ) >"$dir/run.log" 2>&1; then
    echo "curve-vectors-sabotage: FAIL — the vectors passed with the $1 sabotage"
    exit 1
  fi
  echo "curve-vectors-sabotage: $1 — $(grep -c '^FINDING' "$dir/run.log") findings, as there must be"
}

run makeup omx_curve.h 's|    y\[i\] = lv + (double)g + mk;|    y[i] = lv + (double)g + 0.0 * mk;|'
run limiter-ratio omx_curve.h 's|  const float r = ratio ? (float)omx_curve_term_value(ratio) : INFINITY, k|  const float r = ratio ? (float)omx_curve_term_value(ratio) : 4.0f, k|'
run soft-knee omx_gaincomp.h 's|  const float k2 = omx_fm_select(soft, 2.0f \* p->knee_db, 1.0f);|  const float k2 = omx_fm_select(soft, p->knee_db, 1.0f);|'
run gate-range omx_gaincomp.h 's|  gb = OMX_GC_SEL(lanes, gb < p->range_db, p->range_db, gb);|  gb = gb + 0.0f * p->range_db;|'
run head-slope omx_curve.h 's|  out->slope_24 = slope == 24.0;|  out->slope_24 = 0;|'
run time-feedback omx_curve.h 's|      const double lin = wet > 0.0 \&\& f >= 0.0 ? wet \* pow(f, (double)(k - 1u)) : 0.0;|      const double lin = wet > 0.0 \&\& f >= 0.0 ? wet * pow(f, (double)k) : 0.0;|'
run pingpong-leg omx_curve.h 's|!pingpong ? OMX_CURVE_LEGS_BOTH : (k \& 1u) ? OMX_CURVE_LEGS_LEFT : OMX_CURVE_LEGS_RIGHT|!pingpong ? OMX_CURVE_LEGS_BOTH : (k \& 1u) ? OMX_CURVE_LEGS_RIGHT : OMX_CURVE_LEGS_LEFT|'
run precision omx_curve_json.h 's|"%s{\\"x\\":%.17g,\\"y\\":%.17g}"|"%s{\\"x\\":%.6g,\\"y\\":%.6g}"|'
run code-word omx_curve_json.h 's|"ok",          "kindUnknown",   "termMissing",|"ok",          "kindUnknown",   "kindUnknown",|'
echo "curve-vectors-sabotage: all nine sabotages are caught"
