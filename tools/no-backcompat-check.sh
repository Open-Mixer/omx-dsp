#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
#
# no-backcompat-check.sh — no old-release machinery in this tree. The library is a static archive shipped with its
# headers, and its tests judge today's data, so none of these exist:
#   - the patch-release API gate (patch-names check, its sabotage, its proofs list),
#   - proofs pinned to the library's first release or an old commit (the v0.1.3 header, the after-pin lists, the
#     gap-names test),
#   - a read of an old omx-contract release fixture, of the frozen limits sheet, or of the engine definitions frozen at
#     an old commit (test/fixtures/omx-contract-2.*, openmixer-6ee8a9bde101, frozen-limits-sheet.json, limits-0.1.3),
#   - a Breaks or Replaces line on omx-dsp-contract in debian/control.
# The omx-contract change that deletes those fixtures can only merge once nothing here reads them.
#
#   tools/no-backcompat-check.sh [DIR]      the check over DIR (default this checkout); exit 1 names every hit
#   tools/no-backcompat-check.sh --self-test  plants each old thing in a scratch tree and requires red, then a clean tree green
# The history files (debian/changelog, the spec's %changelog) record what happened and are not read.
set -u
SELF=tools/no-backcompat-check.sh
PATTERN='patch-names-check|patch-names-sabotage|patch-count-proofs|contract-gap|after-pin|PIN_TAG|READS_PIN|ALIAS_REMOVAL|MOVED_AFTER_PIN|v0\.1\.3|omx-contract-2\.[0-9]|openmixer-6ee8a9bde101|frozen-limits-sheet|limits-0\.1\.3'

scan() {
  local d=$1 rc=0 f rel out
  while IFS= read -r f; do
    rel=${f#"$d"/}
    [ "$rel" = "$SELF" ] && continue
    [ "$rel" = debian/changelog ] && continue
    if [ "$rel" = packaging/omx-dsp.spec ]; then
      out=$(sed '/^%changelog/,$d' "$f" | grep -nE "$PATTERN")
    else
      out=$(grep -nIE "$PATTERN" "$f")
    fi
    if [ -n "$out" ]; then printf '%s\n' "$out" | sed "s|^|$rel:|"; rc=1; fi
  done < <(find "$d" \( -name .git -o -name node_modules -o -name build \) -prune -o -type f -print)
  if [ -f "$d/debian/control" ]; then
    awk '/^Package:/ { pkg = $2 } pkg == "omx-dsp-contract" && /^(Breaks|Replaces):/ { print "debian/control:" FNR ": " $0; bad = 1 } END { exit bad }' "$d/debian/control" || rc=1
  fi
  return $rc
}

if [ "${1:-}" = --self-test ]; then
  T=$(mktemp -d "${TMPDIR:-/tmp}/no-backcompat.XXXXXX"); trap 'rm -rf "$T"' EXIT
  fail=0
  fresh() { rm -rf "$T/t"; mkdir -p "$T/t/test" "$T/t/tools" "$T/t/debian" "$T/t/packaging"
    printf 'import x\n' > "$T/t/test/a.test.mjs"
    printf 'Package: omx-dsp-contract\nArchitecture: all\nDepends: x\n' > "$T/t/debian/control"
    printf 'Name: x\n%%changelog\n- patch-names-check once existed\n' > "$T/t/packaging/omx-dsp.spec"
    printf 'omx-dsp (0.1.3) unstable\n' > "$T/t/debian/changelog"; }
  fresh
  scan "$T/t" > /dev/null || { echo "no-backcompat-check --self-test: FAIL, a clean tree is red"; fail=1; }
  plant() { fresh; eval "$2"; if scan "$T/t" > /dev/null; then echo "no-backcompat-check --self-test: FAIL, $1 is not caught"; fail=1; fi; }
  plant "a read of an old release fixture" "echo \"readFileSync('test/fixtures/omx-contract-2.4.0/x.h')\" >> $T/t/test/a.test.mjs"
  plant "the frozen engine definitions" "echo 'test/fixtures/openmixer-6ee8a9bde101/e.h' >> $T/t/test/a.test.mjs"
  plant "the frozen limits sheet" "echo 'test/fixtures/frozen-limits-sheet.json' >> $T/t/test/a.test.mjs"
  plant "the v0.1.3 limits" "echo 'test/fixtures/limits-0.1.3/x' >> $T/t/test/a.test.mjs"
  plant "the v0.1.3 pin" "echo \"const PIN_TAG = 'v0.1.3';\" > $T/t/tools/p.mjs"
  plant "the after-pin lists" "echo \"import './after-pin.mjs'\" > $T/t/tools/p.mjs"
  plant "the patch-names check" "echo 'bash tools/patch-names-check.sh' > $T/t/tools/m.mk"
  plant "the patch-count proofs list" "echo 'x' > $T/t/tools/patch-count-proofs.txt; echo patch-count-proofs > $T/t/tools/n"
  plant "a Breaks line on omx-dsp-contract" "echo 'Breaks: libomxdsp-dev (<< 1)' >> $T/t/debian/control"
  plant "a Replaces line on omx-dsp-contract" "echo 'Replaces: libomxdsp-dev (<< 1)' >> $T/t/debian/control"
  plant "the old thing above the spec's %changelog" "sed -i '1i BuildRequires: patch-names-check' $T/t/packaging/omx-dsp.spec"
  [ "$fail" -eq 0 ] && echo "no-backcompat-check --self-test: each old thing planted is red, a clean tree is green"
  exit "$fail"
fi

DIR=${1:-.}
DIR=${DIR%/}
if scan "$DIR"; then echo "no-backcompat-check: none of the old-release machinery is in $DIR"; else
  echo "no-backcompat-check: the lines above are old-release machinery; remove them (no compatibility mechanism without the owner's approval)" >&2; exit 1; fi
