// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * A curve term's unit and a meter's unit are the standard unit ids themselves (C2, plugin-hostd#215 D1/D2: the
 * PARAM_UNITS const is the wire word, its x-symbol the display only; omx-dsp#122). There is no curve word of this
 * library's own beside the id and no table that maps one onto the other: a host sends the unit the description gave
 * the role, as it got it, and omx_curve_points_json reads it. The subset and its order stay this library's.
 *
 * Before: CURVE_UNITS spelt `hz`, `db`, `pc` (a `standard` table mapped them to `Hz`, `dB`, `percent`), so a host
 * that sent the description's `Hz` had its curve refused (unitRefused); METER_UNITS spelt `dbfs` beside a `labels`
 * copy of the symbols.
 */
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { join } from 'node:path';
import { test } from 'node:test';
import { HERE, ROOT } from './helpers.mjs';

const CONSTS = new Set(JSON.parse(readFileSync(join(ROOT, 'contract.d/param/param.json'), 'utf8')).components.schemas.PARAM_UNITS.oneOf.map((w) => w.const));
const CURVE = JSON.parse(readFileSync(join(HERE, 'data/primitives/curve.json'), 'utf8')).CURVE_UNITS;
const METER = JSON.parse(readFileSync(join(HERE, 'data/primitives/meter.json'), 'utf8')).METER_UNITS;

for (const [name, set, copy] of [['CURVE_UNITS', CURVE, 'standard'], ['METER_UNITS', METER, 'labels']]) {
  test(`every ${name} id is a PARAM_UNITS id, with no ${copy} copy beside it`, () => {
    for (const id of set.ids) assert.ok(CONSTS.has(id), `${name}: ${JSON.stringify(id)} is no PARAM_UNITS id`);
    assert.equal(set[copy], undefined, `${name} carries a ${copy} table: the id is the word`);
  });
}

test('omx_curve.h reads the CURVE_UNITS ids, in their order: OMX_CURVE_UNIT_WORDS is no other spelling', () => {
  const h = readFileSync(join(HERE, 'include/omxdsp/omx_curve.h'), 'utf8');
  const m = h.match(/OMX_CURVE_UNIT_WORDS\[OMX_CURVE_UNITS_COUNT\] = \{([^}]*)\};/);
  assert.ok(m, 'no OMX_CURVE_UNIT_WORDS table in omx_curve.h');
  assert.deepEqual(JSON.parse(`[${m[1]}]`), CURVE.ids);
});
