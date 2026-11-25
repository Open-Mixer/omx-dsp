#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
# ci-node-26.sh [DIR] — every Node a workflow runs is Node 26 (owner rule: Node 26 everywhere, #130): a setup-node step
# names node-version 26 or later, and a Fedora job installs the distribution's nodejs26 (Fedora 45's plain `nodejs` is
# Node 22). Debian's archive carries no Node 26, so a Debian job takes it from setup-node, never from apt. --self-test
# proves the scan sees each kind of older Node and passes the same lines at 26.
set -eu
scan() {
  grep -nE 'node-version: *[0-9]+' "$1"/*.yml 2>/dev/null | grep -vE 'node-version: *(2[6-9]|[3-9][0-9])([^0-9]|$)' || true
  grep -nE '(dnf|apt-get|apt) .*install .*(^| |\()nodejs([0-9]*)?( |\)|$)' "$1"/*.yml 2>/dev/null \
    | grep -vE '^[^:]+:[0-9]+: *#' | grep -vE 'dnf .*install .* nodejs26( |$)' || true
  grep -nE 'echo nodejs([0-9]*)?\)' "$1"/*.yml 2>/dev/null | grep -v 'echo nodejs26)' || true
}
if [ "${1:-}" = --self-test ]; then
  t=$(mktemp -d); trap 'rm -rf "$t"' EXIT
  for bad in 'node-version: 22' 'node-version: 24' 'dnf install -y --setopt=tsflags=nodocs nodejs' \
             'apt-get install -y ./x.deb nodejs' 'dnf install -y $(command -v node >/dev/null || echo nodejs) x.rpm'; do
    printf 'jobs:\n  a:\n    steps:\n      - %s\n' "$bad" > "$t/w.yml"
    [ -n "$(scan "$t")" ] || { echo "ci-node-26: FAIL the scan does not see '$bad'" >&2; exit 1; }
  done
  for good in 'node-version: 26' 'dnf install -y --setopt=tsflags=nodocs nodejs26 nodejs26-bin x.rpm' \
              'apt-get install -y ./x.deb'; do
    printf 'jobs:\n  a:\n    steps:\n      - %s\n' "$good" > "$t/w.yml"
    [ -z "$(scan "$t")" ] || { echo "ci-node-26: FAIL the scan refuses '$good'" >&2; exit 1; }
  done
  echo "ci-node-26: self-test ok"; exit 0
fi
dir=${1:-.github/workflows}
found=$(scan "$dir")
[ -z "$found" ] || { printf '%s\n' "$found"; echo "ci-node-26: RED -- a workflow runs a Node below 26" >&2; exit 1; }
echo "ci-node-26: every workflow Node is 26"
