// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
// 2.7.0 (#33): the keyed gate's detector filter edges, hpHz and lpHz, are travel controls of the gate in their own
// table, GATE_KEY_LIMITS, with the console's values (its GATE_KEY_LIMITS: 0..20000 Hz, step 1,
// no come-up value).
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { loadData, resolveData } from './oc.mjs';
import { renderJson } from './oc.mjs';
import { UNION } from './helpers.mjs';

const resolved = (dir = UNION) => resolveData(loadData(dir));
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
