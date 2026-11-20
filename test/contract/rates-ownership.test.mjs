// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * Sorting pass, row V25: RT_HARD_TARGET, ORACLE_FLOOR_RATES and ORACLE_FULL_RATES are this library's
 * (data/primitives/rates.json), no longer omx-contract's data/rates.json, which keeps STANDARD_SAMPLE_RATES and
 * STANDARD_QUANTA. The values are held against the golden frozen from omx-contract's data/rates.json at b89426c; the
 * consumer's read is the C header omxdsp/omxdsp_definitions.h, compiled and run; a union without an item renders
 * a header without its defines.
 */
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';
import { readFileSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';
import { test } from 'node:test';
import { OC, loadData, renderC, resolveData } from './oc.mjs';
import { DATA, HERE, UNION, editData, relOf, scratchData, scratchDir } from './helpers.mjs';

const GOLDEN = JSON.parse(readFileSync(join(HERE, 'test/contract/golden/rates.json'), 'utf8'));
const NAMES = Object.keys(GOLDEN);
const DEFS = (dir) => renderC(resolveData(loadData(dir)), { pkg: 'omx-dsp' }).find((f) => f.path === 'omxdsp/omxdsp_definitions.h').text;

test('the owner is this library: the three items live in its data/primitives/rates.json; omx-contract keeps the standard rates and quanta only', () => {
  for (const n of NAMES) assert.equal(relOf(n), 'data/primitives/rates.json', n);
  const mine = JSON.parse(readFileSync(join(DATA, 'rates.json'), 'utf8'));
  assert.deepEqual(Object.keys(mine).filter((k) => !k.startsWith('$')), ['STANDARD_SAMPLE_RATES', 'STANDARD_QUANTA'], 'omx-contract still holds a moved item (build the chain from its sorting-pass branch)');
});

test('the loader returns the items as the old home held them (golden)', () => {
  const doc = JSON.parse(loadData(UNION).files.find((f) => f.rel === 'data/primitives/rates.json').text);
  for (const n of NAMES) assert.deepEqual(doc[n], GOLDEN[n], n);
});

test('the consumer read: the compiled header gives the 192000/128 target, the 4 floor rates and the 9 full rates', () => {
  const dir = scratchDir('omx-dsp-rates-');
  const src = join(dir, 'read.c');
  writeFileSync(src, `#include <stdio.h>
#include <omxdsp/omxdsp_definitions.h>
int main(void) {
  static const unsigned f[] = OMX_ORACLE_FLOOR_RATES_INIT;
  static const unsigned a[] = OMX_ORACLE_FULL_RATES_INIT;
  printf("%d %d", OMX_RT_HARD_TARGET_RATE, OMX_RT_HARD_TARGET_QUANTUM);
  for (unsigned i = 0; i < OMX_ORACLE_FLOOR_RATES_COUNT; i++) printf(" f%u", f[i]);
  for (unsigned i = 0; i < OMX_ORACLE_FULL_RATES_COUNT; i++) printf(" a%u", a[i]);
  printf(" %u %u\\n", (unsigned)(sizeof f / sizeof *f), (unsigned)(sizeof a / sizeof *a));
  return 0;
}
`);
  execFileSync('cc', ['-std=c11', '-Wall', '-Werror', `-I${join(HERE, 'include')}`, `-I${join(OC, 'include')}`, '-o', join(dir, 'read'), src]);
  const out = execFileSync(join(dir, 'read'), { encoding: 'utf8' }).trim();
  const want = [`${GOLDEN.RT_HARD_TARGET.value.rate} ${GOLDEN.RT_HARD_TARGET.value.quantum}`,
    ...GOLDEN.ORACLE_FLOOR_RATES.values.map((v) => `f${v}`), ...GOLDEN.ORACLE_FULL_RATES.values.map((v) => `a${v}`), '4 9'].join(' ');
  assert.equal(out, want);
});

test('the committed header (held equal to the render by `make definitions-check`) carries the defines of the three items', () => {
  const h = readFileSync(join(HERE, 'include/omxdsp/omxdsp_definitions.h'), 'utf8');
  for (const m of ['OMX_RT_HARD_TARGET_RATE', 'OMX_RT_HARD_TARGET_QUANTUM', 'OMX_ORACLE_FLOOR_RATES_INIT', 'OMX_ORACLE_FULL_RATES_INIT']) assert.ok(h.includes(`#define ${m} `), m);
});

test('sabotage: a union without an item renders a header without its defines', () => {
  for (const [n, macro] of [['RT_HARD_TARGET', 'OMX_RT_HARD_TARGET_RATE'], ['ORACLE_FLOOR_RATES', 'OMX_ORACLE_FLOOR_RATES_INIT'], ['ORACLE_FULL_RATES', 'OMX_ORACLE_FULL_RATES_INIT']]) {
    const dir = scratchData();
    editData(dir, 'primitives/rates.json', (d) => { delete d[n]; });
    assert.ok(!DEFS(dir).includes(`#define ${macro} `), n);
  }
});
