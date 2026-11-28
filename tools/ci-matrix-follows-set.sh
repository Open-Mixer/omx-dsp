#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# Ruling D25: a CI matrix follows the release set, never a hand list. Debian bookworm is dropped from the matrix
# (owner 2026-11-27): no channel level serves a bookworm package and nothing builds one, so a bookworm cell can only
# fail at its install, in the quick set and the full set alike. A workflow line outside a comment names no bookworm at
# all, and choosing it through OMX_RELEASE_MATRIX (the full set's switch, which once carried it) is no exception.
# `ci-matrix-follows-set.sh [workflow...]`; with --self-test it also plants a hand cell and a switch-chosen cell, each red.
set -eu
check() {
  bad=$(awk '/^[[:space:]]*#/ { next } /bookworm/ { print FILENAME ":" FNR ": " $0 }' "$@")
  if [ -n "$bad" ]; then
    printf '%s\n' "$bad" >&2
    echo "FAIL a bookworm cell: Debian bookworm is dropped from the matrix (owner 2026-11-27); CI follows the release set (ruling D25)" >&2
    return 1
  fi
}
self_test=
[ "${1:-}" = --self-test ] && { self_test=1; shift; }
[ $# -gt 0 ] || set -- .github/workflows/*.yml
check "$@"
echo "ok   no workflow names bookworm outside a comment ($#)"
if [ -n "$self_test" ]; then
  tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
  printf '    strategy:\n      matrix:\n        image: [%s]\n' "'fedora:45', 'debian:bookworm'" > "$tmp/planted.yml"
  if check "$tmp/planted.yml" 2>/dev/null; then echo "FAIL the self-test's hand bookworm cell passed" >&2; exit 1; fi
  echo "ok   a hand bookworm cell is red"
  # the line ci.yml carried until 2026-11-27: bookworm in the full set, through the switch
  printf '          deb-releases: %s\n' "\${{ vars.OMX_RELEASE_MATRIX == 'full' && '[\"bookworm\", \"trixie\"]' || '[\"trixie\"]' }}" > "$tmp/switched.yml"
  grep -q "OMX_RELEASE_MATRIX == 'full' && '\[\"bookworm\"" "$tmp/switched.yml" || { echo "FAIL the self-test's switch line was not planted" >&2; exit 1; }
  if check "$tmp/switched.yml" 2>/dev/null; then echo "FAIL the self-test's bookworm chosen through OMX_RELEASE_MATRIX passed" >&2; exit 1; fi
  echo "ok   a bookworm cell chosen through OMX_RELEASE_MATRIX is red"
  printf '    # no bookworm cell: dropped from the matrix\n' > "$tmp/comment.yml"
  check "$tmp/comment.yml" || { echo "FAIL the self-test's comment naming bookworm was red" >&2; exit 1; }
  echo "ok   a comment naming bookworm is not a cell"
fi
