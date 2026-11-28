#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# release-arm-gate-selftest.sh — release.yml's arch-matrix decides, per job, whether the arm64/aarch64
# side of the matrix builds. A tag push carries no `inputs` at all, so inputs.arm is null there, and
# null == false is true in a GitHub expression: a gate that asks only whether inputs.arm is false drops
# the arm leg on every tag. The gate tests the event first, so a push always builds the arm leg.
# This checks the gate release.yml carries, not a copy of it: each job's arch-matrix line must be the
# `(github.event_name != '<event>' && inputs.arm == false) && '<no-arm matrix>' || '<arm matrix>'` shape,
# the arm matrix must carry the job's arm architecture and the no-arm one must not, and the five
# contexts a real run can pass it — a tag push (no inputs at all), workflow_dispatch's default
# (it declares no `arm` input, so inputs.arm is null), the ci.yml dry run (workflow_call, arm: false),
# a plain pull_request and a dispatch that somehow carries arm=false — must each resolve to the
# matrix this gate is meant to produce.
# gate() evaluates the pieces extracted from release.yml (the event literal and the `== <literal>`
# comparison), so changing either in release.yml moves the table. `--sabotage` proves it: it rewrites
# the event literal in a copy of release.yml (the shape check still matches) and requires the table to
# go red on a resolved-row line, not only the literal check. The rpm job builds aarch64 beside x86_64 (owner ruling
# 2026-11-26, the full matrix for every package): --sabotage also plants an x86_64-only rpm arch-matrix and an rpm gate
# with no aarch64, each of which must be named red.
set -uo pipefail
cd "$(dirname "$0")/.."
REL="${RELEASE_YML:-.github/workflows/release.yml}"
fails=0

if [ "${1:-}" = --sabotage ]; then
  tmp="$(mktemp -d)"; trap 'rm -rf "$tmp"' EXIT
  sed "s/github.event_name != 'push'/github.event_name != 'workflow_dispatch'/" .github/workflows/release.yml > "$tmp/release.yml"
  cmp -s .github/workflows/release.yml "$tmp/release.yml" && { echo "release-arm-gate-selftest: FAIL — sabotage sed changed nothing"; exit 1; }
  out="$(RELEASE_YML="$tmp/release.yml" "$0" 2>&1)" && { echo "release-arm-gate-selftest: FAIL — a changed event literal left the selftest green"; exit 1; }
  grep -q 'resolved' <<< "$out" || { echo "release-arm-gate-selftest: FAIL — the changed event literal did not move the table"; echo "$out"; exit 1; }
  echo "release-arm-gate-selftest: sabotage ok — a changed event literal turns the table red"
  # the rpm job narrowed back to x86_64 (the line it carried under the 2026-11-19 ruling) must be red on the rpm part
  awk '{ print } /^  rpm:/ { r = 1 } r && /^      publish: false$/ { print "      arch-matrix: '"'"'[\"x86_64\"]'"'"'"; r = 0 }' \
    .github/workflows/release.yml > "$tmp/rpm.yml"
  grep -q "^      arch-matrix: '\[\"x86_64\"\]'\$" "$tmp/rpm.yml" || { echo "release-arm-gate-selftest: FAIL — the x86_64-only rpm line was not planted"; exit 1; }
  out="$(RELEASE_YML="$tmp/rpm.yml" "$0" 2>&1)" && { echo "release-arm-gate-selftest: FAIL — an x86_64-only rpm arch-matrix left the selftest green"; exit 1; }
  grep -q "rpm's arch-matrix narrows" <<< "$out" || { echo "release-arm-gate-selftest: FAIL — the x86_64-only rpm line was not named"; echo "$out"; exit 1; }
  echo "release-arm-gate-selftest: sabotage ok — an x86_64-only rpm arch-matrix is red"
  # the rpm job gated like deb but with no aarch64 in its arm matrix must be red too
  awk '{ print } /^  rpm:/ { r = 1 } r && /^      publish: false$/ { print "      arch-matrix: ${{ (github.event_name != '"'"'push'"'"' && inputs.arm == false) && '"'"'[\"x86_64\"]'"'"' || '"'"'[\"x86_64\"]'"'"' }}"; r = 0 }' \
    .github/workflows/release.yml > "$tmp/rpmgate.yml"
  out="$(RELEASE_YML="$tmp/rpmgate.yml" "$0" 2>&1)" && { echo "release-arm-gate-selftest: FAIL — an rpm arm gate without aarch64 left the selftest green"; exit 1; }
  grep -q "rpm's arm matrix .* lacks aarch64" <<< "$out" || { echo "release-arm-gate-selftest: FAIL — the rpm gate without aarch64 was not named"; echo "$out"; exit 1; }
  echo "release-arm-gate-selftest: sabotage ok — an rpm arm gate without aarch64 is red"
  exit 0
fi

extract() { # <job> -> sets EVENT_LIT, ARM_ARR, NOARM_ARR; fails if the line is not the gate shape
  local line
  line="$(awk -v job="$1" '
    $0 ~ "^  " job ":" { injob = 1 }
    injob && /^  [a-z]+:$/ && $0 !~ "^  " job ":" { injob = 0 }
    injob && /arch-matrix:/ { print; exit }
  ' "$REL")"
  if [[ "$line" =~ \(github\.event_name\ !=\ \'([^\']*)\'\ \&\&\ inputs\.arm\ ==\ false\)\ \&\&\ \'([^\']*)\'\ \|\|\ \'([^\']*)\' ]]; then
    EVENT_LIT="${BASH_REMATCH[1]}"; NOARM_ARR="${BASH_REMATCH[2]}"; ARM_ARR="${BASH_REMATCH[3]}"
    ARM_LIT=false
    return 0
  fi
  EVENT_LIT=""; ARM_ARR=""; NOARM_ARR=""; ARM_LIT=""
  return 1
}

gate() { # <event> <inputs.arm: true|false|null> -> "arm" or "noarm": the expression, evaluated with the extracted literals
  local arm="$2"
  [ "$arm" = null ] && arm=false   # GitHub expressions: null == false is true (both coerce to 0)
  if [ "$1" != "$EVENT_LIT" ] && [ "$arm" = "$ARM_LIT" ]; then echo noarm; else echo arm; fi
}

check_job() { # <job> <arm architecture name>
  local job="$1" arch="$2"
  if ! extract "$job"; then
    echo "release-arm-gate-selftest: FAIL — $job's arch-matrix is not the (event!='push' && inputs.arm==false) gate"
    fails=$((fails + 1)); return
  fi
  [ "$EVENT_LIT" = push ] || {
    echo "release-arm-gate-selftest: FAIL — $job gates on event '$EVENT_LIT', not 'push'"
    fails=$((fails + 1))
  }
  [[ "$ARM_ARR" == *"$arch"* ]] || {
    echo "release-arm-gate-selftest: FAIL — $job's arm matrix '$ARM_ARR' lacks $arch"
    fails=$((fails + 1))
  }
  [[ "$NOARM_ARR" != *"$arch"* ]] || {
    echo "release-arm-gate-selftest: FAIL — $job's no-arm matrix '$NOARM_ARR' still carries $arch"
    fails=$((fails + 1))
  }

  while read -r event arm want; do
    got="$(gate "$event" "$arm")"
    if [ "$got" != "$want" ]; then
      echo "release-arm-gate-selftest: FAIL — $job: event=$event inputs.arm=$arm resolved $got, want $want"
      fails=$((fails + 1))
    fi
  done << 'EOF'
push null arm
workflow_dispatch null noarm
workflow_dispatch false noarm
workflow_call false noarm
pull_request null noarm
EOF
}

# rpm builds Fedora aarch64 beside x86_64: the owner ruling of 2026-11-26 (the full matrix for every package) supersedes
# the 2026-11-19 one that built x86_64 alone. The rpm job either names no arch-matrix, and takes build-rpm.yml's default
# (x86_64 and aarch64), or carries the same gate as deb with aarch64 in its arm matrix; a fixed list without aarch64,
# '["x86_64"]' among them, is red.
rpm_line="$(awk '
  /^  rpm:/ { injob = 1 }
  injob && /^  [a-z]+:$/ && !/^  rpm:/ { injob = 0 }
  injob && /arch-matrix:/ { print; exit }
' "$REL")"
if [ -n "$rpm_line" ]; then
  if extract rpm; then
    check_job rpm aarch64
  else
    echo "release-arm-gate-selftest: FAIL — rpm's arch-matrix narrows the shared default (x86_64 and aarch64) without the arm gate: $rpm_line"
    fails=$((fails + 1))
  fi
fi
check_job deb arm64

if [ "$fails" -ne 0 ]; then
  echo "release-arm-gate-selftest: FAIL — $fails part(s) of the arm gate wrong"
  exit 1
fi
echo "release-arm-gate-selftest: ok — deb arch-matrix builds arm64 on every tag push, opt-in only on dispatch; rpm builds aarch64 beside x86_64"
