// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * Sorting pass, row V26: CURVE_AXES and CURVE_KINDS are this library's (data/primitives/curve.json), no longer
 * omx-contract's curves.json (the curves file of its data directory). The words are held against the golden frozen from it
 * at b89426c; the read is omx-contract's own loader over the union a consumer installs; the rendered header carries
 * them; a union with an axis removed is refused by name.
 */
import assert from 'node:assert/strict';
import { existsSync, readFileSync } from 'node:fs';
import { join } from 'node:path';
import { test } from 'node:test';
import { loadData, validateData } from './oc.mjs';
import { DATA, HERE, UNION, editData, relOf, scratchData } from './helpers.mjs';

const GOLDEN = JSON.parse(readFileSync(join(HERE, 'test/contract/golden/curve-vocabulary.json'), 'utf8'));
const problems = (dir) => validateData(loadData(dir)).map((p) => `${p.rule}: ${p.message}`);

test('the owner is this library: both items live in its data/primitives/curve.json and omx-contract holds no curves.json', () => {
  assert.equal(relOf('CURVE_AXES'), 'data/primitives/curve.json');
  assert.equal(relOf('CURVE_KINDS'), 'data/primitives/curve.json');
  assert.equal(existsSync(join(DATA, 'curves.json')), false, 'omx-contract still holds its curves.json (build the chain from its lane/sort-v26-curves-drop)');
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
  // the committed header is held equal to the render by `make definitions-check` (tools/definitions-render.sh)
  const h = readFileSync(join(HERE, 'include/omxdsp/omx_dsp_vocabulary.h'), 'utf8');
  for (const item of ['CURVE_AXES', 'CURVE_KINDS']) {
    for (const [key, w] of Object.entries(GOLDEN[item].words)) {
      const macro = `OMX_${item}_${key.replace(/([A-Z])/g, '_$1').toUpperCase()}`;
      assert.ok(h.includes(`#define ${macro} "${w.word}"`), macro);
    }
  }
});

test('sabotage: a union whose CURVE_AXES lacks the frequency axis is refused', () => {
  const dir = scratchData();
  editData(dir, 'primitives/curve.json', (d) => { delete d.CURVE_AXES.words.frequency; });
  assert.ok(problems(dir).some((p) => /frequency/.test(p)), problems(dir).join('\n'));
});

test('the words travel with their translations: 6 catalogs of omx-dsp, one entry per word, each pinned to its source sentence', async () => {
  const { createHash } = await import('node:crypto');
  const words = Object.fromEntries(['CURVE_AXES', 'CURVE_KINDS'].flatMap((i) => Object.values(GOLDEN[i].words).map((w) => [`${i}.${w.word}`, w.text.en])));
  assert.equal(Object.keys(words).length, 21);
  for (const lang of ['ca', 'de', 'es', 'fr', 'it', 'pt']) {
    const c = JSON.parse(readFileSync(join(HERE, `catalogs/${lang}.json`), 'utf8'));
    assert.equal(c.owner, 'omx-dsp');
    assert.equal(c.lang, lang);
    assert.deepEqual(Object.keys(c.messages).sort(), Object.keys(words).sort(), lang);
    for (const [k, m] of Object.entries(c.messages)) assert.equal(m.source, createHash('sha256').update(words[k]).digest('hex'), `${lang} ${k}`);
  }
});
