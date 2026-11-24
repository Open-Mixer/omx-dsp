#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# face-descriptor-check.sh — every kernel the generic face serves (include/omxdsp/fx/omx_fx_kernels.h) carries
# a descriptor that is its kernel file, data/kernels/<k>.json: the same controls, by name, in the declared
# order, of the same kind, with the same range, default and unit, and the same readouts (docs/generic-face.md).
#
#   tools/face-descriptor-check.sh [WORK]              the check; WORK holds the dump program (default a scratch dir)
#   tools/face-descriptor-check.sh --self-test [WORK]  two rows swapped, a default moved and a unit changed in a
#                                                      descriptor, and a readout renamed in a kernel file, must each
#                                                      be named; the tree as it is must pass
#
# The descriptors are read by compiling tools/face-descriptor-dump.c against the headers ($CC, the contract
# include tools/contract-include.sh finds). The kernel file is read as text (the data files are written two
# spaces deep, one key per line) and rendered to the dump's rows:
#   - a travel: its min, max and default, a {"ref": NAME} read as the scalar constant NAME declares in data/,
#     and its unit as the PARAM_UNITS id whose symbol (x-symbol) it is, in omx-contract's param domain;
#   - a choice: 0, its last index and its default's index, unit none;
#   - a readout: its name, its unit's PARAM_UNITS id, its min and max.
# A control the descriptor cannot carry yet (a `count`, an `of`) is rendered as such, so it is named.
set -euo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"
CC="${CC:-cc}"
: "${CONTRACT_INC:=$(sh "$HERE/tools/contract-include.sh")}"
PARAM_JSON="$(pkg-config --variable=datadir omx-contract 2>/dev/null || true)/contract.d/param/param.json"
[ -f "$PARAM_JSON" ] || { echo "face-descriptor-check: no omx-contract param domain at $PARAM_JSON" >&2; exit 2; }

# units: "<symbol> <PARAM_UNITS id>" per unit of the param domain, the empty symbol as "-"
units() {
  awk '
    /"const": "/ { c = $0; sub(/.*"const": "/, "", c); sub(/".*/, "", c) }
    /"x-symbol": "/ { s = $0; sub(/.*"x-symbol": "/, "", s); sub(/".*/, "", s); if (s == "") s = "-"; if (!(s in seen)) { seen[s] = 1; print s, c } }
  ' "$PARAM_JSON"
}

# constants <data dir>: "<NAME> <value>" per scalar constant declared in any data file
constants() {
  find "$1" -name '*.json' -print0 | sort -z | xargs -0 awk '
    /^    "[A-Z][A-Z0-9_]*": \{$/ { n = $0; sub(/^    "/, "", n); sub(/".*/, "", n); next }
    n != "" && /^      "value": -?[0-9.eE+-]+,?$/ { v = $0; sub(/^      "value": /, "", v); sub(/,$/, "", v); print n, v; n = ""; next }
    /^    \}/ { n = "" }
  '
}

# rows <kernel.json> <units file> <constants file>: the kernel file as the dump prints its descriptor
rows() {
  awk -v UF="$2" -v CF="$3" '
    BEGIN {
      while ((getline l < UF) > 0) { split(l, a, " "); unit[a[1]] = a[2] }
      while ((getline l < CF) > 0) { split(l, a, " "); if (!(a[1] in cst)) cst[a[1]] = a[2] }
    }
    function str(s) { sub(/^[^:]*: "/, "", s); sub(/",?$/, "", s); return s }
    function num(s) { sub(/^[^:]*: /, "", s); sub(/,$/, "", s); return s + 0 }
    function uid(s) { if (s == "") s = "-"; return (s in unit) ? unit[s] : "UNKNOWN-UNIT(" s ")" }
    function f(x) { return sprintf("%.7g", x) }
    /^  "controls": \[/ { sect = "c"; next }
    /^  "readouts": \[/ { sect = "r"; next }
    /^  \]/ { sect = "" }
    sect != "" && /^    \{$/ { name = kind = tu = dflt = ""; mn = mx = dv = 0; nids = 0; extra = ""; next }
    sect != "" && /^      "name": "/ { name = str($0) }
    sect == "c" && /^      "kind": "/ { kind = str($0) }
    sect == "c" && /^      "(count|of)": / { k = $0; sub(/^      "/, "", k); sub(/".*/, "", k); extra = extra " " k }
    sect == "c" && /^      "ids": \[/ { inids = 1; next }
    inids && /^      \]/ { inids = 0; next }
    inids { id = $0; gsub(/^ *"|",?$/, "", id); ids[nids++] = id; next }
    sect == "c" && /^      "default": "/ { dflt = str($0) }
    sect == "c" && /^        "(min|max|default)": \{$/ { ref = $0; sub(/^        "/, "", ref); sub(/".*/, "", ref); next }
    ref != "" && /^          "ref": "/ { r = str($0); v = (r in cst) ? cst[r] : "UNRESOLVED(" r ")"
      if (ref == "min") mn = v; else if (ref == "max") mx = v; else dv = v; ref = ""; next }
    sect == "c" && /^        "min": -?[0-9]/ { mn = num($0) }
    sect == "c" && /^        "max": -?[0-9]/ { mx = num($0) }
    sect == "c" && /^        "default": -?[0-9]/ { dv = num($0) }
    sect == "c" && /^        "unit": "/ { tu = str($0) }
    sect == "r" && /^      "unit": "/ { tu = str($0) }
    sect == "r" && /^      "min": -?[0-9]/ { mn = num($0) }
    sect == "r" && /^      "max": -?[0-9]/ { mx = num($0) }
    sect != "" && /^    \}/ {
      if (sect == "r") { print "meter", name, uid(tu), f(mn), f(mx); next }
      if (extra != "") { print "param", name, kind, "carries" extra; next }
      if (kind == "choice") {
        di = "NO-DEFAULT(" dflt ")"; for (i = 0; i < nids; i++) if (ids[i] == dflt) di = i
        print "param", name, "choice", 0, nids - 1, di, "none"
      } else {
        print "param", name, kind, (mn ~ /^UNRESOLVED/ ? mn : f(mn)), (mx ~ /^UNRESOLVED/ ? mx : f(mx)), (dv ~ /^UNRESOLVED/ ? dv : f(dv)), uid(tu)
      }
    }
  ' "$1"
}

# check <include dir> <data dir> <work>: 0 when every served kernel's descriptor is its kernel file
check() {
  local inc="$1" data="$2" work="$3" bad=0 n=0 k want got
  mkdir -p "$work"
  "$CC" -std=c11 -Wall -Wextra -Werror -I"$inc" -isystem "$CONTRACT_INC" -o "$work/dump" "$HERE/tools/face-descriptor-dump.c" -lm
  "$work/dump" > "$work/dump.txt"
  units > "$work/units.txt"
  constants "$data" > "$work/constants.txt"
  for k in $(sed -n 's/^kernel //p' "$work/dump.txt"); do
    n=$((n + 1))
    if [ ! -f "$data/kernels/$k.json" ]; then echo "face-descriptor-check: $k: no kernel file data/kernels/$k.json" >&2; bad=1; continue; fi
    want="$(rows "$data/kernels/$k.json" "$work/units.txt" "$work/constants.txt")"
    got="$(awk -v k="$k" '$1 == "kernel" { on = ($2 == k); next } on' "$work/dump.txt")"
    [ "$got" = "$want" ] && continue
    echo "face-descriptor-check: $k's descriptor is not data/kernels/$k.json:" >&2
    diff <(printf '%s\n' "$want") <(printf '%s\n' "$got") | sed -n 's/^</  kernel file:/p; s/^>/  descriptor: /p' >&2 || true
    bad=1
  done
  [ "$n" -gt 0 ] || { echo "face-descriptor-check: the generic face serves no kernel; the check would prove nothing" >&2; return 2; }
  [ "$bad" = 0 ] && echo "face-descriptor-check: $n descriptors are their kernel files, by name, order, kind, range, default, unit and readout"
  return "$bad"
}

if [ "${1:-}" = --self-test ]; then
  work="${2:-}"; [ -n "$work" ] || { work="$(mktemp -d)"; trap 'rm -rf "$work"' EXIT; }
  mkdir -p "$work"
  s="$work/self"; rm -rf "$s"; mkdir -p "$s"
  cp -r "$HERE/include" "$s/include"; cp -r "$HERE/data" "$s/data"
  check "$s/include" "$s/data" "$s/w" >/dev/null || { echo "face-descriptor-check self-test: the tree as it is does not pass" >&2; exit 1; }
  # name <what> <pattern>: the perturbed tree must fail, naming the pattern
  name() {
    local out
    out="$(check "$s/include" "$s/data" "$s/w" 2>&1)" && { echo "face-descriptor-check self-test: $1 passed" >&2; exit 1; }
    grep -q -- "$2" <<<"$out" || { echo "face-descriptor-check self-test: $1 was NOT named ($2)" >&2; printf '%s\n' "$out" >&2; exit 1; }
  }
  w="$s/include/omxdsp/omx_width.h"; t="$s/include/omxdsp/omx_trim.h"; c="$s/include/omxdsp/omx_clipper.h"
  # Two rows swapped: the width's pseudoMs and bassMonoHz change places.
  awk '/\{"pseudoMs",/ { a = $0; next } /\{"bassMonoHz",/ { print; print b; print a; next } /\{"bassMono",/ { b = $0; next } { print }' \
    "$HERE/include/omxdsp/omx_width.h" > "$w"
  name "the width's pseudoMs and bassMonoHz swapped" "descriptor: param bassMonoHz travel 20 300 120 Hz"
  cp "$HERE/include/omxdsp/omx_width.h" "$w"
  # A default moved: the trim comes up at 1 dB.
  sed -i '/{"trimDb",/s/(float)OMX_TRIM_RANGE_DEFAULT/1.0f/' "$t"
  name "the trim's default moved to 1 dB" "descriptor: param trimDb travel -24 24 1 dB"
  cp "$HERE/include/omxdsp/omx_trim.h" "$t"
  # A unit changed: the clipper's softness in no unit.
  sed -i '/{"softnessPct",/s/OMX_PARAM_UNITS_PERCENT/OMX_PARAM_UNITS_NONE/' "$c"
  name "the clipper's softness unit changed" "descriptor: param softnessPct travel 0 100 0 none"
  cp "$HERE/include/omxdsp/omx_clipper.h" "$c"
  # This tree's kernel file is the one read: the clipper's readout renamed there must be named.
  sed -i 's/^      "name": "gainReductionDb",$/      "name": "reductionDb",/' "$s/data/kernels/clipper.json"
  name "the clipper's readout renamed in its kernel file" "kernel file: meter reductionDb dB -60 0"
  cp "$HERE/data/kernels/clipper.json" "$s/data/kernels/clipper.json"
  check "$s/include" "$s/data" "$s/w" >/dev/null || { echo "face-descriptor-check self-test: the restored tree does not pass" >&2; exit 1; }
  echo "face-descriptor-check self-test: two rows swapped, a default moved, a unit changed and a readout renamed in a kernel file were each named"
  exit 0
fi

work="${1:-}"; [ -n "$work" ] || { work="$(mktemp -d)"; trap 'rm -rf "$work"' EXIT; }
check "$HERE/include" "$HERE/data" "$work"
