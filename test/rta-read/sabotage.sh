#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# sabotage.sh FIXTURE CFLAGS... — the null test (null-test.sh) is not blind. A scratch copy of omx-rta-read is built
# broken, once per sabotage, and the null test must fail the checks named:
#   leak    an output port, fed 1e-3 every period and connected to the PRE input it watches: the reader feeds the
#           mix. Red: structural (an output port) and audio (the capture's samples change);
#   offset  PRE read one period (256 frames) before POST: the two taps are not one instant. Red: instant.
# A sed that matches nothing is a failure, not a skip. The real reader, run by `make rta-read-test`, is the control.
set -uo pipefail
F="${1:?fixture}"; shift
HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$HERE/../../tools/omx-rta-read/omx-rta-read.c"
CC="${CC:-cc}"
S="$(mktemp -d "${TMPDIR:-/tmp}/rta-read-sabotage.XXXXXX")"
trap 'rm -rf "$S"' EXIT
fail=0

run() { # run <name> <expected red checks, space separated> <sed>...
  local name="$1" want="$2"; shift 2
  cp "$SRC" "$S/$name.c"
  for e in "$@"; do
    cp "$S/$name.c" "$S/$name.prev"
    sed -i "$e" "$S/$name.c"
    if cmp -s "$S/$name.prev" "$S/$name.c"; then echo "rta-read-sabotage: FAIL $name: a sed matched nothing: $e"; fail=1; return; fi
  done
  # shellcheck disable=SC2086
  if ! $CC $CFLAGS_READ $(pkg-config --cflags jack) -o "$S/$name" "$S/$name.c" -lm $(pkg-config --libs jack) 2> "$S/$name.cc"; then
    echo "rta-read-sabotage: FAIL $name does not build"; sed 's/^/  /' "$S/$name.cc" | head; fail=1; return
  fi
  bash "$HERE/null-test.sh" "$S/$name" "$F" > "$S/$name.out" 2>&1 && { echo "rta-read-sabotage: FAIL the null test passed with $name"; fail=1; return; }
  for w in $want; do
    grep -q "^null-test: FAIL $w" "$S/$name.out" || { echo "rta-read-sabotage: FAIL $name was not caught by the $w check"; fail=1; return; }
  done
  echo "rta-read-sabotage: $name — $(grep -c '^null-test: FAIL' "$S/$name.out") checks fail ($want among them), as they must"
}

CFLAGS_READ="$*"
run leak "structural audio" \
  's|^static struct tap taps\[TAPS\];|static struct tap taps[TAPS];\nstatic jack_port_t *leak;|' \
  's|^  atomic_store_explicit(&frames_total,|  if (leak) { float *o = jack_port_get_buffer(leak, n); for (jack_nframes_t i = 0; i < n; i++) o[i] = 1e-3f; }\n  atomic_store_explicit(\&frames_total,|' \
  's|^  for (uint32_t l = 0; l < taps\[TAP_PRE\].legs; l++) follow_feeders(c, &taps\[TAP_PRE\].leg\[l\]);|&\n  leak = jack_port_register(c, "leak", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);\n  if (leak \&\& taps[TAP_PRE].legs) jack_connect(c, jack_port_name(leak), taps[TAP_PRE].leg[0].watch);|'
run offset "instant" \
  's|^      tap_window(&taps\[t\], pos, win.block, n);|      tap_window(\&taps[t], t == TAP_PRE ? pos - 256u : pos, win.block, n);|'
[ "$fail" -eq 0 ] && echo "rta-read-sabotage: both sabotages are caught"
exit "$fail"
