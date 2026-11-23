// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * The curve unit words are this library's (CURVE_UNITS, data/primitives/curve.json), and each names the standard unit
 * it stands for (`standard`, coordinator ruling 2026-11-21, omx-dsp#122's M2 row): exactly one PARAM_UNITS const per
 * word, in word order. A host takes a standard id to the curve word through the rendered table
 * OMX_CURVE_UNITS_STANDARD (include/omxdsp/omxdsp_definitions.h), never through a strcmp copy of its own.
 */
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { join } from 'node:path';
import { test } from 'node:test';
import { HERE, ROOT } from './helpers.mjs';

const CURVE = JSON.parse(readFileSync(join(HERE, 'data/primitives/curve.json'), 'utf8')).CURVE_UNITS;
const CONSTS = new Set(JSON.parse(readFileSync(join(ROOT, 'contract.d/param/param.json'), 'utf8')).components.schemas.PARAM_UNITS.oneOf.map((w) => w.const));
const IN_USE = { hz: 'Hz', db: 'dB', pc: 'percent', coef: 'coef' };

test('every curve unit word names exactly one standard id, and each is a PARAM_UNITS const', () => {
  assert.ok(Array.isArray(CURVE.standard), 'CURVE_UNITS carries no standard');
  assert.equal(CURVE.standard.length, CURVE.ids.length, 'one standard id per curve unit word');
  CURVE.standard.forEach((s, i) => assert.ok(CONSTS.has(s), `${CURVE.ids[i]}: ${JSON.stringify(s)} is no PARAM_UNITS const`));
  for (const [w, s] of Object.entries(IN_USE)) assert.equal(CURVE.standard[CURVE.ids.indexOf(w)], s, w);
});

test('the header carries OMX_CURVE_UNITS_STANDARD, the in-use words right by enumerator', () => {
  const h = readFileSync(join(HERE, 'include/omxdsp/omxdsp_definitions.h'), 'utf8');
  const m = h.match(/^static const char \*const OMX_CURVE_UNITS_STANDARD\[OMX_CURVE_UNITS_COUNT\] = \{([^}]*)\};$/m);
  assert.ok(m, 'no OMX_CURVE_UNITS_STANDARD table');
  const table = JSON.parse(`[${m[1]}]`);
  const enumBody = h.match(/^enum omx_curve_units \{\n([\s\S]*?)\n\};$/m)[1];
  const pos = Object.fromEntries([...enumBody.matchAll(/^ {2}OMX_CURVE_UNITS_([A-Z0-9_]+) = (\d+),/gm)].map((x) => [x[1].toLowerCase(), Number(x[2])]));
  for (const [w, s] of Object.entries(IN_USE)) assert.equal(table[pos[w]], s, `OMX_CURVE_UNITS_STANDARD[OMX_CURVE_UNITS_${w.toUpperCase()}]`);
  assert.equal(table.length, CURVE.ids.length);
});
