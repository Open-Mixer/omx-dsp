#!/usr/bin/env sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# below-floor-version.sh — prints the newest published omx-contract-devel version strictly below the floor.
# The contract-floor job in .github/workflows/ci.yml uses it to pick the old contract it installs, so the
# below-floor refusal is proved against a package the current key still verifies (never a version typed
# into the workflow). tools/below-floor-version-selftest.sh holds it.
#
#   tools/below-floor-version.sh [--deb] FLOOR < versions
#
# versions: one published version per line, on stdin. The floor is the one the packaging declares
# (tools/contract-include.sh --floor). Comparison is sort -V for RPM versions, and dpkg --compare-versions
# with --deb. A version equal to the floor is never returned: the floor is not below itself.
#
# Prints the chosen version on stdout and nothing else. An empty below-floor set is refused, naming the
# floor, on stderr with exit 1.
set -eu

deb=0
if [ "${1:-}" = --deb ]; then deb=1; shift; fi
floor="${1:-}"
[ -n "$floor" ] || { echo "below-floor-version: usage: below-floor-version.sh [--deb] FLOOR < versions" >&2; exit 2; }
if [ "$deb" = 1 ]; then command -v dpkg >/dev/null 2>&1 || { echo "below-floor-version: --deb compares with dpkg, which is not installed" >&2; exit 2; }; fi

# lt A B: true when version A is strictly below version B.
lt() {
  if [ "$deb" = 1 ]; then
    dpkg --compare-versions "$1" lt "$2"
  else
    [ "$1" != "$2" ] && [ "$(printf '%s\n%s\n' "$1" "$2" | sort -V | head -n1)" = "$1" ]
  fi
}

best=""
while IFS= read -r v || [ -n "$v" ]; do
  v="$(printf '%s' "$v" | tr -d ' \t\r')"
  [ -n "$v" ] || continue
  lt "$v" "$floor" || continue
  if [ -z "$best" ] || lt "$best" "$v"; then best="$v"; fi
done

[ -n "$best" ] || { echo "below-floor-version: no published omx-contract-devel below the floor $floor" >&2; exit 1; }
printf '%s\n' "$best"
