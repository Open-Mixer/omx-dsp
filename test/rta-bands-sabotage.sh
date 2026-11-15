#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# rta-bands-sabotage.sh <dir> <libomxdsp-contracts.a> — the band reduction arm (test/kernels/rta_bands.c) is not blind.
# The suite is built against a SABOTAGED COPY of the headers, placed ahead of the real include directory, once per
# sabotage, and each build must fail the rta_bands arm:
#   peak-rms     a band's level is its loudest bin, not the RMS across its bins;
#   centre       a band answered at its arithmetic centre, not its geometric one;
#   nyquist      the cap never clamped to the Nyquist;
#   low-gain     the low tier's correction dropped (the bin-density ratio not applied);
#   crossover    the declared crossover moved, 250 for 300 Hz (omxdsp_definitions.h);
#   gate         the declared gate moved, -80 for -60 dB (omxdsp_definitions.h).
# and each build must fail the rta_delta arm (test/kernels/rta_delta.c, the difference of two taps):
#   deadband        a difference of exactly the deadband no longer draws (<= for <);
#   sign            a pair's sign taken from its first band, not from the sum of the two;
#   clamp           the strength not clamped to 1;
#   silence         a silent band's difference not made NAN (the arithmetic of -INFINITY left to draw);
#   full-scale      the strength over 12 dB, not the EQ's gain limit;
#   deadband-moved  the declared deadband moved, 0.5 for 0.4 dB (omxdsp_definitions.h).
# The tree is never edited; a sed that matches nothing is a failure, not a skip. The real suite, run by `make suite`,
# is the control.
# Usage: CC=… CFLAGS=… CONTRACT_INC=… rta-bands-sabotage.sh <scratch-dir> <libomxdsp-contracts.a>
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
PKG="$(cd "$HERE/.." && pwd)"
OUT="${1:?scratch dir}"
LIBC="${2:?contracts archive}"
CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Wextra -Werror -O2}"
: "${CONTRACT_INC:=$(sh "$PKG/tools/contract-include.sh")}"   # make exports it
bash "$PKG/tools/kernels-gen.sh" "$OUT/gen" >/dev/null

run() { # $1 = name, $2 = header under include/, $3 = sed expression, $4 = the arm that must fail (rta_bands)
  local dir="$OUT/$1" h="$2" arm="${4:-rta_bands}"
  rm -rf "$dir" && mkdir -p "$dir"
  cp -r "$PKG/include/omxdsp" "$dir/"
  sed -i "$3" "$dir/$h"
  if cmp -s "$PKG/include/$h" "$dir/$h"; then
    echo "rta-bands-sabotage: FAIL — the $1 sabotage matched nothing; the line it targets is gone"
    exit 1
  fi
  $CC $CFLAGS -ffp-contract=off -I"$dir" -Iinclude -isystem "$CONTRACT_INC" -DOMX_CONTRACTS -I"$OUT/gen" \
    -DOMX_RTA_BANDS_VECTORS="\"$PKG/vectors/rta-bands.json\"" -DOMX_RTA_DELTA_VECTORS="\"$PKG/vectors/rta-delta.json\"" \
    -DOMX_METER_VIEW_VECTORS="\"$PKG/vectors/meter-view.json\"" \
    -o "$dir/suite" "$PKG/test/omxdsp_suite.c" "$LIBC" -lm
  local log="$dir/suite.log"
  if "$dir/suite" > "$log"; then
    echo "rta-bands-sabotage: FAIL — the suite passed with the $1 sabotage"
    exit 1
  fi
  if ! grep -qE "^FAIL \\[$arm\\]" "$log"; then
    echo "rta-bands-sabotage: FAIL — the $1 sabotage was not caught by the $arm arm"
    exit 1
  fi
  echo "rta-bands-sabotage: $1 — $(grep -cE "^FAIL \\[$arm\\]" "$log") $arm checks fail, as they must"
}

B=omxdsp/analysis/omx_rta_bands.h
D=omxdsp/omxdsp_definitions.h
run peak-rms "$B" 's|  \*rms = n > 0 ? sqrt(sum_sq / n) : 0.0;|  *rms = n > 0 ? top + 0.0 * sum_sq : 0.0;|'
run centre "$B" 's|    const double centre = sqrt(f0 \* f1);|    const double centre = (f0 + f1) / 2.0;|'
run nyquist "$B" 's|  return (double)OMX_RTA_BAND_F_MAX_CAP < nyquist ? (double)OMX_RTA_BAND_F_MAX_CAP : nyquist;|  return (void)nyquist, (double)OMX_RTA_BAND_F_MAX_CAP;|'
run low-gain "$B" 's|    const double db = omx_rta_band_db(rms) + (use_low ? low_gain : 0.0);|    const double db = omx_rta_band_db(rms) + 0.0 * low_gain;|'
run crossover "$D" 's|^#define OMX_RTA_TIER_CROSSOVER_HZ 300$|#define OMX_RTA_TIER_CROSSOVER_HZ 250|'
run gate "$D" 's|^#define OMX_RTA_BAND_GATE_DB -60$|#define OMX_RTA_BAND_GATE_DB -80|'
run deadband "$B" 's|    if (magnitude < OMX_RTA_DELTA_DEADBAND_DB_DOUBLE) continue;|    if (magnitude <= OMX_RTA_DELTA_DEADBAND_DB_DOUBLE) continue;|' rta_delta
run sign "$B" 's|    wedges\[k++\] = (struct omx_rta_wedge){i, d0 + d1 >= 0.0,|    wedges[k++] = (struct omx_rta_wedge){i, d0 + 0.0 * d1 >= 0.0,|' rta_delta
run clamp "$B" 's|strength < 1.0 ? strength : 1.0};|strength};|' rta_delta
run silence "$B" 's#    diff\[i\] = isinf(pre\[i\].db) || isinf(post\[i\].db) ? (double)NAN : post\[i\].db - pre\[i\].db;#    diff[i] = post[i].db - pre[i].db;#' rta_delta
run full-scale "$B" 's|  const double full = (double)OMX_EQ_GAIN_RANGE_MAX;|  const double full = 12.0;|' rta_delta
run deadband-moved "$D" 's|^#define OMX_RTA_DELTA_DEADBAND_DB_DOUBLE 0.4$|#define OMX_RTA_DELTA_DEADBAND_DB_DOUBLE 0.5|' rta_delta
echo "rta-bands-sabotage: all twelve sabotages are caught"
