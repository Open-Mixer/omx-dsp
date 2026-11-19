#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# comp_feedback-perturb.sh — test/fx/comp_feedback.test.c sees each defect of the bus compressor's feedback path the spec's O7, O8 and O9 stand on.
# Each sabotage is applied to a COPY of the include directory placed ahead of the real one, and the oracle built against it must go red
# in the named arm:
#   refuse   — the kernel no longer answers factor 1 for a feedback slot with 4x stored (the 4x refusal gone): arm A;
#   latency  — the compressor's face reports the 4x latency whatever its topology (a latency term where L5 says none): arms A, C;
#   flush    — the loop's states (the envelope, both held outputs) no longer flushed on write: arm B;
#   drive    — the drive's declared latency read as 0 (the combination's 72 lost): arm C;
#   makeup   — the loop's gain carries the make-up twice (gain beyond make-up, L7): arm C.
# The tree is never edited; a sed that matches nothing is a failure, not a skip.
# Usage: CC=... CFLAGS=... comp_feedback-perturb.sh <scratch-dir>
set -euo pipefail
PKG="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${1:?scratch dir}"
CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Wextra -Werror -O2 -ffp-contract=off}"
: "${CONTRACT_INC:=$(sh "$PKG/tools/contract-include.sh")}"   # make exports it
rm -rf "$OUT" && mkdir -p "$OUT"
# The sabotages are independent and run side by side; each is waited for and every one must be seen.
pids=()

sabotage() { # <name> <header under include/omxdsp> <sed expression> <arm>...
  local name="$1" hdr="$2" expr="$3" d="$OUT/$1"
  shift 3
  mkdir -p "$d"
  cp -r "$PKG/include/omxdsp" "$d/"
  sed -i "$expr" "$d/omxdsp/$hdr"
  if cmp -s "$PKG/include/omxdsp/$hdr" "$d/omxdsp/$hdr"; then
    echo "comp_feedback-perturb: FAIL — sabotage '$name' matched nothing in $hdr"; exit 1
  fi
  # shellcheck disable=SC2086 # CFLAGS is a flag list
  $CC -I"$d" -I"$PKG/include" -isystem "$CONTRACT_INC" -I"$PKG/test/fx" $CFLAGS -DOMX_CONTRACTS -pthread \
    -o "$d/oracle" "$PKG/test/fx/comp_feedback.test.c" "$PKG"/src/*.c -lm
  if "$d/oracle" >"$d/oracle.log" 2>&1; then echo "comp_feedback-perturb: FAIL — sabotage '$name' left the oracle green"; exit 1; fi
  local arm n
  for arm in "$@"; do
    n=$(grep -c "^FAIL \[$arm @" "$d/oracle.log" || true)
    [ "$n" -gt 0 ] || { echo "comp_feedback-perturb: FAIL — sabotage '$name' did not turn arm $arm red"; head -5 "$d/oracle.log"; exit 1; }
    echo "comp_feedback-perturb: sabotage '$name' turned arm $arm red ($n checks)"
  done
}

sabotage refuse omx_dyn.h 's/ || p->topology == OMX_DETECTOR_TOPOLOGY_FEEDBACK) f = 1u;/) f = 1u;/' A &
pids+=($!)
sabotage latency fx/omx_comp_instance.h 's/(float)OMX_OVS_LATENCY_4X : 0.0f;/(float)OMX_OVS_LATENCY_4X : (float)OMX_OVS_LATENCY_4X;/' A C &
pids+=($!)
sabotage flush omx_dyn.h 's/st->fb_yl = omx_flush(xl\[i\] \* g);/st->fb_yl = xl[i] * g;/; s/st->fb_yr = omx_flush(xr\[i\] \* g);/st->fb_yr = xr[i] * g;/; s/st->fb_env = omx_flush(step < cap ? step : cap);/st->fb_env = step < cap ? step : cap;/; s/omx_onepole_flush(&st->fb_env, d, fb_pr)/omx_onepole(\&st->fb_env, d, fb_pr)/' B &
pids+=($!)
sabotage drive fx/omx_drive.h 's/return p->enabled ? (int)omx_oversampler_latency_for((uint32_t)p->os_factor) : 0;/return p ? 0 : 0;/' C &
pids+=($!)
sabotage makeup omx_dyn.h 's/gain\[i\] = g \* p->gc.makeup_lin;/gain[i] = g * p->gc.makeup_lin * p->gc.makeup_lin;/' C &
pids+=($!)
failed=0
for p in "${pids[@]}"; do wait "$p" || failed=1; done
[ "$failed" = 0 ] || { echo "comp_feedback-perturb: FAIL — a sabotage was not seen"; exit 1; }
echo "comp_feedback-perturb: the oracle sees all five sabotages"
