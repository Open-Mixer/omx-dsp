// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * Sorting pass, row V25: tools/ts-golden.mjs runs its oracles at this library's ORACLE_FULL_RATES, read from its
 * own data/primitives/rates.json, never from omx-contract's TypeScript render, which no longer carries it. Sabotage
 * arm: a planted read from omx-contract.js is named.
 */
import { describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';

const MOVED = ['ORACLE_FULL_RATES', 'ORACLE_FLOOR_RATES', 'RT_HARD_TARGET', 'READOUT_RATE_HZ'];

/** The moved names a source reads from omx-contract's render: a destructured import of omx-contract.js or the package. */
export function contractRateReads(src) {
  const out = [];
  const re = /\{([^}]*)\}\s*=\s*await\s+import\(([^)]*)\)|import\s*\{([^}]*)\}\s*from\s*(['"][^'"]+['"])/g;
  for (const m of src.matchAll(re)) {
    const names = m[1] ?? m[3];
    const from = m[2] ?? m[4];
    if (!/omx-contract(\.js|['"])/.test(from)) continue;
    for (const n of MOVED) if (new RegExp(`\\b${n}\\b`).test(names)) out.push(n);
  }
  return out;
}

const SRC = readFileSync(new URL('../../tools/ts-golden.mjs', import.meta.url), 'utf8');

describe('ts-golden reads the oracle rates from omx-dsp, not omx-contract', () => {
  it('no moved name comes from omx-contract\'s render', () => {
    assert.deepEqual(contractRateReads(SRC), []);
    assert.doesNotMatch(SRC, /ORACLE_\w+[^\n]*omx-contract\.js|omx-contract\.js[^\n]*ORACLE_/);
  });
  it('ORACLE_FULL_RATES is read from data/primitives/rates.json', () => {
    assert.match(SRC, /data\/primitives\/rates\.json/);
    assert.match(SRC, /\.ORACLE_FULL_RATES\.values/);
  });
  it('sabotage: a planted read from omx-contract.js is named', () => {
    const planted = "const { ORACLE_FULL_RATES } = await import(pathToFileURL(`${contractTs}/omx-contract.js`).href);";
    assert.deepEqual(contractRateReads(planted), ['ORACLE_FULL_RATES']);
  });
});
