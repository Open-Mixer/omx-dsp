#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# doc-check.sh — what doxygen cannot check: a function's doc block states the contracts its body evaluates.
#
# For every `include/omxdsp/*.h`, `include/omxdsp/mix/*.h`, `include/omxdsp/fx/*.h` and `include/omxdsp/analysis/*.h`
# (or the files given as arguments), but
# include/omxdsp/omxdsp_definitions.h and include/omxdsp/omx_dsp_vocabulary.h, which `omx-contract render` writes from
# data/: a function named omx_*/omxdsp_* whose body evaluates OMX_PRE carries @pre in the `/** … */` block right
# before it, OMX_POST @post, OMX_INVARIANT @invariant. Doxygen cannot relate a body to its block.
#
# The rest is the standard tools' (#82): that every public declaration is documented, with @brief, one @param per
# parameter and @return, is make docs-check (doxygen, WARN_IF_UNDOCUMENTED and WARN_NO_PARAMDOC against
# docs/doxygen-baseline.txt); the one `@thread audio|worker|control` tag per function is omx-rt-gate --thread-tags
# (make rt-gate-tags, in make test). A function with no block at all is doxygen's to refuse, not this check's.
#
# Exit 1 with one line per missing tag. `--self-test` runs the checker over scratch headers: a body that evaluates each
# contract with no tag for it must be refused, the same functions with their tags accepted.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
PKG="$(cd "$HERE/.." && pwd)"
AWK_PROG='
function trim(s) { sub(/^[ \t]+/, "", s); sub(/[ \t]+$/, "", s); return s }
function report(line, what) { printf("doc-check: %s:%d: %s\n", FILENAME, line, what); bad++ }
function check_body(line, body, docok, blk) {
  if (!docok) return
  if (body ~ /OMX_PRE/ && blk !~ /@pre/) report(line, "body evaluates OMX_PRE but the doc block has no @pre")
  if (body ~ /OMX_POST/ && blk !~ /@post/) report(line, "body evaluates OMX_POST but the doc block has no @post")
  if (body ~ /OMX_INVARIANT/ && blk !~ /@invariant/) report(line, "body evaluates OMX_INVARIANT but the doc block has no @invariant")
}
BEGIN { bad = 0 }
FNR == 1 { depth = 0; in_doc = 0; in_comment = 0; block = ""; block_end = -1; last_nonblank = -1; pending = 0; collecting = 0 }
{
  raw = $0
  # doc blocks and plain comments
  if (in_comment) { if (in_doc) block = block "\n" raw; if (raw ~ /\*\//) { in_comment = 0; if (in_doc) { block_end = FNR; in_doc = 0 } } last_nonblank = FNR; next }
  if (raw ~ /^[ \t]*\/\*\*/) { in_doc = 1; block = raw; if (raw ~ /\*\//) { in_doc = 0; block_end = FNR } else in_comment = 1; last_nonblank = FNR; next }
  if (raw ~ /^[ \t]*\/\*/) { if (raw !~ /\*\//) in_comment = 1; last_nonblank = FNR; next }
  code = raw; sub(/\/\/.*$/, "", code); gsub(/"[^"]*"/, "\"\"", code); sub(/\/\*.*\*\//, "", code)
  if (trim(code) == "") { if (trim(raw) != "") last_nonblank = FNR; next }
  # a signature spanning lines: accumulate until { or ;
  if (pending) {
    if (code ~ /[{;]/) { pending = 0; if (code ~ /\{/) { collecting = 1; body = ""; bdepth = 0 } }
  } else if (depth == 0 && code ~ /^(static[ \t]+inline[ \t]+|)[A-Za-z_][A-Za-z0-9_ \t*]*[ \t*](omx_[a-z0-9_]+|omxdsp_[a-z0-9_]+)[ \t]*\(/ && code !~ /^#/ && code !~ /^typedef/) {
    sig_line = FNR; sig_docok = (block_end == last_nonblank); sig_block = block
    if (code ~ /[{;]/) { if (code ~ /\{/) { collecting = 1; body = ""; bdepth = 0 } }
    else pending = 1
  }
  # brace depth over code, and the body of the function being collected
  o = gsub(/\{/, "{", code); c = gsub(/\}/, "}", code)
  if (collecting) {
    body = body "\n" code; bdepth += o - c
    if (bdepth <= 0) { collecting = 0; check_body(sig_line, body, sig_docok, sig_block) }
  }
  depth += o - c
  last_nonblank = FNR
}
END { if (bad) { printf("doc-check: %d missing contract tag(s)\n", bad); exit 1 } }
'

check() { awk "$AWK_PROG" "$@"; }

if [ "${1:-}" = "--self-test" ]; then
  t="$(mktemp -d)"; trap 'rm -rf "$t"' EXIT
  for c in PRE POST INVARIANT; do
    tag=$(printf '%s' "$c" | tr 'A-Z' 'a-z')
    printf '/** @brief Identity.\n * @param x A sample.\n * @return `x`.\n * @thread audio */\nstatic inline float omx_untold(float x) {\n  OMX_%s(x == x, "a-name");\n  return x;\n}\n' "$c" > "$t/bad.h"
    if check "$t/bad.h" >/dev/null 2>&1; then echo "doc-check --self-test: FAIL — OMX_$c with no @$tag was accepted"; exit 1; fi
    printf '/** @brief Identity.\n * @param x A sample.\n * @return `x`.\n * @%s `a-name`: x is a number.\n * @thread audio */\nstatic inline float omx_told(float x) {\n  OMX_%s(x == x, "a-name");\n  return x;\n}\n' "$tag" "$c" > "$t/good.h"
    if ! check "$t/good.h"; then echo "doc-check --self-test: FAIL — OMX_$c with its @$tag was refused"; exit 1; fi
  done
  echo "doc-check --self-test: ok (refuses a body's OMX_PRE, OMX_POST and OMX_INVARIANT its block does not state, accepts them stated)"
  exit 0
fi

if [ $# -gt 0 ]; then check "$@"; else
  bash "$0" --self-test
  set -- "$PKG"/include/omxdsp/*.h "$PKG"/include/omxdsp/mix/*.h "$PKG"/include/omxdsp/fx/*.h "$PKG"/include/omxdsp/analysis/*.h
  for f; do shift; case "$f" in "$PKG/include/omxdsp/omxdsp_definitions.h"|"$PKG/include/omxdsp/omx_dsp_vocabulary.h") ;; *) set -- "$@" "$f" ;; esac; done
  check "$@" && echo "doc-check: every function in include/omxdsp states the contracts its body evaluates"
fi
