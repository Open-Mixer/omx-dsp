#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
# ci-install-options.sh [DIR] — every package install in a throwaway CI container skips what a container thrown away
# after the job never needs: apt-get install and build-dep carry -o Dpkg::Options::=--force-unsafe-io (no fsync per
# unpacked file), dnf install and builddep carry --setopt=tsflags=nodocs (no documentation unpacked). On a node starved
# of I/O, a job installing without them sat at its timeout unpacking (the 'below the omx-contract floor' job, #16).
# A line run with sudo is on a hosted runner's own machine, not a container, and is left alone. --self-test proves
# the scan sees a bare install of each kind and passes the same lines with their options.
set -eu
scan() {
  grep -nE '(apt-get|apt) +([^ ]+ +)*(install|build-dep)( |$)' "$1"/*.yml 2>/dev/null \
    | grep -vE '^[^:]+:[0-9]+: *#|sudo ' | grep -v -- '--force-unsafe-io' || true
  grep -nE 'dnf +([^ ]+ +)*(install|builddep)( |$)' "$1"/*.yml 2>/dev/null \
    | grep -vE '^[^:]+:[0-9]+: *#|sudo |dnf +([^ ]+ +)*install +-y +https?://' | grep -v -- 'tsflags=nodocs' || true
}
if [ "${1:-}" = --self-test ]; then
  t=$(mktemp -d); trap 'rm -rf "$t"' EXIT
  for bad in 'apt-get install -y gcc' 'apt-get build-dep -y ./' 'dnf install -y gcc' 'dnf builddep -y x.spec'; do
    printf 'jobs:\n  a:\n    steps:\n      - run: %s\n' "$bad" > "$t/w.yml"
    [ -n "$(scan "$t")" ] || { echo "ci-install-options: FAIL the scan does not see '$bad'" >&2; exit 1; }
  done
  for good in 'apt-get -o Dpkg::Options::=--force-unsafe-io install -y gcc' 'dnf install -y --setopt=tsflags=nodocs gcc' \
              'sudo apt-get install -y doxygen'; do
    printf 'jobs:\n  a:\n    steps:\n      - run: %s\n' "$good" > "$t/w.yml"
    [ -z "$(scan "$t")" ] || { echo "ci-install-options: FAIL the scan refuses '$good'" >&2; exit 1; }
  done
  echo "ci-install-options: self-test ok"; exit 0
fi
dir=${1:-.github/workflows}
found=$(scan "$dir")
[ -z "$found" ] || { echo "ci-install-options: these container installs lack --force-unsafe-io (apt) or tsflags=nodocs (dnf):" >&2; echo "$found" | sed 's/^/  /' >&2; exit 1; }
echo "ci-install-options: every container install carries --force-unsafe-io or tsflags=nodocs"
