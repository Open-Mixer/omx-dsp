#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# no-floor-check.sh — no hand-kept version floor on an OpenMixer package (owner ruling 2026-11-24). This level
# builds and installs against the lower level's published package and nothing else; a relation that must exist
# is generated (rpm's automatic soname dependencies, `= %{version}` of what it was built against), never typed.
# Refused, by file and line:
#   packaging/deps.txt                     the floors file itself
#   packaging/*.spec                       a Requires/BuildRequires/Recommends `<ours> >= x` (or > x)
#   debian/control                         a `<ours> (>= x)` (or >> x)
#   *.pc.in                                a Requires `<ours> >= x`
#   package.json, ts/package.json          a dependency on `@openmixer/...` with a typed range
#   data/ and recipes/ JSON                a `requires` or `x-requires` that states a version (">= x.y.z")
# <ours>: omx-*, libomx*, plugin-*, openmixer*. `= ${source:Version}` / `= %{version}-%{release}` of this
# source's own packages are generated relations and stay.
#
#   tools/no-floor-check.sh [ROOT]      ROOT defaults to this tree
#   tools/no-floor-check.sh --self-test plants each floor in a scratch copy and requires red, then the copy green
set -euo pipefail
HERE="$(cd "$(dirname "$0")/.." && pwd)"
OURS='(omx-[a-z0-9-]+|libomx[a-z0-9-]*|plugin-[a-z0-9-]+|openmixer[a-z0-9-]*)'

scan() {
  local root="$1" f
  [ ! -e "$root/packaging/deps.txt" ] || echo "packaging/deps.txt: a floors file"
  for f in "$root"/packaging/*.spec; do
    [ -f "$f" ] || continue
    awk -v ours="^$OURS\$" -v rel="${f#"$root"/}" '
      /^%changelog/ { exit }
      /^(Build)?(Requires|Recommends)(\([a-z]+\))?:/ {
        n = split(substr($0, index($0, ":") + 1), dep, ",")
        for (i = 1; i <= n; i++) { split(dep[i], w, " "); if (w[1] ~ ours && (w[2] == ">=" || w[2] == ">")) print rel ":" FNR ": " w[1] " " w[2] " " w[3] }
      }' "$f"
  done
  [ ! -f "$root/debian/control" ] || grep -nE "(^|[ ,])$OURS \\(>>?=? [0-9]" "$root/debian/control" | grep -vE '^[0-9]+:#' | sed 's/^/debian\/control:/' || true
  for f in "$root"/*.pc.in; do
    [ -f "$f" ] || continue
    grep -nE "^Requires(\\.private)?:.*$OURS *>" "$f" | sed "s|^|${f#"$root"/}:|" || true
  done
  for f in "$root/package.json" "$root/ts/package.json"; do
    [ -f "$f" ] || continue
    grep -nE '"@openmixer/[^"]+": *"[^"*]*[0-9]' "$f" | sed "s|^|${f#"$root"/}:|" || true
  done
  for d in data recipes; do
    [ -d "$root/$d" ] || continue
    find "$root/$d" -name '*.json' -print0 | LC_ALL=C sort -z | xargs -0 -r node -e '
      const fs = require("fs");
      for (const f of process.argv.slice(1)) {
        const walk = (v, at) => {
          if (Array.isArray(v)) return v.forEach((x, i) => walk(x, `${at}[${i}]`));
          if (!v || typeof v !== "object") return;
          for (const [k, x] of Object.entries(v)) {
            if ((k === "requires" || k === "x-requires") && x && typeof x === "object")
              for (const [n, r] of Object.entries(x)) if (typeof r === "string" && /[0-9]/.test(r)) console.log(`${f}: ${at}.${k}.${n} ${JSON.stringify(r)}`);
            walk(x, `${at}.${k}`);
          }
        };
        walk(JSON.parse(fs.readFileSync(f, "utf8")), "$");
      }' | sed "s|^$root/||"
  done
}

check() {
  local found
  found="$(scan "$1")"
  if [ -n "$found" ]; then
    printf '%s\n' "$found" >&2
    echo "no-floor-check: FAIL — a hand-kept version floor (owner ruling 2026-11-24: build against the published lower level; generated relations only)" >&2
    return 1
  fi
}

if [ "${1:-}" = --self-test ]; then
  S="$(mktemp -d)"; trap 'rm -rf "$S"' EXIT
  bad=0
  copy() { rm -rf "$S/t"; mkdir -p "$S/t"; (cd "$HERE" && git ls-files -z packaging debian/control '*.pc.in' package.json ts/package.json data recipes | xargs -0 cp --parents -t "$S/t"); }
  arm() {  # name file sed-or-append
    copy
    case "$3" in append:*) printf '%s\n' "${3#append:}" >> "$S/t/$2" ;; *) sed -i -E "$3" "$S/t/$2" ;; esac
    if check "$S/t" 2>/dev/null; then echo "no-floor-check: FAIL — arm $1 stayed green" >&2; bad=1; else echo "no-floor-check: arm $1 red"; fi
  }
  copy; check "$S/t" || { echo "no-floor-check: FAIL — the clean copy is red" >&2; exit 1; }
  arm deps-txt    packaging/deps.txt   'append:omx-contract-devel libomx-contract-dev 3.21.0'
  arm spec        packaging/omx-dsp.spec 's/^BuildRequires: omx-contract-devel$/BuildRequires: omx-contract-devel >= 3.21.0/'
  arm control     debian/control       's/^(Build-Depends: .*)libomx-contract-dev,/\1libomx-contract-dev (>= 3.21.0~),/'
  arm pc          omxdsp.pc.in         's/^Requires: omx-contract$/Requires: omx-contract >= 3.21.0/'
  arm npm         ts/package.json      's/^(  "name": .*)$/\1\n  "peerDependencies": { "@openmixer\/omx-contract": ">=3.21.0" },/'
  f="$(cd "$HERE" && git ls-files 'data/kernels/*.json' | head -1)"
  arm data        "$f"                 '0,/^  "\$schema": (.*),$/s//  "$schema": \1,\n  "requires": { "omx-contract-schema": ">= 3.0.0" },/'
  [ "$bad" = 0 ] || exit 1
  echo "no-floor-check: self-test: six planted floors red, the clean copy green"
  exit 0
fi
check "${1:-$HERE}"
echo "no-floor-check: no hand-kept floor on an OpenMixer package"
