#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
#
# check-package-derived.sh — make check-package (the package build's check) is DERIVED from make test's list, never
# written a second time: CHECK_PACKAGE is an expression over TEST_TARGETS and PROVE_TESTS (what proves the tests
# rather than the library), so a target added to make test is in the package check by construction.
#
#   tools/check-package-derived.sh [Makefile]   the check, over this tree's Makefile (or the one given)
#   tools/check-package-derived.sh --self-test  a hand-written CHECK_PACKAGE list must be named, and a check-package
#                                               rule that make reads before TEST_TARGETS exists must be refused
#
# A rule's prerequisites are expanded when make READS the rule: with TEST_TARGETS defined below check-package,
# $(CHECK_PACKAGE) is empty there and make check-package runs nothing (green by vacuity, in the rpm %check and the
# deb dh_auto_test). So the check also reads the prerequisites make really gave the rule (make's database) and holds
# them to the full expansion of $(CHECK_PACKAGE), never fewer than one.
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
  local got want
  got=$(make -C "$HERE" -f "$mk" -pnq check-package 2>/dev/null | awk '!seen && /^check-package:/ { sub(/^check-package:[ ]*/, ""); print; seen = 1 }' | tr -s ' ' '\n' | sed '/^$/d' | sort || true)
  want=$(make -C "$HERE" -f "$mk" -s --no-print-directory --eval 'print-check-package-list: ; @echo $(CHECK_PACKAGE)' print-check-package-list 2>/dev/null | tr -s ' ' '\n' | sed '/^$/d' | sort || true)
  if [ -z "$got" ]; then
    echo "check-package-derived: make check-package has NO test prerequisite (it runs nothing): is TEST_TARGETS defined below the rule?"; rc=1
  elif [ "$got" != "$want" ]; then
    echo "check-package-derived: make check-package's prerequisites are not \$(CHECK_PACKAGE) fully expanded"; rc=1
  fi
  return $rc
}

if [ "${1:-}" = "--self-test" ]; then
  tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
  check "$HERE/Makefile" > /dev/null || { echo "check-package-derived: self-test: the tree's Makefile is not green"; exit 1; }
  sed 's/^CHECK_PACKAGE :\{0,1\}= .*/CHECK_PACKAGE = lib suite negative/;/^CHECK_PACKAGE/{n;/^ /d}' "$HERE/Makefile" > "$tmp/hand.mk"
  out=$(check "$tmp/hand.mk" || true)
  grep -q 'a hand-written list' <<<"$out" || { echo "check-package-derived: self-test: a hand-written list went unnamed"; exit 1; }
  # sabotage: TEST_TARGETS moved below the check-package rule leaves the rule with no prerequisite
  awk '/^TEST_TARGETS = / { held = $0; next } { print } /^check-package: / { late = 1 } END { print held }' "$HERE/Makefile" > "$tmp/late.mk"
  out=$(check "$tmp/late.mk" || true)
  grep -q 'NO test prerequisite' <<<"$out" || { echo "check-package-derived: self-test: an empty check-package went unnamed"; exit 1; }
  echo "check-package-derived: self-test green"
  exit 0
fi

check "${1:-$HERE/Makefile}"
echo "check-package-derived: make check-package is derived from make test's list"
