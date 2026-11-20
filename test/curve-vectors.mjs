// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * test/curve-vectors.mjs: vectors/curve-points.json put to omx_curve_points_json through each door it is given, and
 * every answer held to the row's own tolerance. One file, one comparison, so the doors cannot drift apart:
 *
 *   node test/curve-vectors.mjs [--native build/curve_json_cli] [--wasm ts/wasm/omx-dsp.wasm] [--vectors <file>] [--report]
 *
 * `--report` prints each row's largest gap (x relative, y absolute), the measure the row's tolerance is set from.
 * `--native` is tools/curve-json-cli.c (request on stdin, answer on stdout); `--wasm` is the WebAssembly build of the
 * same function, through ts/dist/wasm.js's loader (`npm run build` first). A miss is a finding printed with its row, door, point and
 * the distance in ulps of the value: a tolerance is never widened to absorb it.
 */
import { spawnSync } from 'node:child_process';
import { readFileSync } from 'node:fs';
import { parseArgs } from 'node:util';

const { values: opt } = parseArgs({ options: { native: { type: 'string' }, wasm: { type: 'string' }, vectors: { type: 'string', default: 'vectors/curve-points.json' }, report: { type: 'boolean' } } });
const rows = JSON.parse(readFileSync(opt.vectors, 'utf8')).rows;

/** The distance between two doubles in units in the last place of the larger magnitude. */
function ulps(a, b) {
  if (a === b) return 0;
  const ulp = (v) => { const f = new Float64Array([Math.abs(v)]); const i = new BigUint64Array(f.buffer); i[0] += 1n; return f[0] - Math.abs(v); };
  return Math.abs(a - b) / ulp(Math.max(Math.abs(a), Math.abs(b)));
}

const doors = [];
if (opt.native) {
  doors.push({ name: 'native', call: (req) => {
    const r = spawnSync(opt.native, { input: JSON.stringify(req), encoding: 'utf8', maxBuffer: 1 << 22 });
    if (r.status !== 0) throw new Error(`${opt.native} exited ${r.status}`);
    return JSON.parse(r.stdout);
  } });
}
if (opt.wasm) {
  const { loadCurve } = await import(new URL('../ts/dist/wasm.js', import.meta.url).href);
  const curve = await loadCurve(readFileSync(opt.wasm));
  doors.push({ name: 'wasm', call: (req) => JSON.parse(curve.pointsJson(JSON.stringify(req))) });
}
if (doors.length === 0) { console.error('curve-vectors: no door given (--native, --wasm)'); process.exit(2); }

const findings = [];
const gaps = new Map();
let checked = 0;
const within = (got, want, tol, rel = false) => Math.abs(got - want) <= (rel ? tol * Math.max(Math.abs(want), 1) : tol);

for (const door of doors) {
  for (const row of rows) {
    const ans = door.call(row.request);
    const miss = (what, got, want, tol) => findings.push(`${door.name}: ${row.name}: ${what}: got ${got}, want ${want}, ${Math.abs(got - want)} apart (${ulps(got, want).toFixed(1)} ulp), tolerance ${tol}`);
    const e = row.expect;
    checked++;
    if (e.code !== undefined) {
      if (ans.code !== e.code) findings.push(`${door.name}: ${row.name}: code ${JSON.stringify(ans.code)}, want ${e.code}`);
      continue;
    }
    if (ans.code !== undefined) { findings.push(`${door.name}: ${row.name}: refused as ${ans.code}`); continue; }
    for (const [key, list, xKey] of [['points', ans.points, 'x'], ['stems', ans.stems, 'x']]) {
      const want = e[key];
      if (!want) continue;
      if (!Array.isArray(list)) { findings.push(`${door.name}: ${row.name}: no ${key} in the answer`); continue; }
      if (e.exact && list.length !== want.length) { findings.push(`${door.name}: ${row.name}: ${list.length} ${key}, want ${want.length}`); continue; }
      want.forEach((w, i) => {
        const got = e.exact ? list[i] : list.reduce((b, p) => (Math.abs(p[xKey] - w[xKey]) < Math.abs(b[xKey] - w[xKey]) ? p : b));
        checked++;
        if (opt.report) { const gk = `${door.name}: ${row.name}`; const g = gaps.get(gk) ?? { x: 0, y: 0 }; g.x = Math.max(g.x, Math.abs(got.x - w.x) / Math.max(Math.abs(w.x), 1)); g.y = Math.max(g.y, Math.abs(got.y - w.y)); g.u = Math.max(g.u ?? 0, ulps(got.y, w.y)); gaps.set(gk, g); }
        if (!within(got.x, w.x, row.tolerance.x, true)) miss(`${key}[${i}].x`, got.x, w.x, row.tolerance.x);
        if (!within(got.y, w.y, row.tolerance.y)) miss(`${key}[${i}].y`, got.y, w.y, row.tolerance.y);
        if (w.leg !== undefined && got.leg !== w.leg) findings.push(`${door.name}: ${row.name}: ${key}[${i}].leg ${got.leg}, want ${w.leg}`);
      });
    }
  }
}
if (opt.report) for (const [name, g] of gaps) console.log(`${name}: x ${g.x.toExponential(2)}, y ${g.y.toExponential(2)} (${(g.u ?? 0).toFixed(1)} ulp)`);
// Door against door: the same C compiled twice. A difference is REPORTED with its ulps and its absolute gap (the wasm's
// libm is musl's, the native glibc's) and never absorbed by a tolerance; it does not fail the run, because each door is
// already held to the row's own tolerance against the closed form, which is the contract. docs/wasm-vs-native.md records it.
if (doors.length === 2) {
  let compared = 0, differing = 0, worst = 0, worstAbs = 0;
  const reports = [];
  const walk = (a, b, where, row) => {
    if (typeof a === 'number' && typeof b === 'number') {
      compared++;
      const u = ulps(a, b);
      worst = Math.max(worst, u);
      if (u > 0) { differing++; worstAbs = Math.max(worstAbs, Math.abs(a - b)); reports.push(`${row.name}: ${where}: ${a} against ${b}, ${u.toFixed(1)} ulp`); }
    } else if (a && b && typeof a === 'object' && typeof b === 'object') {
      if (Object.keys(a).length !== Object.keys(b).length) findings.push(`${doors[0].name} vs ${doors[1].name}: ${row.name}: ${where}: ${Object.keys(a).length} members against ${Object.keys(b).length}`);
      for (const k of Object.keys(a)) walk(a[k], b[k], `${where}.${k}`, row);
    } else if (a !== b) findings.push(`${doors[0].name} vs ${doors[1].name}: ${row.name}: ${where}: ${JSON.stringify(a)} against ${JSON.stringify(b)}`);
  };
  for (const row of rows) walk(doors[0].call(row.request), doors[1].call(row.request), '', row);
  console.log(`curve-vectors: ${doors[0].name} against ${doors[1].name}: ${compared} numbers compared, ${differing} differ, worst ${worst.toFixed(1)} ulp, largest gap ${worstAbs.toExponential(2)}`);
  if (opt.report) for (const r of reports) console.log(`REPORT ${doors[0].name} vs ${doors[1].name}: ${r}`);
}
for (const f of findings) console.error(`FINDING ${f}`);
console.log(`curve-vectors: ${doors.map((d) => d.name).join(' + ')}: ${rows.length} rows, ${checked} checks, ${findings.length} findings`);
process.exit(findings.length ? 1 : 0);
