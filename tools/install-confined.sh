#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# install-confined.sh — make install writes and removes nothing outside $(DESTDIR)$(PREFIX) (#75).
#
#   tools/install-confined.sh            the check, in a throwaway podman container (OMX_INSTALL_CONFINED_IMAGE, default
#                                        registry.fedoraproject.org/fedora:44, by tag), so a red run never touches the host
#   OMX_INSTALL_CONFINED_INNER=1 tools/install-confined.sh
#                                        the check itself, on THIS file system. Only an explicit opt-in, set by the CI job
#                                        whose container is thrown away and by the podman call above: a container marker
#                                        (/run/.containerenv, /.dockerenv) is NEVER taken for one, a developer's toolbox has it
#
# Inner check, what it WRITES and REMOVES (all of it in a container that is thrown away):
#   /w                a copy of the tree (removed first); the install runs there
#   a mktemp -d       under /tmp: the prefix, the destdir, the snapshots and the make log (removed at the end)
#   the system contract directory /usr/share/omx-contract/contract.d/3/omx-dsp/: ONE sentinel file planted (the proof the
#                     install does not remove it); and, when omx-contract is not installed, dnf installs gcc, make,
#                     pkgconf, omx-contract-devel and the OpenMixer release package (no chmod anywhere)
# The two installs are `make install PREFIX=<scratch>/p` and `make install DESTDIR=<scratch>/d PREFIX=/usr`. The snapshots
# list every file, directory and link of the file system (find -xdev) and are compared before and after each install. PRUNED
# (not listed): /proc /sys /dev /run /tmp /var/tmp (kernel, runtime and scratch, where the check itself writes), /w and the
# tree copied from (the check's own work; the install is run from /w, never from the source), and CI's /__w and /github (the
# runner's own directories). $HOME and every other path is listed, so an escape into it is seen. An install that wrote into
# /tmp or /var/tmp is not seen: the install's own root and the build's scratch live there.
# Exit 0 green; 1 a path escaped or the install did not land where it should; 2 it could not be judged (no podman and no
# opt-in, no image, no omx-contract), never a pass.
set -euo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"

if [ "${OMX_INSTALL_CONFINED_INNER:-}" != 1 ]; then
  image=${OMX_INSTALL_CONFINED_IMAGE:-registry.fedoraproject.org/fedora:44}
  command -v podman >/dev/null 2>&1 || { echo "install-confined: UNJUDGED: no podman, and OMX_INSTALL_CONFINED_INNER=1 is not set: the check runs only in a throwaway container" >&2; exit 2; }
  exec podman run --rm -e OMX_INSTALL_CONFINED_INNER=1 -v "$HERE:/src:ro" "$image" bash /src/tools/install-confined.sh
fi

SRC=$HERE
scratch=$(mktemp -d); trap 'rm -rf "$scratch"' EXIT
if ! pkg-config --exists omx-contract 2>/dev/null; then
  command -v dnf >/dev/null || { echo "install-confined: UNJUDGED: no omx-contract and no dnf to install it" >&2; exit 2; }
  dnf install -y -q --setopt=tsflags=nodocs --setopt=install_weak_deps=False gcc make binutils gawk diffutils git curl tar gzip pkgconf-pkg-config \
    https://www.openmixer.org/rpm/openmixer-release.noarch.rpm >"$scratch/dnf.log" 2>&1 || { cat "$scratch/dnf.log"; echo "install-confined: UNJUDGED: the build tools did not install" >&2; exit 2; }
  dnf install -y -q --setopt=tsflags=nodocs --setopt=install_weak_deps=False omx-contract-devel >"$scratch/dnf.log" 2>&1 || { cat "$scratch/dnf.log"; echo "install-confined: UNJUDGED: omx-contract-devel did not install" >&2; exit 2; }
  pkg-config --exists omx-contract || { echo "install-confined: UNJUDGED: omx-contract did not install" >&2; exit 2; }
fi
work=/w
rm -rf "$work"; cp -a "$SRC" "$work"; rm -rf "$work/build"
sentinel=/usr/share/omx-contract/contract.d/3/omx-dsp/sentinel-install-confined
mkdir -p "$(dirname "$sentinel")"; echo planted > "$sentinel"

snap() {
  find / -xdev \( -path /proc -o -path /sys -o -path /dev -o -path /run -o -path /tmp -o -path /var/tmp -o -path "$work" -o -path "$SRC" -o -path /__w -o -path /github \) -prune \
    -o -type d -printf '%p\td\n' -o -printf '%p\t%y %s %T@ %l\n' | LC_ALL=C sort > "$1"
}
fail=0
confined() { # <label> <make args...>
  local label=$1; shift
  snap "$scratch/before.txt"
  (cd "$work" && make "$@" >"$scratch/make.log" 2>&1) || { tail -20 "$scratch/make.log"; echo "install-confined: $label: make install failed"; fail=1; return; }
  snap "$scratch/after.txt"
  local d
  d=$( { diff "$scratch/before.txt" "$scratch/after.txt" || true; } | sed -n 's/^[<>] \([^\t]*\)\t.*$/\1/p' | sort -u)
  if [ -n "$d" ]; then
    echo "install-confined: $label: written or removed outside DESTDIR+PREFIX:"; echo "$d" | sed 's/^/  /'; fail=1
  fi
  [ -e "$sentinel" ] || { echo "install-confined: $label: $sentinel is gone (a system contract directory was removed)"; fail=1; }
}

P=$scratch/p D=$scratch/d
confined "PREFIX=$P" install PREFIX=$P
[ -f $P/share/omx-contract/contract.d/3/omx-dsp/MANIFEST.json ] || { echo "install-confined: PREFIX=$P: the contract directory is not under the prefix"; fail=1; }
[ -f $P/include/omxdsp/omx_dsp.h ] || ls $P/include/omxdsp >/dev/null 2>&1 || { echo "install-confined: PREFIX=$P: no headers under the prefix"; fail=1; }
confined "DESTDIR=$D PREFIX=/usr" install DESTDIR=$D PREFIX=/usr
[ -f $D/usr/share/omx-contract/contract.d/3/omx-dsp/MANIFEST.json ] || { echo "install-confined: DESTDIR=$D PREFIX=/usr: the contract directory is not under the destdir"; fail=1; }
[ "$fail" = 0 ] || { echo "install-confined: FAIL"; exit 1; }
echo "install-confined: both installs stayed inside DESTDIR+PREFIX and the planted system directory is intact"
