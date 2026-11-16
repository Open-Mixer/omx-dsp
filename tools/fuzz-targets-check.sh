#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# fuzz-targets-check.sh — the fuzz checks prove themselves in this tree, with the targets libsurface and
# libmackie carry: make fuzz-sabotage (each patch of fuzz/sabotage/ found by the property its header
# names), make fuzz-records (each check's mutant: and input: record met by its own check on its own seed)
# and make fuzz-coverage (each harness at or above its floor, every check reached). Part of make checks.
#
#   tools/fuzz-targets-check.sh [Makefile]   the check, over this tree's Makefile (or the one given)
#   tools/fuzz-targets-check.sh --self-test  a target dropped from the Makefile must be named
#
# A target is there when the Makefile has its rule, names it .PHONY, and the script its recipe runs exists.
set -euo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"
TARGETS="fuzz-sabotage fuzz-records fuzz-coverage"

# The Makefile is read as text, not through make, which needs omx-contract installed to parse it.
check() {
  local mk="$1" t rc=0 recipe s name="${1#"$HERE"/}"
  for t in $TARGETS; do
    if ! grep -q "^$t:" "$mk"; then
      echo "fuzz-targets-check: $name has no $t rule"; rc=1; continue
    fi
    if ! grep -E '^\.PHONY:' "$mk" | grep -qw -- "$t"; then
      echo "fuzz-targets-check: $name does not name $t .PHONY"; rc=1
    fi
    recipe=$(awk -v t="$t:" '$1 == t { on = 1; next } on && /^\t/ { print; next } on { exit }' "$mk")
    for s in $(grep -oE 'fuzz/[a-z_-]+\.sh' <<<"$recipe" | sort -u); do
      [ -f "$(dirname "$mk")/$s" ] || { echo "fuzz-targets-check: $t runs $s, which does not exist"; rc=1; }
    done
  done
  return $rc
}

if [ "${1:-}" = "--self-test" ]; then
  tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
  check "$HERE/Makefile" > /dev/null || { echo "fuzz-targets-check: self-test: the tree's Makefile is not green"; exit 1; }
  for t in $TARGETS; do
    sed "s/^$t:/$t-gone:/" "$HERE/Makefile" > "$tmp/Makefile"
    out=$(check "$tmp/Makefile" || true)
    grep -q "has no $t rule" <<<"$out" || { echo "fuzz-targets-check: self-test: a Makefile without $t went unnamed"; exit 1; }
  done
  echo "fuzz-targets-check: self-test green"
  exit 0
fi

check "${1:-$HERE/Makefile}"
echo "fuzz-targets-check: make fuzz-sabotage, fuzz-records and fuzz-coverage are there"
