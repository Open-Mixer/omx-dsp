#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# comp_key_isolation-perturb.sh — test/fx/comp_key_isolation.test.c sees each defect of the key's
# isolation. Each sabotage is applied to a COPY of the include directory placed ahead of the real one,
# and the oracle built against it must go red in the named arm:
#   leak   — omx_key_build writes the key into the audio leg: arm K;
#   shared — omx_key_build carries a process-wide carry from the last block into the next key: arm T.
# The tree is never edited; a sed that matches nothing is a failure, not a skip.
# Usage: CC=… CFLAGS=… comp_key_isolation-perturb.sh <scratch-dir>
set -euo pipefail
PKG="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${1:?scratch dir}"
CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Wextra -Werror -O2 -ffp-contract=off}"
: "${CONTRACT_INC:=$(sh "$PKG/tools/contract-include.sh")}"   # make exports it
rm -rf "$OUT" && mkdir -p "$OUT"
pids=()

sabotage() { # <name> <header under include/omxdsp> <sed expression> <arm>...
  local name="$1" hdr="$2" expr="$3" d="$OUT/$1"
  shift 3
  mkdir -p "$d"
  cp -r "$PKG/include/omxdsp" "$d/"
  sed -i "$expr" "$d/omxdsp/$hdr"
  if cmp -s "$PKG/include/omxdsp/$hdr" "$d/omxdsp/$hdr"; then
    echo "comp_key_isolation-perturb: FAIL — sabotage '$name' matched nothing in $hdr"; exit 1
  fi
  # shellcheck disable=SC2086 # CFLAGS is a flag list
  $CC -I"$d" -I"$PKG/include" -isystem "$CONTRACT_INC" -I"$PKG/test/fx" $CFLAGS -DOMX_CONTRACTS -pthread \
    -o "$d/oracle" "$PKG/test/fx/comp_key_isolation.test.c" "$PKG"/src/*.c -lm
  if "$d/oracle" >"$d/oracle.log" 2>&1; then echo "comp_key_isolation-perturb: FAIL — sabotage '$name' left the oracle green"; exit 1; fi
  local arm n
  for arm in "$@"; do
    n=$(grep -c "^FAIL \[$arm @" "$d/oracle.log" || true)
    [ "$n" -gt 0 ] || { echo "comp_key_isolation-perturb: FAIL — sabotage '$name' did not turn arm $arm red"; head -5 "$d/oracle.log"; exit 1; }
    echo "comp_key_isolation-perturb: sabotage '$name' turned arm $arm red ($n checks)"
  done
}

sabotage leak mix/omx_key.h 's|^    memcpy(kb, k->src, n \* sizeof(float));$|    memcpy(kb, k->src, n * sizeof(float)); memcpy((float *)l, kb, n * sizeof(float));|' K &
pids+=($!)
sabotage shared mix/omx_key.h 's|^  k->built = 1;$|  { static float carry; kb[0] += carry; carry = 0.5f * kb[n - 1u]; } k->built = 1;|' T &
pids+=($!)
failed=0
for p in "${pids[@]}"; do wait "$p" || failed=1; done
[ "$failed" = 0 ] || { echo "comp_key_isolation-perturb: FAIL — a sabotage was not seen"; exit 1; }
echo "comp_key_isolation-perturb: the key isolation oracle sees both sabotages"
