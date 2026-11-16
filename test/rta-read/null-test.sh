#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# null-test.sh READER FIXTURE — omx-rta-read only reads, on a private JACK server with the dummy backend (48 kHz, 256):
#   audio       src -> pass (gain 0.5) -> capture. The capture checks 48000 samples BIT FOR BIT against the
#               fixture's closed form with the reader absent, and again with the reader watching pass's input (PRE)
#               and output (POST): both runs have no mismatch, so the reader changed no sample;
#   structural  with the reader on, its client has no output port, its only connections are FROM pass:out (POST)
#               and from src:out, the port feeding pass:in (PRE), and every other port's connections are what they
#               were without it;
#   follow      a second source connected to pass:in is followed onto the PRE leg, and dropped when disconnected;
#   diff        every line with both taps carries "diff": null exactly where either tap is silent, else POST - PRE
#               within 0.011 dB of the line's own levels (each printed to 0.01), and -6.02 dB within 0.015 dB;
#   instant     with both taps at one frame position, POST - PRE is 20 log10(0.5) = -6.02 dB within 0.015 dB (the
#               lines carry dB to 0.01) in every band where both taps are above the gate;
#   ends        SIGTERM ends it 0, a closed stdout 3, a watched port gone 4, each naming why on stderr.
# It runs against jack2 (tools/rt-gate/jack2-env.sh) and names the libjackserver jackd loaded and the libjack the
# reader loaded; PipeWire's JACK in either is a FAIL by name (jack), not a measurement.
# Exit 0 when every check holds; the failing checks are printed `null-test: FAIL <check> ...`.
set -uo pipefail
R="${1:?reader}" F="${2:?fixture}"
S="$(mktemp -d "${TMPDIR:-/tmp}/rta-read-null.XXXXXX")"
export JACK_DEFAULT_SERVER="omx-rta-read-null-$$" JACK_NO_AUDIO_RESERVATION=1
pids=()
cleanup() { for p in "${pids[@]}"; do kill -TERM "$p" 2>/dev/null; done; wait 2>/dev/null; rm -rf "$S"; }
trap cleanup EXIT
fail=0
ok() { echo "null-test: ok   $*"; }
bad() { echo "null-test: FAIL $*"; fail=1; }
. "$(dirname "$0")/../../tools/rt-gate/jack2-env.sh"
jack2_env || { echo "null-test: FAIL jack: no jack2 to measure"; exit 2; }

# -p 64 before -d: the server's port maximum, which sizes jack2's shared memory to fit a container's /dev/shm
jackd --no-realtime -p 64 -n "$JACK_DEFAULT_SERVER" -d dummy -r 48000 -p 256 > "$S/jackd.log" 2>&1 &
JACKD=$!
up=0
for _ in $(seq 100); do "$F" graph > /dev/null 2>&1 && { up=1; break; }; sleep 0.1; done
if [ "$up" != 1 ]; then
  js="$(ldd "$(command -v jackd)" 2>/dev/null | awk '/libjackserver/ { print $3 }')"
  case "$js" in
    *pipewire*) echo "null-test: FAIL jack: jackd resolves PipeWire's libjackserver ($js), not jack2's" ;;
    *) echo "null-test: FAIL jack: no dummy JACK server (jackd's libjackserver ${js:-unresolved})" ;;
  esac
  cat "$S/jackd.log"; kill "$JACKD" 2>/dev/null; exit 2
fi

"$F" source src & pids+=($!)
"$F" pass pass 0.5 & pids+=($!)
sleep 0.5
"$F" connect src:out pass:in || bad "setup: src:out -> pass:in"
sleep 0.3
"$F" graph | sort > "$S/g0"

# the audio, without the reader
if "$F" capture capa pass:out 0.5 48000 > "$S/capa.out"; then ok "audio without the reader: $(cat "$S/capa.out")"
else bad "audio without the reader: $(cat "$S/capa.out")"; fi

# the reader on: POST from pass:out, PRE from what feeds pass:in
"$R" --post pass:out --pre pass:in --bands 64 --name reader > "$S/lines" 2> "$S/reader.err" &
RP=$!
sleep 0.3
if jack2_name "jackd" "$JACKD" libjackserver.so && jack2_name "omx-rta-read" "$RP" libjack.so; then ok "jack: jack2's, named above"
else bad "jack: not jack2's (above)"; fi
pids+=("$RP")
sleep 1.5
"$F" graph | sort > "$S/g1"
outs=$(grep -c '^port reader:.* out$' "$S/g1")
[ "$outs" -eq 0 ] && ok "structural: the reader has no output port" || bad "structural: the reader has $outs output port(s)"
grep '^conn reader:' "$S/g1" | awk '{print $2" "$3}' | sort > "$S/rconn"
printf 'reader:post_1 pass:out\nreader:pre_1 src:out\n' > "$S/rwant"
if diff -q "$S/rwant" "$S/rconn" > /dev/null; then ok "structural: the reader connects only from pass:out (POST) and src:out (PRE)"
else bad "structural: the reader's connections: $(tr '\n' ';' < "$S/rconn")"; fi
grep -v 'reader:' "$S/g1" > "$S/g1o"
if diff -q "$S/g0" "$S/g1o" > /dev/null; then ok "structural: every other port's connections are unchanged"
else bad "structural: the others changed: $(diff "$S/g0" "$S/g1o" | tr '\n' ';')"; fi

# the audio, with the reader watching
if "$F" capture capb pass:out 0.5 48000 > "$S/capb.out"; then ok "audio with the reader: $(cat "$S/capb.out")"
else bad "audio with the reader: $(cat "$S/capb.out")"; fi

# the instant: POST - PRE on the reader's lines
node -e '
  const lines = require("fs").readFileSync(process.argv[1], "utf8").trim().split("\n").filter(Boolean).map(JSON.parse);
  if (lines.length < 5) { console.log(`FAIL instant: ${lines.length} lines`); process.exit(1); }
  let worst = 0, bands = 0;
  for (const l of lines.slice(-5)) l.post.db.forEach((p, b) => {
    const q = l.pre.db[b];
    if (p === null || q === null) return;
    bands++;
    worst = Math.max(worst, Math.abs(p - q - 20 * Math.log10(0.5)));
  });
  const ok = bands > 0 && worst <= 0.015;
  console.log(`${ok ? "ok  " : "FAIL"} instant: POST - PRE = -6.02 dB within ${worst.toFixed(4)} dB over ${bands} bands of the last 5 lines (${lines.length} lines)`);
  process.exit(ok ? 0 : 1);
' "$S/lines" > "$S/instant.out"
if [ $? -eq 0 ]; then ok "$(sed 's/^ok *//' "$S/instant.out")"; else bad "$(sed 's/^FAIL //' "$S/instant.out")"; fi

# the diff: the reader's own POST - PRE, on every line
node -e '
  const lines = require("fs").readFileSync(process.argv[1], "utf8").trim().split("\n").filter(Boolean).map(JSON.parse);
  let wrong = 0, missing = 0, numbers = 0, worst = 0;
  for (const l of lines) {
    if (!Array.isArray(l.diff) || l.diff.length !== l.hz.length) { missing++; continue; }
    l.diff.forEach((d, b) => {
      const p = l.post.db[b], q = l.pre.db[b];
      if (p === null || q === null) { if (d !== null) wrong++; return; }
      if (d === null || Math.abs(d - (p - q)) > 0.011) { wrong++; return; }
      numbers++;
      worst = Math.max(worst, Math.abs(d - 20 * Math.log10(0.5)));
    });
  }
  const ok = lines.length > 0 && missing === 0 && wrong === 0 && numbers > 0 && worst <= 0.015;
  console.log(`${ok ? "ok  " : "FAIL"} diff: on ${lines.length - missing} of ${lines.length} lines, ${wrong} bands wrong, ${numbers} are -6.02 dB within ${worst.toFixed(4)} dB`);
  process.exit(ok ? 0 : 1);
' "$S/lines" > "$S/diff.out"
if [ $? -eq 0 ]; then ok "$(sed 's/^ok *//' "$S/diff.out")"; else bad "$(sed 's/^FAIL //' "$S/diff.out")"; fi

# follow the graph: a second feeder of pass:in joins the PRE leg, and leaves it
"$F" source src2 & pids+=($!)
sleep 0.4
"$F" connect src2:out pass:in || bad "setup: src2:out -> pass:in"
sleep 0.6
"$F" graph | grep -q '^conn reader:pre_1 src2:out$' && ok "follow: a new feeder of pass:in is read" || bad "follow: src2:out not followed"
"$F" disconnect src2:out pass:in
sleep 0.6
"$F" graph | grep -q '^conn reader:pre_1 src2:out$' && bad "follow: src2:out still read after it left" || ok "follow: a feeder that left is dropped"

# the ends
kill -TERM "$RP"; wait "$RP"; rc=$?
[ "$rc" -eq 0 ] && grep -q 'ended: signal' "$S/reader.err" && ok "ends: SIGTERM, exit 0" || bad "ends: SIGTERM gave exit $rc ($(cat "$S/reader.err"))"
"$R" --post pass:out --bands 8 --name reader2 2> "$S/r2.err" | head -1 > /dev/null; rc=${PIPESTATUS[0]}
[ "$rc" -eq 3 ] && ok "ends: a closed stdout, exit 3" || bad "ends: a closed stdout gave exit $rc ($(cat "$S/r2.err"))"
"$R" --post src2:out --bands 8 --name reader3 > /dev/null 2> "$S/r3.err" &
R3=$!
sleep 1
kill -TERM "${pids[-1]}"   # src2 goes, and its port with it
for _ in $(seq 50); do kill -0 "$R3" 2>/dev/null || break; sleep 0.1; done
wait "$R3"; rc=$?
[ "$rc" -eq 4 ] && ok "ends: a watched port gone, exit 4" || bad "ends: a port gone gave exit $rc ($(cat "$S/r3.err"))"

kill -TERM "$JACKD" 2>/dev/null
[ "$fail" -eq 0 ] && echo "null-test: omx-rta-read only reads"
exit "$fail"
