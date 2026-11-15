#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# patch-names-check.sh — a patch release adds no public name. v0.18.1 added
# omx_eq_lowpass_nyquist_handover() and two OMX_EQ_LOWPASS_HANDOVER_* defines, v0.20.1 added
# omx_eq_lowpass_nyquist_raise(); a new public name goes in a minor. When include/omxdsp/omxdsp.h says patch > 0,
# every `#define NAME` and `static inline ... name(` under include/ must already exist at the tag vMAJOR.MINOR.0.
# Patch 0, or no such tag, is a minor: nothing to compare. Part of `make lint`.
#
#   tools/patch-names-check.sh [REPO]   the check in REPO (default this checkout)
#   tools/patch-names-check.sh --self-test   a new define and a new inline in a patch are named, a minor's are not
set -euo pipefail

names() { # <repo> <rev-or-WORKTREE>: one public name per line
  local repo=$1 rev=$2 f
  { if [ "$rev" = WORKTREE ]; then cat "$repo"/include/omxdsp/*.h
    else for f in $(git -C "$repo" ls-tree -r --name-only "$rev" include | grep '\.h$'); do git -C "$repo" show "$rev:$f"; done; fi; } |
    sed -nE 's/^[[:space:]]*#[[:space:]]*define[[:space:]]+([A-Za-z_][A-Za-z0-9_]*).*/\1/p;
             s/^static inline[^(]*[[:space:]*]([A-Za-z_][A-Za-z0-9_]*)\(.*/\1/p' | sort -u
}

check() {
  local repo=$1 h=$1/include/omxdsp/omxdsp.h maj min pat base added
  maj=$(sed -nE 's/^#define OMXDSP_VERSION_MAJOR ([0-9]+).*/\1/p' "$h")
  min=$(sed -nE 's/^#define OMXDSP_VERSION_MINOR ([0-9]+).*/\1/p' "$h")
  pat=$(sed -nE 's/^#define OMXDSP_VERSION_PATCH ([0-9]+).*/\1/p' "$h")
  [ -n "$maj$min$pat" ] || { echo "patch-names-check: no version in $h" >&2; return 1; }
  [ "$pat" -gt 0 ] || { echo "patch-names-check: $maj.$min.$pat is a minor, nothing to compare"; return 0; }
  base=v$maj.$min.0
  git -C "$repo" rev-parse -q --verify "refs/tags/$base" >/dev/null || { echo "patch-names-check: no tag $base, nothing to compare"; return 0; }
  added=$(comm -13 <(names "$repo" "$base") <(names "$repo" WORKTREE))
  if [ -n "$added" ]; then
    echo "patch-names-check: $maj.$min.$pat adds public names to $base; they go in a minor:" >&2
    echo "$added" >&2
    return 1
  fi
  echo "patch-names-check: $maj.$min.$pat adds no public name over $base"
}

if [ "${1:-}" = "--self-test" ]; then
  T=$(mktemp -d "${TMPDIR:-/tmp}/patch-names-check.XXXXXX"); trap 'rm -rf "$T"' EXIT
  mkdir -p "$T/include/omxdsp/analysis" "$T/include/omxdsp/mix"; cd "$T"; git init -q .
  ver() { sed -i -E "s/^#define OMXDSP_VERSION_PATCH [0-9]+/#define OMXDSP_VERSION_PATCH $1/" include/omxdsp/omxdsp.h; }
  printf '%s\n' '#ifndef OMXDSP_H' '#define OMXDSP_H' '#define OMXDSP_VERSION_MAJOR 0' '#define OMXDSP_VERSION_MINOR 1' \
    '#define OMXDSP_VERSION_PATCH 0' '#define OMX_OLD 1 /* a macro */' \
    'static inline double omx_old(double q) { return q; }' '#endif' > include/omxdsp/omxdsp.h
  printf '%s\n' '#ifdef __cplusplus' 'extern "C" {' '#endif' '/* omx_place(int) once */' \
    'typedef struct OmxA { double x; /* the first */' '  uint32_t n; } OmxA;' \
    'enum omx_a_kind { OMX_A_ONE = 1, OMX_A_TWO, };' 'typedef void (*omx_a_cb)(void *user);' \
    'extern const double omx_a_table[4];' \
    'uint32_t omx_a_place(double f0_hz, const double *gains_db,' '                     uint32_t n);' \
    'int omx_a_in_range(double freq_hz);' '#ifdef __cplusplus' '}' '#endif' > include/omxdsp/analysis/omx_a.h
  printf '%s\n' 'OMXDSP_LAYOUT(m, struct omx_m)' 'struct omx_m { int on; };' 'OMXDSP_LAYOUT(n, int)' 'static inline int omx_m_on(const struct omx_m *m) {' '  return m->on;' '}' \
    > include/omxdsp/mix/omx_m.h
  git add -A; git -c user.name=t -c user.email=t@t commit -qm base; git tag v0.1.0
  fail=0
  must() { # <what> <0: passes | 1: refused> [<the text the refusal must name>]
    local out rc=0; out=$(check "$T" 2>&1) || rc=$?
    if [ "$2" = 0 ] && [ "$rc" != 0 ]; then echo "patch-names-check --self-test: FAIL, $1 is refused:"; sed 's/^/    /' <<< "$out"; fail=1; fi
    if [ "$2" = 1 ] && [ "$rc" = 0 ]; then echo "patch-names-check --self-test: FAIL, $1 is not named"; fail=1; fi
    if [ "$2" = 1 ] && [ -n "${3:-}" ] && ! grep -qF -- "$3" <<< "$out"; then echo "patch-names-check --self-test: FAIL, $1 is refused without naming '$3'"; fail=1; fi
    return 0
  }
  reset() { git checkout -q -- include; git clean -qfd include; ver "$1"; }
  ver 1; must "an unchanged patch" 0
  # what a patch may add: names in every header, an appended field and enumerator, a new header
  echo '#define OMX_NEW 2' >> include/omxdsp/omxdsp.h
  echo 'static inline double omx_new(double q) { return q; }' >> include/omxdsp/omxdsp.h
  sed -i 's/^int omx_a_in_range(double freq_hz);/&\ndouble omx_a_step(double current, double target);/' include/omxdsp/analysis/omx_a.h
  sed -i 's/^  uint32_t n; } OmxA;/  uint32_t n;\n  double appended; } OmxA;/; s/OMX_A_TWO, };/OMX_A_TWO, OMX_A_THREE };/' include/omxdsp/analysis/omx_a.h
  echo 'struct omx_b { int on; }; int omx_b_on(const struct omx_b *b);' > include/omxdsp/mix/omx_b.h
  must "an additive patch (a define, an inline, a function in analysis/, an appended field and enumerator, a new header)" 0
  sed -i 's/^uint32_t omx_a_place(double f0_hz, const double \*gains_db,/uint32_t  omx_a_place( double f0_hz , const double* gains_db, \/* re-wrapped *\//' include/omxdsp/analysis/omx_a.h
  must "a declaration re-spaced and commented" 0
  reset 1; sed -i '/^int omx_a_in_range/d' include/omxdsp/analysis/omx_a.h
  must "a function removed from analysis/" 1 "removed: function omx_a_in_range"
  reset 1; sed -i '/static inline int omx_m_on/,/^}/d' include/omxdsp/mix/omx_m.h
  must "a static inline removed from mix/" 1 "removed: function omx_m_on"
  reset 1; sed -i '/#define OMX_OLD/d' include/omxdsp/omxdsp.h
  must "a removed macro" 1 "removed: macro OMX_OLD"
  reset 1; sed -i 's/^                     uint32_t n);/                     uint32_t n, double q);/' include/omxdsp/analysis/omx_a.h
  must "a changed signature" 1 "changed: function omx_a_place"
  reset 1; sed -i 's/static inline double omx_old(double q)/static inline float omx_old(float q)/' include/omxdsp/omxdsp.h
  must "a changed inline signature" 1 "changed: function omx_old"
  reset 1; sed -i 's/^typedef struct OmxA { double x; \/\* the first \*\//typedef struct OmxA {/' include/omxdsp/analysis/omx_a.h
  must "a removed field" 1 "changed: struct OmxA"
  reset 1; sed -i 's/^typedef struct OmxA { double x; \/\* the first \*\//typedef struct OmxA { uint32_t n;/; s/^  uint32_t n; } OmxA;/  double x; } OmxA;/' include/omxdsp/analysis/omx_a.h
  must "reordered fields" 1 "changed: struct OmxA"
  reset 1; sed -i 's/OMX_A_ONE = 1, OMX_A_TWO,/OMX_A_TWO, OMX_A_ONE = 1,/' include/omxdsp/analysis/omx_a.h
  must "reordered enumerators" 1 "changed: enum omx_a_kind"
  reset 1; sed -i 's/typedef void (\*omx_a_cb)(void \*user);/typedef int (*omx_a_cb)(void *user);/' include/omxdsp/analysis/omx_a.h
  must "a changed typedef" 1 "changed: typedef omx_a_cb"
  reset 1; sed -i 's/omx_a_table\[4\]/omx_a_table[3]/' include/omxdsp/analysis/omx_a.h
  must "a changed extern" 1 "changed: variable omx_a_table"
  reset 1; sed -i 's/struct omx_m { int on; };/struct omx_m { long on; };/' include/omxdsp/mix/omx_m.h
  must "a changed field in mix/" 1 "changed: struct omx_m"
  # what an earlier patch added is held too
  reset 1; echo 'int omx_a_later(void);' >> include/omxdsp/analysis/omx_a.h
  git add -A; git -c user.name=t -c user.email=t@t commit -qm patch1; git tag v0.1.1
  ver 2; must "a patch over two tags" 0
  sed -i '/omx_a_later/d' include/omxdsp/analysis/omx_a.h
  must "a name the previous patch added, removed" 1 "removed: function omx_a_later"
  # a minor compares nothing
  reset 0; sed -i '/^int omx_a_in_range/d' include/omxdsp/analysis/omx_a.h
  must "a minor's removal" 0
  [ "$fail" -eq 0 ] && echo "patch-names-check --self-test: an additive patch passes; a removed or changed macro, function, inline, typedef, extern, field or enumerator, in any header, is named; a minor compares nothing"
  exit "$fail"
fi
check "$(cd "${1:-$(dirname "$0")/..}" && pwd)"
