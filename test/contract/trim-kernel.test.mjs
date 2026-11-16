// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
// 2.3.0: the input trim is a kernel, `trim`, with the one control `trimDb`, so omx-dsp's trim face takes it by name
// (face-conformance) and reads its travel from the C render as OMX_TRIM_RANGE_*. TRIM_TRAVELS, that control's aggregate,
// was retired in dfbc289: nothing read it.
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { loadData, resolveData } from './oc.mjs';
import { renderC } from './oc.mjs';
import { renderJson } from './oc.mjs';
import { DATA, UNION, editData, scratchData } from './helpers.mjs';

const header = (dir = UNION) => renderC(resolveData(loadData(dir))).find((f) => f.path === 'omxcontract/omx_contract_limits.h').text;
const json = (dir = UNION) => JSON.parse(renderJson(resolveData(loadData(dir)), '0.0.0')[0].text);

test('trim: one control, trimDb, a travel whose item is TRIM_RANGE', () => {
  assert.deepEqual(json().kernels.trim.controls, [{ name: 'trimDb', kind: 'travel', global: 'TRIM_RANGE' }]);
});

test('trim: a moved come-up value moves the C define (perturbation; TRIM_TRAVELS, the table that read it, was retired in dfbc289)', () => {
  const moved = scratchData();
  editData(moved, 'kernels/trim.json', (d, at) => { at('TRIM_RANGE').travel.default = -6; });
  assert.match(header(moved), /^#define OMX_TRIM_RANGE_DEFAULT -6$/m);
});
