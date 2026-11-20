// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * Sorting pass, row V22: READOUT_RATE_HZ, the rate the kernels publish a readout at (50 Hz, the rate they decimate to),
 * is this library's (data/primitives/readout.json), no longer omx-contract's data/primitives.json. The kernels' refs
 * to it resolve unchanged; the consumer read is the C macro OMX_READOUT_RATE_HZ of omxdsp/omxdsp_definitions.h.
 * Golden frozen from omx-contract's data/primitives.json at b89426c.
 */
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';
import { readFileSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';
import { test } from 'node:test';
import { OC, loadData, resolveData, validateData } from './oc.mjs';
import { DATA, HERE, UNION, editData, relOf, scratchData, scratchDir } from './helpers.mjs';

const GOLDEN = JSON.parse(readFileSync(join(HERE, 'test/contract/golden/readout.json'), 'utf8'));
const problems = (dir) => validateData(loadData(dir)).map((p) => `${p.rule}: ${p.message}`);

test('the owner is this library, and omx-contract no longer holds the item', () => {
  assert.equal(relOf('READOUT_RATE_HZ'), 'data/primitives/readout.json');
  const prim = JSON.parse(readFileSync(join(DATA, 'primitives.json'), 'utf8'));
  assert.equal('READOUT_RATE_HZ' in prim, false, 'omx-contract data/primitives.json still holds it (build the chain from its sorting-pass branch)');
});

test('the loader returns the item as the old home held it (golden), and the kernels\' refs to it resolve', () => {
  const doc = JSON.parse(loadData(UNION).files.find((f) => f.rel === 'data/primitives/readout.json').text);
  assert.deepEqual(doc.READOUT_RATE_HZ, GOLDEN.READOUT_RATE_HZ);
  assert.deepEqual(problems(UNION), []);
});

test('the consumer read: the compiled header gives OMX_READOUT_RATE_HZ = 50', () => {
  const dir = scratchDir('omx-dsp-readout-');
  const src = join(dir, 'read.c');
  writeFileSync(src, '#include <stdio.h>\n#include <omxdsp/omxdsp_definitions.h>\nint main(void) { printf("%d", (int)OMX_READOUT_RATE_HZ); return 0; }\n');
  execFileSync('cc', ['-std=c11', '-Wall', '-Werror', `-I${join(HERE, 'include')}`, `-I${join(OC, 'include')}`, '-o', join(dir, 'read'), src]);
  assert.equal(execFileSync(join(dir, 'read'), { encoding: 'utf8' }), String(GOLDEN.READOUT_RATE_HZ.value));
});

test('sabotage: a union without the item is refused by the kernels that read it', () => {
  const dir = scratchData();
  editData(dir, 'primitives/readout.json', (d) => { delete d.READOUT_RATE_HZ; });
  assert.throws(() => resolveData(loadData(dir)), /READOUT_RATE_HZ/);
});
