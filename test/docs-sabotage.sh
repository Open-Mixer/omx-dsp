#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
#
# docs-sabotage.sh <dir> — `make docs-check` (the Doxygen documentation gate, #70) proven able to see
# its own defects, each on a COPY of the tree (a git repository of its own with a bare clone as its
# remote `base`, so the tracked-only rule and the base rule apply; the base holds the tree's baseline
# unless an arm says otherwise):
#   new         — an undocumented public function added to an installed header: red, naming it;
#   documented  — the same function with its doc block: green, no row (a documented function warns nothing);
#   listed      — the undocumented function's row in the baseline, in the base too: green, the baseline is
#                 the one allowance;
#   swap        — the listed copy, its probe row replaced by a row nothing warns (the same count): red, naming
#                 the dropped probe row's warning as not listed (the baseline is compared as a set, never as a
#                 count); built on the probe, so it holds over any tree baseline, the empty one included;
#   nonl        — the listed baseline without its last newline: green, the same set;
#   grew        — the function's row added to the baseline copy, the tree's baseline as the base: red,
#                 the baseline only shrinks;
#   stale       — a baseline row nothing warns: red, the row must be dropped;
#   dup         — a baseline row twice: red, the baseline must be sorted and unique;
#   nodoxyfile  — no Doxyfile: red, naming it;
#   nobaseline  — no baseline: red, naming it;
#   nolog       — the Doxyfile's WARN_LOGFILE moved, so the gate's log is missing: red, naming the line;
#   exclude     — the header on the Doxyfile's EXCLUDE line: red, naming the gate's line it replaced;
#   cond        — the function inside @cond: red, naming the command;
#   nofile      — the header's @file removed: red, naming the header;
#   untracked   — an untracked header under include/omxdsp: red, naming it;
#   nogit       — the copy is no git checkout (its .git removed, GIT_DIR pointed nowhere): red, git ls-files failed;
#   update      — `make docs-baseline` on the 'new' copy: red, it may only lower, naming the row it refuses;
#   base-unset  — no DOCS_BASE: red, naming it;
#   base-empty  — DOCS_BASE empty: red, naming it;
#   base-form   — DOCS_BASE without a remote: red, naming the form;
#   base-noref  — the remote has no such branch (a depth-1 single-branch clone's case): red, the fetch failed;
#   base-stale  — the remote moved away while a remote-tracking ref remains: red, the fetch failed, the stale ref no base;
#   base-nofile — the base commit has no baseline: red, nothing to shrink from;
#   base-self   — DOCS_BASE=./HEAD, this clone as the remote: red, head-vs-head is no base;
#   base-head   — the copy reset to its base's commit, so base/main is HEAD: red, head-vs-head is no base.
# The base arms carry the documented probe, so the base is the one thing wrong with them.
# The tree is never edited. Usage: docs-sabotage.sh <scratch-dir>
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
PKG="$(cd "$HERE/.." && pwd)"
OUT="${1:?scratch dir}"
PROBE=omx_docs_sabotage_probe
HEADER=include/omxdsp/fx/omx_instance_io.h
rm -rf "$OUT" && mkdir -p "$OUT"

copy() { # name [probe-doc-block] [probe-tail] — the copy committed on main, a bare clone of it as remote `base`
  local d="$OUT/$1"
  mkdir -p "$d/docs"
  cp -r "$PKG/Makefile" "$PKG/Doxyfile" "$PKG/include" "$PKG/tools" "$PKG/packaging" "$d/"
  cp "$PKG/docs/doxygen-baseline.txt" "$d/docs/"
  printf '\n%sint %s(float x);\n%s' "${2:-}" "$PROBE" "${3:-}" >> "$d/$HEADER"
  track "$d"
  base_of "$d"
}
track() { git -C "$1" -c init.defaultBranch=main init -q && git -C "$1" add -A && git -C "$1" -c user.name=sabotage -c user.email=sabotage@localhost commit -q -m copy; }
base_of() { # dir — a bare clone of the copy as it is committed now, the copy's remote `base` (an absolute
  # path); the copy then moves one empty commit past it, as a branch sits past its base
  local a; a=$(cd "$1" && pwd -P)
  rm -rf "$a.base.git"; git clone -q --bare "$a" "$a.base.git"
  git -C "$a" remote remove base 2>/dev/null || :
  git -C "$a" remote add base "$a.base.git"
  git -C "$a" -c user.name=sabotage -c user.email=sabotage@localhost commit -q --allow-empty -m head
}
check() { # name expected(red|green) [target] [make args] ; the gate's output in $OUT/<name>.log
  local name=$1 want=$2 target=${3:-docs-check} rc=0; shift 2; [ $# -gt 0 ] && shift
  local base=(DOCS_BASE=base/main); [ "${NOBASE:-}" = 1 ] && base=()
  make -s -C "$OUT/$name" "$target" "${base[@]}" "$@" >"$OUT/$name.log" 2>&1 || rc=$?
  if [ "$want" = red ] && [ "$rc" -eq 0 ]; then
    echo "docs-sabotage.sh: FAIL — arm '$name' passed $target; the gate cannot see it"; return 1
  fi
  if [ "$want" = green ] && [ "$rc" -ne 0 ]; then
    echo "docs-sabotage.sh: FAIL — arm '$name' is red (rc $rc) where it must pass:"; cat "$OUT/$name.log"; return 1
  fi
  echo "docs-sabotage.sh: arm '$name' $want as expected"
}
names() { # name words... — the arm's log names every word
  local name=$1 w; shift
  for w in "$@"; do
    grep -q -- "$w" "$OUT/$name.log" || { echo "docs-sabotage.sh: FAIL — the red of arm '$name' does not name $w:"; cat "$OUT/$name.log"; return 1; }
  done
}
probe_row() { sed "s#$(cd "$OUT/new" && pwd -P)/##g" "$OUT/new/build/docs/warnings.txt" | grep "$PROBE"; }
add_rows() { # dir — stdin rows into the copy's baseline, kept sorted
  cat >> "$1/docs/doxygen-baseline.txt"
  LC_ALL=C sort -u -o "$1/docs/doxygen-baseline.txt" "$1/docs/doxygen-baseline.txt"
}

copy new
check new red
names new "$PROBE"

copy documented '/** @brief A probe.\n * @param x A word.\n * @return 0.\n * @thread audio */\n'
check documented green
! grep -q "$PROBE" "$OUT/documented/build/docs/warnings.txt" || { echo "docs-sabotage.sh: FAIL — the documented probe still warns:"; grep "$PROBE" "$OUT/documented/build/docs/warnings.txt"; exit 1; }

copy listed
probe_row | add_rows "$OUT/listed"
git -C "$OUT/listed" add -A && git -C "$OUT/listed" -c user.name=sabotage -c user.email=sabotage@localhost commit -q -m listed && base_of "$OUT/listed"
check listed green

copy swap
probe_row | add_rows "$OUT/swap"
git -C "$OUT/swap" add -A && git -C "$OUT/swap" -c user.name=sabotage -c user.email=sabotage@localhost commit -q -m listed && base_of "$OUT/swap"
before=$(wc -l < "$OUT/swap/docs/doxygen-baseline.txt")
sed -i "s/$PROBE/omx_docs_never_declared/g" "$OUT/swap/docs/doxygen-baseline.txt"
LC_ALL=C sort -u -o "$OUT/swap/docs/doxygen-baseline.txt" "$OUT/swap/docs/doxygen-baseline.txt"
[ "$(wc -l < "$OUT/swap/docs/doxygen-baseline.txt")" = "$before" ] || { echo "docs-sabotage.sh: FAIL — the swap changed the baseline's count"; exit 1; }
check swap red
names swap "$PROBE" "not in docs/doxygen-baseline.txt"

copy nonl
probe_row | add_rows "$OUT/nonl"
git -C "$OUT/nonl" add -A && git -C "$OUT/nonl" -c user.name=sabotage -c user.email=sabotage@localhost commit -q -m listed && base_of "$OUT/nonl"
head -c -1 "$OUT/nonl/docs/doxygen-baseline.txt" > "$OUT/nonl/baseline.nonl" && mv "$OUT/nonl/baseline.nonl" "$OUT/nonl/docs/doxygen-baseline.txt"
check nonl green

copy grew
probe_row | add_rows "$OUT/grew"
check grew red
names grew 'grew against' "$PROBE"

copy stale
{ probe_row; probe_row | sed "s/$PROBE/omx_docs_never_declared/g"; } | add_rows "$OUT/stale"
check stale red
names stale 'documented now' omx_docs_never_declared

copy dup
{ probe_row; probe_row; } >> "$OUT/dup/docs/doxygen-baseline.txt"
check dup red
names dup 'not sorted and unique'

copy nodoxyfile
rm "$OUT/nodoxyfile/Doxyfile"
check nodoxyfile red
names nodoxyfile 'Doxyfile is missing'

copy nolog
probe_row | add_rows "$OUT/nolog"
sed -i 's#^WARN_LOGFILE .*#WARN_LOGFILE = build/docs/elsewhere.txt#' "$OUT/nolog/Doxyfile"
check nolog red
names nolog 'warnings.txt is missing'

copy exclude
sed -i "s#^EXCLUDE  .*#EXCLUDE = include/omxdsp/omxdsp_definitions.h $HEADER#" "$OUT/exclude/Doxyfile"
check exclude red
names exclude "lacks the gate's line: EXCLUDE"

copy cond '/** @cond HIDDEN */\n' '/** @endcond */\n'
check cond red
names cond '@cond' 'conditional doc command'

copy nofile
sed -i 's/@file omx_instance_io.h/the file/' "$OUT/nofile/$HEADER"
probe_row | add_rows "$OUT/nofile"
check nofile red
names nofile "$HEADER" 'without a doc-comment @file'

copy nobaseline
rm "$OUT/nobaseline/docs/doxygen-baseline.txt"
check nobaseline red
names nobaseline 'doxygen-baseline.txt is missing'

copy untracked
probe_row | add_rows "$OUT/untracked"
printf '/** @file omx_docs_scratch.h\n * @brief A scratch header nobody tracks. */\n' > "$OUT/untracked/include/omxdsp/omx_docs_scratch.h"
check untracked red
names untracked 'untracked headers' omx_docs_scratch.h

copy nogit
probe_row | add_rows "$OUT/nogit"
rm -rf "$OUT/nogit/.git"
GIT_DIR="$OUT/nogit/no-such-dir" check nogit red
names nogit 'git ls-files failed'

copy base-unset '/** @brief A probe.\n * @param x A word.\n * @return 0.\n * @thread audio */\n'
NOBASE=1 check base-unset red
names base-unset 'DOCS_BASE is unset'

copy base-empty '/** @brief A probe.\n * @param x A word.\n * @return 0.\n * @thread audio */\n'
check base-empty red docs-check DOCS_BASE=
names base-empty 'DOCS_BASE is unset'

copy base-form '/** @brief A probe.\n * @param x A word.\n * @return 0.\n * @thread audio */\n'
check base-form red docs-check DOCS_BASE=main
names base-form 'is not <remote>/<branch>'

copy base-noref '/** @brief A probe.\n * @param x A word.\n * @return 0.\n * @thread audio */\n'
check base-noref red docs-check DOCS_BASE=base/no-such-branch
names base-noref 'git fetch base no-such-branch failed'

copy base-stale '/** @brief A probe.\n * @param x A word.\n * @return 0.\n * @thread audio */\n'
git -C "$OUT/base-stale" fetch -q base main
git -C "$OUT/base-stale" rev-parse -q --verify base/main >/dev/null || { echo "docs-sabotage.sh: FAIL — arm 'base-stale' has no remote-tracking ref to be stale"; exit 1; }
git -C "$OUT/base-stale" remote set-url base "$(cd "$OUT/base-stale" && pwd -P).moved-away.git"
check base-stale red
names base-stale 'git fetch base main failed'

copy base-self '/** @brief A probe.\n * @param x A word.\n * @return 0.\n * @thread audio */\n'
check base-self red docs-check DOCS_BASE=./HEAD
names base-self 'names this clone or a path as the remote'

copy base-head '/** @brief A probe.\n * @param x A word.\n * @return 0.\n * @thread audio */\n'
git -C "$OUT/base-head" reset -q --hard "$(git -C "$OUT/base-head.base.git" rev-parse HEAD)"
check base-head red
names base-head 'resolves to HEAD'

mkdir -p "$OUT/base-nofile/docs"
cp -r "$PKG/Makefile" "$PKG/Doxyfile" "$PKG/include" "$PKG/tools" "$PKG/packaging" "$OUT/base-nofile/"
printf '\n%sint %s(float x);\n' '/** @brief A probe.\n * @param x A word.\n * @return 0.\n * @thread audio */\n' "$PROBE" >> "$OUT/base-nofile/$HEADER"
track "$OUT/base-nofile" && base_of "$OUT/base-nofile"
cp "$PKG/docs/doxygen-baseline.txt" "$OUT/base-nofile/docs/"
check base-nofile red
names base-nofile 'has no docs/doxygen-baseline.txt'

copy update
check update red docs-baseline
names update 'only lowers' "$PROBE"
cmp -s "$OUT/update/docs/doxygen-baseline.txt" "$PKG/docs/doxygen-baseline.txt" || { echo "docs-sabotage.sh: FAIL — docs-baseline rewrote the baseline it refused"; exit 1; }

echo "docs-sabotage.sh: ok — docs-check sees a new undocumented declaration, a swapped, stale or duplicated row, a grown baseline, a missing Doxyfile, baseline or log, an exclusion, a @cond, a header without @file, an untracked header, a tree without git, and a base unset, empty, malformed, this clone, resolving to HEAD, unreachable, without the branch or without a baseline; passes a documented or listed one and a set without its last newline; docs-baseline only lowers"
