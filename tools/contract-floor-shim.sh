#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# contract-floor-shim.sh — the build refuses an omx-contract below its floor, proved with a contract this script
# makes itself: a scratch omx-contract.pc (and the header path tools/contract-include.sh looks for) at a version of our
# choosing, first on PKG_CONFIG_PATH. The refusal is pkg-config's word alone (contract-include.sh:
# pkg-config --atleast-version=<floor> omx-contract), so no old omx-contract has to be served by the channel, which
# keeps only its newest versions (omx-dsp#18).
#
#   contract-floor-shim.sh --check [OLD]   the three arms below (OLD default 2.6.0, which must be below the floor)
#   contract-floor-shim.sh --make VERSION DIR   write the shim for VERSION under DIR; prints DIR/pkgconfig
#
# Arms of --check:
#   1. a shim at OLD: `make lib` fails before compiling anything, naming omx-contract, the floor and OLD;
#   2. the sabotage: the same shim at the floor version is ACCEPTED (contract-include.sh prints the shim's include
#      directory), so arm 1 is red for the version and nothing else;
#   3. the CI job that runs this installs no omx-contract from the channel to prove the refusal.
set -uo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"

make_shim() {  # VERSION DIR
  mkdir -p "$2/pkgconfig" "$2/include/omxcontract"
  : > "$2/include/omxcontract/omx_contract_limits.h"
  cat > "$2/pkgconfig/omx-contract.pc" <<PC
prefix=$2
includedir=\${prefix}/include
datadir=\${prefix}/share/omx-contract
contractdir=\${prefix}/share/omx-contract/contract

Name: omx-contract
Description: a below-floor stand-in made by tools/contract-floor-shim.sh
Version: $1
Cflags: -I\${includedir}
PC
  printf '%s\n' "$2/pkgconfig"
}

case "${1:-}" in
  --make) [ $# = 3 ] || { echo "usage: $0 --make VERSION DIR" >&2; exit 2; }; make_shim "$2" "$3"; exit 0 ;;
  --check) ;;
  *) echo "usage: $0 --check [OLD] | --make VERSION DIR" >&2; exit 2 ;;
esac
OLD="${2:-2.6.0}"
floor="$(sh "$HERE/tools/contract-include.sh" --floor)" || exit 1
S="$(mktemp -d)"; trap 'rm -rf "$S"' EXIT
bad=0
fail() { echo "contract-floor-shim: FAIL — $*"; bad=1; }

# 1. below the floor: refused before anything compiles, naming the package, the floor and the version
pc="$(make_shim "$OLD" "$S/old")"
PKG_CONFIG_PATH="$pc" pkg-config --atleast-version="$floor" omx-contract && fail "OLD=$OLD is not below the floor $floor"
cp -a "$HERE" "$S/tree" 2>/dev/null; rm -rf "$S/tree/build"
( cd "$S/tree" && PKG_CONFIG_PATH="$pc" make lib > "$S/out.txt" 2>&1 ); rc=$?
if [ "$rc" = 0 ]; then fail "make lib built against omx-contract $OLD, below the floor $floor"; fi
! ls "$S/tree/build/"*.o >/dev/null 2>&1 || fail "make lib compiled before refusing"
for word in omx-contract "$floor" "$OLD"; do
  grep -q "contract-include:.*$word" "$S/out.txt" || fail "the refusal does not name '$word': $(head -3 "$S/out.txt")"
done
[ "$bad" = 0 ] && echo "ok   omx-contract $OLD (a shim, nothing served) is refused below the floor $floor: $(grep -m1 'contract-include:' "$S/out.txt")"

# 2. sabotage: the shim at the floor is accepted
pc="$(make_shim "$floor" "$S/floor")"
got="$(PKG_CONFIG_PATH="$pc" sh "$HERE/tools/contract-include.sh" 2>"$S/err.txt")"; rc=$?
if [ "$rc" = 0 ] && [ "$got" = "$S/floor/include" ]; then
  echo "ok   the sabotage: the same shim at the floor $floor is accepted ($got)"
else
  fail "the shim at the floor $floor is not accepted (rc $rc, '$got', $(cat "$S/err.txt"))"
fi

# 3. the job proves the refusal with this script, never with an old served omx-contract
job="$(awk '/^  contract-floor:/{p=1;next} p&&/^  [a-z]/{p=0} p' "$HERE/.github/workflows/ci.yml")"
[ -n "$job" ] || fail "ci.yml has no contract-floor job"
if printf '%s\n' "$job" | grep -qE 'omx-contract-devel-\$|libomx-contract-dev=\$|add-openmixer'; then
  fail "ci.yml's contract-floor job installs an old omx-contract from the channel: the channel keeps only its newest versions"
fi
printf '%s\n' "$job" | grep -q 'tools/contract-floor-shim.sh --check' || fail "ci.yml's contract-floor job does not run tools/contract-floor-shim.sh --check"
[ "$bad" = 0 ] && echo "ok   ci.yml's contract-floor job serves itself its below-floor contract"

# 4. the job runs where it is not starved (omx-dsp#16): a cheap job on GitHub's hosted runners, never the shared
#    self-hosted pool whose saturated disks left its debian cell cancelled in the package install; and its install is
#    bounded by its own timeout, so a starved install is a red naming the step, never a cancel at the job's timeout
printf '%s\n' "$job" | grep -qE '^    runs-on: ubuntu-latest$' || fail "ci.yml's contract-floor job does not run on ubuntu-latest (omx-dsp#16)"
printf '%s\n' "$job" | awk '/- name: Install the build tools/{f=1;next} f&&/^      - /{exit} f&&/timeout-minutes:/{ok=1} END{exit !ok}' \
  || fail "ci.yml's contract-floor job does not bound 'Install the build tools' with its own timeout-minutes (omx-dsp#16)"
[ "$bad" = 0 ] && echo "ok   ci.yml's contract-floor job runs on GitHub's runners, its install bounded by its own timeout"
exit "$bad"
