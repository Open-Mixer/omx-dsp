// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * The multiband compressor's limits (#62): the band floor and ceiling, the corner line as a fraction of the rate,
 * and the come-up corners for each band count: the render defines the names omx_mbcomp.h reads with the value the
 * kernel runs, and the lists are shaped as the kernel's come-up table needs. The corner line is computed in double
 * there, so its value is held against the _DOUBLE define.
 */
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { loadData, resolveData } from './oc.mjs';
import { renderC } from './oc.mjs';
import { UNION } from './helpers.mjs';

/** NAME -> the numbers its define carries, read as C reads them (`2u` is 2, `1000.0f` is 1000). */
const numbers = (text) => new Map([...text.matchAll(/^#define (OMX_\w+) (.+)$/gm)].map((m) => [m[1], (m[2].match(/-?\d+(?:\.\d+)?/g) ?? []).map(Number)]));
const header = (dir = UNION) => renderC(resolveData(loadData(dir)))[0].text;

test('the render defines the multiband limits with the numbers omx-dsp runs', () => {
  const h = header();
  assert.match(h, /^#define OMX_MBC_MIN_BANDS 2$/m);
  assert.match(h, /^#define OMX_MBC_MAX_BANDS 5$/m);
  assert.match(h, /^#define OMX_MBC_CORNER_MAX_FRACTION_DOUBLE 0\.45$/m);
  assert.match(h, /^#define OMX_MBC_CROSSOVER_DEFAULTS_5_COUNT 4u$/m);
  assert.match(h, /^#define OMX_MBC_CROSSOVER_DEFAULTS_5_INIT \{ 100, 400, 1600, 6400 \}$/m);
});

test('each band count from the floor to the ceiling has its come-up list, bands - 1 corners, ascending, under the corner line at 48 kHz', () => {
  const h = numbers(header());
  const [lo] = h.get('OMX_MBC_MIN_BANDS');
  const [hi] = h.get('OMX_MBC_MAX_BANDS');
  const [frac] = h.get('OMX_MBC_CORNER_MAX_FRACTION_DOUBLE');
  for (let b = lo; b <= hi; b++) {
    const hz = h.get(`OMX_MBC_CROSSOVER_DEFAULTS_${b}_INIT`);
    assert.ok(hz, `${b} bands has a come-up list`);
    assert.equal(hz.length, b - 1, `${b} bands`);
    assert.ok(hz.every((f, i) => i === 0 || f > hz[i - 1]), `${b} bands: ascending`);
    assert.ok(hz.every((f) => f < frac * 48000), `${b} bands: under the line at 48 kHz`);
  }
});
