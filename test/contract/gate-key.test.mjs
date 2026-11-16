// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
// 2.7.0 (#33): the keyed gate's detector filter edges, hpHz and lpHz, are travel controls of the gate in their own
// table, GATE_KEY_LIMITS, with the console's values (its GATE_KEY_LIMITS: 0..20000 Hz, step 1,
// no come-up value).
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { main } from './oc.mjs';
import { loadData, resolveData } from './oc.mjs';
import { renderC } from './oc.mjs';
import { renderJson } from './oc.mjs';
import { DATA, RENDERS, UNION, editData, scratchData } from './helpers.mjs';

const resolved = (dir = UNION) => resolveData(loadData(dir));
const header = (dir) => renderC(resolved(dir))[0].text;
const json = (dir) => JSON.parse(renderJson(resolved(dir), '0.0.0')[0].text);

const CONSOLE = { min: 0, max: 20000, step: 1, unit: 'Hz' };

// hpHz and lpHz were the last controls in 2.7.0; key listen and key tilt (5d7e9c6, FX-5) came after them.
test('gate: hpHz and lpHz are the fields of GATE_KEY_LIMITS with the console\'s travel, followed by the FX-5 key controls', () => {
  const c = json().kernels.gate.controls;
  const at = c.findIndex((x) => x.name === 'hpHz');
  assert.deepEqual(c.slice(at + 2).map((x) => x.name), ['keyListen', 'keyTiltDb']);
  assert.deepEqual(c.slice(at, at + 2), [
    { name: 'hpHz', kind: 'travel', global: 'GATE_KEY_LIMITS', table: 'GATE_KEY_LIMITS' },
    { name: 'lpHz', kind: 'travel', global: 'GATE_KEY_LIMITS', table: 'GATE_KEY_LIMITS' },
  ]);
  assert.deepEqual(json().items.GATE_KEY_LIMITS.value, { hpHz: CONSOLE, lpHz: CONSOLE });
});

test('perturbation: a moved key filter edge leaves the console\'s travel, and --check goes red', () => {
  const dir = scratchData();
  editData(dir, 'kernels/gate.json', (d, at) => { at('GATE_KEY_LIMITS.lpHz').max = 22000; });
  assert.notDeepEqual(json(dir).items.GATE_KEY_LIMITS.value.lpHz, CONSOLE);
  assert.match(header(dir), /^#define OMX_GATE_KEY_LP_HZ_MAX 22000\.0f$/m);
  assert.equal(main(['render', '--target', 'c', '--out', `${RENDERS}/include`, '--check', '--data', dir]), 1);
});
