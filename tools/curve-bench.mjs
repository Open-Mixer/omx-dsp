// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * curve-bench.mjs: the curve as WebAssembly against the TypeScript twin it replaced, over the shared vectors' requests
 * (vectors/curve-points.json): CPU (time per call, median and p99), RAM (peak resident set of a fresh process that loads
 * the door and makes the calls, against a bare Node's), and the load (compile + instantiate of the module, import of the
 * twin). Each door runs in its OWN child process, so one's heap is not the other's. tools/curve-bench.sh builds the twin
 * from the commit before its deletion and calls this.
 *
 *   node tools/curve-bench.mjs --wasm build/wasm/omx-dsp.wasm --loader ts/dist/wasm.js --twin <twin dist/index.js>
 *        [--reps 300] [--json]
 */
import { spawnSync } from 'node:child_process';
import { readFileSync } from 'node:fs';
import { parseArgs } from 'node:util';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { resolve } from 'node:path';

const { values: o } = parseArgs({ options: { wasm: { type: 'string' }, loader: { type: 'string' }, twin: { type: 'string' }, reps: { type: 'string', default: '300' }, json: { type: 'boolean' }, child: { type: 'string' } } });
const reqs = JSON.parse(readFileSync(new URL('../vectors/curve-points.json', import.meta.url), 'utf8')).rows.map((r) => r.request);
const reps = Number(o.reps);
const pct = (a, p) => { const s = [...a].sort((x, y) => x - y); return s[Math.min(s.length - 1, Math.floor((p / 100) * s.length))]; };
const now = () => Number(process.hrtime.bigint());

if (o.child === 'load') { // module load time, in its own process: its twenty instances must not count in the calls' peak memory
  const { loadCurve } = await import(pathToFileURL(resolve(o.loader)).href);
  const bytes = readFileSync(o.wasm);
  const loads = [];
  for (let i = 0; i < 20; i++) { const a = now(); await loadCurve(bytes); loads.push(now() - a); }
  console.log(JSON.stringify({ loadMsMedian: pct(loads, 50) / 1e6, loadMsFirst: loads[0] / 1e6 }));
  process.exit(0);
}
if (o.child) {
  const t0 = now();
  let call;
  const extra = {};
  if (o.child === 'wasm') {
    const { loadCurve } = await import(pathToFileURL(resolve(o.loader)).href);
    const bytes = readFileSync(o.wasm);
    const c = await loadCurve(bytes);
    call = c.curvePoints;
    extra.moduleBytes = bytes.length;
    const raw = [];
    for (let k = 0; k < reps; k++) for (const r of reqs) { const s = JSON.stringify(r); const a = now(); c.pointsJson(s); raw.push(now() - a); }
    extra.rawJson = { medianUs: pct(raw, 50) / 1e3, p99Us: pct(raw, 99) / 1e3 };
  } else {
    const m = await import(pathToFileURL(resolve(o.twin)).href);
    extra.firstImportMs = (now() - t0) / 1e6;
    call = m.curvePoints;
  }
  for (let k = 0; k < 20; k++) for (const r of reqs) call(r); // warm: every row, JIT and caches
  const t = [];
  for (let k = 0; k < reps; k++) for (const r of reqs) { const a = now(); call(r); t.push(now() - a); }
  const answers = reqs.map((r) => call(r));
  console.log(JSON.stringify({ calls: t.length, medianUs: pct(t, 50) / 1e3, p99Us: pct(t, 99) / 1e3, maxRssKb: process.resourceUsage().maxRSS, answers, ...extra }));
  process.exit(0);
}

const run = (door) => {
  const a = [fileURLToPath(import.meta.url), '--child', door, '--reps', String(reps)];
  for (const k of ['wasm', 'loader', 'twin']) if (o[k]) a.push(`--${k}`, o[k]);
  const r = spawnSync(process.execPath, a, { encoding: 'utf8', maxBuffer: 1 << 28 });
  if (r.status !== 0) { console.error(r.stderr); process.exit(1); }
  return JSON.parse(r.stdout);
};
const bare = Number(spawnSync(process.execPath, ['-e', 'console.log(process.resourceUsage().maxRSS)'], { encoding: 'utf8' }).stdout);
const w = { ...run('wasm'), ...run('load') }, t = o.twin ? run('twin') : undefined;

// precision: the twin's answer against the module's, number for number, in ulps of max(|v|, 1)
const ulp = (v) => { v = Math.max(Math.abs(v), 1); const f = new Float64Array([v]); new BigUint64Array(f.buffer)[0] += 1n; return f[0] - v; };
let worstUlp = 0, worstAbs = 0, nums = 0, codeMismatch = 0;
if (t) w.answers.forEach((a, i) => {
  const b = t.answers[i];
  if (a.code !== b.code) { codeMismatch++; return; }
  for (const key of ['points', 'stems', 'userPoints']) for (let j = 0; j < (a[key]?.length ?? 0); j++) for (const c of ['x', 'y']) {
    const x = a[key][j][c], y = b[key]?.[j]?.[c], d = Math.abs(x - y);
    nums++; worstAbs = Math.max(worstAbs, d); worstUlp = Math.max(worstUlp, d / ulp(Math.max(Math.abs(x), Math.abs(y))));
  }
});
const result = {
  requests: reqs.length, repetitions: reps, calls: w.calls, bareNodeMaxRssKb: bare,
  wasm: { medianUs: w.medianUs, p99Us: w.p99Us, rawJsonMedianUs: w.rawJson.medianUs, rawJsonP99Us: w.rawJson.p99Us, loadMsMedian: w.loadMsMedian, loadMsFirst: w.loadMsFirst, moduleBytes: w.moduleBytes, maxRssKb: w.maxRssKb, rssOverBareKb: w.maxRssKb - bare },
  twin: t && { medianUs: t.medianUs, p99Us: t.p99Us, firstImportMs: t.firstImportMs, maxRssKb: t.maxRssKb, rssOverBareKb: t.maxRssKb - bare },
  precision: t && { numbersCompared: nums, answersWithAnotherCode: codeMismatch, largestUlp: worstUlp, largestAbsolute: worstAbs },
};
if (o.json) { console.log(JSON.stringify(result, null, 1)); process.exit(0); }
const f = (v, d = 2) => v.toFixed(d);
console.log(`curve-bench: ${reqs.length} requests x ${reps} = ${w.calls} calls per door, each door in its own process (node ${process.version})`);
console.log(`                   median us   p99 us   load ms (median of 20)  peak RSS KB (a bare node: ${bare})`);
console.log(`  wasm curvePoints ${f(w.medianUs).padStart(9)} ${f(w.p99Us).padStart(8)} ${f(w.loadMsMedian).padStart(9)} ${String(w.maxRssKb).padStart(10)} (+${w.maxRssKb - bare})   module ${w.moduleBytes} bytes, first load ${f(w.loadMsFirst)} ms`);
console.log(`  wasm pointsJson  ${f(w.rawJson.medianUs).padStart(9)} ${f(w.rawJson.p99Us).padStart(8)}   (the C call on text, no JSON.stringify or parse)`);
if (t) {
  console.log(`  twin curvePoints ${f(t.medianUs).padStart(9)} ${f(t.p99Us).padStart(8)} ${f(t.firstImportMs).padStart(9)} ${String(t.maxRssKb).padStart(10)} (+${t.maxRssKb - bare})   (load = first import)`);
  console.log(`  precision, wasm against the twin: ${nums} numbers, ${codeMismatch} answers with another code, largest distance ${f(worstUlp, 1)} ulp of max(|v|,1), ${worstAbs.toExponential(2)} absolute`);
}
