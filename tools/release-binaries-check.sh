#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# release-binaries-check.sh — the deb release job releases every binary of debian/control that another
# binary of this source depends on, so the channel never has to supply it from an older release.
#
#   tools/release-binaries-check.sh [debian/control] [.github/workflows/release.yml]  the check
#   tools/release-binaries-check.sh --self-test  a binary a sibling depends on and the deb job does not
#                                                release must be named; a complete list must pass
#
# The deb job's binary-names is the list of packages the build must produce and the publish stage
# releases. A binary that a sibling depends on (libomxdsp-dev needs omx-dsp-contract) and is left out of
# that list is built, but not published in the same pass: channel-installability then resolves it from
# the channel, which holds an older version, and refuses the release.
set -euo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"

# the names the deb job's binary-names lists, space separated
released() {
  awk '/build-deb\.yml@/ { on = 1 }
       on && /binary-names:/ { sub(/.*binary-names:[ \t]*/, ""); gsub(/[\x27"]/, ""); print; exit }' "$1"
}

# the binaries of the control file that another binary of the same file depends on, one per line
needed() {
  awk '
    /^Package:/ { pkg = $2; pk[pkg] = 1; next }
    /^Depends:/ { dep[pkg] = dep[pkg] " " substr($0, 9) }
    END {
      for (p in dep) {
        s = dep[p]; gsub(/\([^)]*\)/, "", s); gsub(/,/, " ", s)
        n = split(s, f, " ")
        for (i = 1; i <= n; i++) if ((f[i] in pk) && f[i] != p) need[f[i]] = 1
      }
      for (x in need) print x
    }' "$1" | sort
}

check() {
  local ctl="$1" wf="$2" rel b rc=0
  rel=$(released "$wf")
  [ -n "$rel" ] || { echo "release-binaries-check: $wf names no binary-names for the deb job"; return 1; }
  for b in $(needed "$ctl"); do
    if [[ " $rel " != *" $b "* ]]; then
      echo "release-binaries-check: $b is a binary of $ctl that a sibling depends on, and the deb job's binary-names in $wf does not release it"
      rc=1
    fi
  done
  return $rc
}

self_test() {
  local d rc=0 ctl wf
  d=$(mktemp -d)
  ctl="$d/control"; wf="$d/release.yml"
  cat > "$ctl" <<'EOF'
Package: libfoo-dev
Architecture: any
Depends: foo-contract (= ${source:Version}), ${misc:Depends}

Package: foo-contract
Architecture: all
Depends: ${misc:Depends}

Package: foo-tool
Architecture: any
Depends: libfoo-dev (= ${binary:Version}), ${misc:Depends}
EOF
  cat > "$wf" <<'EOF'
  deb:
    uses: Open-Mixer/.github/.github/workflows/build-deb.yml@openmixer-v1
    with:
      binary-names: libfoo-dev foo-tool
EOF
  if check "$ctl" "$wf" >/dev/null 2>&1; then
    echo "release-binaries-check --self-test: a sibling-dependency left out of binary-names passed"; rc=1
  fi
  sed -i 's/binary-names: libfoo-dev foo-tool/binary-names: libfoo-dev foo-tool foo-contract/' "$wf"
  if ! check "$ctl" "$wf" >/dev/null 2>&1; then
    echo "release-binaries-check --self-test: a complete binary-names was refused"; rc=1
  fi
  rm -rf "$d"
  [ $rc -eq 0 ] && echo "release-binaries-check --self-test: green"
  return $rc
}

case "${1:-}" in
  --self-test) self_test ;;
  *)
    ctl="${1:-$HERE/debian/control}"
    wf="${2:-$HERE/.github/workflows/release.yml}"
    if check "$ctl" "$wf"; then
      echo "release-binaries-check: green"
    else
      exit 1
    fi
    ;;
esac
