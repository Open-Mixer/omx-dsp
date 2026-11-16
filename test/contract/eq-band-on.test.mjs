// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
// 2.9.0 (#26): an EQ band comes up off (decided 2026-10-13). The contract declares bandOn's come-up value,
// EQ_BAND_ONS defaults to off, and every strip reads it from here: an omx-eq plugin already brings its bands up off,
// and the console's channel EQ follows.
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { main } from './oc.mjs';
import { loadData, resolveData } from './oc.mjs';
import { renderC } from './oc.mjs';
import { renderJson } from './oc.mjs';
import { renderTs } from './oc.mjs';
import { DATA, RENDERS, UNION, editData, scratchData } from './helpers.mjs';

const resolved = (dir = UNION) => resolveData(loadData(dir));
const header = (dir) => renderC(resolved(dir))[0].text;
const json = (dir) => JSON.parse(renderJson(resolved(dir), '0.0.0')[0].text);

test('eq: bandOn is a choice of EQ_BAND_ONS (off, on) that comes up off', () => {
  const c = json().kernels.eq.controls.find((x) => x.name === 'bandOn');
  assert.deepEqual(c, { name: 'bandOn', kind: 'choice', global: 'EQ_BAND_ONS', count: 'EQ_BAND_COUNTS' });
  assert.deepEqual(json().items.EQ_BAND_ONS.value, ['off', 'on']);
  assert.equal(json().items.EQ_BAND_ONS.default, 'off');
  assert.match(header(), /^#define OMX_EQ_BAND_ONS_DEFAULT OMX_EQ_BAND_ONS_OFF$/m);
  assert.match(renderTs(resolved())[0].text, /^export const EQ_BAND_ONS = \[\n {2}"off",\n {2}"on"\n\];$/m);
});

test('perturbation: the band switch\'s default removed, the C default goes and --check goes red', () => {
  const dir = scratchData();
  editData(dir, 'kernels/eq.json', (d, at) => { delete at('EQ_BAND_ONS').default; });
  assert.doesNotMatch(header(dir), /OMX_EQ_BAND_ONS_DEFAULT/);
  assert.equal(json(dir).items.EQ_BAND_ONS.default, undefined);
  assert.equal(main(['render', '--target', 'c', '--out', `${RENDERS}/include`, '--check', '--data', dir]), 1);
});
