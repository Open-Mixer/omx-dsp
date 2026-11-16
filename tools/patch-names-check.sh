#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# patch-names-check.sh — a patch release only adds to the public headers.
#
# The policy (BUILDING.md, "What a patch release may change"): omx-dsp ships a static archive and its headers
# together, so a program is always built against the headers of the archive it links. A PATCH may therefore add
# public names (a function, a macro, a type, an enumerator) and append fields to a struct that its init function
# zeroes: nothing built against an earlier patch can tell. A MINOR is needed for anything an earlier build could
# tell: a changed signature, a removed or reordered field or enumerator, a removed name, or changed behaviour.
#
# The check, run by `make lint`: when include/omxdsp/omxdsp.h says patch > 0, every header under include/omxdsp/
# (analysis/, fx/ and mix/ too) is read, at every earlier tag of the same minor (vMAJOR.MINOR.0 up to the patch before
# this one) and in the working tree, for its public declarations:
#   macro     `#define NAME`                                    must still be defined
#   function  a declaration or a static inline definition      must still be declared, with the same signature
#   typedef   a typedef other than of a struct/union/enum body  must still exist, unchanged
#   variable  an extern declaration                            must still exist, unchanged
#   struct, union, enum   its members at the tag must be the FIRST members now, in the same order and unchanged
#                         (appending is a patch)
# A removed or changed one is refused, naming it; an added one is allowed and counted.
#
# A macro's VALUE is read for one kind of macro, a count or size: one named so (_COUNT _SIZE _SLOTS _CAP _LEN _LENGTH
# _CAPACITY), one an array bound reads in a public header, and every macro the value of one of those reads. A moved
# value there can resize a struct or array that a program built against the earlier patch shares with this one, so it
# is refused (`moved: NAME`) unless tools/patch-count-proofs.txt carries its proof, one line per moved macro:
#   <this version> <NAME> <its new value, without spaces> <the proof: no struct or array shared across the ABI is
#                                                          sized by it, and no consumer stores it>
# Otherwise it is a minor. Any other macro's value is behaviour (below). Behaviour is not something this
# check can read: the kernels' golden vectors hold it (`make golden-write` only in a minor). Patch 0, or no earlier
# tag of the minor, compares nothing. A signature is compared as text with its whitespace and comments normalised
# away, so renaming a parameter reads as a change: rename in a minor.
#
#   tools/patch-names-check.sh [REPO]           the check in REPO (default this checkout)
#   tools/patch-names-check.sh --list REPO REV  the public declarations at REV (WORKTREE: the files on disk)
#   tools/patch-names-check.sh --self-test      an additive patch passes; a removed or changed name, field or
#                                               enumerator, in any header, is named; a minor compares nothing
# test/patch-names-sabotage.sh breaks this check's rules one at a time and requires the self-test to fail.
set -euo pipefail

# Which files are the public headers: every .h under include/omxdsp/, at any depth.
HEADER_RE='^include/omxdsp/.*\.h$'

# The declarations of the headers on stdin (each file after a line "\001FILE <path>"), one per line:
# KIND <tab> NAME <tab> VALUE, KIND one of macro function typedef variable struct union enum, and two that only the
# count and size reading uses: define (the value of an object-like macro) and bound (a name an array bound reads).
EXTRACT='
function rec(kind, name, value,   k) {
  if (name == "") return
  k = kind "\t" name
  if (k in seen) { if (index(" | " seen[k] " | ", " | " value " | ") == 0) seen[k] = seen[k] " | " value }
  else { seen[k] = value; order[++nrec] = k }
}
function norec(kind, name, value) { return }
function norm(s) {
  gsub(/[ \t\r\n]+/, " ", s)
  gsub(/ ?\( ?/, "(", s); gsub(/ ?\) ?/, ")", s); gsub(/ ?, ?/, ",", s); gsub(/ ?\* ?/, "*", s)
  gsub(/ ?\[ ?/, "[", s); gsub(/ ?\] ?/, "]", s); gsub(/ ?= ?/, "=", s); gsub(/ ?; ?/, ";", s)
  sub(/^ /, "", s); sub(/ $/, "", s)
  return s
}
# the end of the string or char literal that opens at i
function litend(s, i,   q, j, c, n) {
  q = substr(s, i, 1); n = length(s)
  for (j = i + 1; j <= n; j++) {
    c = substr(s, j, 1)
    if (c == "\\") { j++; continue }
    if (c == q) return j
  }
  return n
}
# the brace that closes the one at i
function closing(s, i,   d, j, c, n) {
  d = 0; n = length(s)
  for (j = i; j <= n; j++) {
    c = substr(s, j, 1)
    if (c == "\"" || c == "\047") { j = litend(s, j); continue }
    if (c == "{") d++
    else if (c == "}") { d--; if (d == 0) return j }
  }
  return n
}
# a statement without the macro calls that lead it (OMXDSP_STATIC_ASSERT(...), OMXDSP_STATE_LAYOUT(...)): an
# upper-case name called at the start, which carries no ";" of its own
function unmacro(s,   d, j, c, n) {
  while (match(s, /^[A-Z_][A-Z0-9_]*\(/)) {
    d = 0; n = length(s)
    for (j = RLENGTH; j <= n; j++) {
      c = substr(s, j, 1)
      if (c == "(") d++
      else if (c == ")") { d--; if (d == 0) break }
    }
    s = substr(s, j + 1); sub(/^ /, "", s)
  }
  return s
}
function lastident(s,   t) {
  t = s
  gsub(/\[[^]]*\]/, "", t)
  if (match(t, /[A-Za-z_][A-Za-z0-9_]*$/)) return substr(t, RSTART, RLENGTH)
  return ""
}
# the name a function declaration declares: the identifier before its first parameter list
function fname(s,   t, id) {
  t = s
  while (match(t, /[A-Za-z_][A-Za-z0-9_]*\(/)) {
    id = substr(t, RSTART, RLENGTH - 1)
    if (id != "__attribute__" && id != "__typeof__" && id != "sizeof" && id != "_Alignas") return id
    t = substr(t, RSTART + RLENGTH)
  }
  return ""
}
# an aggregate body as its members in order, joined by ";": fields split at the top level at ";", enumerators at ","
function members(kind, body,   out, cur, d, j, c, sep, n) {
  sep = (kind == "enum") ? "," : ";"
  out = ""; cur = ""; d = 0; n = length(body)
  for (j = 1; j <= n; j++) {
    c = substr(body, j, 1)
    if (c == "{" || c == "(") d++
    else if (c == "}" || c == ")") d--
    if (c == sep && d == 0) { cur = norm(cur); if (cur != "") out = out (out == "" ? "" : ";") cur; cur = ""; continue }
    cur = cur c
  }
  cur = norm(cur); if (cur != "") out = out (out == "" ? "" : ";") cur
  return out
}
function statement(s,   kind, tail, name) {
  s = unmacro(s)
  if (s == "") return
  if (agg != "") {
    kind = agg; agg = ""
    tail = s; sub(/^.*\{\}/, "", tail)
    name = aggtag
    if (name == "" && s ~ /^typedef /) name = lastident(norm(tail))
    if (name != "") rec(kind, name, aggbody)
    return
  }
  if (s ~ /^(_Static_assert|static_assert)\(/) return
  if (s ~ /^(struct|union|enum) [A-Za-z_][A-Za-z0-9_]*$/) return
  if (s ~ /^typedef /) {
    if (match(s, /\(\*[A-Za-z_][A-Za-z0-9_]*\)/)) name = substr(s, RSTART + 2, RLENGTH - 3)
    else name = lastident(s)
    rec("typedef", name, s)
    return
  }
  if (s ~ /\(/ && substr(s, 1, index(s, "(")) !~ /=/) { rec("function", fname(s), s); return }
  name = s; sub(/=.*$/, "", name)
  rec("variable", lastident(name), s)
}
function file(text,   code, i, n, c, j, lines, nl, l, line, s, h, b, m, ext, bt, bb, bp, nb) {
  # comments out (their newlines kept, so a preprocessor line never swallows code), literals kept
  code = ""; n = length(text); i = 1
  while (i <= n) {
    c = substr(text, i, 1)
    if (c == "/" && substr(text, i + 1, 1) == "*") {
      j = index(substr(text, i + 2), "*/"); j = j ? i + 2 + j : n
      s = substr(text, i, j - i + 1); m = gsub(/\n/, "\n", s)
      code = code " "; while (m-- > 0) code = code "\n"
      i = j + 1; continue
    }
    if (c == "/" && substr(text, i + 1, 1) == "/") {
      j = index(substr(text, i), "\n"); i = j ? i + j - 1 : n + 1; continue
    }
    if (c == "\"" || c == "\047") { j = litend(text, i); code = code substr(text, i, j - i + 1); i = j + 1; continue }
    # a run with nothing to look at, copied whole
    if (match(substr(text, i), /^[^\/"\047]+/)) { code = code substr(text, i, RLENGTH); i += RLENGTH; continue }
    code = code c; i++
  }
  # preprocessor lines out, their #define names kept
  nl = split(code, lines, "\n"); code = ""; line = ""
  for (l = 1; l <= nl; l++) {
    line = line lines[l]
    if (line ~ /^[ \t]*#/ && line ~ /\\$/) { sub(/\\$/, " ", line); continue }
    if (line ~ /^[ \t]*#/) {
      if (match(line, /^[ \t]*#[ \t]*define[ \t]+[A-Za-z_][A-Za-z0-9_]*/)) {
        s = substr(line, RSTART, RLENGTH); sub(/^.*[ \t]/, "", s); rec("macro", s, "")
        # the value of an object-like macro (a function-like one has no value to read)
        m = substr(line, RSTART + RLENGTH); if (m !~ /^\(/) rec("define", s, norm(m))
      }
    } else code = code line " "
    line = ""
  }
  # every upper-case name an array bound reads: [NAME], [NAME * 2], [(A + B)]
  bt = code
  while (match(bt, /\[[^]]*\]/)) {
    bb = substr(bt, RSTART + 1, RLENGTH - 2); bt = substr(bt, RSTART + RLENGTH)
    nb = split(bb, bp, /[^A-Za-z0-9_]+/)
    for (j = 1; j <= nb; j++) if (bp[j] ~ /^[A-Z][A-Z0-9_]*$/) rec("bound", bp[j], "")
  }
  # top-level statements
  s = ""; agg = ""; ext = 0; n = length(code); i = 1
  while (i <= n) {
    c = substr(code, i, 1)
    if (c == "\"" || c == "\047") { j = litend(code, i); s = s substr(code, i, j - i + 1); i = j + 1; continue }
    if (c == "{") {
      h = unmacro(norm(s))
      if (h == "extern \"C\"") { ext++; s = ""; i++; continue }
      j = closing(code, i); b = substr(code, i + 1, j - i - 1)
      if (h ~ /\)$/ && h !~ /=/) { rec("function", fname(h), h); s = ""; i = j + 1; continue }
      if (match(h, /(^|[ ])(struct|union|enum)( [A-Za-z_][A-Za-z0-9_]*)?$/)) {
        m = substr(h, RSTART, RLENGTH); sub(/^ /, "", m)
        agg = m; sub(/ .*$/, "", agg)
        aggtag = ""; if (m ~ / /) { aggtag = m; sub(/^[a-z]+ /, "", aggtag) }
        aggbody = members(agg, b)
        s = h " {}"; i = j + 1; continue
      }
      s = s " {}"; i = j + 1; continue
    }
    if (c == "}") { if (ext > 0) ext--; s = ""; i++; continue }
    if (c == ";") { statement(norm(s)); s = ""; i++; continue }
    if (match(substr(code, i), /^[^{};"\047]+/)) { s = s substr(code, i, RLENGTH); i += RLENGTH; continue }
    s = s c; i++
  }
}
/^\001FILE / { if (path != "") file(text); path = substr($0, 7); text = ""; next }
{ text = text $0 "\n" }
END {
  if (path != "") file(text)
  for (r = 1; r <= nrec; r++) print order[r] "\t" seen[order[r]]
}'

# The declarations at <rev> (WORKTREE: the files on disk) of the public headers of <repo>.
list() {
  local repo=$1 rev=$2 f
  if [ "$rev" = WORKTREE ]; then
    (cd "$repo" && find include/omxdsp -name '*.h' | LC_ALL=C sort | { grep -E "$HEADER_RE" || true; }) |
      while read -r f; do printf '\001FILE %s\n' "$f"; cat "$repo/$f"; echo; done
  else
    git -C "$repo" ls-tree -r --name-only "$rev" include | LC_ALL=C sort | { grep -E "$HEADER_RE" || true; } |
      while read -r f; do printf '\001FILE %s\n' "$f"; git -C "$repo" show "$rev:$f"; echo; done
  fi | LC_ALL=C awk "$EXTRACT"
}

# The declarations at a tag (file 1) against the working tree's (file 2): a removed or changed one is printed and
# fails; a count or size macro whose value moved is printed "MOVED <name> <tab> <was> <tab> <now>" (the caller
# looks for its proof); the last line is "ADDED <count> <the first names>".
COMPARE='
BEGIN { FS = "\t" }
function prefix(now, was) { return was == "" || now == was || index(now, was ";") == 1 }
function kept(kind) { return kind != "define" && kind != "bound" }
NR == FNR { was[$1 FS $2] = $3; wk[++nw] = $1 FS $2; next }
{ now[$1 FS $2] = $3; if (kept($1) && !(($1 FS $2) in was)) { added++; if (added <= 12) addl = addl " " $2 } }
END {
  # the counts and sizes: named so, or read by an array bound, and every name their values read, to a fixed point
  for (k in now) { split(k, p, FS); if (p[1] == "bound" || (p[1] == "define" && p[2] ~ /(_COUNT|_SIZE|_SLOTS|_CAP|_LEN|_LENGTH|_CAPACITY)$/)) size[p[2]] = 1 }
  do {
    grew = 0
    for (k in now) {
      split(k, p, FS); if (p[1] != "define" || !(p[2] in size)) continue
      nv = split(now[k], vp, /[^A-Za-z0-9_]+/)
      for (j = 1; j <= nv; j++) if (vp[j] ~ /^[A-Z][A-Z0-9_]*$/ && !(vp[j] in size)) { size[vp[j]] = 1; grew = 1 }
    }
  } while (grew)
  for (i = 1; i <= nw; i++) {
    k = wk[i]; split(k, p, FS)
    if (!kept(p[1])) {
      if (p[1] == "define" && (p[2] in size) && (k in now) && now[k] != was[k]) print "MOVED " p[2] FS was[k] FS now[k]
      continue
    }
    if (!(k in now)) { bad++; print "  removed: " p[1] " " p[2]; continue }
    if (p[1] == "struct" || p[1] == "union" || p[1] == "enum") {
      if (!prefix(now[k], was[k])) { bad++; print "  changed: " p[1] " " p[2] " (a member removed, reordered or changed): was {" was[k] "} now {" now[k] "}" }
    } else if (now[k] != was[k]) { bad++; print "  changed: " p[1] " " p[2] ": was `" was[k] "` now `" now[k] "`" }
  }
  printf "ADDED %d%s%s\n", added + 0, addl, (added > 12 ? " ..." : "")
  exit bad > 0
}'

check() {
  local repo=$1 h=$1/include/omxdsp/omxdsp.h maj min pat k now base out fail=0 bases="" added="" moved="" name was val proof
  maj=$(sed -nE 's/^#define OMXDSP_VERSION_MAJOR ([0-9]+).*/\1/p' "$h")
  min=$(sed -nE 's/^#define OMXDSP_VERSION_MINOR ([0-9]+).*/\1/p' "$h")
  pat=$(sed -nE 's/^#define OMXDSP_VERSION_PATCH ([0-9]+).*/\1/p' "$h")
  [ -n "$maj$min$pat" ] || { echo "patch-names-check: no version in $h" >&2; return 1; }
  [ "$pat" -gt 0 ] || { echo "patch-names-check: $maj.$min.$pat is a minor, nothing to compare"; return 0; }
  now=$(mktemp "${TMPDIR:-/tmp}/patch-names-now.XXXXXX"); base=$(mktemp "${TMPDIR:-/tmp}/patch-names-base.XXXXXX")
  list "$repo" WORKTREE > "$now"
  for ((k = 0; k < pat; k++)); do
    git -C "$repo" rev-parse -q --verify "refs/tags/v$maj.$min.$k" > /dev/null || continue
    list "$repo" "v$maj.$min.$k" > "$base"
    if out=$(LC_ALL=C awk "$COMPARE" "$base" "$now"); then :; else
      echo "patch-names-check: $maj.$min.$pat removes or changes what v$maj.$min.$k declares; that is a minor:" >&2
      grep -vE '^(ADDED|MOVED) ' <<< "$out" >&2 || true
      fail=1
    fi
    bases="$bases v$maj.$min.$k"; added=$(sed -n 's/^ADDED //p' <<< "$out")
    moved="$moved$(sed -n 's/^MOVED //p' <<< "$out")"$'\n'
  done
  rm -f "$now" "$base"
  # each moved count or size once (its value against the earliest tag that differs), and its proof for this version
  # and this value
  while IFS=$'\t' read -r name was val; do
    [ -n "$name" ] || continue
    proof=$(awk -v v="$maj.$min.$pat" -v n="$name" -v x="${val// /}" \
      '$1 == v && $2 == n && $3 == x { sub(/^[^ ]+ +[^ ]+ +[^ ]+ */, ""); print; exit }' \
      "$repo/tools/patch-count-proofs.txt" 2> /dev/null || true)
    if [ -n "$proof" ]; then
      echo "patch-names-check: $maj.$min.$pat moves $name, a count or size (was \`$was\` now \`$val\`): a patch by its proof: $proof"
    else
      echo "patch-names-check: $maj.$min.$pat moves a count or size; that is a minor unless tools/patch-count-proofs.txt carries its proof:" >&2
      echo "  moved: $name: was \`$was\` now \`$val\` (prove no struct or array shared across the ABI is sized by it and no consumer stores it, then list \"$maj.$min.$pat $name ${val// /} <the proof>\")" >&2
      fail=1
    fi
  done < <(awk -F '\t' 'NF && !seen[$1]++' <<< "$moved")
  [ "$fail" -eq 0 ] || return 1
  if [ -z "$bases" ]; then echo "patch-names-check: no earlier tag of $maj.$min, nothing to compare"; return 0; fi
  if [ "${added%% *}" = 0 ]; then echo "patch-names-check: $maj.$min.$pat removes and changes nothing declared at$bases, and adds nothing"
  else echo "patch-names-check: $maj.$min.$pat removes and changes nothing declared at$bases; it adds ${added%% *} (a patch may add):${added#* }"; fi
}

if [ "${1:-}" = "--list" ]; then list "$(cd "$2" && pwd)" "$3"; exit; fi

if [ "${1:-}" = "--self-test" ]; then
  T=$(mktemp -d "${TMPDIR:-/tmp}/patch-names-check.XXXXXX"); trap 'rm -rf "$T"' EXIT
  mkdir -p "$T/include/omxdsp/analysis" "$T/include/omxdsp/mix"; cd "$T"; git init -q .
  ver() { sed -i -E "s/^#define OMXDSP_VERSION_PATCH [0-9]+/#define OMXDSP_VERSION_PATCH $1/" include/omxdsp/omxdsp.h; }
  printf '%s\n' '#ifndef OMXDSP_H' '#define OMXDSP_H' '#define OMXDSP_VERSION_MAJOR 0' '#define OMXDSP_VERSION_MINOR 1' \
    '#define OMXDSP_VERSION_PATCH 0' '#define OMX_OLD 1 /* a macro */' \
    '#define OMX_A_COUNT 2u' '#define OMX_A_GAIN 3.0' '#define OMX_A_ROWS 4' '#define OMX_A_TABLE_N OMX_A_ROWS' \
    'static inline double omx_old(double q) { return q; }' '#endif' > include/omxdsp/omxdsp.h
  printf '%s\n' '#ifdef __cplusplus' 'extern "C" {' '#endif' '/* omx_place(int) once */' \
    'typedef struct OmxA { double x; /* the first */' '  uint32_t n; } OmxA;' \
    'enum omx_a_kind { OMX_A_ONE = 1, OMX_A_TWO, };' 'typedef void (*omx_a_cb)(void *user);' \
    'extern const double omx_a_table[4];' 'extern const double omx_a_rows[OMX_A_TABLE_N];' \
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
  reset() { git checkout -q -- include; git clean -qfd include; rm -f tools/patch-count-proofs.txt; ver "$1"; }
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
  # a count or size macro (by its name, or an array bound in a public header, directly or through another macro)
  # whose value moves is a patch only with its proof in tools/patch-count-proofs.txt; any other value is not read
  reset 1; sed -i 's/^#define OMX_A_COUNT 2u/#define OMX_A_COUNT 3u/' include/omxdsp/omxdsp.h
  must "a moved _COUNT macro without its proof" 1 "moved: OMX_A_COUNT"
  mkdir -p tools; echo '0.1.1 OMX_A_COUNT 3u no ABI-shared struct or array is sized by it' > tools/patch-count-proofs.txt
  must "a moved _COUNT macro with its proof" 0
  sed -i 's/^0\.1\.1 /0.1.9 /' tools/patch-count-proofs.txt
  must "a moved _COUNT macro with another version's proof" 1 "moved: OMX_A_COUNT"
  echo '0.1.1 OMX_A_COUNT 4u a proof for another value' > tools/patch-count-proofs.txt
  must "a moved _COUNT macro with another value's proof" 1 "moved: OMX_A_COUNT"
  reset 1; sed -i 's/^#define OMX_A_ROWS 4/#define OMX_A_ROWS 5/' include/omxdsp/omxdsp.h
  must "a macro an array bound reads through another macro, moved" 1 "moved: OMX_A_ROWS"
  reset 1; sed -i 's/^#define OMX_A_GAIN 3.0/#define OMX_A_GAIN 4.0/' include/omxdsp/omxdsp.h
  must "a moved value that sizes nothing" 0
  # what an earlier patch added is held too
  reset 1; echo 'int omx_a_later(void);' >> include/omxdsp/analysis/omx_a.h
  git add -A; git -c user.name=t -c user.email=t@t commit -qm patch1; git tag v0.1.1
  ver 2; must "a patch over two tags" 0
  sed -i '/omx_a_later/d' include/omxdsp/analysis/omx_a.h
  must "a name the previous patch added, removed" 1 "removed: function omx_a_later"
  # a minor compares nothing
  reset 0; sed -i '/^int omx_a_in_range/d' include/omxdsp/analysis/omx_a.h
  must "a minor's removal" 0
  [ "$fail" -eq 0 ] && echo "patch-names-check --self-test: an additive patch passes; a removed or changed macro, function, inline, typedef, extern, field or enumerator, in any header, is named; a count or size macro whose value moves is named unless its proof is listed; a minor compares nothing"
  exit "$fail"
fi
check "$(cd "${1:-$(dirname "$0")/..}" && pwd)"
