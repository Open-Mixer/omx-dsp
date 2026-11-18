#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# wasm-loader-sabotage.sh <share-dir> <scratch-dir> — tools/wasm-loader-check.mjs is not blind. The installed tree
# (DATADIR/omx-dsp, as make install-wasm puts it) is copied once per sabotage and the check must go red, NAMING the file:
#   noloader    the loader dropped from the package: dist/ gone (what a %files or .install line that leaves it out installs)
#   nomodule    the module dropped: wasm/omx-dsp.wasm gone
#   drift       a loader file's bytes changed, so it is no longer the file npm publishes
#   extra       a hand-written file beside the published ones
# The control (the untouched tree) must go green. Usage: wasm-loader-sabotage.sh <share-dir> <scratch-dir>
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
PKG="$(cd "$HERE/.." && pwd)"
SHARE="${1:?installed share directory}"
OUT="${2:?scratch dir}"
TS="$PKG/ts"
NODE="${NODE:-node}"
rm -rf "$OUT" && mkdir -p "$OUT"

"$NODE" "$PKG/tools/wasm-loader-check.mjs" "$SHARE" "$TS" >/dev/null || { echo "wasm-loader-sabotage: FAIL — the control is red"; exit 1; }

run() { # $1 = name, $2 = the sabotage (a shell command on the copy in $dir), $3 = a name the red must carry
  local dir="$OUT/$1"
  rm -rf "$dir" && mkdir -p "$dir"
  cp -R "$SHARE/." "$dir/"
  (cd "$dir" && eval "$2")
  if "$NODE" "$PKG/tools/wasm-loader-check.mjs" "$dir" "$TS" >"$dir/log" 2>&1; then
    echo "wasm-loader-sabotage: FAIL — the check passed with the $1 sabotage"
    exit 1
  fi
  grep -q -- "$3" "$dir/log" || { echo "wasm-loader-sabotage: FAIL — the $1 red does not name $3"; cat "$dir/log"; exit 1; }
  echo "wasm-loader-sabotage: $1 — red, naming $3"
}

run noloader 'rm -rf dist' 'dist/index.js'
run nomodule 'rm -f wasm/omx-dsp.wasm' 'wasm/omx-dsp.wasm'
run drift 'printf "\n// hand edit\n" >> dist/index.js' 'dist/index.js: differs from the file npm publishes'
run extra 'printf "export {};\n" > dist/hand.js' 'dist/hand.js: installed but not published by npm'
echo "wasm-loader-sabotage: all four sabotages are caught"
