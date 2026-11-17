#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# install-confined.sh — make install writes and removes nothing outside $(DESTDIR)$(PREFIX) (#75).
#
#   tools/install-confined.sh            the check, inside a throwaway container (podman), so a red run never touches the host
#   tools/install-confined.sh --inner    the check itself, on THIS filesystem: only ever run it in a container
#
# Two installs run on a copy of the tree: `make install PREFIX=<tmp>/p` and `make install DESTDIR=<tmp>/d PREFIX=/usr`.
# Before them the check plants a sentinel in the contract directory the image already carries
# (/usr/share/omx-contract/contract.d/3/omx-dsp/), lists every file, directory and link of the container's
# filesystem (outside /proc /sys /dev /run /tmp /var/tmp and the work copy), and lists it again after each install. Any path
# that appeared, vanished or changed outside the install's own root fails the check, each named.
# The image (OMX_INSTALL_CONFINED_IMAGE, default localhost/hostd124-fc44) needs gcc, make, pkg-config and an
# installed omx-contract. Exit 0 green or UNJUDGED (no podman: said, never a pass); 1 a path escaped or the install
# did not land where it should; 2 the image is missing.
set -euo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"

if [ "${1:-}" != --inner ]; then
  image=${OMX_INSTALL_CONFINED_IMAGE:-localhost/hostd124-fc44}
  command -v podman >/dev/null 2>&1 || { echo "install-confined: UNJUDGED: no podman, the check runs only in a throwaway container"; exit 0; }
  podman image exists "$image" || { echo "install-confined: no image $image (set OMX_INSTALL_CONFINED_IMAGE)" >&2; exit 2; }
  exec podman run --rm --network=none -v "$HERE:/src:ro,z" "$image" bash /src/tools/install-confined.sh --inner
fi

[ -f /run/.containerenv ] || [ -f /.dockerenv ] || { echo "install-confined: --inner refuses to run outside a container" >&2; exit 2; }
work=/w
rm -rf "$work"; cp -a /src "$work"; rm -rf "$work/build"
sentinel=/usr/share/omx-contract/contract.d/3/omx-dsp/sentinel-install-confined
mkdir -p "$(dirname "$sentinel")"; echo planted > "$sentinel"

snap() {
  find / -xdev \( -path /proc -o -path /sys -o -path /dev -o -path /run -o -path /tmp -o -path /var/tmp -o -path "$work" \) -prune \
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
