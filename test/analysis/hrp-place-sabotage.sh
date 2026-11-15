#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# hrp-place-sabotage.sh <dir> — one sabotage per rule of the C placement (src/omx_hrp_place.c): a SABOTAGED COPY
# of the unit is linked ahead of the archive, test/analysis/hrp_place.test.c is run over the shared vectors, and it
# must fail on the line that rule owns. The tree is never edited, and a sed that matches nothing is a failure.
#
#   refuse-q         a Q that is not finite is placed
#   refuse-cut       a negative cut bound is placed
#   refuse-boost     a negative boost bound is placed
#   refuse-bands     an infinite band count is placed
#   harmonic-index   gains[k] is placed as harmonic k+2
#   non-finite       a gain that is not finite is placed
#   range-clamped    a band outside EQ_FREQ_RANGE is kept
#   low-edge         a band at EQ_FREQ_RANGE's lower bound is dropped
#   high-edge        a band at EQ_FREQ_RANGE's upper bound is dropped
#   cut-unclamped    the cut bound is not applied
#   sparse           a zero gain is a band
#   bell-q           the bell's Q is not clamped
#   cap-smallest     the cap keeps the smallest corrections
#   cap-tie          the cap's tie goes to the higher harmonic
#   step-size        a move goes one and a half steps
#   step-float       the step is computed in float
#   step-nan         a NaN target is answered with no move
#
# Two refusals have no arm of their own, because another rule gives the same answer and no vector can tell them
# apart (the f0-* and bands-* lines hold both): an f0 that is zero, negative or not finite puts every harmonic
# outside EQ_FREQ_RANGE, so the range drop places nothing; a band count below one caps to no band.
#
# CC, CFLAGS and CONTRACT_INC from the environment (make exports CONTRACT_INC).
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
PKG="$(cd "$HERE/../.." && pwd)"
OUT="${1:?scratch dir}"
CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Wextra -Werror -O2 -ffp-contract=off}"
: "${CONTRACT_INC:?CONTRACT_INC is not set}"
cd "$PKG"
UNIT=src/omx_hrp_place.c
LIB=build/libomxdsp.a
[ -f "$LIB" ] || { echo "hrp-place-sabotage.sh: no $LIB (make lib)"; exit 1; }
rm -rf "$OUT" && mkdir -p "$OUT"

# sabotage <arm> <line that must fail> <sed expression>
sabotage() {
  local arm="$1" want="$2" expr="$3" dir="$OUT/$1"
  mkdir -p "$dir"
  cp "$UNIT" "$dir/unit.c"
  sed -i "$expr" "$dir/unit.c"
  if cmp -s "$UNIT" "$dir/unit.c"; then
    echo "hrp-place-sabotage.sh: FAIL — $arm: the line the sabotage targets in $UNIT is gone; it would test nothing"
    exit 1
  fi
  $CC $CFLAGS -Wno-unused-function -Iinclude -isystem "$CONTRACT_INC" -o "$dir/oracle" test/analysis/hrp_place.test.c \
    "$dir/unit.c" "$LIB" -lm
  if "$dir/oracle" > "$dir/log" 2>&1; then
    echo "hrp-place-sabotage.sh: FAIL — $arm: the vectors pass over a sabotaged $UNIT"
    exit 1
  fi
  if ! grep -q "^FAIL $want:" "$dir/log"; then
    echo "hrp-place-sabotage.sh: FAIL — $arm: failed, but not on $want"
    cat "$dir/log"
    exit 1
  fi
  echo "hrp-place-sabotage.sh: $arm: $want fails ($(grep -c '^FAIL' "$dir/log") lines)"
}

sabotage refuse-q q-nan 's/^  if (!usable(q)) return 1;$/  (void)q;/'
sabotage refuse-cut cut-negative 's/l->max_cut_db < 0.0) return 1;/l->max_cut_db < -1.0e9) return 1;/'
sabotage refuse-boost boost-negative 's/l->max_boost_db < 0.0) return 1;/l->max_boost_db < -1.0e9) return 1;/'
sabotage refuse-bands bands-inf 's/if (!usable(l->max_bands) || l->max_bands < 1.0) return 1;/if (isnan(l->max_bands) || l->max_bands < 1.0) return 1;/'
sabotage harmonic-index fundamental-allowed 's/const uint32_t harmonic = k + 1u;/const uint32_t harmonic = k + 2u;/'
sabotage non-finite non-finite-gains-skipped 's/^    if (!usable(requested)) continue;$//'
sabotage range-clamped below-range-dropped 's/^    if (!omx_hrp_place_in_range(freq_hz)) continue;$//'
sabotage low-edge low-edge-kept 's/freq_hz < (double)OMX_EQ_FREQ_RANGE_MIN/freq_hz <= (double)OMX_EQ_FREQ_RANGE_MIN/'
sabotage high-edge high-edge-kept 's/freq_hz > (double)OMX_EQ_FREQ_RANGE_MAX/freq_hz >= (double)OMX_EQ_FREQ_RANGE_MAX/'
sabotage cut-unclamped clamps-and-sparse 's/clamp(requested, -limits->max_cut_db, limits->max_boost_db)/clamp(requested, -1.0e9, limits->max_boost_db)/'
sabotage sparse clamps-and-sparse 's/^    if (gain_db == 0.0) continue;.*$//'
sabotage bell-q q-above-clamped 's/^  const double band_q = bell_q(q);$/  const double band_q = q;/'
sabotage cap-smallest cap-largest 's/if (ga != gb) return ga > gb;/if (ga != gb) return ga < gb;/'
sabotage cap-tie cap-tie-lower-harmonic 's/return a->harmonic < b->harmonic;/return a->harmonic > b->harmonic;/'
sabotage step-size plant-first-move 's/return current + sign \* (double)OMX_HRP_GAIN_STEP_DB;/return current + sign * 1.5 * (double)OMX_HRP_GAIN_STEP_DB;/'
sabotage step-float doubles-not-floats 's/return current + sign \* (double)OMX_HRP_GAIN_STEP_DB;/return (double)((float)current + (float)sign * (float)OMX_HRP_GAIN_STEP_DB);/'
sabotage step-nan nan-target 's/delta < 0.0 ? -1.0 : delta;/delta < 0.0 ? -1.0 : 0.0;/'
echo "hrp-place-sabotage.sh: every rule's arm fails its line"
