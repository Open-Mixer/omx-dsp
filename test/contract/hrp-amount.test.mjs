// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * #30, #64: the HRP corrector's amount is declared once, here, with its come-up value: HRP_AMOUNT_RANGE, 0..1,
 * continuous, coming up at 0.5 from the desk (decided 2026-10-13: the default lives in the contract and every
 * consumer derives from it). The readers take it by name, never as a number of their own:
 *
 * - omx-dsp (0f23bed) include/omxdsp/analysis/omx_hrp_correct.h:103 defines OMX_HRP_DEFAULT_AMOUNT as
 *   OMX_HRP_AMOUNT_RANGE_DEFAULT, and the LV2 face comes up there (omx_hrp_lv2.h:219);
 * - the console engine re-exports the ts render's HRP_AMOUNT_RANGE and seeds a fresh channel, and the clamp's
 *   fallback, from its `default` (0.5 today, the value its literal held before it read the travel).
 *
 * So the value and the shape both matter: the C default, and the ts render's literal type, which the console's
 * travel checks hold assignable to a full travel.
 */
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { loadData, resolveData } from './oc.mjs';
import { renderC } from './oc.mjs';
import { renderJson } from './oc.mjs';
import { renderTs } from './oc.mjs';
import { UNION } from './helpers.mjs';

const resolved = (dir = UNION) => resolveData(loadData(dir));
const items = (dir) => JSON.parse(renderJson(resolved(dir), '0.0.0')[0].text).items;
const header = (dir) => renderC(resolved(dir))[0].text;
const dts = (dir) => renderTs(resolved(dir)).find((f) => f.path.endsWith('.d.ts')).text;

/** The value both readers were checked against: the come-up value the console engine and omx-dsp use. */
const CONSUMER = { min: 0, max: 1, step: 0, unit: '', default: 0.5, defaultFrom: 'desk' };

test('HRP_AMOUNT_RANGE declares its full travel, the come-up value 0.5 from the desk among it', () => {
  assert.deepEqual(items().HRP_AMOUNT_RANGE.value, CONSUMER);
});

test('the C render carries the come-up value omx-dsp\'s OMX_HRP_DEFAULT_AMOUNT reads', () => {
  assert.match(header(), /^#define OMX_HRP_AMOUNT_RANGE_DEFAULT 0\.5f$/m);
  assert.match(header(), /^#define OMX_HRP_AMOUNT_RANGE_DEFAULT_DOUBLE 0\.5$/m);
});

test('the ts render types the travel with its default, the shape the console re-exports', () => {
  const m = /^export declare const HRP_AMOUNT_RANGE: \{\n([\s\S]*?)\n\};$/m.exec(dts());
  assert.ok(m, 'the d.ts declares HRP_AMOUNT_RANGE');
  assert.deepEqual(m[1].split('\n').map((l) => l.trim()), [
    'readonly min: 0;', 'readonly max: 1;', 'readonly step: 0;', 'readonly unit: "";', 'readonly default: 0.5;', 'readonly defaultFrom: "desk";',
  ]);
});
