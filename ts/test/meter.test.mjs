// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * THE METER VIEW TWIN AGREES WITH ITS KERNEL AND WITH THE VECTORS: meter.ts (dist/) against mix/omx_meter_view.h through
 * the oracle, on the held and gain-reduction reads of vectors/meter-view.json (the nine oracle rates, quanta 32 to
 * 1024) and on grids of its own, and against the file itself, the one every copy of the law reads. Each word of the law
 * has a sabotage arm that must go red: the marker stepped per change, the fall one dB per second off, the hysteresis
 * removed, the floor moved.
 */
import { describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { against, ask, compare, oracleKernels, oracleRates } from '../oracle/ts-twin-oracle.mjs';
import {
  METER_VIEW,
  meterBarStep,
  meterFill,
  meterFillDb,
  meterMarker,
  meterStep,
} from '../dist/index.js';
import { TOLERANCES as T } from './tolerances.mjs';

const VECTORS = JSON.parse(readFileSync(join(dirname(fileURLToPath(import.meta.url)), '..', '..', 'vectors', 'meter-view.json'), 'utf8'));
const cases = (set) => VECTORS.cases.filter((c) => c.set === set);
const { floorDb: FLOOR, fallDbPerSecond: FALL } = METER_VIEW;

/** A marker law run over a sequence of (fill, dt): the markers. */
const markers = (fills, dts, step = meterMarker, fall = FALL, floor = FLOOR) => {
  let m;
  return fills.map((f, i) => (m = step(m, f, dts[i], fall, floor)).marker);
};

/** Sabotaged twins, one per word. */
const perChange = (p, fill, dt, fall, floor) => {
  const q = p ?? { peak: 0, elapsedS: 0, live: 0, marker: 0 };
  const f = Math.min(1, Math.max(0, fill || 0));
  return meterMarker(q, f, f !== q.live ? dt : 0, fall, floor);
};
describe('the oracle answers the meter view kernels', () => {
  it('has every handler the twins need', () => {
    const k = oracleKernels();
    for (const n of ['meter_view_fill_db', 'meter_view_fill', 'meter_view_step', 'meter_view_ladder', 'meter_view_bar_step']) assert.ok(k.includes(n), n);
  });
  it('the vectors cover every oracle rate at quanta 32 to 1024', () => {
    const rates = oracleRates();
    for (const set of ['held']) {
      assert.deepEqual([...new Set(cases(set).map((c) => c.rate))].sort((a, b) => a - b), [...rates].sort((a, b) => a - b), set);
      assert.deepEqual([...new Set(cases(set).map((c) => c.quantum))], [32, 64, 128, 256, 512, 1024], set);
    }
    assert.deepEqual(cases('fall').map((c) => c.drawHz), [5, 20, 30, 60, 5, 20, 30, 60]);
  });
});

describe('the declared numbers', () => {
  it('are the spec\'s, and the file\'s', () => {
    assert.deepEqual({ ...METER_VIEW }, { floorDb: -60, stepHysteresis: 0.25, fallDbPerSecond: 18, hostReadHz: 50, streamHz: 20 });
    const { grHoldMs, ...law } = VECTORS.law;
    assert.deepEqual(law, { ...METER_VIEW });
    assert.ok(grHoldMs > 0);
  });
});

describe('the fills agree with omx_meter_view_fill_db and omx_meter_view_fill', () => {
  const DB = [-Infinity, -200, -120, -60.0001, -60, -59.9, -45, -30, -18, -12, -6, -0.5, -1e-9, 0, 0.5, 6, Infinity, NaN];
  const LIN = [-1, 0, 1e-9, 1e-6, 0.001, 0.0010001, 0.01, 0.1, 0.25, 0.5, 0.7071, 0.999, 1, 2, Infinity, NaN];
  const floors = [-60, -48, -90];
  const dbAsked = ask('meter_view_fill_db', floors.flatMap((fl) => DB.map((d) => [d, fl])));
  const linAsked = ask('meter_view_fill', floors.flatMap((fl) => LIN.map((l) => [l, fl])));
  it('agree', () => {
    assert.deepEqual(against(dbAsked, (r) => [meterFillDb(r[0], r[1])], T.meterView).failures, []);
    assert.deepEqual(against(linAsked, (r) => [meterFill(r[0], r[1])], T.meterFill).failures, []);
  });
  it('SABOTAGE the floor moved to -54 dBFS: red', () => {
    assert.ok(against(dbAsked, (r) => [meterFillDb(r[0], r[1] === -60 ? -54 : r[1])], T.meterView).failures.length > 0);
    const f = cases('fillDb')[0];
    assert.ok(compare(f.db.map((d) => meterFillDb(d, -54)), f.fill, T.meterView).failures.length > 0);
  });
});

describe('the marker agrees with omx_meter_view_step on every held read of the vectors', () => {
  const held = cases('held');
  const asked = ask('meter_view_step', held.map((c) => [FALL, FLOOR, ...c.fill.flatMap((f, i) => [f, c.dt[i]])]));
  const twin = (step, fall = FALL) => (r) => {
    const k = (r.length - 2) / 2;
    return markers(Array.from({ length: k }, (_, i) => r[2 + 2 * i]), Array.from({ length: k }, (_, i) => r[3 + 2 * i]), step, fall);
  };
  it('at every oracle rate and quantum', () => {
    assert.equal(asked.rows.length, 54);
    assert.deepEqual(against(asked, twin(meterMarker), T.meterView).failures, []);
  });
  it('SABOTAGE the marker stepped per change, not by elapsed time: red (it hangs on silence)', () => {
    assert.ok(against(asked, twin(perChange), T.meterView).failures.length > 54);
  });
  it('SABOTAGE the fall one dB per second off: red', () => {
    assert.ok(against(asked, twin(meterMarker, FALL + 1), T.meterView).failures.length > 54);
  });
});

describe('the marker falls by elapsed time: the same at draw rates 5 to 60 Hz, on silence too', () => {
  const run = (hz, live, step = meterMarker, fall = FALL) => {
    const n = Math.round(4 * hz);
    let m = step(undefined, 1, 0, fall, FLOOR);
    const out = [m.marker];
    for (let k = 1; k <= n; k++) out.push((m = step(m, live, 1 / hz, fall, FLOOR)).marker);
    return out;
  };
  it('is the file\'s at every draw rate, and the closed form 1 - 18 t / 60', () => {
    for (const c of cases('fall')) {
      const got = run(c.drawHz, c.live);
      assert.deepEqual(compare(got, c.marker, T.meterView).failures, [], `${c.drawHz} Hz`);
      const closed = got.map((_, k) => Math.max(c.live, 1 - (18 * k) / c.drawHz / 60));
      assert.deepEqual(compare(got, closed, { abs: 1e-12 }).failures, [], `${c.drawHz} Hz closed form`);
    }
    for (let s = 0; s <= 20; s++) {
      const at = [5, 20, 30, 60].map((hz) => run(hz, 0)[Math.round(0.2 * s * hz)]);
      for (const a of at) assert.ok(Math.abs(a - at[0]) < 1e-12, `t ${0.2 * s}: ${at}`);
    }
    assert.equal(run(60, 0)[201], 0);
  });
  it('SABOTAGE per change: on silence the marker hangs at full scale, red', () => {
    const c = cases('fall')[3];
    const hung = run(c.drawHz, c.live, perChange);
    assert.ok(compare(hung, c.marker, T.meterView).failures.length > 100);
    assert.ok(hung.at(-1) > 0.9);
  });
});

describe('the ladder and the bar agree with omx_meter_view_ladder and omx_meter_view_bar_step', () => {
  const FILLS = Array.from({ length: 101 }, (_, i) => i / 100).concat([-0.5, 1.5, NaN]);
  const ladder = ask('meter_view_ladder', [8, 12, 13].flatMap((steps) => [-1, 0, 3, 4, 5, 8].flatMap((prev) => FILLS.map((f) => [steps, 0.25, prev, f]))));
  const LEVELS = [0, 1e-4, 0.001, 0.01, 0.0316, 0.1, 0.25, 0.5, 0.7, 1, 2];
  const bar = ask('meter_view_bar_step', [8, 13].flatMap((steps) => [-1, 0, 2, 6, 8].flatMap((prev) => LEVELS.map((l) => [steps, prev, l]))));
  const step = (hyst) => (r) => [meterStep(r[2] < 0 ? undefined : r[2], r[3], r[0], hyst)];
  it('agree, and the ladder is the file\'s', () => {
    assert.deepEqual(against(ladder, step(0.25), T.meterView).failures, []);
    assert.deepEqual(against(bar, (r) => [meterBarStep(r[1] < 0 ? undefined : r[1], r[2], r[0])], T.meterView).failures, []);
    const c = cases('ladder')[0];
    let prev;
    assert.deepEqual(c.fill.map((f) => (prev = meterStep(prev, f, c.steps, c.hysteresis))), c.count);
  });
  it('SABOTAGE the hysteresis removed: red, and the edge case toggles', () => {
    assert.ok(against(ladder, step(0), T.meterView).failures.length > 10);
    const c = cases('ladder')[0];
    let prev;
    assert.ok(compare(c.fill.map((f) => (prev = meterStep(prev, f, c.steps, 0))), c.count, T.meterView).failures.length > 5);
  });
});

describe('the twin reproduces the held reads of the vectors from their peaks', () => {
  it('the fill of every read and the marker', () => {
    let bad = 0;
    for (const c of cases('held')) {
      const fills = c.peak.map((p) => meterFill(p));
      bad += compare(fills, c.fill, T.meterFill).failures.length;
      bad += compare(markers(fills, c.dt), c.marker, { abs: 1e-14 }).failures.length;
    }
    assert.equal(bad, 0);
  });
  it('SABOTAGE the floor moved, the fall off by one, per change: each red', () => {
    const red = (fill, step, fall) => cases('held').reduce((n, c) => {
      const fills = c.peak.map((p) => fill(p));
      return n + compare(markers(fills, c.dt, step, fall), c.marker, { abs: 1e-14 }).failures.length;
    }, 0);
    assert.ok(red((p) => meterFill(p, -54), meterMarker, FALL) > 54);
    assert.ok(red((p) => meterFill(p), meterMarker, FALL + 1) > 54);
    assert.ok(red((p) => meterFill(p), perChange, FALL) > 54);
  });
});
