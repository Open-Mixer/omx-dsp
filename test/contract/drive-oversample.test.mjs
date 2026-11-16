// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
// 2.7.0 (#32): the drive's oversampling factor is a control, oversample (DRIVE_OVERSAMPLES: 1, 2, 4, the id is the
// factor), coming up at 4 as the console's drive row (DRIVE_OVERSAMPLE_FACTORS) and omx-dsp's drive
// face (OMX_DRIVE_INSTANCE_FACTOR, the oversampler's largest) do.
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { loadData, resolveData } from './oc.mjs';
import { renderJson } from './oc.mjs';
import { UNION } from './helpers.mjs';

const resolved = (dir = UNION) => resolveData(loadData(dir));
const json = (dir) => JSON.parse(renderJson(resolved(dir), '0.0.0')[0].text);

test('drive: oversample is the last control, a choice of DRIVE_OVERSAMPLES (1, 2, 4) that comes up at 4', () => {
  assert.deepEqual(json().kernels.drive.controls.slice(-1), [{ name: 'oversample', kind: 'choice', global: 'DRIVE_OVERSAMPLES' }]);
  assert.deepEqual(json().items.DRIVE_OVERSAMPLES.value, [1, 2, 4]);
  assert.equal(json().items.DRIVE_OVERSAMPLES.default, 4);
});
