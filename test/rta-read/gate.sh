#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# gate.sh GATE TABLES GRAPHS TAGS CFLAGS — omx-rta-read's process callback, judged whole by omx-rt-gate (--abi jack):
#   control     the reader as shipped: its root is `process`, stored in omx_jack_process.process, and nothing is red;
#   malloc      a malloc planted in the callback reads RT-ALLOC (red first: what the gate exists to see);
#   jack-call   a JACK call outside the host-buffer class (jack_port_name) in the callback reads RT-EXTERNAL;
#   no-field    the callback registered directly, not from the omx_jack_process field: the gate finds no root and
#               judges nothing (exit 2), so a reader that hides its callback from the gate cannot pass.
# Each sabotage is a sed over a scratch copy; one that matches nothing is a failure, not a skip.
set -uo pipefail
GATE="${1:?gate}" TABLES="${2:?tables}" GRAPHS="${3:?graphs}" TAGS="${4:?tags}" CFLAGS="${5:-}"
HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$HERE/../../tools/omx-rta-read/omx-rta-read.c"
CC="${CC:-cc}"
S="$(mktemp -d "${TMPDIR:-/tmp}/rta-read-gate.XXXXXX")"
trap 'rm -rf "$S"' EXIT
fail=0

judge() { # judge <name> <unit> -> exit status in $rc, output in $S/<name>.out
  # shellcheck disable=SC2086
  "$GATE" --tables "$TABLES" --abi jack --graphs "$GRAPHS" --tags "$TAGS" --work "$S/work.$1" \
    -- "$CC" -Wall -Wextra -O2 $CFLAGS $(pkg-config --cflags jack) "$2" > "$S/$1.out" 2>&1
  rc=$?
}

judge control "$SRC"
"$GATE" --tables "$TABLES" --abi jack --graphs "$GRAPHS" --tags "$TAGS" --work "$S/work.roots" --list-roots \
  -- "$CC" $CFLAGS $(pkg-config --cflags jack) "$SRC" > "$S/roots.out" 2>&1
if [ "$rc" -eq 0 ] && grep -q 'process' "$S/roots.out"; then
  echo "rta-read-gate: ok   control: the callback is a root ($(grep -m1 process "$S/roots.out")) and nothing is red"
else
  echo "rta-read-gate: FAIL control: exit $rc"; sed 's/^/  /' "$S/control.out" "$S/roots.out" | head -20; fail=1
fi

sabotage() { # sabotage <name> <sed> <expect: a code, or EXIT2>
  cp "$SRC" "$S/$1.c"
  sed -i "$2" "$S/$1.c"
  if cmp -s "$SRC" "$S/$1.c"; then echo "rta-read-gate: FAIL $1: the sabotage matched nothing"; fail=1; return; fi
  judge "$1" "$S/$1.c"
  if [ "$3" = EXIT2 ]; then
    if [ "$rc" -eq 2 ]; then echo "rta-read-gate: ok   $1: judged nothing (exit 2), as it must"
    else echo "rta-read-gate: FAIL $1: exit $rc, not 2"; fail=1; fi
  elif [ "$rc" -eq 1 ] && grep -q "^$3 " "$S/$1.out"; then
    echo "rta-read-gate: ok   $1: $(grep -m1 "^$3 " "$S/$1.out")"
  else
    echo "rta-read-gate: FAIL $1: exit $rc, no $3"; sed 's/^/  /' "$S/$1.out" | head -10; fail=1
  fi
}

sabotage malloc 's|      omx_ring_write(&taps\[t\].leg\[l\].ring, in, n);|      omx_ring_write(\&taps[t].leg[l].ring, in, n);\n      free(malloc(n));|' RT-ALLOC
sabotage jack-call 's|      omx_ring_write(&taps\[t\].leg\[l\].ring, in, n);|      omx_ring_write(\&taps[t].leg[l].ring, in, n);\n      (void)jack_port_name(taps[t].leg[l].port);|' RT-EXTERNAL
sabotage no-field 's|^static const struct omx_jack_process RT = {.process = process, .arg = NULL};|static const struct { JackProcessCallback p; void *arg; } RT = {process, NULL};|; s|RT\.process, RT\.arg|RT.p, RT.arg|' EXIT2

[ "$fail" -eq 0 ] && echo "rta-read-gate: the callback is judged whole; three sabotages are caught"
exit "$fail"
