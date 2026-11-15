#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# pi-budget.sh [READER] [FIXTURE] — omx-rta-read's CPU budget on a Raspberry Pi 5 (Zynthian's board), the figures its
# README owes. On the Pi, as root, with the reader and its test fixture built there (make rta-read
# build/rta-read/rta-read-fixture; the defaults are those paths).
#
# Takes /root/pi-busy only when it is free (exit 75 when it is held) and releases it, whatever happens. Starts a private
# jack2 server, realtime, on the dummy backend, so Zynthian's card and its own server are not touched: at each rate of
# RATES (48000 96000) with a period of PERIOD (256) frames. On it, a fixed source (the null test's fx(frame)) plays
# through a pass-through at 0.5, the processor the reader watches. Then, for each cell:
#   none                  no reader: the server's own figures, the baseline
#   post, pre, both+diff  omx-rta-read --bands BANDS (160) on pass:out, on what feeds pass:in, on both; with both taps
#                         it also writes "diff", so the delta costs what both does (there is no separate delta run)
#   each without and with --low (the low zoom)
# Once the reader has written its first line (its window full: about 11 s with --low at 48 kHz), SECONDS_PER_CELL (60) are
# measured: the reader's CPU, all its threads, as % of one core, from /proc/<pid>/stat; jackd's the same way; the
# server's DSP load (mean and max of jack_cpu_load ten times a second) and its xruns, by the fixture's watch client;
# and the reader's lines per second, so a figure from a reader that was not working cannot pass for a measurement.
#
# ZYNTHIAN=keep (default) leaves the zynthian unit running, as on a Pi in use, and says so in the header; ZYNTHIAN=stop
# stops it for the run and starts it again after, if it was running.
#
# Prints a header (host, kernel, jackd, the libjack the reader loaded, the tree, the settings) and one Markdown table
# row per cell, then `PI-BUDGET ok cells=<n>` or `PI-BUDGET FAILED <why>`. Exit 0 ok, 1 a cell failed, 2 usage,
# 75 the Pi is busy.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
TREE="$(cd "$HERE/../.." && pwd)"
R="${1:-$TREE/build/rta-read/omx-rta-read}"
F="${2:-$TREE/build/rta-read/rta-read-fixture}"
RATES="${RATES:-48000 96000}" PERIOD="${PERIOD:-256}" BANDS="${BANDS:-160}" SECS="${SECONDS_PER_CELL:-60}"
ZYNTHIAN="${ZYNTHIAN:-keep}"
for b in "$R" "$F"; do [ -x "$b" ] || { echo "PI-BUDGET FAILED usage: no $b (make rta-read build/rta-read/rta-read-fixture)"; exit 2; }; done
command -v jackd > /dev/null || { echo "PI-BUDGET FAILED usage: no jackd"; exit 2; }
[ "$(id -u)" = 0 ] || { echo "PI-BUDGET FAILED usage: run as root (the Pi's lock and a realtime server)"; exit 2; }

[ -e /root/pi-busy ] && { echo "BUSY: $(cat /root/pi-busy)"; exit 75; }
printf 'holder: %s, omx-rta-read CPU budget, ~%s min\nsince: %s\n' "${HOLDER:-linuxnow-1}" \
  "$(( ($(wc -w <<< "$RATES") * 7 * (SECS + 15)) / 60 + 1 ))" "$(date -u +%FT%TZ)" > /root/pi-busy
S="$(mktemp -d /tmp/pi-budget.XXXXXX)"
WAS="$(systemctl is-active zynthian 2>/dev/null || true)"
pids=()
finish() {
  for p in "${pids[@]}"; do kill -TERM "$p" 2>/dev/null; done
  wait 2>/dev/null
  [ "$ZYNTHIAN" = stop ] && [ "$WAS" = active ] && systemctl start zynthian
  rm -f /root/pi-busy
  rm -rf "$S"
  echo "RESTORED zynthian=$(systemctl is-active zynthian 2>/dev/null) lock=$([ -e /root/pi-busy ] && echo held || echo free)"
}
trap finish EXIT
[ "$ZYNTHIAN" = stop ] && [ "$WAS" = active ] && systemctl stop zynthian

. "$TREE/tools/rt-gate/jack2-env.sh"
jack2_env > "$S/jack2" || true
export JACK_NO_AUDIO_RESERVATION=1 JACK_DEFAULT_SERVER="omx-rta-budget-$$"
TCK="$(getconf CLK_TCK)"
ticks() { awk '{ sub(/^.*\) /, ""); print $12 + $13 }' "/proc/$1/stat" 2>/dev/null || echo 0; }   # utime + stime
pct() { awk -v a="$1" -v b="$2" -v s="$3" -v t="$TCK" 'BEGIN { printf "%.1f", (b - a) / t / s * 100 }'; }

echo "PI-BUDGET $(date -u +%FT%TZ) host=$(hostname) $(tr -d '\0' 2>/dev/null < /proc/device-tree/model) kernel=$(uname -r) $(uname -m)"
echo "  $(cat "$S/jack2")"
echo "  tree $(git -C "$TREE" describe --always --dirty 2>/dev/null) bands=$BANDS seconds=$SECS period=$PERIOD zynthian=${WAS:-none}->$([ "$ZYNTHIAN" = stop ] && echo stopped || echo kept) governor=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null)"
echo
echo "| rate | period | mode | low zoom | reader CPU, % of one core | jackd CPU, % | DSP load mean / max, % | xruns | lines/s |"
echo "|---|---|---|---|---|---|---|---|---|"

cells=0 failed=""
for rate in $RATES; do
  jackd -R -P 70 -p 64 -n "$JACK_DEFAULT_SERVER" -d dummy -r "$rate" -p "$PERIOD" > "$S/jackd-$rate.log" 2>&1 &
  JD=$!
  up=0
  for _ in $(seq 100); do "$F" graph > /dev/null 2>&1 && { up=1; break; }; sleep 0.1; done
  if [ "$up" != 1 ]; then failed="$failed no server at $rate ($(tail -1 "$S/jackd-$rate.log"))"; kill "$JD"; wait "$JD" 2>/dev/null; continue; fi
  "$F" source src & SP=$!
  "$F" pass pass 0.5 & PP=$!
  pids=("$JD" "$SP" "$PP")
  sleep 0.5
  "$F" connect src:out pass:in || failed="$failed connect at $rate"
  for cell in "none -" "post off" "post on" "pre off" "pre on" "both+diff off" "both+diff on"; do
    set -- $cell
    mode="$1" low="$2"
    RP="" lines0=0
    if [ "$mode" != none ]; then
      args=(--bands "$BANDS" --name reader)
      case "$mode" in post) args+=(--post pass:out) ;; pre) args+=(--pre pass:in) ;; *) args+=(--post pass:out --pre pass:in) ;; esac
      [ "$low" = on ] && args+=(--low)
      "$R" "${args[@]}" > "$S/lines" 2> "$S/reader.err" &
      RP=$!
      for _ in $(seq 300); do [ -s "$S/lines" ] && break; kill -0 "$RP" 2>/dev/null || break; sleep 0.1; done
      if ! [ -s "$S/lines" ]; then
        failed="$failed $rate/$mode/$low: no line in 30 s ($(tail -1 "$S/reader.err"))"
        kill -TERM "$RP" 2>/dev/null; wait "$RP" 2>/dev/null
        continue
      fi
      [ "$cells" = 0 ] && echo "  reader $(jack2_name omx-rta-read "$RP" libjack.so)" >&2
      lines0=$(wc -l < "$S/lines")
    fi
    r0=$([ -n "$RP" ] && ticks "$RP" || echo 0) j0=$(ticks "$JD")
    w="$("$F" watch "$SECS")"
    r1=$([ -n "$RP" ] && ticks "$RP" || echo 0) j1=$(ticks "$JD")
    lines1=$([ -n "$RP" ] && wc -l < "$S/lines" || echo 0)
    if [ -n "$RP" ]; then kill -TERM "$RP" 2>/dev/null; wait "$RP"; rc=$?; [ "$rc" = 0 ] || failed="$failed $rate/$mode/$low: reader exit $rc"; fi
    lm=$(sed -n 's/.*load_mean=\([0-9.]*\).*/\1/p' <<< "$w") lx=$(sed -n 's/.*load_max=\([0-9.]*\).*/\1/p' <<< "$w")
    xr=$(sed -n 's/.*xruns=\([0-9]*\).*/\1/p' <<< "$w")
    [ -n "$lm" ] || { failed="$failed $rate/$mode/$low: watch said '$w'"; continue; }
    rcpu=$([ -n "$RP" ] && pct "$r0" "$r1" "$SECS" || echo "-")
    lps=$([ -n "$RP" ] && awk -v a="$lines0" -v b="$lines1" -v s="$SECS" 'BEGIN { printf "%.1f", (b - a) / s }' || echo "-")
    echo "| $rate | $PERIOD | $mode | $low | $rcpu | $(pct "$j0" "$j1" "$SECS") | $lm / $lx | $xr | $lps |"
    cells=$((cells + 1))
  done
  kill -TERM "$PP" "$SP" 2>/dev/null; wait "$PP" "$SP" 2>/dev/null   # the clients first, then their server
  kill -TERM "$JD" 2>/dev/null; wait "$JD" 2>/dev/null
  pids=()
done
echo
if [ -z "$failed" ]; then echo "PI-BUDGET ok cells=$cells"; exit 0; fi
echo "PI-BUDGET FAILED cells=$cells:$failed"
exit 1
