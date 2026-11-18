#!/usr/bin/env sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# below-floor-version-selftest.sh — holds tools/below-floor-version.sh, the picker the contract-floor job
# uses to choose the omx-contract-devel it installs below the floor. The cases: the newest published version
# strictly below the floor is picked (never the newest overall, never the floor itself); an empty below-floor
# set is refused by name; the Debian comparison (dpkg) is the one used with --deb. The sabotage arm plants
# the defect the picker exists to avoid (the newest overall) in a scratch copy and requires a case to go red.
set -eu
HERE="$(cd "$(dirname "$0")/.." && pwd)"
PICK="$HERE/tools/below-floor-version.sh"
S="$(mktemp -d)"; trap 'rm -rf "$S"' EXIT
bad=0
fail() { echo "below-floor-version-selftest: FAIL — $*" >&2; bad=1; }

[ -f "$PICK" ] || { echo "below-floor-version-selftest: FAIL — $PICK does not exist" >&2; exit 1; }

# expect PICKER WANT_EXIT WANT_OUT NAME FLOOR [--deb] < versions
# Runs the picker, then checks its exit status and, when it answers, its one line of output.
# A refusal must name the floor, so a refusal with the wrong floor is red too.
check() {
  picker="$1"; want_rc="$2"; want="$3"; name="$4"; shift 4
  rc=0; got="$(sh "$picker" "$@" 2>"$S/err")" || rc=$?
  if [ "$want_rc" = 0 ]; then
    [ "$rc" = 0 ] || { fail "$name: refused ($(cat "$S/err")), expected $want"; return; }
    [ "$got" = "$want" ] || fail "$name: picked '$got', expected '$want'"
  else
    [ "$rc" != 0 ] || { fail "$name: picked '$got', expected a refusal"; return; }
    case "$(cat "$S/err")" in
      *"no published omx-contract-devel below the floor"*) ;;
      *) fail "$name: the refusal does not name the case: $(cat "$S/err")" ;;
    esac
  fi
}

# Case 1: the newest published version strictly below 3.18.0 is 3.17.0, not 3.20.0 nor 3.19.1.
printf '%s\n' 2.6.0 3.17.0 3.19.0 3.19.1 3.20.0 > "$S/list1"
check "$PICK" 0 3.17.0 "picks 3.17.0 under 3.18.0" 3.18.0 < "$S/list1"

# Case 2: an empty below-floor set is refused by name.
printf '%s\n' 3.18.0 3.19.0 3.20.0 > "$S/list2"
check "$PICK" 1 "" "refuses an empty below-floor set" 3.18.0 < "$S/list2"

# Case 3: the floor itself is never returned; with only the floor published the set is empty.
printf '%s\n' 3.18.0 > "$S/list3"
check "$PICK" 1 "" "never returns the floor itself" 3.18.0 < "$S/list3"
printf '%s\n' 3.18.0 3.17.9 3.17.0 > "$S/list3b"
check "$PICK" 0 3.17.9 "the floor is skipped when a lower one exists" 3.18.0 < "$S/list3b"

# Case 4: nothing published at all is refused by name.
: > "$S/list4"
check "$PICK" 1 "" "refuses an empty listing" 3.18.0 < "$S/list4"

# Case 5: with --deb the comparison is dpkg's: 3.18.0~rc1 is below 3.18.0 and is newer than 3.17.0.
printf '%s\n' 3.17.0-1 3.18.0~rc1-1 3.19.0-1 > "$S/list5"
if command -v dpkg >/dev/null 2>&1; then
  check "$PICK" 0 3.18.0~rc1-1 "--deb uses dpkg: 3.18.0~rc1 is below 3.18.0" --deb 3.18.0 < "$S/list5"
else
  echo "below-floor-version-selftest: dpkg is not here, the --deb case is not run"
fi

# Case 6: no floor at all is a usage error, not a pick.
rc=0; sh "$PICK" < "$S/list1" >/dev/null 2>&1 || rc=$?
[ "$rc" != 0 ] || fail "no floor given and the picker answered"

# Sabotage: the newest overall, which is the defect this picker exists to avoid. The scratch copy must
# fail the 3.17.0 case, or the case proves nothing.
mkdir -p "$S/sab/tools"
sed '/lt "\$v" "\$floor" || continue/d' "$PICK" > "$S/sab/tools/below-floor-version.sh"
cmp -s "$PICK" "$S/sab/tools/below-floor-version.sh" && fail "the sabotage did not change the picker; it would prove nothing"
got="$(sh "$S/sab/tools/below-floor-version.sh" 3.18.0 < "$S/list1" 2>/dev/null || true)"
[ "$got" != 3.17.0 ] || fail "the newest-overall sabotage still picks 3.17.0; the case does not see it"

[ "$bad" = 0 ] || exit 1
echo "below-floor-version-selftest: picks the newest published version below the floor (3.17.0 of 2.6.0 3.17.0 3.19.0 3.19.1 3.20.0 under 3.18.0); refuses an empty set by name; never the floor; dpkg for --deb; the newest-overall sabotage is red"
