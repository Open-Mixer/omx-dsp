#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# curve-bench.sh [twin-ref] [curve-bench.mjs options] — CPU, RAM and precision of the curve as WebAssembly against the
# TypeScript twin it replaced. The twin is built from `twin-ref` (default: the commit before the one that deleted
# ts/src/curve.ts) into a scratch directory; the module is build/wasm/omx-dsp.wasm (`make wasm`) and the loader
# ts/dist/wasm.js (`npm run build` in ts/). Needs git, node and ts/node_modules (npm ci in ts/). Writes nothing in the tree.
set -eu
HERE="$(cd "$(dirname "$0")/.." && pwd)"
case "${1:-}" in -*|"") ref="" ;; *) ref="$1"; shift ;; esac
[ -n "$ref" ] || ref="$(git -C "$HERE" log --diff-filter=D -1 --format=%H -- ts/src/curve.ts)^"
git -C "$HERE" rev-parse -q --verify "$ref^{commit}" >/dev/null || { echo "curve-bench: no commit $ref" >&2; exit 1; }
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
git -C "$HERE" archive "$ref" ts/src ts/tsconfig.json | tar -x -C "$tmp"
[ -f "$tmp/ts/src/curve.ts" ] || { echo "curve-bench: $ref has no TypeScript curve twin" >&2; exit 1; }
"$HERE/ts/node_modules/.bin/tsc" -p "$tmp/ts/tsconfig.json"
echo "curve-bench: twin built from $ref"
exec node "$HERE/tools/curve-bench.mjs" --wasm "$HERE/build/wasm/omx-dsp.wasm" --loader "$HERE/ts/dist/wasm.js" --twin "$tmp/ts/dist/index.js" "$@"
