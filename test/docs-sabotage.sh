#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# docs-sabotage.sh <dir> — `make docs-check` (the Doxygen documentation gate, #70) proven able to see
# its own defects, each on a COPY of the tree:
#   new    — an undocumented public function added to an installed header: red, naming it;
#   listed — the same with the function's row in the baseline copy: green, the baseline is the one allowance;
#   grew   — the function's row added to the baseline copy, the tree's baseline as the base: red,
#            the baseline only shrinks;
#   stale  — a baseline row nothing warns: red, the row must be dropped.
# The tree is never edited. Usage: docs-sabotage.sh <scratch-dir>
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
PKG="$(cd "$HERE/.." && pwd)"
OUT="${1:?scratch dir}"
PROBE=omx_docs_sabotage_probe
rm -rf "$OUT" && mkdir -p "$OUT"

copy() { # name
  local d="$OUT/$1"
  mkdir -p "$d/docs"
  cp -r "$PKG/Makefile" "$PKG/Doxyfile" "$PKG/include" "$PKG/tools" "$PKG/packaging" "$d/"
  cp "$PKG/docs/doxygen-baseline.txt" "$d/docs/"
  printf '\nint %s(float x);\n' "$PROBE" >> "$d/include/omxdsp/fx/omx_instance_io.h"
}
check() { # name expected(red|green) [make args] ; the gate's output in $OUT/<name>.log
  local name=$1 want=$2 rc=0; shift 2
  make -s -C "$OUT/$name" docs-check "$@" >"$OUT/$name.log" 2>&1 || rc=$?
  if [ "$want" = red ] && [ "$rc" -eq 0 ]; then
    echo "docs-sabotage.sh: FAIL — arm '$name' passed docs-check; the gate cannot see it"; return 1
  fi
  if [ "$want" = green ] && [ "$rc" -ne 0 ]; then
    echo "docs-sabotage.sh: FAIL — arm '$name' is red (rc $rc) where it must pass:"; cat "$OUT/$name.log"; return 1
  fi
  echo "docs-sabotage.sh: arm '$name' $want as expected"
}

copy new
check new red
grep -q "$PROBE" "$OUT/new.log" || { echo "docs-sabotage.sh: FAIL — the red of arm 'new' does not name $PROBE:"; cat "$OUT/new.log"; exit 1; }

probe_row() { sed "s#$(cd "$OUT/new" && pwd -P)/##g" "$OUT/new/build/docs/warnings.txt" | grep "$PROBE"; }
add_rows() { # dir — stdin rows into the copy's baseline, kept sorted
  cat >> "$1/docs/doxygen-baseline.txt"
  LC_ALL=C sort -u -o "$1/docs/doxygen-baseline.txt" "$1/docs/doxygen-baseline.txt"
}

copy listed
probe_row | add_rows "$OUT/listed"
check listed green

copy grew
probe_row | add_rows "$OUT/grew"
check grew red DOCS_BASELINE_BASE="$PKG/docs/doxygen-baseline.txt"
grep -q 'grew against' "$OUT/grew.log" || { echo "docs-sabotage.sh: FAIL — arm 'grew' is red for another reason:"; cat "$OUT/grew.log"; exit 1; }

copy stale
{ probe_row; probe_row | sed "s/$PROBE/omx_docs_never_declared/g"; } | add_rows "$OUT/stale"
check stale red
grep -q 'documented now' "$OUT/stale.log" || { echo "docs-sabotage.sh: FAIL — arm 'stale' is red for another reason:"; cat "$OUT/stale.log"; exit 1; }
echo "docs-sabotage.sh: ok — docs-check sees a new undocumented declaration, a grown baseline and a stale row, and passes a listed one"
