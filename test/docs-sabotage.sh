#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
#
# docs-sabotage.sh <dir> — `make docs-check` (the Doxygen documentation gate, #70) proven able to see
# its own defects, each on a COPY of the tree (a git repository of its own, so the tracked-only rule
# applies):
#   new         — an undocumented public function added to an installed header: red, naming it;
#   documented  — the same function with its doc block: green, no row (a documented function warns nothing);
#   listed      — the undocumented function's row in the baseline copy: green, the baseline is the one allowance;
#   swap        — one baseline row replaced by the function's row: red, naming the dropped row's warning as
#                 not listed (the baseline is compared as a set, never as a count);
#   nonl        — the listed baseline without its last newline: green, the same set;
#   grew        — the function's row added to the baseline copy, the tree's baseline as the base: red,
#                 the baseline only shrinks;
#   stale       — a baseline row nothing warns: red, the row must be dropped;
#   nodoxyfile  — no Doxyfile: red, naming it;
#   nobaseline  — no baseline: red, naming it;
#   untracked   — an untracked header under include/omxdsp: red, naming it;
#   update      — `make docs-baseline` on the 'new' copy: red, it may only lower, naming the row it refuses.
# The tree is never edited. Usage: docs-sabotage.sh <scratch-dir>
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
PKG="$(cd "$HERE/.." && pwd)"
OUT="${1:?scratch dir}"
PROBE=omx_docs_sabotage_probe
HEADER=include/omxdsp/fx/omx_instance_io.h
rm -rf "$OUT" && mkdir -p "$OUT"

copy() { # name [probe-doc-block]
  local d="$OUT/$1"
  mkdir -p "$d/docs"
  cp -r "$PKG/Makefile" "$PKG/Doxyfile" "$PKG/include" "$PKG/tools" "$PKG/packaging" "$d/"
  cp "$PKG/docs/doxygen-baseline.txt" "$d/docs/"
  printf '\n%sint %s(float x);\n' "${2:-}" "$PROBE" >> "$d/$HEADER"
  git -C "$d" init -q && git -C "$d" add -A && git -C "$d" -c user.name=sabotage -c user.email=sabotage@localhost commit -q -m copy
}
check() { # name expected(red|green) [target] [make args] ; the gate's output in $OUT/<name>.log
  local name=$1 want=$2 target=${3:-docs-check} rc=0; shift 2; [ $# -gt 0 ] && shift
  make -s -C "$OUT/$name" "$target" "$@" >"$OUT/$name.log" 2>&1 || rc=$?
  if [ "$want" = red ] && [ "$rc" -eq 0 ]; then
    echo "docs-sabotage.sh: FAIL — arm '$name' passed $target; the gate cannot see it"; return 1
  fi
  if [ "$want" = green ] && [ "$rc" -ne 0 ]; then
    echo "docs-sabotage.sh: FAIL — arm '$name' is red (rc $rc) where it must pass:"; cat "$OUT/$name.log"; return 1
  fi
  echo "docs-sabotage.sh: arm '$name' $want as expected"
}
names() { # name words... — the arm's log names every word
  local name=$1 w; shift
  for w in "$@"; do
    grep -q -- "$w" "$OUT/$name.log" || { echo "docs-sabotage.sh: FAIL — the red of arm '$name' does not name $w:"; cat "$OUT/$name.log"; return 1; }
  done
}
probe_row() { sed "s#$(cd "$OUT/new" && pwd -P)/##g" "$OUT/new/build/docs/warnings.txt" | grep "$PROBE"; }
add_rows() { # dir — stdin rows into the copy's baseline, kept sorted
  cat >> "$1/docs/doxygen-baseline.txt"
  LC_ALL=C sort -u -o "$1/docs/doxygen-baseline.txt" "$1/docs/doxygen-baseline.txt"
}

copy new
check new red
names new "$PROBE"

copy documented '/** @brief A probe.\n * @param x A word.\n * @return 0.\n * @thread audio */\n'
check documented green
! grep -q "$PROBE" "$OUT/documented/build/docs/warnings.txt" || { echo "docs-sabotage.sh: FAIL — the documented probe still warns:"; grep "$PROBE" "$OUT/documented/build/docs/warnings.txt"; exit 1; }

copy listed
probe_row | add_rows "$OUT/listed"
check listed green

copy swap
dropped=$(head -1 "$PKG/docs/doxygen-baseline.txt")
{ tail -n +2 "$PKG/docs/doxygen-baseline.txt"; probe_row; } | LC_ALL=C sort -u > "$OUT/swap/docs/doxygen-baseline.txt"
check swap red
names swap "$(printf '%s' "$dropped" | cut -c1-60)" "not in docs/doxygen-baseline.txt"

copy nonl
probe_row | add_rows "$OUT/nonl"
head -c -1 "$OUT/nonl/docs/doxygen-baseline.txt" > "$OUT/nonl/baseline.nonl" && mv "$OUT/nonl/baseline.nonl" "$OUT/nonl/docs/doxygen-baseline.txt"
check nonl green

copy grew
probe_row | add_rows "$OUT/grew"
check grew red docs-check DOCS_BASELINE_BASE="$PKG/docs/doxygen-baseline.txt"
names grew 'grew against' "$PROBE"

copy stale
{ probe_row; probe_row | sed "s/$PROBE/omx_docs_never_declared/g"; } | add_rows "$OUT/stale"
check stale red
names stale 'documented now' omx_docs_never_declared

copy nodoxyfile
rm "$OUT/nodoxyfile/Doxyfile"
check nodoxyfile red
names nodoxyfile 'Doxyfile is missing'

copy nobaseline
rm "$OUT/nobaseline/docs/doxygen-baseline.txt"
check nobaseline red
names nobaseline 'doxygen-baseline.txt is missing'

copy untracked
probe_row | add_rows "$OUT/untracked"
printf '/** @file omx_docs_scratch.h\n * @brief A scratch header nobody tracks. */\n' > "$OUT/untracked/include/omxdsp/omx_docs_scratch.h"
check untracked red
names untracked 'untracked headers' omx_docs_scratch.h

copy update
check update red docs-baseline
names update 'only lowers' "$PROBE"
cmp -s "$OUT/update/docs/doxygen-baseline.txt" "$PKG/docs/doxygen-baseline.txt" || { echo "docs-sabotage.sh: FAIL — docs-baseline rewrote the baseline it refused"; exit 1; }

echo "docs-sabotage.sh: ok — docs-check sees a new undocumented declaration, a swapped or stale row, a grown baseline, a missing Doxyfile or baseline and an untracked header; passes a documented or listed one and a set without its last newline; docs-baseline only lowers"
