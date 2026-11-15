// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * THE PLACEMENT TWIN READS THE C PLACEMENT'S VECTORS: hrpPlaceBands and hrpStepToward (dist/, the published code)
 * run every line of test/analysis/fixtures/hrp_place.vectors, the file test/analysis/hrp_place.test.c runs
 * omx_hrp_place.h over, and must give every value it states inside the file's tolerances.
 */
import { describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { EQ_FREQ_RANGE, hrpPlaceBands, hrpPlaceInRange, hrpStepToward } from '../dist/index.js';

const VECTORS = join(dirname(fileURLToPath(import.meta.url)), '..', '..', 'test', 'analysis', 'fixtures', 'hrp_place.vectors');
const num = (w) => (w === 'nan' ? NaN : w === 'inf' ? Infinity : w === '-inf' ? -Infinity : Number(w));
/** Exact unless a tolerance is given; a NaN is matched only by a NaN (Object.is would split 0 and -0). */
const same = (a, b, tol = 0) => (Number.isNaN(b) ? Number.isNaN(a) : tol > 0 ? Math.abs(a - b) <= tol : a === b);

/** The file's lines, each with its words, and its tolerances. */
export function readLines(text = readFileSync(VECTORS, 'utf8')) {
  const tol = {};
  const lines = [];
  for (const raw of text.split('\n')) {
    const line = raw.replace(/#.*/, '').trim();
    if (!line) continue;
    const w = line.split(/\s+/);
    if (w[0] === 'tolerance') tol[w[1]] = Number(w[2]);
    else lines.push({ line, w });
  }
  return { tol, lines };
}

/** One line's verdict: undefined when it holds, else what was read. */
export function judge({ w }, tol, place = hrpPlaceBands, step = hrpStepToward) {
  if (w[0] === 'step') {
    const r = step(num(w[2]), num(w[3]));
    return same(r, num(w[4])) ? undefined : String(r);
  }
  if (w[0] === 'place') {
    const arrow = w.indexOf('=>');
    const gains = w.slice(8, arrow).map(num);
    const want = w.slice(arrow + 1);
    const got = place(num(w[2]), gains, num(w[3]), { maxCutDb: num(w[4]), maxBoostDb: num(w[5]), maxBands: num(w[6]) });
    const read = `${got.length} band(s): ${got.map((b) => `${b.harmonic} ${b.freqHz} ${b.gainDb} ${b.q}`).join(' ')}`;
    if (got.length !== want.length / 4) return read;
    for (let b = 0; b < got.length; b++) {
      const e = want.slice(4 * b, 4 * b + 4);
      const ok = got[b].harmonic === Number(e[0]) && same(got[b].freqHz, num(e[1])) && same(got[b].gainDb, num(e[2])) &&
        same(got[b].q, num(e[3]), tol.q);
      if (!ok) return read;
    }
    return undefined;
  }
  return 'a line this test does not read';
}

describe('hrp-place twin vs the shared vectors', () => {
  const { tol, lines } = readLines();
  it('the file holds lines and states its Q tolerance', () => {
    assert.ok(lines.length > 0);
    assert.ok(tol.q > 0);
  });
  for (const l of lines) {
    it(`${l.w[0]} ${l.w[1]}`, () => {
      assert.equal(judge(l, tol), undefined, `${l.line}`);
    });
  }
  it('the range predicate keeps both bounds and drops just outside them, and NaN', () => {
    assert.equal(hrpPlaceInRange(EQ_FREQ_RANGE.min), true);
    assert.equal(hrpPlaceInRange(EQ_FREQ_RANGE.max), true);
    assert.equal(hrpPlaceInRange(EQ_FREQ_RANGE.min - 1e-9), false);
    assert.equal(hrpPlaceInRange(EQ_FREQ_RANGE.max + 1e-9), false);
    assert.equal(hrpPlaceInRange(NaN), false);
  });
});
