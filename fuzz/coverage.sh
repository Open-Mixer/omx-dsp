#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# THE COVERAGE WITNESS (`make fuzz-coverage`). Each harness, built for llvm-cov without
# sanitizers, runs once over its seed corpus (-runs=0 executes the corpus and nothing else);
# this prints the line coverage of the library the harness reaches, every line of src/ and
# include/ the build of the harness holds, file by file. The files are what the harness
# includes and links, not a list kept here.
#
# THE FLOOR. fuzz/coverage-floor.txt holds, per harness, the number of covered lines of the
# library the committed corpus reaches. A harness whose covered lines fall below its floor
# turns this red: a seed lost, or a harness that stopped driving a function. Raise a floor
# when coverage rises; lower one only together with the change that removed the lines.
#
# THE HARNESS FLOOR. Every property check of the harness itself, each abort() in
# fuzz/fuzz_<harness>.c and the statement that guards it (fuzz/checks.awk), must be executed
# by at least one seed. A check no seed reaches is a property nobody tests, and the
# library's line count cannot see it; this names the harness file and the line of every
# such check and turns red. So does a check whose comment holds no `mutant:` and `input:`
# record and no `not a property:` line (fuzz/records.sh proves the records): a check that
# names no bug it catches is a property nobody showed can fail. A stop after a failed
# allocation of the harness's own is not a property, and no seed is owed to reach it.
#
# Usage: coverage.sh <cov build dir> <harness>...   (LLVM_COV names llvm-cov)
set -eu
B=$1
shift
LLVM_COV=${LLVM_COV:-llvm-cov}
FLOOR=fuzz/coverage-floor.txt
[ -f "$FLOOR" ] || { echo "coverage.sh: no $FLOOR" >&2; exit 2; }
low=
dead=

for h in "$@"; do
	bin=$B/fuzz_$h prof=$B/$h.profdata
	echo "== fuzz_$h: $(ls fuzz/corpus/$h | wc -l) seed inputs"
	# Filename Regions Missed Cover Functions Missed Executed Lines Missed Cover ...: the files
	# of src/ and include/ the harness reached, and the total of all it holds
	tot=$B/$h.covered
	"$LLVM_COV" report "$bin" -instr-profile="$prof" src include |
	awk -v tot="$tot" '
		NF < 10 || $1 == "Filename" { next }
		$1 == "TOTAL" { printf "  %-40s lines %4d  missed %4d  %s\n", $1, $8, $9, $10; print $8 - $9 > tot; next }
		$8 > $9 { printf "  %-40s lines %4d  missed %4d  %s\n", $1, $8, $9, $10 }'
	src=fuzz/fuzz_$h.c
	"$LLVM_COV" show "$bin" -instr-profile="$prof" "$src" |
		awk -v file="$src" -f fuzz/checks.awk >"$B/$h.checks"
	nchk=$(wc -l <"$B/$h.checks")
	if [ "$nchk" -eq 0 ]; then
		echo "  CHECKS: no property check found in $src"
		dead="$dead $h"
	elif awk -F: '$4 == "0" && $5 != "not-a-property" { n++ } END { exit !n }' "$B/$h.checks"; then
		awk -F: '$4 == "0" && $5 != "not-a-property" { t = $0; sub(/^([^:]*:){5}/, "", t)
			printf "  CHECK: %s:%d no seed reaches (abort at line %d): %s\n", $1, $2, $3, t }' \
			"$B/$h.checks"
		dead="$dead $h"
	elif awk -F: '$5 == "NONE" || $5 == "BAD" { n++ } END { exit !n }' "$B/$h.checks"; then
		awk -F: '$5 == "NONE" || $5 == "BAD" { t = $0; sub(/^([^:]*:){5}/, "", t)
			printf "  RECORD: %s:%d has no mutant: and input: record and is no stop after a failed allocation (%s): %s\n",
			       $1, $3, $5, t }' "$B/$h.checks"
		dead="$dead $h"
	else
		echo "  checks: all $nchk property checks of $src reached"
	fi
	covered=$(cat "$tot")
	floor=$(awk -v h="$h" '$1 == h { print $2 }' "$FLOOR")
	if [ -z "$floor" ]; then
		echo "  FLOOR: fuzz_$h has no line in $FLOOR"
		low="$low $h"
	elif [ "$covered" -lt "$floor" ]; then
		echo "  FLOOR: fuzz_$h covers $covered library lines, below its floor of $floor"
		low="$low $h"
	else
		echo "  floor $floor: held ($covered covered)"
	fi
done

[ -z "$low" ] || echo "make fuzz-coverage: RED -- below the coverage floor:$low"
[ -z "$dead" ] || echo "make fuzz-coverage: RED -- harness checks no seed reaches, or with no record:$dead"
[ -z "$low$dead" ] || exit 1
echo "make fuzz-coverage: every harness at or above its coverage floor, every check reached"
