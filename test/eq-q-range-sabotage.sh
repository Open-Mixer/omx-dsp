#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# eq-q-range-sabotage.sh — the npm twin's EQ_Q_RANGE and EQ_NOTCH_Q_RANGE are read from the
# declared range at build time, not a copy of their own (omx-dsp#5). Moved the same way on both
# sides — a sabotaged copy of include/omxdsp/omxdsp_definitions.h, ahead of the real include
# directory (omx_eq_lv2_clamp_q, in omx_eq_instance.h, is header-only: no library rebuild needed),
# and the same move in a scratch copy of ts/src/definitions.ts — the C kernel's clamp moves and the
# twin-vs-C agreement test (ts/test/agreement.test.mjs) still passes. Left at the committed value on
# the TypeScript side while the kernel moved — a stale literal, what a hand-typed copy becomes the
# day the declaration changes — the same test goes red. Neither ts/ nor include/ is edited: each
# side is a scratch copy.
#
#   eq-q-range-sabotage.sh <scratch-dir> <libomxdsp.a>
#
# Needs node and ts/node_modules installed (npm ci --prefix ts). Exit 0 both arms behave as ruled,
# 1 one does not, 2 usage or environment.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
PKG="$(cd "$HERE/.." && pwd)"
LIB="${2:?libomxdsp.a}"
CFLAGS="${CFLAGS:--Wall -Wextra -Werror -O2}"
command -v node >/dev/null 2>&1 || { echo "eq-q-range-sabotage: node is missing" >&2; exit 2; }
[ -d "$PKG/ts/node_modules" ] || { echo "eq-q-range-sabotage: ts/node_modules is missing (npm ci --prefix ts)" >&2; exit 2; }
[ -f "$LIB" ] || { echo "eq-q-range-sabotage: no $LIB (make lib)" >&2; exit 2; }
LIB="$(cd "$(dirname "$LIB")" && pwd)/$(basename "$LIB")"
rm -rf "${1:?scratch dir}" && mkdir -p "$1"
OUT="$(cd "$1" && pwd)"

# the sabotaged header, ahead of the real include directory: OMX_EQ_Q_RANGE_MAX moved from 8 to 9
HDR="$OUT/hdr"
mkdir -p "$HDR" && cp -r "$PKG/include/omxdsp" "$HDR/omxdsp"
sed -i 's/^#define OMX_EQ_Q_RANGE_MAX 8$/#define OMX_EQ_Q_RANGE_MAX 9/' "$HDR/omxdsp/omxdsp_definitions.h"
if cmp -s "$PKG/include/omxdsp/omxdsp_definitions.h" "$HDR/omxdsp/omxdsp_definitions.h"; then
  echo "eq-q-range-sabotage: FAIL — the header sabotage matched nothing"; exit 1
fi
export OMXDSP_CFLAGS="$CFLAGS -ffp-contract=off -I$HDR -I$PKG/include"
export OMXDSP_LIBS="$LIB -lm"

# two scratch copies of the pieces ts/test needs: moved the same way as the header, and left as
# committed (the real ts/ is never edited or built by this script)
copy_ts() { # $1 = dest
  mkdir -p "$1"
  cp -r "$PKG/ts/src" "$PKG/ts/test" "$PKG/ts/oracle" "$PKG/ts/tsconfig.json" "$PKG/ts/package.json" "$1/"
  ln -s "$PKG/ts/node_modules" "$1/node_modules"
}
TS_MOVED="$OUT/ts-moved"
TS_STALE="$OUT/ts-stale"
copy_ts "$TS_MOVED"
copy_ts "$TS_STALE"
sed -i 's/^export const EQ_Q_RANGE = { min: 0.3, max: 8 } as const;$/export const EQ_Q_RANGE = { min: 0.3, max: 9 } as const;/' "$TS_MOVED/src/definitions.ts"
if cmp -s "$TS_STALE/src/definitions.ts" "$TS_MOVED/src/definitions.ts"; then
  echo "eq-q-range-sabotage: FAIL — the twin sabotage matched nothing"; exit 1
fi
( cd "$TS_MOVED" && npx --no-install tsc -p tsconfig.json )
( cd "$TS_STALE" && npx --no-install tsc -p tsconfig.json )

run() { # $1 = name, $2 = ts dir, $3 = pass|fail
  local log="$OUT/$1.log" arm
  ( cd "$2" && node --test test/agreement.test.mjs ) >"$log" 2>&1 || true
  arm="$(grep -oE '^(ok|not ok) [0-9]+ - clampEqQFor agrees with omx_eq_lv2_clamp_q$' "$log" || true)"
  [ -n "$arm" ] || { echo "eq-q-range-sabotage: FAIL — $1: no clampEqQFor arm in the log"; cat "$log" >&2; return 1; }
  case "$3" in
    pass) [[ "$arm" == ok\ * ]] || { echo "eq-q-range-sabotage: FAIL — $1: the agreement arm is red when both sides moved together ($arm)"; return 1; } ;;
    fail) [[ "$arm" == "not ok"* ]] || { echo "eq-q-range-sabotage: FAIL — $1: the agreement arm is green with a stale twin ($arm)"; return 1; } ;;
  esac
  echo "eq-q-range-sabotage: $1 — $arm"
}

bad=0
run moved "$TS_MOVED" pass || bad=1
run stale "$TS_STALE" fail || bad=1
[ "$bad" = 0 ] || exit 1
echo "eq-q-range-sabotage: a moved declaration moves the kernel and the twin together; a twin left at the old literal goes red"
