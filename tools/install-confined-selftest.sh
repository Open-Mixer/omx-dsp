#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# install-confined-selftest.sh — install-confined.sh never runs its inner check because it THINKS it is in a container (#75).
# A developer's long-lived toolbox carries /run/.containerenv, and as root there the inner check would plant a sentinel in
# the real system contract directory and run make install for real. This arm runs, in a throwaway podman container (which
# carries the marker), as a non-root user, two cases and requires: with the marker and a podman on the PATH, install-confined.sh
# SPAWNS that podman (a recording stand-in) and starts no inner check; with the marker and no podman it exits 2, UNJUDGED.
# Needs podman (exit 2 without). Exit 0 both held; 1 one failed.
set -euo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"
if [ "${1:-}" != --inner ]; then
  command -v podman >/dev/null 2>&1 || { echo "install-confined-selftest: needs podman" >&2; exit 2; }
  image=${OMX_INSTALL_CONFINED_IMAGE:-registry.fedoraproject.org/fedora:44}
  exec podman run --rm --security-opt label=disable --user 1000 -v "$HERE:/src:ro" "$image" bash /src/tools/install-confined-selftest.sh --inner
fi
[ -f /run/.containerenv ] || [ -f /.dockerenv ] || { echo "install-confined-selftest: --inner refuses to run outside a container" >&2; exit 2; }
t=$(mktemp -d); mkdir "$t/bin"
printf '#!/bin/sh\necho spawned > %s/spawned\nexit 0\n' "$t" > "$t/bin/podman"; chmod +x "$t/bin/podman"
rc=0
# case 1: marker present, a podman on the PATH: it must be spawned
env -u OMX_INSTALL_CONFINED_INNER PATH="$t/bin:$PATH" bash /src/tools/install-confined.sh >"$t/out1" 2>&1 || rc=$?
{ [ "$rc" = 0 ] && [ -f "$t/spawned" ]; } || { echo "install-confined-selftest: FAIL: a container marker without the opt-in did not spawn the throwaway container (exit $rc)"; cat "$t/out1"; exit 1; }
# case 2: marker present, no podman: UNJUDGED, exit 2
rc=0; rm -f "$t/spawned"
env -u OMX_INSTALL_CONFINED_INNER PATH=/usr/bin:/bin bash /src/tools/install-confined.sh >"$t/out2" 2>&1 || rc=$?
{ [ "$rc" = 2 ] && grep -q UNJUDGED "$t/out2"; } || { echo "install-confined-selftest: FAIL: a container marker without podman gave exit $rc, want 2 UNJUDGED"; cat "$t/out2"; exit 1; }
echo "install-confined-selftest: a container marker alone never starts the inner check"
