#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# flavour-check.sh — each archive carries its flavour, and a contracts consumer sees the compiled
# unit's contracts only through an archive built with them.
#
#  1. SYMBOLS. libomxdsp.a references neither the contract ledger nor the TSan runtime;
#     libomxdsp-contracts.a references the ledger and not TSan; libomxdsp-tsan.a references both.
#  2. CONTRACTS. test/omxdsp_flavour.c (built with the test flags, -DOMX_CONTRACTS) linked against
#     libomxdsp-contracts.a reads the oversampler's own precondition violation in its ledger.
#  3. BLIND CONTROL. The same program linked against libomxdsp.a must FAIL: a contracts consumer
#     on the plain archive is the mistake this check exists to catch.
#  4. TSAN. The same program linked against libomxdsp-tsan.a under -fsanitize=thread reads it too,
#     where the toolchain links and runs TSan programs; elsewhere NOT RUN (exit 0, never a pass).
#
# Usage: flavour-check.sh <scratch-dir> "<test cflags>" <plain.a> <contracts.a> <tsan.a>
set -uo pipefail
OUT="${1:?scratch dir}"; FLAGS="${2:?test cflags}"; PLAIN="${3:?}"; CONTRACTS="${4:?}"; TSAN="${5:?}"
CC="${CC:-cc}"; NM="${NM:-nm}"
mkdir -p "$OUT"
fail() { echo "flavour-check: FAIL — $*"; exit 1; }

# The bound on every compile this script starts, declared once: a compiler that never returns ends the check RED,
# naming the compile, not a pass and not an hour-long wait. A normal compile here is under a second; 60 s is the
# default, and the bound is a whole number of seconds from 1 to 3600.
BOUND="${FLAVOUR_COMPILE_TIMEOUT:-60}"
# timeout 0 never fires and a 23-digit bound is none either (it may overflow timeout), so anything but a whole number
# from 1 to 3600 is refused before any compile.
[[ "$BOUND" =~ ^[1-9][0-9]{0,3}$ ]] && [ "$BOUND" -le 3600 ] || fail "FLAVOUR_COMPILE_TIMEOUT=$BOUND: must be a whole number of seconds from 1 to 3600"
# What ended a compile that came back red, told apart: timeout exits 124 when its TERM ended the compiler, and sends
# KILL (exit 137) only 5 s after the bound (-k 5). A 137 is therefore the bound only when that much time passed; the
# same 137 sooner is a KILL from elsewhere (the OOM killer), which is a failure of its own and not a timeout.
# $SECONDS counts whole seconds, so the elapsed figure can read one short: BOUND+4 stands for BOUND+5.
# ended <what> <rc> <seconds at start>: returns when the compile was not ended by the bound or a kill.
ended() {
  local what="$1" rc="$2" t0="$3"
  if [ "$rc" -eq 124 ] || { [ "$rc" -eq 137 ] && [ $((SECONDS - t0)) -ge $((BOUND + 4)) ]; }; then
    fail "$what: the compile timed out after $BOUND s (FLAVOUR_COMPILE_TIMEOUT)"
  fi
  [ "$rc" -eq 137 ] && fail "$what: the compile was killed (signal 9)"
  return 0
}
# bounded <what> <command...>: the command's own status, or a red end naming <what> when the bound or a kill ended it.
bounded() {
  local what="$1" rc t0=$SECONDS; shift
  timeout -k 5 "$BOUND" "$@"; rc=$?
  ended "$what" "$rc" "$t0"
  return "$rc"
}

refs() { "$NM" "$1" 2>/dev/null | grep -cE "$2"; }
for a in "$PLAIN" "$CONTRACTS" "$TSAN"; do [ -f "$a" ] || fail "$a is missing"; done
[ "$(refs "$PLAIN" ' omx_contract_log$')" -eq 0 ] || fail "$PLAIN references the contract ledger"
[ "$(refs "$PLAIN" '__tsan_')" -eq 0 ] || fail "$PLAIN is TSan-instrumented"
[ "$(refs "$CONTRACTS" ' omx_contract_log$')" -gt 0 ] || fail "$CONTRACTS has no contract compiled in"
[ "$(refs "$CONTRACTS" '__tsan_')" -eq 0 ] || fail "$CONTRACTS is TSan-instrumented"
[ "$(refs "$TSAN" ' omx_contract_log$')" -gt 0 ] || fail "$TSAN has no contract compiled in"
[ "$(refs "$TSAN" '__tsan_')" -gt 0 ] || fail "$TSAN is not TSan-instrumented"
echo "flavour-check: the three archives carry their flavours"

# shellcheck disable=SC2086
bounded "the consumer on the contracts archive" $CC $FLAGS -o "$OUT/contracts" test/omxdsp_flavour.c "$CONTRACTS" -lm || fail "the consumer does not link the contracts archive"
"$OUT/contracts" || fail "the contracts consumer on $CONTRACTS"

# shellcheck disable=SC2086
bounded "the consumer on the plain archive" $CC $FLAGS -o "$OUT/plain" test/omxdsp_flavour.c "$PLAIN" -lm || fail "the consumer does not link the plain archive"
if out="$("$OUT/plain")"; then
  fail "the consumer linked against $PLAIN passed: the check cannot tell the flavours apart"
fi
printf '%s\n' "$out" | grep -q '^flavour: FAIL' || fail "the consumer on $PLAIN failed without naming the flavour: $out"
echo "flavour-check: on $PLAIN the same consumer fails, as it must"

# TSan maps its shadow at fixed addresses, and a kernel with more ASLR bits (vm.mmap_rnd_bits 32) can place the program
# where the shadow goes: "FATAL: ThreadSanitizer: unexpected memory mapping", by chance, run to run. The TSan programs
# therefore run under `setarch -R` (no address randomisation) wherever it is allowed; where it is not (a container
# whose seccomp refuses the personality) they run as they are, and that one fatal, by its own words, is NOT RUN.
# The probe, an empty TSan program, says first whether TSan can be read here at all, and when it cannot, it says why
# in its own words: it does not link, it hit the mapping fatal, or it failed some other way (its first line, its exit).
NOASLR=()
if command -v setarch > /dev/null && setarch "$(uname -m)" -R true 2> /dev/null; then NOASLR=(setarch "$(uname -m)" -R); fi
printf 'int main(void) { return 0; }\n' > "$OUT/probe.c"
probe_ok=0
t0=$SECONDS
plink="$(timeout -k 5 "$BOUND" $CC -fsanitize=thread -o "$OUT/probe" "$OUT/probe.c" 2>&1)"; plrc=$?
ended "the TSan probe" "$plrc" "$t0"
if [ "$plrc" -ne 0 ]; then
  echo "flavour-check: NOT RUN — the TSan probe does not link: $(grep -m1 . <<< "$plink")"
else
  pout="$("${NOASLR[@]}" "$OUT/probe" 2>&1)"; prc=$?
  if [ "$prc" -eq 0 ]; then
    probe_ok=1
  elif grep -q 'ThreadSanitizer: unexpected memory mapping' <<< "$pout"; then
    if [ "${#NOASLR[@]}" -eq 0 ]; then why="address randomisation, and setarch -R is not allowed here"
    else why="even under ${NOASLR[*]}"; fi
    echo "flavour-check: NOT RUN — the TSan probe: $(grep -m1 'unexpected memory mapping' <<< "$pout") ($why)"
  else
    echo "flavour-check: NOT RUN — the TSan probe does not run: $(grep -m1 . <<< "$pout") (exit $prc)"
  fi
fi
if [ "$probe_ok" = 1 ]; then
  # shellcheck disable=SC2086
  bounded "the consumer on the TSan archive" $CC $FLAGS -fsanitize=thread -o "$OUT/tsan" test/omxdsp_flavour.c "$TSAN" -lm || fail "the consumer does not link the TSan archive"
  if ! tout="$(TSAN_OPTIONS="halt_on_error=1 exitcode=66" "${NOASLR[@]}" "$OUT/tsan" 2>&1)"; then
    if [ "${#NOASLR[@]}" -eq 0 ] && grep -q 'ThreadSanitizer: unexpected memory mapping' <<< "$tout"; then
      echo "flavour-check: NOT RUN — the TSan consumer: $(grep -m1 'unexpected memory mapping' <<< "$tout") (address randomisation, and setarch -R is not allowed here)"
    else
      printf '%s\n' "$tout"; fail "the TSan consumer on $TSAN"
    fi
  else
    printf '%s\n' "$tout"
    echo "flavour-check: the TSan consumer reads the contracts${NOASLR[*]:+ (under ${NOASLR[*]})}"
  fi
fi
echo "flavour-check: RAN — a contracts consumer reads the compiled unit's contracts through the contracts archive"
