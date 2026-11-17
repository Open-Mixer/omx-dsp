// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/** The loader: the five WASI stand-ins throw and no call reaches one; the twin's signature; refusals. */
import { describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { WASI_IMPORTS, loadCurve, wasiStandIns } from '../dist/index.js';

const bytes = readFileSync(new URL('../wasm/omx-dsp.wasm', import.meta.url));
const DYN = { kind: 'transfer_curve', family: 'dynamics', sampleRate: 48000, points: 3, extents: { min: -40, max: 0 }, terms: [{ role: 'threshold', value: -20, unit: 'db' }, { role: 'ratio', value: 4, unit: 'ratio' }] };

describe('the curve module', () => {
  it('imports exactly the five stdio WASI functions the loader stands in for', async () => {
    const imports = WebAssembly.Module.imports(new WebAssembly.Module(bytes));
    assert.deepEqual(imports.map((i) => `${i.module}.${i.name}`).sort(), WASI_IMPORTS.map((n) => `wasi_snapshot_preview1.${n}`).sort());
  });
  it('answers with the twin\'s signature, and every refusal and malformed request without calling a stand-in', async () => {
    const curve = await loadCurve(bytes);
    assert.deepEqual(curve.curvePoints(DYN), { points: [{ x: -40, y: -40 }, { x: -20, y: -20 }, { x: 0, y: -15 }], marks: [] });
    assert.deepEqual(curve.curvePoints({ ...DYN, terms: [] }), { code: 'termMissing' });
    assert.throws(() => curve.pointsJson('{"kind":'), /no curve request/);
    assert.throws(() => curve.pointsJson('x'.repeat(70000)), /does not fit/);
    for (let i = 0; i < 200; i++) curve.curvePoints({ ...DYN, points: 2 + (i % 250) }); // a stand-in throws: none did
  });
  it('a stand-in THROWS when one is called, naming the function (a module that reached WASI would be red, not wrong)', () => {
    for (const n of WASI_IMPORTS) assert.throws(() => wasiStandIns().wasi_snapshot_preview1[n](), new RegExp(`called WASI ${n}`));
  });
});
