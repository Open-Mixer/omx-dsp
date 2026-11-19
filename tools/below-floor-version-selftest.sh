#!/usr/bin/env sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# below-floor-version-selftest.sh — holds tools/below-floor-version.sh, the picker the contract-floor job
# uses to choose the omx-contract-devel it installs below the floor. The cases: the newest published version
# strictly below the floor is picked (never the newest overall, never the floor itself); an empty below-floor
# set is refused by name; with --deb the comparison is dpkg's. The sabotage arm runs the same cases against a
# scratch copy that picks the newest overall, and requires them to go red.
set -eu
HERE="$(cd "$(dirname "$0")/.." && pwd)"
PICK="$HERE/tools/below-floor-version.sh"
S="$(mktemp -d)"; trap 'rm -rf "$S"' EXIT
bad=0
fail() { echo "below-floor-version-selftest: FAIL — $*" >&2; bad=1; }

[ -f "$PICK" ] || { echo "below-floor-version-selftest: FAIL — $PICK does not exist" >&2; exit 1; }

# check PICKER WANT_RC WANT_OUT NAME [FLOOR-ARGS...] < versions
# WANT_RC 0: the picker answers WANT_OUT. WANT_RC 1: it refuses, and the refusal names the case.
check() {
  picker="$1"; want_rc="$2"; want="$3"; name="$4"; shift 4
  rc=0; got="$(sh "$picker" "$@" 2>"$S/err")" || rc=$?
  if [ "$want_rc" = 0 ]; then
    [ "$rc" = 0 ] || { fail "$name: refused ($(cat "$S/err")), expected $want"; return 0; }
    [ "$got" = "$want" ] || fail "$name: picked '$got', expected '$want'"
  else
    [ "$rc" != 0 ] || { fail "$name: picked '$got', expected a refusal"; return 0; }
    case "$(cat "$S/err")" in
      *"no published omx-contract-devel below the floor"*) ;;
      *) fail "$name: the refusal does not name the case: $(cat "$S/err")" ;;
    esac
  fi
}

cases() {
  p="$1"
  printf '%s\n' 2.6.0 3.17.0 3.19.0 3.19.1 3.20.0 > "$S/list1"
  check "$p" 0 3.17.0 "picks 3.17.0 under 3.18.0" 3.18.0 < "$S/list1"

  printf '%s\n' 3.18.0 3.19.0 3.20.0 > "$S/list2"
  check "$p" 1 "" "refuses an empty below-floor set" 3.18.0 < "$S/list2"

  printf '%s\n' 3.18.0 > "$S/list3"
  check "$p" 1 "" "never returns the floor itself" 3.18.0 < "$S/list3"
  printf '%s\n' 3.18.0 3.17.9 3.17.0 > "$S/list3b"
  check "$p" 0 3.17.9 "skips the floor when a lower one exists" 3.18.0 < "$S/list3b"

  : > "$S/list4"
  check "$p" 1 "" "refuses an empty listing" 3.18.0 < "$S/list4"

  if command -v dpkg >/dev/null 2>&1; then
    printf '%s\n' 3.17.0-1 3.18.0~rc1-1 3.19.0-1 > "$S/list5"
    check "$p" 0 3.18.0~rc1-1 "--deb uses dpkg: 3.18.0~rc1 is below 3.18.0" --deb 3.18.0 < "$S/list5"
  fi

  rc=0; sh "$p" < "$S/list1" >/dev/null 2>&1 || rc=$?
  [ "$rc" != 0 ] || fail "no floor given and the picker answered"
}

cases "$PICK"
[ "$bad" = 0 ] || exit 1

# Sabotage: the newest overall, the defect this picker exists to avoid. The same cases must go red on it.
mkdir -p "$S/sab/tools"
sed '/lt .* || continue$/d' "$PICK" > "$S/sab/tools/below-floor-version.sh"
cmp -s "$PICK" "$S/sab/tools/below-floor-version.sh" && { echo "below-floor-version-selftest: FAIL — the sabotage changed nothing" >&2; exit 1; }
bad=0
cases "$S/sab/tools/below-floor-version.sh" 2>/dev/null
[ "$bad" != 0 ] || { echo "below-floor-version-selftest: FAIL — the newest-overall sabotage passes every case; the cases prove nothing" >&2; exit 1; }

dpkg_note=""
command -v dpkg >/dev/null 2>&1 || dpkg_note=" (dpkg is not here: the --deb case was not run)"
echo "below-floor-version-selftest: picks the newest published version below the floor (3.17.0 of 2.6.0 3.17.0 3.19.0 3.19.1 3.20.0 under 3.18.0); refuses an empty set by name; never the floor; the newest-overall sabotage is red${dpkg_note}"
