// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
// 2.6.0: the sample-rate converter's design constants (the src kernel) and the generators' kind
// and waveform sets (the generator kernel), the kernels omx-dsp takes over from the engine.
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
/** The value the c render gives a name: an enumerator's or a define's. */
const cValue = (h, name) => {
  const m = h.match(new RegExp(`^(?:#define ${name} |  ${name} = )([\\d.]+)[uf]?,?$`, 'm'));
  return m ? Number(m[1]) : undefined;
};
/** The value each name carries in the render: what the converter and the generators run. */
const WANT = [
  ['OMX_SRC_TAPS', 64],
  ['OMX_SRC_STOPBAND_DB', 96],
  ['OMX_SRC_ROLLOFF_DOUBLE', 0.92],
  ['OMX_SRC_MAX_L', 1024],
  ['OMX_GENERATOR_KINDS_OSCILLATOR', 0],
  ['OMX_GENERATOR_KINDS_WHITE', 1],
  ['OMX_GENERATOR_KINDS_PINK', 2],
  ['OMX_WAVEFORMS_SINE', 0],
  ['OMX_WAVEFORMS_SQUARE', 1],
  ['OMX_WAVEFORMS_SAW', 2],
  ['OMX_WAVEFORMS_TRIANGLE', 3],
];
/** The names whose value the render does not give as above. */
const disagree = (h) => WANT.filter(([c, v]) => v !== cValue(h, c)).map(([c, v]) => `${c} ${v}, render ${cValue(h, c)}`);

test('src: four constants in data/kernels/src.json, the design the converter runs at', () => {
  const items = json().items;
  for (const n of ['SRC_TAPS', 'SRC_STOPBAND_DB', 'SRC_ROLLOFF', 'SRC_MAX_L']) assert.equal(items[n].rel, 'data/kernels/src.json', n);
  assert.deepEqual(json().kernels.src.controls, []);
});

test('generator: two choices, kind (GENERATOR_KINDS) and waveform (WAVEFORMS), in surface order, then the oscillator\'s frequency travel (2.14.0)', () => {
  assert.deepEqual(json().kernels.generator.controls, [
    { name: 'kind', kind: 'choice', global: 'GENERATOR_KINDS' },
    { name: 'waveform', kind: 'choice', global: 'WAVEFORMS' },
    { name: 'oscillatorFreq', kind: 'travel', global: 'OSCILLATOR_FREQ_RANGE' },
  ]);
  assert.deepEqual(json().items.GENERATOR_KINDS.value, ['oscillator', 'white', 'pink']);
  assert.deepEqual(json().items.WAVEFORMS.value, ['sine', 'square', 'saw', 'triangle']);
});

test('the c render gives the converter\'s and the generators\' values: four src constants, three kinds, four waveforms', () => {
  assert.equal(WANT.length, 11);
  assert.deepEqual(disagree(header()), []);
});

test('perturbation: a moved tap count or a reordered waveform set leaves the values above, and --check goes red', () => {
  const taps = scratchData();
  editData(taps, 'kernels/src.json', (d, at) => { at('SRC_TAPS').value = 32; });
  assert.deepEqual(disagree(header(taps)), ['OMX_SRC_TAPS 64, render 32']);
  assert.equal(main(['render', '--target', 'c', '--out', `${RENDERS}/include`, '--check', '--data', taps]), 1);
  const order = scratchData();
  editData(order, 'kernels/generator.json', (d, at) => { at('WAVEFORMS').ids.reverse(); });
  assert.deepEqual(disagree(header(order)), [
    'OMX_WAVEFORMS_SINE 0, render 3',
    'OMX_WAVEFORMS_SQUARE 1, render 2',
    'OMX_WAVEFORMS_SAW 2, render 1',
    'OMX_WAVEFORMS_TRIANGLE 3, render 0',
  ]);
  assert.equal(main(['render', '--target', 'c', '--out', `${RENDERS}/include`, '--check', '--data', order]), 1);
});
