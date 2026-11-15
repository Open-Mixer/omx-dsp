#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# patch-names-sabotage.sh — tools/patch-names-check.sh's self-test is not blind. Each sabotage breaks one rule of
# the check in a copy, and the copy's --self-test must fail; a sabotage whose edit does not apply is a failure too.
# Run by `make lint`.
set -euo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$HERE/tools/patch-names-check.sh"
T=$(mktemp -d "${TMPDIR:-/tmp}/patch-names-sabotage.XXXXXX"); trap 'rm -rf "$T"' EXIT
fail=0 n=0
sabotage() { # <name> <sed expression>
  local copy="$T/$1.sh"
  sed -e "$2" "$SRC" > "$copy"
  if cmp -s "$SRC" "$copy"; then echo "patch-names-sabotage: $1 — the edit did not apply"; fail=1; return; fi
  n=$((n + 1))
  if bash "$copy" --self-test > /dev/null 2>&1; then echo "patch-names-sabotage: $1 — the self-test still passes"; fail=1
  else echo "patch-names-sabotage: $1 — the self-test fails, as it must"; fi
}
sabotage top-level-only    's|^HEADER_RE=.*|HEADER_RE='"'"'^include/omxdsp/[^/]*\\.h$'"'"'|'
sabotage no-functions      's/rec("function", /norec("function", /g'
sabotage no-macros         's/rec("macro", /norec("macro", /'
sabotage removal-allowed   's/{ bad++; print "  removed: "/{ print "  removed: "/'
sabotage change-allowed    's/} else if (now\[k\] != was\[k\]) { bad++; /} else if (now[k] != was[k]) { /'
sabotage members-unordered 's/if (!prefix(now\[k\], was\[k\]))/if (0)/'
sabotage additions-refused 's/{ added++; /{ added++; bad++; /'
sabotage first-tag-only    's/for ((k = 0; k < pat; k++))/for ((k = 0; k < 1; k++))/'
[ "$fail" -eq 0 ] && echo "patch-names-sabotage: all $n sabotages are caught"
exit "$fail"
