#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
#
# check-package-derived.sh — make check-package (the package build's check) is DERIVED from make test's list, never
# written a second time: CHECK_PACKAGE is an expression over TEST_TARGETS and PROVE_TESTS (what proves the tests
# rather than the library), so a target added to make test is in the package check by construction.
#
#   tools/check-package-derived.sh [Makefile]   the check, over this tree's Makefile (or the one given)
#   tools/check-package-derived.sh --self-test  a hand-written CHECK_PACKAGE list must be named
set -euo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"

check() {
  local mk="$1" def rc=0
  def=$(awk '/^CHECK_PACKAGE[ :]*= / { on = 1 }
             on { more = /\\$/; sub(/\\$/, ""); printf "%s ", $0; if (!more) exit }' "$mk")
  [ -n "$def" ] || { echo "check-package-derived: $mk names no CHECK_PACKAGE"; return 1; }
  case "$def" in
    *'$(filter-out'*'$(PROVE_TESTS)'*'$(TEST_TARGETS)'*) ;;
    *) echo "check-package-derived: CHECK_PACKAGE is not derived from TEST_TARGETS and PROVE_TESTS (a hand-written list)"; rc=1 ;;
  esac
  grep -q '^test-parallel: \$(TEST_TARGETS)' "$mk" || { echo "check-package-derived: test-parallel is not \$(TEST_TARGETS)"; rc=1; }
  return $rc
}

if [ "${1:-}" = "--self-test" ]; then
  tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
  check "$HERE/Makefile" > /dev/null || { echo "check-package-derived: self-test: the tree's Makefile is not green"; exit 1; }
  sed 's/^CHECK_PACKAGE :\{0,1\}= .*/CHECK_PACKAGE = lib suite negative/;/^CHECK_PACKAGE/{n;/^ /d}' "$HERE/Makefile" > "$tmp/hand.mk"
  out=$(check "$tmp/hand.mk" || true)
  grep -q 'a hand-written list' <<<"$out" || { echo "check-package-derived: self-test: a hand-written list went unnamed"; exit 1; }
  echo "check-package-derived: self-test green"
  exit 0
fi

check "${1:-$HERE/Makefile}"
echo "check-package-derived: make check-package is derived from make test's list"
