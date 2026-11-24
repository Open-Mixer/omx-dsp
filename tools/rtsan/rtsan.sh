#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# rtsan.sh [SCRATCH] — the per-block entry points under clang's RealtimeSanitizer (make rtsan, BUILDING.md).
# A dev tool: not part of make test, check-package or CI. The tree is never edited: each run works on a copy of it
# under SCRATCH (default build/rtsan), whose include/ tools/rtsan/annotate.py marks __attribute__((nonblocking)).
#
#  1. PROBE. An empty program compiled, linked and run with $RTSAN_CC -fsanitize=realtime (default clang). Where it
#     fails: `rtsan: UNJUDGED: no clang rtsan runtime`, exit 2. Never a pass.
#  2. SELF-TEST. A copy marked AND with the control planted (a usleep(1) in omx_gate_run), its gate kernel test
#     (fx-run-kernel-gate) run: RTSan must report, naming usleep with omx_gate_run on the stack. Otherwise
#     `rtsan: FAIL the sanitizer saw nothing it should have`, exit 1: a clean run below would prove nothing.
#  3. RUN. A fresh marked copy; the oracle targets built and run with -fsanitize=realtime at -O1, FASTMATH=off,
#     make -k. Each target's exit and report count is printed; any report, or a target that does not exit 0, is red.
#     The library suite runs too: any report in it is red and its summary line must print, but its exit is printed
#     and not judged (under clang -O1 four clamp checks fail on a signed zero, a separate item of the tracker).
# Logs and RESULT.txt are kept under SCRATCH. RTSAN_JOBS (default 4) sizes make -j.
set -u
SRC="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${1:-$SRC/build/rtsan}"
mkdir -p "$OUT" || exit 1
OUT="$(cd "$OUT" && pwd)"
CC="${RTSAN_CC:-clang}"
JOBS="${RTSAN_JOBS:-4}"
TARGETS="fx-oracles mix-golden analysis-oracles engine-kernels-golden consumer"
CONTROL_TARGET=fx-run-kernel-gate
RTFLAGS="-fsanitize=realtime -g -fno-omit-frame-pointer -Wno-function-effects -Wno-error"
export RTSAN_OPTIONS="halt_on_error=false:abort_on_error=false:suppress_equal_stacks=true"
REPORT='ERROR: RealtimeSanitizer'
RESULT="$OUT/RESULT.txt"
case "$OUT/" in "$SRC/build/"*) ;; "$SRC/"*) echo "rtsan: SCRATCH $OUT is inside the tree outside build/: the copy would copy itself" >&2; exit 1 ;; esac

# 1. PROBE
printf 'int main(void) { return 0; }\n' > "$OUT/probe.c"
if ! "$CC" -fsanitize=realtime -o "$OUT/probe" "$OUT/probe.c" > "$OUT/probe.log" 2>&1 || ! "$OUT/probe" >> "$OUT/probe.log" 2>&1; then
  echo "rtsan: UNJUDGED: no clang rtsan runtime ($CC -fsanitize=realtime cannot build and run an empty program: $(head -n 1 "$OUT/probe.log"))" >&2
  exit 2
fi

copy() { # copy <dir>: the tree without build/ and .git into <dir>
  rm -rf "$1" && mkdir -p "$1" &&
    tar -C "$SRC" --exclude=./build --exclude=./.git --exclude=./fuzz/findings -cf - . | tar -C "$1" -xf -
}
build() { # build <tree> <target> <log>: prints "<exit> <reports>"
  env -u MAKEFLAGS -u MFLAGS -u MAKELEVEL make -C "$1" -k -j"$JOBS" "$2" CC="$CC" \
    CFLAGS="-Wall -O1 $RTFLAGS" FASTMATH=off > "$3" 2>&1
  local rc=$?
  echo "$rc $(grep -c "$REPORT" "$3")"
}

# 2. SELF-TEST
copy "$OUT/control" || exit 1
python3 -I "$SRC/tools/rtsan/annotate.py" "$OUT/control/include" --control > "$OUT/control.annotate" || exit 1
read -r rc n <<< "$(build "$OUT/control" "$CONTROL_TARGET" "$OUT/control.log")"
if [ "$n" -eq 0 ] || ! grep -q 'unsafe function .usleep.' "$OUT/control.log" || ! grep -q ' in omx_gate_run ' "$OUT/control.log"; then
  echo "rtsan: FAIL the sanitizer saw nothing it should have: the usleep(1) planted in omx_gate_run gave $n reports" \
    "($CONTROL_TARGET exit=$rc; $(head -n 1 "$OUT/control.annotate"); log $OUT/control.log)" >&2
  tail -n 5 "$OUT/control.log" >&2
  exit 1
fi
echo "rtsan self-test: the planted usleep(1) in omx_gate_run seen, $n reports ($CONTROL_TARGET exit=$rc)" | tee "$RESULT"

# 3. RUN
copy "$OUT/tree" || exit 1
python3 -I "$SRC/tools/rtsan/annotate.py" "$OUT/tree/include" > "$OUT/tree.annotate" || exit 1
head -n 1 "$OUT/tree.annotate" | tee -a "$RESULT"
red=0 total=0
for t in $TARGETS; do
  read -r rc n <<< "$(build "$OUT/tree" "$t" "$OUT/$t.log")"
  total=$((total + n))
  echo "$t exit=$rc rtsan-reports=$n" | tee -a "$RESULT"
  [ "$rc" -eq 0 ] && [ "$n" -eq 0 ] || red=1
done
read -r rc n <<< "$(build "$OUT/tree" suite "$OUT/suite.log")"
total=$((total + n))
summary="$(grep -m1 '^omxdsp_suite: ' "$OUT/suite.log")"
echo "suite exit=$rc rtsan-reports=$n (exit not judged; ${summary:-no summary line})" | tee -a "$RESULT"
{ [ "$n" -eq 0 ] && [ -n "$summary" ]; } || red=1
if [ "$red" -ne 0 ]; then
  echo "rtsan: FAIL $total RealtimeSanitizer reports, or a target that did not run clean (logs in $OUT)" | tee -a "$RESULT" >&2
  for t in $TARGETS suite; do grep -h -A 12 "$REPORT" "$OUT/$t.log"; done | grep -v '^--$' | head -n 40 >&2
  exit 1
fi
echo "rtsan: 0 RealtimeSanitizer reports from the marked entry points over $TARGETS suite" | tee -a "$RESULT"
