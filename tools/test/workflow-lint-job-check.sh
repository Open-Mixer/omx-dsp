#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
# Refuses a lint step outside ci.yml's lint job: the licence check (spdx-check), the dependency render (deps-render)
# and the floor check (tools/no-floor-check.sh) run in the job `lint` and in no other job, so a lint red fails the run
# in its own job and never skips a step of the suite (a lint step before make test in the same job did). The job
# `lint` must hold all three. Usage: workflow-lint-job-check.sh [repo-root] (default: .), or --self-test, which runs
# this check on planted workflows: a lint step back in the test job, one missing from lint, and a clean one.
# Exit 0 clean, 1 a hit (each named), 2 no ci.yml to check.
set -eu
self=$0
LINTS='spdx-check deps-render no-floor-check'

# jobs_of <ci.yml>: "<job> <lint>" for every lint step, one per line
jobs_of() {
	awk '
		/^jobs:[[:space:]]*$/ { in_jobs = 1; next }
		in_jobs && /^[^ #]/ { in_jobs = 0 }
		in_jobs && /^  [A-Za-z0-9_-]+:[[:space:]]*$/ { job = $1; sub(/:$/, "", job); next }
		in_jobs && job != "" && !/^[[:space:]]*#/ {
			if ($0 ~ /actions\/spdx-check@/) print job, "spdx-check"
			if ($0 ~ /actions\/deps-render@/) print job, "deps-render"
			if ($0 ~ /tools\/no-floor-check\.sh/) print job, "no-floor-check"
		}
	' "$1"
}

check() {
	local f=$1/.github/workflows/ci.yml bad=0 l
	[ -f "$f" ] || { echo "workflow-lint-job-check: no $f to check (refusing: an empty check proves nothing)" >&2; return 2; }
	local found
	found=$(jobs_of "$f")
	while read -r job lint; do
		[ -n "$job" ] || continue
		[ "$job" = lint ] || { echo "$f: job $job runs the lint step $lint: it belongs in the job lint alone"; bad=1; }
	done <<EOF
$found
EOF
	for l in $LINTS; do
		grep -qx "lint $l" <<<"$found" || { echo "$f: the job lint does not run $l"; bad=1; }
	done
	[ "$bad" = 0 ] || { echo "workflow-lint-job-check: a lint step outside the lint job, or missing from it" >&2; return 1; }
}

if [ "${1:-}" = --self-test ]; then
	t=$(mktemp -d); trap 'rm -rf "$t"' EXIT
	mkdir -p "$t/ok/.github/workflows" "$t/in-test/.github/workflows" "$t/missing/.github/workflows"
	clean='jobs:
  lint:
    steps:
      - uses: Open-Mixer/.github/.github/actions/spdx-check@openmixer-v1
      - uses: Open-Mixer/.github/.github/actions/deps-render@openmixer-v1
      - run: bash tools/no-floor-check.sh
  test:
    steps:
      # a comment naming tools/no-floor-check.sh is no step
      - run: make test'
	printf '%s\n' "$clean" > "$t/ok/.github/workflows/ci.yml"
	printf '%s\n' "$clean" | sed 's|      - run: make test|      - uses: Open-Mixer/.github/.github/actions/spdx-check@openmixer-v1\n      - run: make test|' > "$t/in-test/.github/workflows/ci.yml"
	printf '%s\n' "$clean" | sed '/deps-render/d' > "$t/missing/.github/workflows/ci.yml"
	"$self" "$t/ok" >/dev/null || { echo "workflow-lint-job-check --self-test: FAIL — the clean workflow was refused" >&2; exit 1; }
	for c in in-test missing; do
		if "$self" "$t/$c" >/dev/null 2>&1; then echo "workflow-lint-job-check --self-test: FAIL — $c was accepted" >&2; exit 1; fi
	done
	rc=0; "$self" "$t/none" >/dev/null 2>&1 || rc=$?
	[ "$rc" = 2 ] || { echo "workflow-lint-job-check --self-test: FAIL — no ci.yml answered $rc, not 2" >&2; exit 1; }
	echo "workflow-lint-job-check: self-test ok (a lint step in the test job and one missing from lint refused, a clean workflow taken)"
	exit 0
fi

check "${1:-.}"
echo "workflow-lint-job-check: spdx-check, deps-render and no-floor-check run in the job lint alone"
