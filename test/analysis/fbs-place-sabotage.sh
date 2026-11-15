#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# fbs-place-sabotage.sh <dir> — one sabotage per rule of the C placement (src/omx_fbs_place.c): a SABOTAGED COPY
# of the unit is linked ahead of the archive, test/analysis/fbs_place.test.c is run over the shared vectors, and it
# must fail on the vector that rule owns. The tree is never edited, and a sed that matches nothing is a failure.
#
#   merge-radius    the merge radius one cent wider
#   fade-stage      the release skips a stage after the first
#   pools-swapped   a fixed find plants the live bell and a live one the notch
#   cap-origin      the per-origin gain cap applied to the wrong origins
#   cap-dropped     the gain cap dropped
#   slots-compact   a release moves the slots after it down by one
#   merge-youngest  the lowest slot answers a merge, not the oldest plant
#   edge            a ring at FBS_RING_HZ_MIN is refused
#   deepen          a re-arm snaps back without deepening
#   hold-doubling   a re-clamp does not double the next hold
#   memory          a re-plant forgets the ring memory
#   remove-ignored  a band the operator deleted keeps its slot and its release
#   edit-gain       an operator's gain edit is dropped
#   retag-kept      a band re-tagged to a user band stays the placement's
#   retag-no-adopt  a fixed band re-tagged fbs-live is never adopted
#   others-ignored  the EQ's other bands are not counted against its band cap
#   seat-no-adopt   a seated live band is never adopted
#   adopt-hold      an adopted band's hold starts at time zero, not at its adoption
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
UNIT=src/omx_fbs_place.c
LIB=build/libomxdsp.a
[ -f "$LIB" ] || { echo "fbs-place-sabotage.sh: no $LIB (make lib)"; exit 1; }
rm -rf "$OUT" && mkdir -p "$OUT"

# sabotage <arm> <vector that must fail> <sed expression>
sabotage() {
  local arm="$1" want="$2" expr="$3" dir="$OUT/$1"
  mkdir -p "$dir"
  cp "$UNIT" "$dir/unit.c"
  sed -i "$expr" "$dir/unit.c"
  if cmp -s "$UNIT" "$dir/unit.c"; then
    echo "fbs-place-sabotage.sh: FAIL — $arm: the line the sabotage targets in $UNIT is gone; it would test nothing"
    exit 1
  fi
  $CC $CFLAGS -Iinclude -isystem "$CONTRACT_INC" -o "$dir/oracle" test/analysis/fbs_place.test.c "$dir/unit.c" "$LIB" -lm
  if "$dir/oracle" > "$dir/log" 2>&1; then
    echo "fbs-place-sabotage.sh: FAIL — $arm: the vectors pass over a sabotaged $UNIT"
    exit 1
  fi
  if ! grep -q "^FAIL $want:" "$dir/log"; then
    echo "fbs-place-sabotage.sh: FAIL — $arm: failed, but not on $want"
    cat "$dir/log"
    exit 1
  fi
  echo "fbs-place-sabotage.sh: $arm: $want fails ($(grep -c '^FAIL' "$dir/log") lines)"
}

sabotage merge-radius merge-across-the-radius 's/<= p->merge_cents;/<= p->merge_cents + 1.0;/'
sabotage fade-stage staged-fade 's/begin_stage(p, (uint32_t)k, s->stage + 1u);/begin_stage(p, (uint32_t)k, s->stage + 2u);/'
sabotage pools-swapped pools-fixed-notch-live-bell 's/const int fixed = p->pool == OMX_FBS_POOL_FIXED;/const int fixed = p->pool != OMX_FBS_POOL_FIXED;/'
sabotage cap-origin gain-cap-every-origin 's/^  if (is_fbs(origin)) {$/  if (!is_fbs(origin)) {/'
sabotage cap-dropped gain-cap-every-origin 's/^double omx_fbs_place_gain_cap(int origin, double db) {$/&\n  (void)is_fbs; (void)origin; return db;/'
sabotage slots-compact slots-keep-their-finds 's/^    free_slot(s);$/    memmove(s, s + 1, (OMX_FBS_PLACE_SLOTS - 1u - k) * sizeof(*s)); free_slot(\&p->slot[OMX_FBS_PLACE_SLOTS - 1u]);/'
sabotage merge-youngest merge-the-oldest-answers '0,/for (int k = next_by_seq(p, 0); k >= 0; k = next_by_seq(p, p->slot\[k\].seq)) {/s//for (int k = 0; k < (int)OMX_FBS_PLACE_SLOTS; k++) { if (p->slot[k].origin == OMX_CURVE_ORIGINS_USER) continue;/'
sabotage edge edge-refused-never-clamped 's/c->freq_hz >= (double)OMX_FBS_RING_HZ_MIN/c->freq_hz > (double)OMX_FBS_RING_HZ_MIN/'
sabotage deepen regrowth-re-arms-deeper-and-doubles-the-hold 's/s->planted_db - (double)OMX_FBS_RECLAMP_DEEPEN_DB;/s->planted_db - 0.0;/'
sabotage hold-doubling regrowth-re-arms-deeper-and-doubles-the-hold 's/(double)(1u << doublings);/1.0; (void)doublings;/'
sabotage memory ring-memory-keeps-the-escalation 's/: take_memory(p, s->freq_hz);/: 0u; (void)take_memory;/'
sabotage remove-ignored operator-removes-mid-release 's/^  free_slot(\&p->slot\[k\]);$/  (void)0;/'
sabotage edit-gain operator-regains-a-live-band 's/^  s->gain_db = omx_fbs_place_gain_cap(origin, gain_db);$/  (void)gain_db;/'
sabotage retag-kept operator-retags-a-band 's/if (!is_fbs(origin)) return omx_fbs_place_remove(p, k);/if (!is_fbs(origin)) return 0;/'
sabotage retag-no-adopt operator-retags-fixed-to-live 's/\&\& !s->live) s->adopt = 1;/\&\& !s->live) s->adopt = 0;/'
sabotage others-ignored shared-eq-cap 's/omx_fbs_place_count(p) + p->others >=/omx_fbs_place_count(p) + 0u >=/'
sabotage seat-no-adopt seat-a-restored-band 's/^  s->adopt = !fixed;$/  s->adopt = 0; (void)fixed;/'
sabotage adopt-hold seat-a-restored-band 's/^    s->last_ring_ms = p->now_ms;$/    s->last_ring_ms = 0.0;/'
echo "fbs-place-sabotage.sh: every rule's arm fails its vector"
