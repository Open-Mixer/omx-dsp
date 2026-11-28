#!/usr/bin/env sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# contract-include-selftest.sh — holds tools/contract-include.sh (FreeMixer/omx-dsp#95, #107). It renders a
# scratch omx-contract package at several versions and runs the script against each: whatever version is installed
# is taken (no version is typed: owner ruling 2026-11-24, tools/no-floor-check.sh), and with nothing installed it
# is refused naming the packages. The spec, debian/control and omxdsp.pc.in carry no ceiling, and no tracked file
# may name a pins file again. No network is used.
set -eu
HERE="$(cd "$(dirname "$0")/.." && pwd)"
S="$(mktemp -d)"; trap 'rm -rf "$S"' EXIT
bad=0
fail() { echo "contract-include-selftest: FAIL — $*" >&2; bad=1; }

# No ceiling on a dependency between our libraries (owner rule 2026-11-16).
missing() {  # tree: every place a ceiling on omx-contract has crept back in
  grep -nE '^(Build)?Requires: omx-contract-(devel|data) <[^=]' "$1/packaging/omx-dsp.spec" | sed 's/^/spec ceiling: /'
  grep -nE 'omx-contract(-data)?-?(dev)? \(<< ' "$1/debian/control" | sed 's/^/control ceiling: /'
  grep -nE 'omx-contract *<' "$1/omxdsp.pc.in" | sed 's/^/omxdsp.pc.in ceiling: /'
}
[ -z "$(missing "$HERE")" ] || fail "a ceiling on omx-contract is back: $(missing "$HERE" | tr '\n' ';')"
# Positive control: the scan sees a ceiling put back in each file.
mkdir -p "$S/ceil/packaging" "$S/ceil/debian"
cp "$HERE/packaging/omx-dsp.spec" "$S/ceil/packaging/"; cp "$HERE/debian/control" "$S/ceil/debian/"; cp "$HERE/omxdsp.pc.in" "$S/ceil/"
[ -z "$(missing "$S/ceil")" ] || fail "the scratch copy is not clean before the sabotage"
echo 'Requires: omx-contract-data < 4' >> "$S/ceil/packaging/omx-dsp.spec"
[ -n "$(missing "$S/ceil")" ] || fail "the scan does not see a spec ceiling put in; it would prove nothing"
cp "$HERE/packaging/omx-dsp.spec" "$S/ceil/packaging/"; sed -i 's/libomx-contract-dev,/libomx-contract-dev (<< 4~),/' "$S/ceil/debian/control"
[ -n "$(missing "$S/ceil")" ] || fail "the scan does not see a debian/control ceiling put in; it would prove nothing"
cp "$HERE/debian/control" "$S/ceil/debian/"; sed -i 's/^Requires: omx-contract$/Requires: omx-contract < 4/' "$S/ceil/omxdsp.pc.in"
[ -n "$(missing "$S/ceil")" ] || fail "the scan does not see an omxdsp.pc.in ceiling put in; it would prove nothing"

# A scratch package. omx-contract.json carries one kernel, pretty-printed as the release ships it.
mkdir -p "$S/lib/pkgconfig" "$S/include/omxcontract" "$S/share/omx-contract"
: > "$S/include/omxcontract/omx_contract.h"
printf '%s\n' '{' '  "name": "x",' '  "kernels": {' '    "gate": {' '      "controls": [' '        {' '          "name": "keySource",' \
  '          "kind": "choice"' '        }' '      ]' '    }' '  }' '}' > "$S/share/omx-contract/omx-contract.json"
render() {
  printf 'prefix=%s\nincludedir=${prefix}/include\ndatadir=${prefix}/share/omx-contract\nName: omx-contract\nDescription: scratch\nVersion: %s\nCflags: -I${includedir}\n' \
    "$S" "$1" > "$S/lib/pkgconfig/omx-contract.pc"
}
# $1: the tree whose tools/contract-include.sh runs; the rest are its arguments.
run() { t="$1"; shift; env -u OMX_CONTRACT_INC PKG_CONFIG_PATH="$S/lib/pkgconfig" PKG_CONFIG_LIBDIR="$S/lib/pkgconfig" \
        http_proxy=http://127.0.0.1:9 https_proxy=http://127.0.0.1:9 sh "$t/tools/contract-include.sh" "$@"; }

# Any installed version is taken: the tree builds against what the lower level published.
for v in 2.6.0 3.0.0 3.21.0 3.22.0 4.0.0; do
  render "$v"
  out="$(run "$HERE" 2>&1)" || { fail "omx-contract $v is installed and was refused: $out"; continue; }
  [ "$out" = "$S/include" ] || fail "$v printed '$out', not $S/include"
done

# Nothing installed.
rm -f "$S/lib/pkgconfig/omx-contract.pc"
if out="$(run "$HERE" 2>&1)"; then fail "no package installed and the script answered: $out"
else for word in omx-contract-devel libomx-contract-dev 'not installed'; do
  case "$out" in *"$word"*) ;; *) fail "the refusal with nothing installed does not name '$word': $out" ;; esac
done; fi

# debian/changelog is rewritten from the commit subjects at build time, and history may name what it removed, so the scan leaves changelogs out.
# The removed paths stay removed: no download, no exact version, no pins file anywhere in the tree.
if grep -nE 'curl|wget|codeload|release\.tgz|exact-version' "$HERE/tools/contract-include.sh" | grep -v '^[0-9]*:#'; then
  fail "tools/contract-include.sh carries a download or exact-version path again"
fi
pins() { grep -rIl --exclude-dir=.git --exclude-dir=build --exclude=contract-include-selftest.sh --exclude=changelog -e 'pins\.txt' "$1" || true; }
mkdir -p "$S/plant/.github"; cp "$HERE/Makefile" "$S/plant/"; echo "X := \$(shell cat .github/pins.txt)" >> "$S/plant/Makefile"
[ -n "$(pins "$S/plant")" ] || fail "the pins-file scan does not see a planted line; it would prove nothing"
found="$(pins "$HERE")"; [ -z "$found" ] || fail "these files name a pins file: $found"
[ ! -e "$HERE/.github/pins.txt" ] || fail ".github/pins.txt is back"

# No ceiling in the prose either: BUILDING.md names none in any form (`< 5`, `<< 4~`, `<= 4`) (owner rule 2026-11-16).
prose="$(grep -nE '`<{1,2}=? *[0-9]' "$HERE/BUILDING.md" || true)"
[ -z "$prose" ] || fail "BUILDING.md states a ceiling on omx-contract: $prose"

# The Makefile finds the contract whenever a goal compiles, also beside a goal that needs none
# (`make copy-check test-parallel`): an empty CONTRACT_INC made `-isystem` swallow the next flag
# (-c, -o, -DOMX_CONTRACTS). A goal list of contract-free goals alone still asks for nothing.
# -B: an object already built still prints its compile; env -u: a parent make's CONTRACT_INC and flags stay out.
isys() {  # makefile goal...: the -isystem argument the compile of omx_align_fit.o would get
  mf="$1"; shift
  env -u CONTRACT_INC -u MAKEFLAGS -u MFLAGS -u MAKELEVEL OMX_CONTRACT_INC="$S/include" make --no-print-directory -n -B -C "$HERE" -f "$mf" "$@" 2>/dev/null \
    | sed -n 's/.*-isystem \([^ ]*\) .*src\/omx_align_fit\.c.*/\1/p' | head -n 1
}
got="$(isys "$HERE/Makefile" copy-check build/omx_align_fit.o)"
[ "$got" = "$S/include" ] || fail "make copy-check build/omx_align_fit.o compiles with -isystem '$got', not $S/include"
env -u CONTRACT_INC -u MAKEFLAGS -u MFLAGS -u MAKELEVEL OMX_CONTRACT_INC="$S/missing" make --no-print-directory -n -B -C "$HERE" copy-check >/dev/null 2>&1 \
  || fail "make copy-check alone asked for the contract (it compiles nothing against it)"
# Positive control: the goal test that skipped the lookup when any contract-free goal was named.
sed 's/^ifneq (\$(if \$(MAKECMDGOALS),\$(filter-out \$(NO_CONTRACT_GOALS),\$(MAKECMDGOALS)),all),)$/ifeq ($(filter $(NO_CONTRACT_GOALS),$(MAKECMDGOALS)),)/' \
  "$HERE/Makefile" > "$S/Makefile.goals"
! cmp -s "$HERE/Makefile" "$S/Makefile.goals" || fail "the goal-test sabotage did not apply; it would prove nothing"
got="$(isys "$S/Makefile.goals" copy-check build/omx_align_fit.o)"
[ "$got" != "$S/include" ] || fail "the check does not see the contract lookup skipped beside copy-check; it would prove nothing"

[ "$bad" = 0 ] || exit 1
echo "contract-include-selftest: no version typed and no ceiling in the spec, debian/control and omxdsp.pc.in; every installed version taken, nothing installed refused naming the package; no download path, no pins file; the Makefile finds it beside a contract-free goal"
