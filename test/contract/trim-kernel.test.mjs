// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
// 2.3.0: the input trim is a kernel, `trim`, with the one control `trimDb`, so omx-dsp's trim face takes it by name
// (face-conformance) and reads its travel from the C render as OMX_TRIM_RANGE_*. TRIM_TRAVELS, that control's aggregate,
// was retired in dfbc289: nothing read it.
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { loadData, resolveData } from './oc.mjs';
import { renderJson } from './oc.mjs';
import { UNION } from './helpers.mjs';

const json = (dir = UNION) => JSON.parse(renderJson(resolveData(loadData(dir)), '0.0.0')[0].text);

test('trim: one control, trimDb, a travel whose item is TRIM_RANGE', () => {
  assert.deepEqual(json().kernels.trim.controls, [{ name: 'trimDb', kind: 'travel', global: 'TRIM_RANGE' }]);
});
