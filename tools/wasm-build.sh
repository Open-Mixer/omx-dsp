#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# wasm-build.sh <out.wasm> <source.c>... — one C program built to one WebAssembly module with the DISTRIBUTION's
# toolchain: clang, lld and wasi-libc, nothing downloaded, no Emscripten. This is the one documented recipe (`make wasm`
# calls it); omx-panel's gesture module is built by the same script, with its own sources.
#
#   Fedora 44/45   dnf install clang lld wasi-libc-devel wasi-libc-static
#   Debian trixie  apt-get install clang lld wasi-libc   (unconfirmed until the debian cell of ci.yml builds it)
#
# The flags are the library's: -ffp-contract=off and no fast-math, so the arithmetic is the native build's. The module is
# freestanding but for wasi-libc's math and stdio: -nostartfiles and -nodefaultlibs, with -lc -lm named, because Fedora
# ships no wasm32 compiler-rt builtins (a link with the default libraries fails on libclang_rt.builtins.a); nothing here
# needs a builtin, and an undefined symbol would fail the link, so none is hidden. Every `export_name` attribute of the
# sources is exported, and so is __wasm_call_ctors, which no host calls: exported, the linker does not wrap every export in
# a call to the C runtime's constructors (stdio, locale and stack-protector set-up, which reach WASI's random_get). The module still imports five WASI functions of wasi-libc's stdio (snprintf's FILE machinery); no call reaches them, and
# tools/wasm-check.mjs runs it with stand-ins that throw.
#
# omx-contract's headers are added with -idirafter, never -isystem: where they are installed under /usr/include a -isystem
# would put the HOST's /usr/include (glibc's math.h) ahead of the wasi-libc sysroot's.
#
# Environment: OMX_DSP_INCLUDE (the headers, default this tree's include/: a sabotage puts a broken copy here), OMX_DSP_ROOT (this tree, default the script's own), CLANG (default clang), WASM_SYSROOT (default /usr/wasm32-wasi when it exists, else /usr),
#   WASM_CFLAGS (extra), CONTRACT_INC (omx-contract's include directory, default from tools/contract-include.sh),
#   WASM_STACK (bytes, default 262144), WASM_STRIP (default -Wl,--strip-debug).
set -eu
HERE="${OMX_DSP_ROOT:-$(cd "$(dirname "$0")/.." && pwd)}"
out="${1:?usage: wasm-build.sh <out.wasm> <source.c>...}"
shift
[ "$#" -gt 0 ] || { echo "wasm-build: no source" >&2; exit 2; }
CLANG="${CLANG:-clang}"
# Fedora keeps the wasm sysroot apart (/usr/wasm32-wasi), Debian under /usr (include/wasm32-wasi, lib/wasm32-wasi)
if [ -n "${WASM_SYSROOT:-}" ]; then SYSROOT="$WASM_SYSROOT"; elif [ -d /usr/wasm32-wasi ]; then SYSROOT=/usr/wasm32-wasi; else SYSROOT=/usr; fi
: "${CONTRACT_INC:=$(sh "$HERE/tools/contract-include.sh")}"
command -v "$CLANG" >/dev/null 2>&1 || { echo "wasm-build: $CLANG not found: install clang lld wasi-libc (see this file)" >&2; exit 1; }
mkdir -p "$(dirname "$out")"
# the triple is the one the sysroot's headers are filed under: wasm32-wasip1 from newer wasi-libc, wasm32-wasi before; clang
# accepts both (the old spelling is deprecated, a warning)
if [ -d "$SYSROOT/include/wasm32-wasip1" ]; then target=wasm32-wasip1; else target=wasm32-wasi; fi
# shellcheck disable=SC2086
"$CLANG" -Wno-deprecated --target=$target --sysroot="$SYSROOT" -std=gnu11 -O2 -Wall -Wextra -Werror -ffp-contract=off -fno-stack-protector \
  -I"${OMX_DSP_INCLUDE:-$HERE/include}" -idirafter "$CONTRACT_INC" ${WASM_CFLAGS:-} \
  -nostartfiles -nodefaultlibs -Wl,--no-entry -Wl,--export=__wasm_call_ctors ${WASM_STRIP--Wl,--strip-debug} \
  -Wl,-z,stack-size="${WASM_STACK:-262144}" -o "$out" "$@" -lm -lc
echo "wasm-build: $out $(wc -c <"$out") bytes"
