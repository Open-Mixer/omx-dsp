// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * THE DIFFERENCE TWIN AGREES WITH ITS KERNEL: bandDelta (dist/, the published code) against vectors/rta-delta.json,
 * which tools/rta-delta-golden.c writes from omx_rta_band_delta (analysis/omx_rta_bands.h) and the suite's rta_delta
 * arm reads back. Each case is run again from its own inputs (preDb, postDb; null is silence): every difference must
 * agree within 1e-9 dB (null with NaN) and every wedge exactly in band and sign and within 1e-12 in strength. Then the
 * closed form the C arm checks, and five copies of the built twin, each with one rule broken, must each disagree.
 */
import { describe, it, after } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, writeFileSync, rmSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { EQ_GAIN_RANGE, RTA_DELTA_DEADBAND_DB, bandDelta } from '../dist/index.js';

const HERE = dirname(fileURLToPath(import.meta.url));
const VECTORS = JSON.parse(readFileSync(join(HERE, '..', '..', 'vectors', 'rta-delta.json'), 'utf8'));
const level = (x) => (x === null ? -Infinity : x);

/** The cases of the file `delta` disagrees with, named. */
function disagreements(delta) {
  const bad = [];
  for (const c of VECTORS.cases) {
    const { diff, wedges } = delta(c.preDb.map(level), c.postDb.map(level));
    const name = `${c.case} at ${c.rate} Hz, ${c.n} bands`;
    if (diff.length !== c.diff.length || wedges.length !== c.wedges.length) {
      bad.push(`${name}: ${diff.length} diffs, ${wedges.length} wedges`);
      continue;
    }
    const d = c.diff.findIndex((w, i) => (w === null ? !Number.isNaN(diff[i]) : !(Math.abs(diff[i] - w) <= 1e-9)));
    if (d >= 0) bad.push(`${name}: diff[${d}] ${diff[d]}, the file ${c.diff[d]}`);
    const j = c.wedges.findIndex(
      ([band, boost, strength], k) =>
        wedges[k].band !== band || wedges[k].boost !== (boost === 1) || !(Math.abs(wedges[k].strength - strength) <= 1e-12),
    );
    if (j >= 0) bad.push(`${name}: wedge ${j} ${JSON.stringify(wedges[j])}, the file ${JSON.stringify(c.wedges[j])}`);
  }
  return bad;
}

describe('the difference twin', () => {
  it("reads the file's law as its own: the deadband and the EQ's gain limit", () => {
    assert.equal(VECTORS.law.deadbandDb, RTA_DELTA_DEADBAND_DB);
    assert.equal(VECTORS.law.fullScaleDb, EQ_GAIN_RANGE.max);
  });

  it('answers every case of vectors/rta-delta.json as omx_rta_band_delta does', () => {
    assert.equal(VECTORS.cases.length, 57, 'seven pairs at two rates and four band counts, and the edges');
    assert.ok(VECTORS.cases.some((c) => c.case === 'edges'), 'the file holds the edges case');
    assert.ok(VECTORS.cases.some((c) => c.wedges.length > 0 && c.wedges.some((w) => w[1] === 0)), 'the file holds cuts');
    assert.ok(VECTORS.cases.some((c) => c.diff.includes(null)), 'the file holds silent bands');
    assert.deepEqual(disagreements(bandDelta), []);
  });

  it('holds the closed form: a boost, a +3/-5 pair, the deadband, silence, the clamp', () => {
    const at = (d) => bandDelta(d.map(() => -30), d.map((x) => -30 + x));
    const b = at([0, 0, 0, 6, 6, 6, 0, 0]);
    assert.deepEqual(b.diff, [0, 0, 0, 6, 6, 6, 0, 0]);
    assert.deepEqual(
      b.wedges,
      [2, 3, 4, 5].map((band) => ({ band, boost: true, strength: 6 / EQ_GAIN_RANGE.max })),
    );
    const m = at([0, 0, 0, 3, -5, 0, 0, 0]).wedges.find((w) => w.band === 3);
    assert.deepEqual(m, { band: 3, boost: false, strength: 5 / EQ_GAIN_RANGE.max });
    const edge = (x) => bandDelta([-1, -1, -1], [-1, -1 + x, -1]).wedges.length;
    assert.equal(edge(RTA_DELTA_DEADBAND_DB), 2, 'exactly the deadband draws');
    assert.equal(edge(RTA_DELTA_DEADBAND_DB * 0.99), 0, 'under it does not');
    const s = at([0, 0, 12, -Infinity, 12, 0, 0, 0]);
    assert.ok(Number.isNaN(s.diff[3]));
    assert.deepEqual(s.wedges.map((w) => w.band), [1, 4]);
    assert.equal(at([0, 0, 0, 30, 0, 0, 0, 0]).wedges[0].strength, 1);
    assert.deepEqual(bandDelta([], []), { diff: [], wedges: [] });
  });

  it('refuses two readings of different band counts', () => {
    assert.throws(() => bandDelta([-30, -30], [-30]), RangeError);
  });
});

/* The sabotage arms: a copy of the built twin with one rule broken, beside dist/rta.js so its imports resolve,
 * imported and removed. A replacement that matches nothing is a failure, not a skip. */
const DIST = join(HERE, '..', 'dist');
const SOURCE = readFileSync(join(DIST, 'rta.js'), 'utf8');
const made = [];
after(() => {
  for (const p of made) rmSync(p, { force: true });
});

const SABOTAGES = {
  'the deadband exclusive': ['if (magnitude < RTA_DELTA_DEADBAND_DB)', 'if (magnitude <= RTA_DELTA_DEADBAND_DB)'],
  "the sign of the first band": ['boost: d0 + d1 >= 0,', 'boost: d0 >= 0,'],
  'no clamp': ['strength: strength < 1 ? strength : 1', 'strength'],
  'silence left to the arithmetic': ['!Number.isFinite(a) || !Number.isFinite(b) ? NaN : b - a', 'b - a'],
  'a 12 dB full scale': ['const full = EQ_GAIN_RANGE.max;', 'const full = 12;'],
};

describe('the difference twin test is not blind', () => {
  for (const [name, [from, to]] of Object.entries(SABOTAGES))
    it(`goes red with ${name}`, async () => {
      assert.equal(SOURCE.split(from).length, 2, `the ${name} sabotage targets one line of dist/rta.js`);
      const p = join(DIST, `rta.delta-sabotage-${process.pid}-${made.length}.js`);
      made.push(p);
      writeFileSync(p, SOURCE.replace(from, to));
      const broken = await import(pathToFileURL(p).href);
      assert.ok(disagreements(broken.bandDelta).length > 0, `the twin with ${name} still agrees with the file`);
    });
});
