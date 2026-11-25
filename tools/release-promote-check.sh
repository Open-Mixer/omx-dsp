#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# release-promote-check.sh — a v* tag PROMOTES the staged build and never rebuilds (the ruled release mechanism,
# coordinator 2026-11-24, as omx-contract#231): release.yml and publish.yml are read as YAML and held to it.
#
#   tools/release-promote-check.sh             the check of the tree's workflows
#   tools/release-promote-check.sh --sabotage  each careless edit below, planted in a copy, must be named red
#
# release.yml: the rpm and deb build jobs never run on a tag and never publish (publish: false), and take the matrix
# input; the matrix input offers quick and full, quick by default; a check job holds the tag to `make version` and to
# main; a promote job calls Open-Mixer/.github promote-staged.yml@openmixer-v1 on a tag (after check) or a dispatched
# version, with binary-names the packages BOTH build jobs list, the rpm architectures the rpm build's, and the secrets.
# publish.yml: no push trigger (npm never moves on the bare tag), a workflow_run of release, a gate that refuses a run
# with no promote job or a promote job that did not succeed, and a checkout of the tag's commit.
set -uo pipefail
cd "$(dirname "$0")/.." || exit 2
REL="${RELEASE_YML:-.github/workflows/release.yml}"
PUB="${PUBLISH_YML:-.github/workflows/publish.yml}"

check() { # <release.yml> <publish.yml>: one line per finding, exit 1 on any
  python3 -I - "$1" "$2" <<'PY'
import sys, yaml
rel = yaml.load(open(sys.argv[1]), Loader=yaml.BaseLoader)
pub = yaml.load(open(sys.argv[2]), Loader=yaml.BaseLoader)
bad = []
jobs = rel.get("jobs", {})
on = rel.get("on", {})
disp = (on.get("workflow_dispatch") or {}).get("inputs", {})
m = disp.get("matrix", {})
if m.get("options") != ["quick", "full"] and sorted(m.get("options") or []) != ["full", "quick"]:
    bad.append("release.yml: the matrix input does not offer quick and full")
if m.get("default") != "quick":
    bad.append("release.yml: the matrix input's default is not quick (a staged run is the quick set unless a promotion candidate asks for full)")
for k in ("version", "commit"):
    if k not in disp:
        bad.append(f"release.yml: no {k} input for a dry run of the promotion")
names = {}
for j in ("rpm", "deb"):
    job = jobs.get(j, {})
    w = job.get("with", {})
    if "github.ref_type != 'tag'" not in job.get("if", ""):
        bad.append(f"release.yml: the {j} build runs on a tag (its if does not exclude github.ref_type == 'tag')")
    if w.get("publish") != "false":
        bad.append(f"release.yml: the {j} build publishes ({w.get('publish')!r}): a build only stages")
    if "inputs.matrix" not in w.get("matrix", ""):
        bad.append(f"release.yml: the {j} build does not take the matrix input")
    names[j] = set(w.get("binary-names", "").split())
chk = jobs.get("check", {})
steps = " ".join(s.get("run", "") for s in chk.get("steps", []))
if "make -s version" not in steps:
    bad.append("release.yml: no check job holds the tag to make version")
if "merge-base --is-ancestor" not in steps or "origin/main" not in steps:
    bad.append("release.yml: no check job holds the tag's commit to main")
pr = jobs.get("promote", {})
pw = pr.get("with", {})
if not pr.get("uses", "").startswith("Open-Mixer/.github/.github/workflows/promote-staged.yml@"):
    bad.append("release.yml: no promote job calls promote-staged.yml")
if "github.ref_type == 'tag'" not in pr.get("if", "") or "check" not in str(pr.get("needs", "")):
    bad.append("release.yml: the promote job does not run on a tag after check")
if "inputs.version" not in pr.get("if", ""):
    bad.append("release.yml: the promote job has no dry run on a dispatched version")
pn = set(pw.get("binary-names", "").split())
both = names.get("rpm", set()) & names.get("deb", set())
if not pn or not pn <= both:
    bad.append(f"release.yml: the promotion's binary-names {sorted(pn)} are not packages both builds list {sorted(both)}")
if pw.get("rpm-arch-matrix") != jobs.get("rpm", {}).get("with", {}).get("arch-matrix"):
    bad.append("release.yml: the promotion's rpm architectures are not the rpm build's")
if pr.get("secrets") != "inherit":
    bad.append("release.yml: the promote job does not inherit the secrets")
pon = pub.get("on", {})
if "push" in pon:
    bad.append("publish.yml: a push trigger publishes npm on the bare tag")
wr = pon.get("workflow_run") or {}
if wr.get("workflows") != ["release"] or wr.get("types") != ["completed"]:
    bad.append("publish.yml: it does not run on the release workflow's completion")
gate = " ".join(s.get("run", "") for j in pub.get("jobs", {}).values() for s in j.get("steps", []))
if "grep -c '^promote / '" not in gate or '-gt 0' not in gate or '$2 != "success"' not in gate or "'Promote the DEBs'" not in gate:
    bad.append("publish.yml: no gate refuses a release run whose promote jobs did not all succeed")
co = [s for s in pub.get("jobs", {}).get("publish", {}).get("steps", []) if s.get("uses", "").startswith("actions/checkout")]
if not co or "env.SHA" not in co[0].get("with", {}).get("ref", ""):
    bad.append("publish.yml: the publish does not check out the tag's commit")
print("\n".join(bad))
sys.exit(1 if bad else 0)
PY
}

if [ "${1:-}" = --sabotage ]; then
  tmp=$(mktemp -d) || exit 2; trap 'rm -rf "$tmp"' EXIT; rc=0
  plant() { # <name> <file: rel|pub> <python replace old> <new> <expected finding pattern>
    cp "$REL" "$tmp/rel.yml"; cp "$PUB" "$tmp/pub.yml"
    python3 -I -c 'import sys; p,o,n=sys.argv[1:4]; s=open(p).read(); assert o in s, "the plant matched nothing: "+o; open(p,"w").write(s.replace(o,n,1))' "$tmp/$2.yml" "$3" "$4" || { echo "FAIL sabotage $1: nothing to plant"; rc=1; return; }
    out=$(check "$tmp/rel.yml" "$tmp/pub.yml") && { echo "FAIL sabotage $1: still green"; rc=1; return; }
    grep -qE "$5" <<< "$out" || { echo "FAIL sabotage $1: red, but not by name: $out"; rc=1; return; }
    echo "ok   sabotage $1: $(grep -m1 -E "$5" <<< "$out")"
  }
  plant "the rpm build back on the tag" rel "    if: github.ref_type != 'tag' && !inputs.version
    uses: Open-Mixer/.github/.github/workflows/build-rpm.yml" "    uses: Open-Mixer/.github/.github/workflows/build-rpm.yml" 'rpm build runs on a tag'
  plant "the deb build publishes" rel "      binary-names: libomxdsp-dev omx-rt-gate omx-dsp-contract
      publish: false" "      binary-names: libomxdsp-dev omx-rt-gate omx-dsp-contract
      publish: \${{ github.event_name == 'push' }}" 'deb build publishes'
  plant "no promote job" rel "promote-staged.yml@openmixer-v1" "build-rpm.yml@openmixer-v1" 'no promote job calls'
  plant "full by default" rel "        options: [quick, full]
        default: quick" "        options: [quick, full]
        default: full" "default is not quick"
  plant "a binary only the deb builds" rel "      binary-names: omx-rt-gate
      rpm-arch-matrix" "      binary-names: omx-rt-gate omx-dsp-contract
      rpm-arch-matrix" 'not packages both builds list'
  plant "no on-main step" rel "git merge-base --is-ancestor" "git merge-base" 'commit to main'
  plant "no secrets" rel "      commit: \${{ inputs.commit || '' }}
    secrets: inherit" "      commit: \${{ inputs.commit || '' }}" 'inherit the secrets'
  plant "npm on the bare tag" pub "on:
  workflow_run:" "on:
  push:
    tags: ['v*']
  workflow_run:" 'push trigger publishes npm'
  plant "the channel jobs not required" pub "'Promote the RPMs' 'Promote the DEBs'" "'Promote the RPMs'" 'no gate refuses'
  plant "no promote gate" pub "grep -c '^promote / '" "grep -c '^anything / '" 'no gate refuses'
  [ $rc = 0 ] && echo "release-promote-check --sabotage: every planted edit named"; exit $rc
fi

out=$(check "$REL" "$PUB"); rc=$?
[ -n "$out" ] && printf '%s\n' "$out"
[ $rc = 0 ] && echo "release-promote-check: a tag promotes the staged build, never rebuilds; npm follows the promotion" || echo "release-promote-check: FAIL"
exit $rc
