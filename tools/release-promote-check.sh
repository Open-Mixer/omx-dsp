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
# Publish only by tagging (Open-Mixer/.github#126; omx-dsp#94, where a dispatch of a tag ref with publish=true attached
# the API reference to the release): no trigger declares a publish input and nothing reads inputs.publish (a dispatch is a
# dry run), and every step that writes to the GitHub release (gh release upload, create or edit) runs under an if, its
# own or its job's, holding both github.event_name == 'push' and startsWith(github.ref, 'refs/tags/').
set -uo pipefail
cd "$(dirname "$0")/.." || exit 2
REL="${RELEASE_YML:-.github/workflows/release.yml}"
PUB="${PUBLISH_YML:-.github/workflows/publish.yml}"

check() { # <release.yml> <publish.yml>: one line per finding, exit 1 on any
  python3 -I - "$1" "$2" <<'PY'
import re, sys, yaml
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
# publish only by tagging (omx-dsp#94): a dispatch is a dry run, whatever its inputs say
for trigger in ("workflow_dispatch", "workflow_call"):
    if "publish" in ((on.get(trigger) or {}).get("inputs") or {}):
        bad.append(f"release.yml: a publish input under {trigger} (a dispatch is a dry run: only a tag push publishes)")
if "inputs.publish" in open(sys.argv[1]).read():
    bad.append("release.yml: inputs.publish is read, an input no trigger declares (a dispatch publish is gone)")
TAG_PUSH = ("github.event_name == 'push'", "startsWith(github.ref, 'refs/tags/')")
tag_push = lambda cond: all(t in cond for t in TAG_PUSH)
releasing = 0
for j, job in jobs.items():
    for s in job.get("steps", []) or []:
        verb = re.search(r"gh release (upload|create|edit)", s.get("run", ""))
        if not verb:
            continue
        releasing += 1
        if not (tag_push(s.get("if", "")) or tag_push(job.get("if", ""))):
            bad.append(f"release.yml: job {j} step '{s.get('name') or verb.group(0)}' writes to the release (gh release {verb.group(1)}) under if `{s.get('if') or job.get('if') or ''}`: not held to a tag push ({' and '.join(TAG_PUSH)})")
if not releasing:
    bad.append("release.yml: no step writes to the GitHub release (the API reference is attached by one): nothing for the tag-push law to hold")
pon = pub.get("on", {})
if "push" in pon:
    bad.append("publish.yml: a push trigger publishes npm on the bare tag")
wr = pon.get("workflow_run") or {}
if wr.get("workflows") != ["release"] or wr.get("types") != ["completed"]:
    bad.append("publish.yml: it does not run on the release workflow's completion")
gate = " ".join(s.get("run", "") for j in pub.get("jobs", {}).values() for s in j.get("steps", []))
if "grep -c '^promote / '" not in gate or '-gt 0' not in gate or '$2 != "success"' not in gate or "'Promote the DEBs'" not in gate or ", nothing is published$/" not in gate:
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
      version:" "      binary-names: omx-rt-gate omx-dsp-contract
      version:" 'not packages both builds list'
  plant "no on-main step" rel "git merge-base --is-ancestor" "git merge-base" 'commit to main'
  plant "no secrets" rel "      commit: \${{ inputs.commit || '' }}
    secrets: inherit" "      commit: \${{ inputs.commit || '' }}" 'inherit the secrets'
  plant "the API reference attached on a dispatch of a tag ref (omx-dsp#94)" rel "        if: \${{ github.ref_type == 'tag' && (github.event_name == 'push' && startsWith(github.ref, 'refs/tags/')) }}" "        if: \${{ github.ref_type == 'tag' && (github.event_name == 'push' || inputs.publish == true) }}" "step 'Attach to the release' writes to the release .* not held to a tag push"
  plant "a publish dispatch input" rel "      staged:
        description: 'Build and publish to the staging channel" "      publish:
        description: 'Sign and publish'
        type: boolean
        default: false
      staged:
        description: 'Build and publish to the staging channel" 'publish input under workflow_dispatch'
  plant "a read of inputs.publish" rel "          name: omx-dsp-api-docs
          path: omx-dsp-api-docs-*.tar.gz" "          name: omx-dsp-api-docs
          path: omx-dsp-api-docs-*.tar.gz
          retention-days: \${{ inputs.publish && 90 || 2 }}" 'inputs.publish is read'
  plant "the attach step moved out of its if" rel "        if: \${{ github.ref_type == 'tag' && (github.event_name == 'push' && startsWith(github.ref, 'refs/tags/')) }}
        env:" "        env:" "step 'Attach to the release' writes to the release .* under if .*needs.promote.result.*: not held to a tag push"
  plant "npm on the bare tag" pub "on:
  workflow_run:" "on:
  push:
    tags: ['v*']
  workflow_run:" 'push trigger publishes npm'
  plant "the channel jobs not required" pub "'Promote the RPMs' 'Promote the DEBs'" "'Promote the RPMs'" 'no gate refuses'
  plant "any skipped promote job passes" pub '&& !($2 == "skipped" && $1 ~ /, nothing is published$/)' '&& $2 != "skipped"' 'no gate refuses'
  plant "no promote gate" pub "grep -c '^promote / '" "grep -c '^anything / '" 'no gate refuses'
  [ $rc = 0 ] && echo "release-promote-check --sabotage: every planted edit named"; exit $rc
fi

out=$(check "$REL" "$PUB"); rc=$?
[ -n "$out" ] && printf '%s\n' "$out"
[ $rc = 0 ] && echo "release-promote-check: a tag promotes the staged build, never rebuilds; npm follows the promotion" || echo "release-promote-check: FAIL"
exit $rc
