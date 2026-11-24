#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# definitions-render.sh — this library's own C header, rendered from its definitions (data/) by omx-contract's tool:
# the defines of every item omx-dsp owns, the same names and bytes omx-contract's header carried for them.
#
#   tools/definitions-render.sh [--check] <omx-contract checkout> <build dir>
#
# data/ is installed as omx-dsp beside the checkout's own data/ as omx-contract (the way `make install` installs it),
# and `omx-contract render --target c --package omx-dsp` renders omx-dsp's items alone: the header its files' render
# word names (include/omxdsp/omxdsp_definitions.h) and the vocabulary header of its words items
# (include/omxdsp/omx_dsp_vocabulary.h). --check writes nothing and fails when a committed header differs from the render;
# without it the headers are written. A release build never runs this (R-056: no node): it ships the committed header.
# Exit 0 rendered or equal; 1 the committed header differs; 2 usage or environment (no node, no checkout, a tool
# that cannot render one package), never a pass.
set -euo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"
check=''
[ "${1-}" = --check ] && { check=--check; shift; }
[ "$#" -eq 2 ] || { sed -n '8p' "$0" >&2; exit 2; }
OC="$(cd "$1" 2>/dev/null && pwd)" || { echo "definitions-render: no omx-contract checkout at $1" >&2; exit 2; }
out="$(mkdir -p "$2" && cd "$2" && pwd)"
command -v node >/dev/null 2>&1 || { echo "definitions-render: node is missing" >&2; exit 2; }
[ -f "$OC/bin/omx-contract.mjs" ] && [ -f "$OC/tools/install-contract-dir.sh" ] && [ -d "$OC/node_modules" ] || { echo "definitions-render: $OC has no bin/omx-contract.mjs, tools/install-contract-dir.sh or node_modules (npm ci there)" >&2; exit 2; }
grep -qE 'render --target c\|ts\|json(\|[a-z0-9]+)* .*--package' "$OC/bin/omx-contract.mjs" || { echo "definitions-render: the tool at $OC renders no single package (render --package): it needs omx-contract 2.15.0's tool" >&2; exit 2; }
version="$(make -s -C "$HERE" version)"
major="$(sed -n 's/^  "x-omx-schema-version": "\([0-9]*\)\..*",$/\1/p' "$OC/schema/omx-contract.schema.json")"
ocversion="$(sed -n 's/^  "version": "\(.*\)",$/\1/p' "$OC/package.json")"
[ -n "$major" ] && [ -n "$ocversion" ] || { echo "definitions-render: $OC states no schema major or version" >&2; exit 2; }
dir="$out/dir"
rm -rf "$dir"; mkdir -p "$dir"
sh "$OC/tools/install-contract-dir.sh" -x scale-vectors.json omx-contract "$ocversion" "$major" "$OC/data" "$dir" data >/dev/null
# omx-contract's domains (contract.d/<domain>: PARAM_UNITS lives in contract.d/param), installed as its package installs them;
# without them the render resolves no unit set's titles and writes the enumerators from the raw ids (OMX_CURVE_UNITS_D_B)
for d in "$OC"/contract.d/*/; do
  [ -d "$d" ] || continue
  sh "$OC/tools/install-contract-dir.sh" --domain "${d%/}" "$dir" omx-contract >/dev/null
done
sh "$OC/tools/install-contract-dir.sh" omx-dsp "$version" "$major" "$HERE/data" "$dir" data >/dev/null
header=omxdsp/omxdsp_definitions.h
vocabulary=omxdsp/omx_dsp_vocabulary.h
[ -z "$check" ] || { [ -f "$HERE/include/$header" ] && [ -f "$HERE/include/$vocabulary" ]; } || { echo "definitions-render: no committed include/$header or include/$vocabulary to check" >&2; exit 2; }
# the render lands in a scratch directory: the vocabulary header (<package>_vocabulary.h) is written at its root, and
# is installed with the others under include/omxdsp/
work="$out/render"
rm -rf "$work"; mkdir -p "$work"
rc=0
got="$(cd "$OC" && node bin/omx-contract.mjs render --target c --package omx-dsp --contract-dir "$dir" --out "$work" 2>&1)" || rc=$?
printf '%s\n' "$got"
[ "$rc" = 0 ] || exit "$rc"
# a render of no file is no render: the files' render word must name the header
grep -Eq ' [1-9][0-9]* files? ' <<<"$got" && [ -f "$work/$header" ] && [ -f "$work/omx_dsp_vocabulary.h" ] || { echo "definitions-render: the render wrote no $header or omx_dsp_vocabulary.h" >&2; exit 2; }
differs=0
for pair in "$header:$header" "omx_dsp_vocabulary.h:$vocabulary"; do
  from="$work/${pair%%:*}" to="$HERE/include/${pair#*:}"
  if [ -n "$check" ]; then
    cmp -s "$from" "$to" || { echo "omx-contract render --check: $to differs" >&2; differs=1; }
  else
    install -m 0644 "$from" "$to"
  fi
done
exit "$differs"
