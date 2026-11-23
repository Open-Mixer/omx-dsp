#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# support-install-check.sh — the test support a consumer's native tests read from the devel package: under an install
# prefix, omxdsp.pc declares testsupportdir (the directory holding log10f_cr.h and log10f_subst.h) and tsangate (the
# ThreadSanitizer gate script), and both exist there.
#
#   support-install-check.sh <install prefix>
#
# Exit 0 held; 1 a failure; 2 usage.
set -eu

[ "$#" -eq 1 ] || { sed -n '9p' "$0" >&2; exit 2; }
prefix=$1
pcdir=
for d in "$prefix/lib64/pkgconfig" "$prefix/lib/pkgconfig"; do [ -f "$d/omxdsp.pc" ] && pcdir=$d; done
[ -n "$pcdir" ] || { echo "support-install-check: no omxdsp.pc under $prefix" >&2; exit 2; }
var() { PKG_CONFIG_PATH="$pcdir" pkg-config --variable="$1" omxdsp; }
dir=$(var testsupportdir)
[ -n "$dir" ] || { echo "support-install-check: omxdsp.pc declares no testsupportdir" >&2; exit 1; }
for f in log10f_cr.h log10f_subst.h; do
  [ -f "$dir/$f" ] || { echo "support-install-check: $dir ships no $f" >&2; exit 1; }
  cmp -s "$dir/$f" "test/support/$f" || { echo "support-install-check: $dir/$f differs from test/support/$f" >&2; exit 1; }
done
gate=$(var tsangate)
[ -n "$gate" ] || { echo "support-install-check: omxdsp.pc declares no tsangate" >&2; exit 1; }
[ -x "$gate" ] || { echo "support-install-check: tsangate $gate is not an executable file" >&2; exit 1; }
cmp -s "$gate" tools/tsan-gate.sh || { echo "support-install-check: $gate differs from tools/tsan-gate.sh" >&2; exit 1; }
echo "support-install-check: testsupportdir $dir and tsangate $gate shipped"
