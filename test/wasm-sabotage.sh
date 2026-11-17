#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# wasm-sabotage.sh <scratch-dir> — tools/wasm-check.mjs is not blind. The module is built from a SABOTAGED COPY of
# wasm/omx_curve_wasm.c (or with a sabotaged flag), once per sabotage, and the check must go red:
#   ctors       __wasm_call_ctors not exported, so the linker wraps every export in the C runtime's constructors, which
#               call WASI random_get (the stand-in throws);
#   capacity    the capacity export answering half the buffer;
#   stack       the stack cut to 4 kB, which the curve function overruns;
#   export      the answer buffer's export renamed.
# The control (the real source) must go green. Needs the wasm toolchain (tools/wasm-build.sh) and node.
# Usage: CONTRACT_INC=… NODE=… wasm-sabotage.sh <scratch-dir>
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
PKG="$(cd "$HERE/.." && pwd)"
OUT="${1:?scratch dir}"
NODE="${NODE:-node}"
mkdir -p "$OUT"
sh "$PKG/tools/wasm-build.sh" "$OUT/control.wasm" "$PKG/wasm/omx_curve_wasm.c" >/dev/null
"$NODE" "$PKG/tools/wasm-check.mjs" "$OUT/control.wasm" >/dev/null || { echo "wasm-sabotage: FAIL — the control is red"; exit 1; }

run() { # $1 = name, $2 = sed expression on the source ('' keeps it), $3 = WASM_STACK ('' for the default), $4 = sed on the build script
  local dir="$OUT/$1"
  rm -rf "$dir" && mkdir -p "$dir"
  cp "$PKG/wasm/omx_curve_wasm.c" "$dir/s.c"
  if [ -n "$2" ]; then
    sed -i -z "$2" "$dir/s.c"
    cmp -s "$PKG/wasm/omx_curve_wasm.c" "$dir/s.c" && { echo "wasm-sabotage: FAIL — the $1 sabotage matched nothing"; exit 1; }
  fi
  cp "$PKG/tools/wasm-build.sh" "$dir/build.sh"
  if [ -n "${4:-}" ]; then
    sed -i "$4" "$dir/build.sh"
    cmp -s "$PKG/tools/wasm-build.sh" "$dir/build.sh" && { echo "wasm-sabotage: FAIL — the $1 sabotage of the script matched nothing"; exit 1; }
  fi
  # a copy of the script finds the real tree through OMX_DSP_ROOT
  ( cd "$PKG" && WASM_STACK="${3:-262144}" OMX_DSP_ROOT="$PKG" sh "$dir/build.sh" "$dir/m.wasm" "$dir/s.c" >/dev/null )
  if "$NODE" "$PKG/tools/wasm-check.mjs" "$dir/m.wasm" >"$dir/log" 2>&1; then
    echo "wasm-sabotage: FAIL — the check passed with the $1 sabotage"
    exit 1
  fi
  echo "wasm-sabotage: $1 — red, as it must be"
}

run ctors '' '' 's| -Wl,--export=__wasm_call_ctors||'
run capacity 's|return OMX_CURVE_JSON_OUT_MAX; }|return OMX_CURVE_JSON_OUT_MAX / 2u; }|' ''
run stack "" 4096
run export 's|OMX_WASM_EXPORT(omx_curve_wasm_answer)|OMX_WASM_EXPORT(omx_curve_wasm_reply)|' ''
echo "wasm-sabotage: all four sabotages are caught"
