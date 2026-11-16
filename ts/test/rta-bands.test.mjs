// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * THE BAND REDUCTION TWIN AGREES WITH ITS KERNEL: aggregateSpectrumBands (dist/, the published code) against
 * vectors/rta-bands.json, which tools/rta-bands-golden.c writes from omx_rta_bands (analysis/omx_rta_bands.h) and the
 * suite's rta_bands arm reads back. Each case's spectrum is built again here exactly as
 * test/kernels/fixtures/rta_bands_scenarios.h builds it (float arithmetic: a Float32Array rounds every store), and
 * every band must agree within 0.01 dB, silence with silence. A twin whose sabotage stays green would be decoration:
 * four copies of the built twin, each with one rule broken, must each disagree with the file.
 */
import { describe, it, after } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, writeFileSync, rmSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import {
  RTA_BAND_F_MAX_CAP,
  RTA_BAND_F_MIN,
  RTA_BAND_GATE_DB,
  RTA_TIER_CROSSOVER_HZ,
  aggregateSpectrumBands,
  bandFMax,
  fftSizeForRate,
  lowTierGainDb,
  spectrumBandCentres,
} from '../dist/index.js';

const HERE = dirname(fileURLToPath(import.meta.url));
const VECTORS = JSON.parse(readFileSync(join(HERE, '..', '..', 'vectors', 'rta-bands.json'), 'utf8'));
const DB_TOL = 0.01;
/** omx_rta.h's low zoom: OMX_RTA_LOW_FFT_SIZE points at the rate over OMX_RTA_LOW_DECIM_FACTOR. A wrong copy here
 * reads the tiers cases at the wrong bins and the file turns this test red. */
const LOW_FFT_SIZE = 8192;
const LOW_DECIM = 8;
const f = Math.fround;

function tone(m, binHz, hz, amp) {
  const b = Math.round(f(hz / binHz));
  if (b < 1 || b + 1 >= m.length) return;
  m[b] += amp;
  m[b - 1] += f(amp * 0.5);
  m[b + 1] += f(amp * 0.5);
}

function slope(m) {
  m[0] = 0;
  for (let k = 1; k < m.length; k++) m[k] = f(f(0.1) / f(Math.sqrt(k)));
}

/** The scenario `kind` at `rate`: the high tier and, for tiers, the low one. */
function build(kind, rate) {
  const fft = fftSizeForRate(rate);
  const high = new Float32Array(fft / 2);
  const binHz = f(rate / fft);
  const lrate = f(rate / LOW_DECIM);
  const low = new Float32Array(LOW_FFT_SIZE / 2);
  const lbinHz = f(lrate / LOW_FFT_SIZE);
  let hasLow = false;
  if (kind === 'slope') slope(high);
  else if (kind === 'tones') {
    tone(high, binHz, 40, f(0.5));
    tone(high, binHz, 1000, 1);
    tone(high, binHz, 12000, f(0.25));
  } else if (kind === 'tiers') {
    slope(high);
    tone(high, binHz, 50, 1);
    slope(low);
    tone(low, lbinHz, 50, 1);
    hasLow = true;
  } else if (kind === 'quiet') high.fill(f(0.0002));
  else if (kind !== 'silence') throw new Error(`unknown scenario ${kind}`);
  return {
    high: { mags: high, sampleRate: rate, fftSize: fft },
    low: hasLow ? { mags: low, sampleRate: lrate, fftSize: LOW_FFT_SIZE } : undefined,
  };
}

const silent = (x) => x === null || x === -Infinity;
const sameDb = (got, want) => (silent(want) ? got === -Infinity : Math.abs(got - want) <= DB_TOL);

/** Every disagreement of `reduce` with the file, as text. */
function disagreements(reduce) {
  const out = [];
  for (const c of VECTORS.cases) {
    const { high, low } = build(c.kind, c.rate);
    const bands = reduce(high, c.n, low ? { low } : {});
    if (bands.length !== c.n) {
      out.push(`${c.kind}@${c.rate}/${c.n}: ${bands.length} bands`);
      continue;
    }
    for (let b = 0; b < c.n; b++) {
      const x = bands[b];
      if (Math.abs(x.hz - c.hz[b]) > 1e-9 * c.hz[b] || !sameDb(x.db, c.db[b]) || !sameDb(x.peakDb, c.peak[b])) {
        out.push(`${c.kind}@${c.rate}/${c.n} band ${b}: ${x.hz} ${x.db} ${x.peakDb} vs ${c.hz[b]} ${c.db[b]} ${c.peak[b]}`);
        break;
      }
    }
  }
  return out;
}

describe('the band reduction twin', () => {
  it('reads the declared numbers the file was written with', () => {
    assert.deepEqual(VECTORS.law, {
      fMin: RTA_BAND_F_MIN,
      fMaxCap: RTA_BAND_F_MAX_CAP,
      crossoverHz: RTA_TIER_CROSSOVER_HZ,
      gateDb: RTA_BAND_GATE_DB,
    });
  });

  it('the file holds every scenario at 48 and 96 kHz and 1, 64 and 640 bands', () => {
    assert.equal(VECTORS.cases.length, 30);
    for (const kind of ['slope', 'tones', 'tiers', 'quiet', 'silence'])
      for (const rate of [48000, 96000])
        for (const n of [1, 64, 640])
          assert.ok(VECTORS.cases.some((c) => c.kind === kind && c.rate === rate && c.n === n), `${kind}@${rate}/${n}`);
  });

  it('agrees with omx_rta_bands on every case of the file, within 0.01 dB', () => {
    assert.deepEqual(disagreements(aggregateSpectrumBands), []);
  });

  it('tiles at the centres spectrumBandCentres regenerates, and clamps the cap to the Nyquist', () => {
    for (const c of VECTORS.cases) {
      const centres = spectrumBandCentres(c.n, c.rate);
      assert.equal(centres.length, c.n);
      centres.forEach((hz, b) => assert.ok(Math.abs(hz - c.hz[b]) <= 1e-9 * c.hz[b], `${c.kind}@${c.rate}/${c.n} band ${b}`));
    }
    assert.equal(bandFMax(48000), RTA_BAND_F_MAX_CAP);
    assert.equal(bandFMax(32000), 16000);
  });

  it('level-matches the low tier by its bin-density ratio, and declines on a degenerate tier', () => {
    const { high, low } = build('tiers', 48000);
    assert.ok(Math.abs(lowTierGainDb(high, low) - 10 * Math.log10(8)) < 1e-12);
    assert.equal(lowTierGainDb(high, { ...low, sampleRate: 0 }), 0);
  });

  it('answers no band for no band asked', () => {
    assert.deepEqual(aggregateSpectrumBands(build('slope', 48000).high, 0), []);
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
  'level as the loudest bin': ['rms: n > 0 ? Math.sqrt(sumSq / n) : 0', 'rms: n > 0 ? peak : 0'],
  'arithmetic centre': ['const centre = Math.sqrt(f0 * f1);', 'const centre = (f0 + f1) / 2;'],
  'no low-tier gain': ['(tier === low ? lowGainDb : 0)', '0'],
  'gate at -80 dB': ['if (maxDb < RTA_BAND_GATE_DB)', 'if (maxDb < -80)'],
};

describe('the twin test is not blind', () => {
  for (const [name, [from, to]] of Object.entries(SABOTAGES))
    it(`goes red with ${name}`, async () => {
      assert.equal(SOURCE.split(from).length, 2, `the ${name} sabotage targets one line of dist/rta.js`);
      const p = join(DIST, `rta.sabotage-${process.pid}-${made.length}.js`);
      made.push(p);
      writeFileSync(p, SOURCE.replace(from, to));
      const broken = await import(pathToFileURL(p).href);
      assert.ok(disagreements(broken.aggregateSpectrumBands).length > 0, `the twin with ${name} still agrees with the file`);
    });
});
