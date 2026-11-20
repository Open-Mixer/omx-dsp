// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * Sorting pass, row V26: CURVE_AXES and CURVE_KINDS are this library's (data/primitives/curve.json), no longer
 * omx-contract's data/curves.json. The words are held against the golden frozen from omx-contract's data/curves.json
 * at b89426c; the read is omx-contract's own loader over the union a consumer installs; the rendered header carries
 * them; a union with an axis removed is refused by name.
 */
import assert from 'node:assert/strict';
import { existsSync, readFileSync } from 'node:fs';
import { join } from 'node:path';
import { test } from 'node:test';
import { loadData, validateData } from './oc.mjs';
import { DATA, HERE, RENDERS, UNION, editData, relOf, scratchData } from './helpers.mjs';

const GOLDEN = JSON.parse(readFileSync(join(HERE, 'test/contract/golden/curve-vocabulary.json'), 'utf8'));
const problems = (dir) => validateData(loadData(dir)).map((p) => `${p.rule}: ${p.message}`);

test('the owner is this library: both items live in its data/primitives/curve.json and omx-contract holds no curves.json', () => {
  assert.equal(relOf('CURVE_AXES'), 'data/primitives/curve.json');
  assert.equal(relOf('CURVE_KINDS'), 'data/primitives/curve.json');
  assert.equal(existsSync(join(DATA, 'curves.json')), false, 'omx-contract still holds data/curves.json (build the chain from its lane/sort-v26-curves-drop)');
});

test('the consumer read: the loader returns the words, their order and every field as the old home held them (golden)', () => {
  const doc = JSON.parse(loadData(UNION).files.find((f) => f.rel === 'data/primitives/curve.json').text);
  for (const name of ['CURVE_AXES', 'CURVE_KINDS']) assert.deepEqual(doc[name], GOLDEN[name], name);
  assert.deepEqual(Object.keys(doc.CURVE_KINDS.words), Object.keys(GOLDEN.CURVE_KINDS.words));
});

test('every kind names two axes of CURVE_AXES, and every axis unit is a standard unit symbol', () => {
  assert.deepEqual(problems(UNION), []);
  const axes = new Set(Object.values(GOLDEN.CURVE_AXES.words).map((w) => w.word));
  for (const [k, w] of Object.entries(GOLDEN.CURVE_KINDS.words)) {
    assert.ok(axes.has(w.x) && axes.has(w.y), k);
  }
});

test('the rendered header carries the axes and the kinds', () => {
  const h = readFileSync(join(RENDERS, 'include/omxdsp/omxdsp_definitions.h'), 'utf8');
  assert.match(h, /OMX_CURVE_AXES_/);
  assert.match(h, /OMX_CURVE_KINDS_/);
  assert.equal(h, readFileSync(join(HERE, 'include/omxdsp/omxdsp_definitions.h'), 'utf8'));
});

test('sabotage: a union whose CURVE_AXES lacks the frequency axis is refused', () => {
  const dir = scratchData();
  editData(dir, 'primitives/curve.json', (d) => { delete d.CURVE_AXES.words.frequency; });
  assert.ok(problems(dir).some((p) => /frequency/.test(p)), problems(dir).join('\n'));
});
