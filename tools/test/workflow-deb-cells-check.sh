#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
# Refuses a Debian CI job that picks its own runner or its own architectures: the jobs ci.yml test, deb, contract-floor
# and twins, and publish.yml publish, take their cell (image, architecture and runner) from the check-matrix action,
# so each says `runs-on: ${{ matrix.cell.runner }}`, needs the checks job, reads its cell from that job's outputs and
# writes no amd64, matrix include or ubuntu: image of its own: while the release matrix is quick the action renders no amd64 deb cell, and a job that
# names amd64 would bring one back. The workflow also calls the action with the x86 runner it already used.
# Usage: workflow-deb-cells-check.sh [repo-root] (default: .), or --self-test, which runs this check on planted trees.
# Exit 0 clean, 1 a hit (each line named), 2 no workflow to check.
set -eu
self=$0

jobs_of() { # <file>: the jobs this check names in that file
	case $1 in ci.yml) echo test deb contract-floor twins ;; publish.yml) echo publish ;; esac
}

check() {
	local wf=$1/.github/workflows f j block bad=0
	for f in ci.yml publish.yml; do
		if [ ! -f "$wf/$f" ]; then
			echo "workflow-deb-cells-check: no $wf/$f to check (refusing: an empty check proves nothing)" >&2
			return 2
		fi
	done
	for f in ci.yml publish.yml; do
		grep -qF 'Open-Mixer/.github/.github/actions/check-matrix@openmixer-v1' "$wf/$f" ||
			{ echo "$f: no check-matrix step" >&2; bad=1; }
		grep -qF "x86-runner: \${{ vars.OMX_RUNNER || 'ubuntu-latest' }}" "$wf/$f" ||
			{ echo "$f: check-matrix is not given the x86 runner (x86-runner: vars.OMX_RUNNER || 'ubuntu-latest')" >&2; bad=1; }
		for j in $(jobs_of "$f"); do
			block=$(awk -v j="$j" '$0 ~ "^  " j ":[[:space:]]*$" { on = 1; next } on && /^  [A-Za-z0-9_-]+:[[:space:]]*$/ { exit } on' "$wf/$f")
			if [ -z "$block" ]; then echo "$f: job $j not found" >&2; bad=1; continue; fi
			grep -qE '^    runs-on: \$\{\{ matrix\.cell\.runner \}\}[[:space:]]*$' <<< "$block" ||
				{ echo "$f: job $j does not take runs-on from matrix.cell.runner" >&2; bad=1; }
			grep -qE '^    needs: .*checks' <<< "$block" ||
				{ echo "$f: job $j does not need the checks job" >&2; bad=1; }
			grep -qE 'cell: \$\{\{ fromJSON\(needs\.checks\.outputs\.[a-z-]+\) \}\}' <<< "$block" ||
				{ echo "$f: job $j reads no cell from the checks job's outputs" >&2; bad=1; }
			if grep -nE '^        include:|ubuntu:' <<< "$block" | sed "s|^|$f: job $j has an include or an ubuntu: image outside the check-matrix cells: |" | grep . >&2; then bad=1; fi
			if grep -n 'amd64' <<< "$block" | sed "s|^|$f: job $j names amd64: |" | grep . >&2; then bad=1; fi
		done
	done
	if [ "$bad" != 0 ]; then
		echo "workflow-deb-cells-check: a Debian job does not take its cell and runner from check-matrix" >&2
		return 1
	fi
	echo "workflow-deb-cells-check: the Debian jobs take their cells and runner from check-matrix"
}

if [ "${1:-}" = "--self-test" ]; then
	t=$(mktemp -d)
	trap 'rm -rf "$t"' EXIT
	fails=0
	act='Open-Mixer/.github/.github/actions/check-matrix@openmixer-v1'
	x86="x86-runner: \${{ vars.OMX_RUNNER || 'ubuntu-latest' }}"
	job() { # <name> <runs-on> <extra matrix line>
		printf '  %s:\n    needs: checks\n    runs-on: %s\n    strategy:\n      matrix:\n        cell: ${{ fromJSON(needs.checks.outputs.deb) }}\n%s\n' "$1" "$2" "$3"
	}
	good_runs='${{ matrix.cell.runner }}'
	tree() { # <name> <ci job runs-on> <ci extra> <ci action> <ci x86>
		local d=$t/$1
		mkdir -p "$d/.github/workflows"
		{
			printf 'jobs:\n  checks:\n    steps:\n      - uses: %s\n        with:\n          %s\n' "$4" "$5"
			for j in test deb contract-floor twins; do job "$j" "$2" "$3"; done
		} > "$d/.github/workflows/ci.yml"
		{
			printf 'jobs:\n  checks:\n    steps:\n      - uses: %s\n        with:\n          %s\n' "$act" "$x86"
			job publish "$good_runs" ''
		} > "$d/.github/workflows/publish.yml"
	}
	expect() { # <name> <rc>
		local rc=0
		"$self" "$t/$1" > /dev/null 2>&1 || rc=$?
		if [ "$rc" != "$2" ]; then echo "self-test FAIL $1: rc $rc, want $2" >&2; fails=$((fails + 1)); fi
	}
	tree clean "$good_runs" '' "$act" "$x86"; expect clean 0
	tree hand 'ubuntu-latest' '' "$act" "$x86"; expect hand 1
	tree var "\${{ vars.OMX_RUNNER || 'ubuntu-latest' }}" '' "$act" "$x86"; expect var 1
	tree include "$good_runs" '        include:\n          - cell: x' "$act" "$x86"; expect include 1
	tree ubuntu "$good_runs" '    container: ubuntu:24.04' "$act" "$x86"; expect ubuntu 1
	tree amd64 "$good_runs" '        arch: [amd64]' "$act" "$x86"; expect amd64 1
	tree noaction "$good_runs" '' 'actions/checkout@v4' "$x86"; expect noaction 1
	tree nox86 "$good_runs" '' "$act" 'wave: x'; expect nox86 1
	mkdir -p "$t/empty"; expect empty 2
	[ "$fails" = 0 ] || exit 1
	echo "workflow-deb-cells-check: self-test ok"
	exit 0
fi

check "${1:-.}"
