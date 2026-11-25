#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# `make fuzz-sabotage`: A PROPERTY NO MUTANT CAN BREAK CHECKS NOTHING. fuzz/sabotage/
# <harness>-<property>.patch is a bug in the code the harness drives, at least one for each
# harness, as `git diff` writes it (a further bug of the same property is
# <harness>-<property>.<what>.patch); above its `diff --git` line it names the check it breaks
# and the seed that shows it, as OSS Verify reads a breakage patch:
#
#   Breaks the check of fuzz/fuzz_<harness>.c:<abort line> (<what the bug is>).
#   Caught by the seed input "<seed>" of fuzz_<harness>.
#
# This applies each, rebuilds the sanitizer harness, and runs it over every seed of its
# corpus, one at a time; no fuzzing, so the corpus alone must catch it. A patch is found when
# the seed it names aborts with the message of the check it names: the FUZZ_SAY() of that
# check, its conversions read as any text. The optimizer merges the abort() calls of a harness,
# so the stack cannot tell one check from another; the line is fuzz/records.sh's to prove, in
# a build that keeps them apart. Every seed that fails must fail by a check's abort(): a bug
# the harness reads in two ways may stop another seed at another check, but a sanitizer report
# or a crash outside a check makes it red. The source is restored with `git checkout --` on
# every exit path, and the sanitizer build is rebuilt as the tree is at the end.
#
# Usage: FUZZ_BUILD=build/fuzz MAKE=make sabotage.sh
set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"
B=${FUZZ_BUILD:-build/fuzz}
MAKE=${MAKE:-make}
OUT=$B/sabotage
mkdir -p "$OUT"

patched=
restore() {
	[ -n "$patched" ] && git checkout -- $patched
	patched=
}
trap 'restore' EXIT
trap 'restore; exit 130' INT TERM

# a tree git cannot read (in a CI container: a checkout another user owns, no safe.directory) is no proof of a change:
# there git diff runs outside any repository and answers 1, so the tree is asked for first and named as what it is
if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
	echo "fuzz-sabotage: git cannot read this tree as a repository (a checkout another user owns needs safe.directory); refusing to patch it" >&2
	exit 2
fi
if ! git diff --quiet -- src include; then
	echo "fuzz-sabotage: src/ or include/ has uncommitted changes; refusing to patch over them" >&2
	exit 2
fi

# the message of the check whose abort() is on line $2 of $1, as an extended regular
# expression over the line FUZZ_SAY() writes: the format string of the last FUZZ_SAY() above
# the abort, a %s any text and a number's conversion any word, so no check's message is read as
# another's
said() {
	awk -v l="$2" 'NR <= l && /FUZZ_SAY\("/ { s = $0 } NR == l { print s; exit }' "$1" |
	sed -n 's/^.*FUZZ_SAY("\(\([^"\\]\|\\.\)*\)".*$/\1/p' |
	sed -e 's/[][\.*+?(){}|^$]/\\&/g' -e 's/%[-+ #0-9.]*s/.*/g' -e 's/%[-+ #0-9.]*l*[a-zA-Z]/[^ ]*/g' \
		-e 's/^/^fuzz property broken: /' -e 's/$/$/'
}

# whether the log $1 is a check's abort(): a property's message, then libFuzzer's word on the
# signal, and no sanitizer's report
by_check() {
	grep -q '^fuzz property broken: ' "$1" && grep -q 'ERROR: libFuzzer: deadly signal' "$1" &&
		! grep -qE 'ERROR: [A-Za-z]+Sanitizer|runtime error:' "$1"
}

missed=
n=0
for p in fuzz/sabotage/*.patch; do
	[ -f "$p" ] || continue
	m=$(basename "$p" .patch)
	h=${m%%-*}
	n=$((n + 1))
	# the check and the seed its header names
	at=$(sed -n "s|^Breaks the check of fuzz/fuzz_$h\.c:\([0-9]*\) (.*)\.\$|\1|p" "$p" | head -n 1)
	named=$(sed -n "s|^Caught by the seed input \"\([^\"]*\)\" of fuzz_$h\.\$|\1|p" "$p" | head -n 1)
	if [ -z "$at" ] || [ -z "$named" ] || [ ! -f "fuzz/corpus/$h/$named" ]; then
		echo "$m: the header does not name a check of fuzz/fuzz_$h.c and a seed of fuzz/corpus/$h/"
		missed="$missed $m"
		continue
	fi
	want=$(said "fuzz/fuzz_$h.c" "$at")
	if [ -z "$want" ] || ! sed -n "${at}p" "fuzz/fuzz_$h.c" | grep -q 'abort();'; then
		echo "$m: fuzz/fuzz_$h.c:$at is no abort() with a FUZZ_SAY() above it"
		missed="$missed $m"
		continue
	fi
	files=$(git apply --numstat "$p" | awk '{ print $3 }')
	if ! git apply "$p"; then
		echo "$m: $p no longer applies"
		missed="$missed $m"
		continue
	fi
	patched=$files
	echo "== $m: fuzz_$h against $(grep -m1 '^+[^+]' "$p" | sed 's/^+[[:space:]]*//')"
	if ! $MAKE --no-print-directory "$B/fuzz_$h" >"$OUT/$m.build.log" 2>&1; then
		cat "$OUT/$m.build.log"
		echo "$m: the mutant does not build"
		missed="$missed $m"
		restore
		continue
	fi
	found=0
	own=0
	other=
	for seed in fuzz/corpus/$h/*; do
		log=$OUT/$m.$(basename "$seed").log
		"$B/fuzz_$h" -runs=1 -artifact_prefix="$OUT/$m-" "$seed" >"$log" 2>&1 && { rm -f "$log"; continue; }
		if by_check "$log"; then
			found=$((found + 1))
			grep -qE "$want" "$log" && own=$((own + 1))
		else
			other="$other $(basename "$seed")($(grep -m1 -E '^SUMMARY: |ERROR: ' "$log"))"
		fi
	done
	restore
	log=$OUT/$m.$named.log
	if [ -n "$other" ]; then
		echo "$m: seeds that failed outside a check:$other"
		echo "$m: RED -- the patch is not read as a broken property (logs in $OUT/)"
		missed="$missed $m"
	elif [ ! -f "$log" ] || ! grep -qE "$want" "$log"; then
		echo "$m: RED -- the seed $named does not abort at fuzz/fuzz_$h.c:$at"
		missed="$missed $m"
	else
		echo "$m: found by $found seeds, $own at fuzz/fuzz_$h.c:$at, $named among them"
	fi
done

# leave the sanitizer build as the tree is, not as the last mutant was
$MAKE --no-print-directory fuzz >"$OUT/rebuild.log" 2>&1 || { cat "$OUT/rebuild.log"; exit 1; }

if [ "$n" -eq 0 ]; then
	echo "make fuzz-sabotage: RED -- no patch in fuzz/sabotage/"
	exit 1
fi
if [ -n "$missed" ]; then
	echo "make fuzz-sabotage: RED --$missed"
	exit 1
fi
echo "make fuzz-sabotage: all $n patches found, each at the check its header names"
