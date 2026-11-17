#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
#
# reverb-perturb.sh — test/fx/reverb.test.c sees omx_pool_take's wrap guard reverted. A SABOTAGED
# copy of omx_reverb.h puts back the summed bound (`s->next_free + len > s->pool_len`), which a
# length near UINT32_MAX wraps past, handing out a pointer beyond the pool. Built against it with
# the tree's own CFLAGS, the oracle must BUILD, RUN TO ITS END and COUNT the revert as a failure of
# the pool-take arm: a build error or a crash would hide which arm caught it, and depends on the
# compiler and the optimiser (#71 review).
# The tree is never edited; a sed that matches nothing is a failure, not a skip.
# Usage: CC=… CFLAGS=… reverb-perturb.sh <scratch-dir>
set -euo pipefail
PKG="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${1:?scratch dir}"
CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Wextra -Werror -O2 -ffp-contract=off}"
: "${CONTRACT_INC:=$(sh "$PKG/tools/contract-include.sh")}"   # make exports it
rm -rf "$OUT" && mkdir -p "$OUT"

sabotage() { # name sed-expression arm-line
  local name=$1 expr=$2 arm=$3 d="$OUT/$1"
  mkdir -p "$d"
  cp -r "$PKG/include/omxdsp" "$d/"
  sed -i "$expr" "$d/omxdsp/fx/omx_reverb.h"
  if cmp -s "$PKG/include/omxdsp/fx/omx_reverb.h" "$d/omxdsp/fx/omx_reverb.h"; then
    echo "reverb-perturb: FAIL — the $name sabotage matched nothing in omx_reverb.h"; exit 1
  fi
  # shellcheck disable=SC2086 # CFLAGS is a flag list
  if ! $CC -I"$d" -I"$PKG/include" -isystem "$CONTRACT_INC" -I"$PKG/test/fx" $CFLAGS \
      -o "$d/oracle" "$PKG/test/fx/reverb.test.c" "$PKG"/src/*.c -lm >"$d/build.log" 2>&1; then
    echo "reverb-perturb: FAIL — the $name sabotage does not build, so no arm counts it:"; tail -5 "$d/build.log"; exit 1
  fi
  local rc=0
  "$d/oracle" >"$d/oracle.log" 2>&1 || rc=$?
  if [ "$rc" -eq 0 ]; then echo "reverb-perturb: FAIL — the $name sabotage left fx/reverb green"; exit 1; fi
  if [ "$rc" -gt 128 ]; then
    echo "reverb-perturb: FAIL — the $name sabotage killed fx/reverb (signal $((rc - 128))), no arm counted it"; exit 1
  fi
  grep -qF "FAIL: $arm" "$d/oracle.log" \
    || { echo "reverb-perturb: FAIL — the $name sabotage did not turn the arm '$arm' red"; tail -5 "$d/oracle.log"; exit 1; }
  echo "reverb-perturb: the $name sabotage turned $(grep -c '^FAIL' "$d/oracle.log" || true) fx/reverb checks red, '$arm' among them"
}

sabotage summed-bound \
  's|if (len > s->pool_len - s->next_free) { /\* no unsigned wrap \*/|if (s->next_free + len > s->pool_len) {|' \
  'a length whose sum wraps is refused, the cursor unmoved'
