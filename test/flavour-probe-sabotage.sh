#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# flavour-probe-sabotage.sh <plain.a> <contracts.a> <tsan.a> "<test cflags>" — flavour-check's TSan probe names its
# own failure, and runs under setarch -R where that is allowed. A stand-in compiler builds the probe (and the TSan
# consumer) as small programs that act out each case; a stand-in setarch is allowed or refused. Each case runs the real
# tools/flavour-check.sh and reads what it says:
#   link       the probe does not link             -> NOT RUN, "the TSan probe does not link"
#   mapping    the probe hits TSan's mapping fatal  -> NOT RUN, "unexpected memory mapping", setarch -R refused here
#   other      the probe fails some other way       -> NOT RUN, "the TSan probe does not run", with its own words
#   noaslr     setarch -R allowed: the probe and the consumer run under it, and the probe, which acts out the mapping
#              fatal unless it runs under setarch -R, passes; the consumer runs
#   phang      only the TSan probe's compile hangs -> RED, "the TSan probe: the compile timed out after 3 s"
#   badbound   FLAVOUR_COMPILE_TIMEOUT 0, abc, -1, 1.5 -> RED at once, "FLAVOUR_COMPILE_TIMEOUT=<v>: must be a whole number
#              of seconds above 0", before any compile (0 would be no bound at all)
#   pdeaf      the TSan probe's compile ignores SIGTERM and is killed by timeout -k -> RED as a timeout, never NOT RUN
#   masq       the real compiler is a ccache-style masquerade wrapper (a cc that runs the next cc on PATH after its own
#              directory): the whole script run again with one first on PATH must pass, not loop through the stand-in
#   hang       the compiler never returns          -> RED by flavour-check's own bound, naming the compile and the bound
# Exit 0 when every case reads as it must; each failing case prints `flavour-probe-sabotage: FAIL <case> ...`.
set -u
SELF="$(readlink -f "$0")"; ARGS=("$@")
PLAIN="${1:?}"; CONTRACTS="${2:?}"; TSAN="${3:?}"; FLAGS="${4:?}"
# The real compiler by its absolute path: the stand-in is named cc and goes first on PATH, so a bare "cc" there
# would be the stand-in calling itself.
REAL_CC="$(command -v "${CC:-cc}")" || { echo "flavour-probe-sabotage: FAIL no compiler ${CC:-cc}"; exit 2; }
REAL_PATH="$PATH"
S="$(mktemp -d "${TMPDIR:-/tmp}/flavour-probe.XXXXXX")"
trap 'rm -rf "$S"' EXIT
fail=0
mkdir -p "$S/bin"

# The stand-in setarch: allowed (FAKE_SETARCH=allow) runs the program with SETARCH_R=1 in its environment; refused
# (FAKE_SETARCH=refuse) fails as a container's seccomp makes the real one fail.
cat > "$S/bin/setarch" <<'SH'
#!/bin/bash
[ "${FAKE_SETARCH:-refuse}" = allow ] || { echo "setarch: failed to set personality to $1: Function not implemented" >&2; exit 1; }
[ "${2:-}" = -R ] || { echo "stand-in setarch: expected -R, got $*" >&2; exit 1; }
shift 2; SETARCH_R=1 exec "$@"
SH
# The stand-in compiler: a -fsanitize=thread build of the probe or of the consumer becomes a stub acting out
# FAKE_PROBE (link, mapping, other, noaslr); every other build is the real compiler's.
cat > "$S/stub.c" <<'C'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void) {
  const char *mode = STUB_MODE;
  int under_r = getenv("SETARCH_R") != NULL;
  if (!strcmp(mode, "consumer")) { puts("flavour: the stand-in TSan consumer ran"); return under_r ? 0 : 9; }
  if (!strcmp(mode, "mapping") || (!strcmp(mode, "noaslr") && !under_r)) {
    fputs("FATAL: ThreadSanitizer: unexpected memory mapping 0x5a7e61318000-0x5a7e61319000\n", stderr); return 66;
  }
  if (!strcmp(mode, "other")) { fputs("stand-in: the probe broke its own way\n", stderr); return 3; }
  return 0;
}
C
cat > "$S/bin/cc" <<SH
#!/bin/bash
REAL_PATH='$REAL_PATH'
tsan=0 probe=0 consumer=0 out=""
prev=""
for a in "\$@"; do
  case "\$a" in -fsanitize=thread) tsan=1 ;; */probe.c) probe=1 ;; */omxdsp_flavour.c) consumer=1 ;; esac
  [ "\$prev" = -o ] && out="\$a"; prev="\$a"
done
if [ "\$tsan" = 1 ] && [ "\$probe" = 1 ]; then
  [ "\$FAKE_PROBE" = hang ] && exec sleep 25
  [ "\$FAKE_PROBE" = deaf ] && { trap '' TERM; exec sleep 25; }
  [ "\$FAKE_PROBE" = link ] && { echo "ld: cannot find -ltsan" >&2; exit 1; }
  PATH="\$REAL_PATH" exec "$REAL_CC" -DSTUB_MODE="\"\$FAKE_PROBE\"" -o "\$out" "$S/stub.c"
fi
if [ "\$tsan" = 1 ] && [ "\$consumer" = 1 ]; then PATH="\$REAL_PATH" exec "$REAL_CC" -DSTUB_MODE='"consumer"' -o "\$out" "$S/stub.c"; fi
PATH="\$REAL_PATH" exec "$REAL_CC" "\$@"
SH
chmod +x "$S/bin/setarch" "$S/bin/cc"

# case <name> <FAKE_PROBE> <FAKE_SETARCH> <rc wanted> <line wanted (grep -E)> [<line that must not be there>]
case_() {
  local name="$1" probe="$2" arch="$3" rcw="$4" want="$5" nope="${6:-}" out rc
  out="$(FLAVOUR_COMPILE_TIMEOUT=3 PATH="$S/bin:$PATH" FAKE_PROBE="$probe" FAKE_SETARCH="$arch" CC="$S/bin/cc" NM="${NM:-nm}" \
    timeout 30 bash tools/flavour-check.sh "$S/out-$name" "$FLAGS" "$PLAIN" "$CONTRACTS" "$TSAN" 2>&1)"; rc=$?
  if [ "$rc" != "$rcw" ]; then echo "flavour-probe-sabotage: FAIL $name: exit $rc, wanted $rcw"; printf '%s\n' "$out" | tail -3; fail=1; return; fi
  if ! grep -qE "$want" <<< "$out"; then echo "flavour-probe-sabotage: FAIL $name: no line /$want/"; printf '%s\n' "$out" | grep 'flavour-check:' | tail -3; fail=1; return; fi
  if [ -n "$nope" ] && grep -qE "$nope" <<< "$out"; then echo "flavour-probe-sabotage: FAIL $name: /$nope/ is there"; fail=1; return; fi
  echo "flavour-probe-sabotage: ok   $name: $(grep -E "$want" <<< "$out" | head -1)"
}
# hang: a compiler that sleeps forever. flavour-check must end red by its own bound (FLAVOUR_COMPILE_TIMEOUT, here 2 s),
# naming the compile; the outer bound is only so that this test never hangs, and it killing flavour-check (exit 124)
# is the failure this case exists to catch.
cat > "$S/hang-cc" <<'SH'
#!/bin/bash
exec sleep 25
SH
chmod +x "$S/hang-cc"
hang_() {
  local out rc
  out="$(FLAVOUR_COMPILE_TIMEOUT=2 CC="$S/hang-cc" NM="${NM:-nm}" \
    timeout 20 bash tools/flavour-check.sh "$S/out-hang" "$FLAGS" "$PLAIN" "$CONTRACTS" "$TSAN" 2>&1)"; rc=$?
  if [ "$rc" = 124 ]; then echo "flavour-probe-sabotage: FAIL hang: a hanging compiler was ended only by the outer 20 s bound, flavour-check has none"; fail=1; return; fi
  if [ "$rc" != 1 ] || ! grep -qE '^flavour-check: FAIL — .*timed out after 2 s \(FLAVOUR_COMPILE_TIMEOUT\)' <<< "$out"; then
    echo "flavour-probe-sabotage: FAIL hang: exit $rc, wanted 1 with a timed-out line"; printf '%s\n' "$out" | tail -3; fail=1; return
  fi
  echo "flavour-probe-sabotage: ok   hang: $(grep -E 'timed out' <<< "$out" | head -1)"
}
# badbound: a bound that is not a whole number above 0 is refused by name before any compile; the hanging compiler
# and the outer bound show that nothing was bounded when it is not refused (timeout 0 never fires).
badbound_() {
  local v out rc
  for v in 0 abc -1 1.5; do
    out="$(FLAVOUR_COMPILE_TIMEOUT="$v" CC="$S/hang-cc" NM="${NM:-nm}" \
      timeout 15 bash tools/flavour-check.sh "$S/out-bad" "$FLAGS" "$PLAIN" "$CONTRACTS" "$TSAN" 2>&1)"; rc=$?
    if [ "$rc" != 1 ] || ! grep -qxE "flavour-check: FAIL — FLAVOUR_COMPILE_TIMEOUT=$v: must be a whole number of seconds above 0" <<< "$out"; then
      echo "flavour-probe-sabotage: FAIL badbound $v: exit $rc, wanted 1 with the named refusal"; printf '%s\n' "$out" | tail -2; fail=1; continue
    fi
    echo "flavour-probe-sabotage: ok   badbound $v: refused by name"
  done
}
# masq: the stand-in must reach the real compiler through the wrapper without the wrapper finding the stand-in again.
# The script runs itself with such a wrapper first on PATH (FLAVOUR_PROBE_MASQ marks the inner run, which skips this arm
# and the hang arms); the outer bound is only for the loop this arm exists to catch.
masq_() {
  local out rc
  mkdir -p "$S/masq"
  cat > "$S/masq/cc" <<'SH'
#!/bin/bash
me="$(dirname "$(readlink -f "$0")")"; P=""; IFS=:
for d in $PATH; do [ "$(readlink -f "$d")" = "$me" ] || P="$P:$d"; done
PATH="${P#:}" exec cc "$@"
SH
  chmod +x "$S/masq/cc"
  out="$(FLAVOUR_PROBE_MASQ=1 PATH="$S/masq:$PATH" timeout 60 bash "$SELF" "${ARGS[@]}" 2>&1)"; rc=$?
  if [ "$rc" != 0 ]; then echo "flavour-probe-sabotage: FAIL masq: with a masquerade wrapper as the compiler the arms exit $rc, wanted 0"; printf '%s\n' "$out" | grep -E 'FAIL|timed out' | head -3; fail=1; return; fi
  echo "flavour-probe-sabotage: ok   masq: the arms pass through a masquerade wrapper ($(grep -c ' ok ' <<< "$out") ok)"
}
if [ -z "${FLAVOUR_PROBE_MASQ:-}" ]; then hang_; badbound_; masq_; fi
case_ link    link    allow  0 '^flavour-check: NOT RUN — the TSan probe does not link'                  'TSan consumer reads'
case_ mapping mapping refuse 0 '^flavour-check: NOT RUN — the TSan probe: FATAL: ThreadSanitizer: unexpected memory mapping.*setarch -R is not allowed' 'TSan consumer reads'
case_ other   other   allow  0 '^flavour-check: NOT RUN — the TSan probe does not run: stand-in: the probe broke its own way \(exit 3\)' 'TSan consumer reads'
case_ phang   hang    allow  1 '^flavour-check: FAIL — the TSan probe: the compile timed out after 3 s \(FLAVOUR_COMPILE_TIMEOUT\)' 'TSan consumer reads'
if [ -z "${FLAVOUR_PROBE_MASQ:-}" ]; then
case_ pdeaf   deaf    allow  1 '^flavour-check: FAIL — the TSan probe: the compile timed out after 3 s \(FLAVOUR_COMPILE_TIMEOUT\)' 'NOT RUN'
fi
case_ noaslr  noaslr  allow  0 '^flavour-check: the TSan consumer reads the contracts \(under setarch [^ ]+ -R\)' 'NOT RUN'
exit "$fail"
