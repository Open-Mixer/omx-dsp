#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# install-confined.sh — make install writes and removes nothing outside $(DESTDIR)$(PREFIX) (#75).
#
#   tools/install-confined.sh            the check: in a throwaway podman container (OMX_INSTALL_CONFINED_IMAGE, default
#                                        registry.fedoraproject.org/fedora:44, which installs gcc, make, pkgconf and
#                                        omx-contract-devel from the OpenMixer channel), so a red run never touches the host;
#                                        run inside a container already (a CI job's), it runs there, which is as throwaway
#   tools/install-confined.sh --inner    the check itself, on THIS filesystem: only ever run it in a container
#
# Two installs run on a copy of the tree: `make install PREFIX=<tmp>/p` and `make install DESTDIR=<tmp>/d PREFIX=/usr`.
# Before them the check plants a sentinel in the system contract directory (/usr/share/omx-contract/contract.d/3/omx-dsp/),
# lists every file, directory and link of the container's filesystem (outside /proc /sys /dev /run /tmp /var/tmp and the
# work copy), and lists it again after each install. Any path that appeared, vanished or changed outside the install's
# own root fails the check, each named.
# Exit 0 green; 1 a path escaped or the install did not land where it should; 2 it could not be judged (no podman and
# not in a container, no image, no omx-contract), never a pass.
set -euo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"
in_container() { [ -f /run/.containerenv ] || [ -f /.dockerenv ]; }

if [ "${1:-}" != --inner ]; then
  if in_container; then exec bash "$HERE/tools/install-confined.sh" --inner; fi
  image=${OMX_INSTALL_CONFINED_IMAGE:-registry.fedoraproject.org/fedora:44}
  command -v podman >/dev/null 2>&1 || { echo "install-confined: UNJUDGED: no podman and not in a container; the check runs only in a throwaway container" >&2; exit 2; }
  exec podman run --rm -v "$HERE:/src:ro,z" "$image" bash /src/tools/install-confined.sh --inner
fi

in_container || { echo "install-confined: --inner refuses to run outside a container" >&2; exit 2; }
SRC=$HERE
if ! pkg-config --exists omx-contract 2>/dev/null; then
  command -v dnf >/dev/null || { echo "install-confined: UNJUDGED: no omx-contract and no dnf to install it" >&2; exit 2; }
  dnf install -y -q --setopt=tsflags=nodocs --setopt=install_weak_deps=False gcc make binutils gawk diffutils git curl tar gzip pkgconf-pkg-config \
    https://www.openmixer.org/rpm/openmixer-release.noarch.rpm >/var/tmp/dnf.log 2>&1 || { cat /var/tmp/dnf.log; echo "install-confined: UNJUDGED: the build tools did not install" >&2; exit 2; }
  dnf install -y -q --setopt=tsflags=nodocs --setopt=install_weak_deps=False omx-contract-devel >/var/tmp/dnf.log 2>&1 || { cat /var/tmp/dnf.log; echo "install-confined: UNJUDGED: omx-contract-devel did not install" >&2; exit 2; }
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
  snap /var/tmp/before.txt
  (cd "$work" && make "$@" >/var/tmp/make.log 2>&1) || { tail -20 /var/tmp/make.log; echo "install-confined: $label: make install failed"; fail=1; return; }
  snap /var/tmp/after.txt
  local d
  d=$( { diff /var/tmp/before.txt /var/tmp/after.txt || true; } | sed -n 's/^[<>] \([^\t]*\)\t.*$/\1/p' | sort -u)
  if [ -n "$d" ]; then
    echo "install-confined: $label: written or removed outside DESTDIR+PREFIX:"; echo "$d" | sed 's/^/  /'; fail=1
  fi
  [ -e "$sentinel" ] || { echo "install-confined: $label: $sentinel is gone (a system contract directory was removed)"; fail=1; }
}

rm -rf /tmp/p /tmp/d
confined "PREFIX=/tmp/p" install PREFIX=/tmp/p
[ -f /tmp/p/share/omx-contract/contract.d/3/omx-dsp/MANIFEST.json ] || { echo "install-confined: PREFIX=/tmp/p: the contract directory is not under the prefix"; fail=1; }
[ -f /tmp/p/include/omxdsp/omx_dsp.h ] || ls /tmp/p/include/omxdsp >/dev/null 2>&1 || { echo "install-confined: PREFIX=/tmp/p: no headers under the prefix"; fail=1; }
confined "DESTDIR=/tmp/d PREFIX=/usr" install DESTDIR=/tmp/d PREFIX=/usr
[ -f /tmp/d/usr/share/omx-contract/contract.d/3/omx-dsp/MANIFEST.json ] || { echo "install-confined: DESTDIR=/tmp/d PREFIX=/usr: the contract directory is not under the destdir"; fail=1; }
[ "$fail" = 0 ] || { echo "install-confined: FAIL"; exit 1; }
echo "install-confined: both installs stayed inside DESTDIR+PREFIX and the planted system directory is intact"
