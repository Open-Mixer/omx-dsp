// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/** The twins of omx_rta.h omx_rta_fft_size_for and omx_rta_bands.h omx_rta_bands, moved verbatim from the console (its spectrum bands); every limit is omx-dsp's own (data/kernels/rta.json). */

import {
  RTA_TARGET_BIN_HZ,
  RTA_FFT_SIZE_MAX,
  RTA_BAND_F_MIN,
  RTA_BAND_F_MAX_CAP,
  RTA_TIER_CROSSOVER_HZ,
  RTA_BAND_GATE_DB,
} from './definitions.js';
import { linearGainToDb } from './units.js';

/**
 * The RTA FFT length for `rate` — the TypeScript mirror of `mix_dsp.h`'s `omx_rta_fft_size_for`:
 * the smallest power of two whose bins are at or under {@link RTA_TARGET_BIN_HZ} (44.1/48 kHz →
 * 8192, 88.2/96 kHz → 16384, 176.4/192 kHz → 32768). Window TIME stays ~171 ms at every rate —
 * pinning the length instead of deriving it was how moving the rig to 96 kHz silently halved the
 * resolution (2026-09-17 declare-derive audit, F2). Every consumer of an offline/synth or a
 * server RTA frame derives its window from the graph rate through this function rather than
 * declaring one of its own.
 */
export function fftSizeForRate(rate: number): number {
  let n = 2048;
  while (n < RTA_FFT_SIZE_MAX && rate / n > RTA_TARGET_BIN_HZ) n *= 2;
  return n;
}

/**
 * A magnitude series as a consumer receives it: one `Float32Array` from a native reader, or a plain
 * array from the wire, a test or an offline synth. Every reader indexes and measures length only.
 */
export type MagnitudeSeries = readonly number[] | Float32Array;

/** One tier of a reading: the linear magnitudes (bin 0 = DC) and the transform that produced them. */
export interface SpectrumTierInput {
  readonly mags: MagnitudeSeries;
  readonly sampleRate: number;
  readonly fftSize: number;
}

/** One log-spaced band: its geometric centre (Hz), the RMS across its bins and its loudest bin (dBFS). */
export interface SpectrumBand {
  readonly hz: number;
  readonly db: number;
  readonly peakDb: number;
}

/** What {@link aggregateSpectrumBands} may be asked to vary; everything else is declared in rta.json. */
export interface SpectrumBandOptions {
  /** Lowest band edge, Hz. Defaults to RTA_BAND_F_MIN. */
  readonly fMin?: number;
  /** Highest band edge, Hz. Defaults to {@link bandFMax} of the high tier's rate. */
  readonly fMax?: number;
  /** The decimated tier, where the reading carries one. Bands centred under the crossover sample it. */
  readonly low?: SpectrumTierInput;
}

/** The (fractional) bin holding `hz` in a transform of `fftSize` at `sampleRate`. */
export function hzToBin(hz: number, sampleRate: number, fftSize: number): number {
  return (hz * fftSize) / sampleRate;
}

/** The twin of omx_rta_band_f_max: RTA_BAND_F_MAX_CAP, or the Nyquist where that is lower. */
export function bandFMax(sampleRate: number): number {
  return Math.min(RTA_BAND_F_MAX_CAP, sampleRate / 2);
}

/**
 * The twin of omx_rta_low_gain_db: the dB that level-matches a low-tier band to the high tier. A band's
 * value is an RMS across its bins and the low tier packs more bins into the same Hz, so it reads lower by
 * the bin-density ratio; a degenerate tier declines to correct rather than emit a NaN.
 */
export function lowTierGainDb(high: SpectrumTierInput, low: SpectrumTierInput): number {
  const ratio = low.fftSize / low.sampleRate / (high.fftSize / high.sampleRate);
  if (!Number.isFinite(ratio) || ratio <= 0) return 0;
  return 10 * Math.log10(ratio);
}

/** RMS and peak (linear) of `tier` over the bins covering [f0, f1); a band spanning none samples the nearest. DC never. */
function tierBandStats(tier: SpectrumTierInput, f0: number, f1: number): { rms: number; peak: number } {
  const bins = tier.mags.length;
  let i0 = Math.max(1, Math.floor(hzToBin(f0, tier.sampleRate, tier.fftSize)));
  const i1 = Math.min(bins - 1, Math.ceil(hzToBin(f1, tier.sampleRate, tier.fftSize)));
  if (i1 < i0) i0 = i1;
  let sumSq = 0;
  let peak = 0;
  let n = 0;
  for (let i = i0; i <= i1; i++) {
    const m = tier.mags[i] ?? 0;
    sumSq += m * m;
    if (m > peak) peak = m;
    n++;
  }
  return { rms: n > 0 ? Math.sqrt(sumSq / n) : 0, peak };
}

/**
 * The band tiling: `nBands` log-even bands from `fMin` to `fMax`, each at its GEOMETRIC centre, by the
 * expressions {@link aggregateSpectrumBands} tiles with, so a client can regenerate the centres of a banded
 * frame without the frame carrying them.
 */
export function spectrumBandCentres(
  nBands: number,
  sampleRate: number,
  options: Pick<SpectrumBandOptions, 'fMin' | 'fMax'> = {},
): number[] {
  if (!Number.isFinite(nBands) || nBands < 1) return [];
  const fMinV = options.fMin ?? RTA_BAND_F_MIN;
  const fMaxV = Math.max(options.fMax ?? bandFMax(sampleRate), fMinV * 2);
  const lo = Math.log10(fMinV);
  const hi = Math.log10(fMaxV);
  const centres = new Array<number>(nBands);
  for (let b = 0; b < nBands; b++) {
    const f0 = 10 ** (lo + ((hi - lo) * b) / nBands);
    const f1 = 10 ** (lo + ((hi - lo) * (b + 1)) / nBands);
    centres[b] = Math.sqrt(f0 * f1);
  }
  return centres;
}

/**
 * The twin of omx_rta_bands (analysis/omx_rta_bands.h), moved from the console unchanged: reduce `high` (and
 * `low`, for a band centred under RTA_TIER_CROSSOVER_HZ) to `nBands` log-spaced bands. `db` is the RMS across
 * a band's bins, `peakDb` its loudest bin, never tier-corrected; when no band rises above RTA_BAND_GATE_DB
 * every band reads as silence (-Infinity). `nBands` below 1 answers an empty set. Held to the kernel by
 * vectors/rta-bands.json (test/rta-bands.test.mjs).
 */
export function aggregateSpectrumBands(
  high: SpectrumTierInput,
  nBands: number,
  options: SpectrumBandOptions = {},
): SpectrumBand[] {
  if (!Number.isFinite(nBands) || nBands < 1) return [];
  const low = options.low;
  const fMinV = options.fMin ?? RTA_BAND_F_MIN;
  const fMaxV = Math.max(options.fMax ?? bandFMax(high.sampleRate), fMinV * 2);
  const lo = Math.log10(fMinV);
  const hi = Math.log10(fMaxV);
  const lowGainDb = low ? lowTierGainDb(high, low) : 0;
  const bands = new Array<SpectrumBand>(nBands);
  let maxDb = -Infinity;
  for (let b = 0; b < nBands; b++) {
    const f0 = 10 ** (lo + ((hi - lo) * b) / nBands);
    const f1 = 10 ** (lo + ((hi - lo) * (b + 1)) / nBands);
    const centre = Math.sqrt(f0 * f1);
    const tier = low && centre < RTA_TIER_CROSSOVER_HZ ? low : high;
    const { rms, peak } = tierBandStats(tier, f0, f1);
    const db = linearGainToDb(rms) + (tier === low ? lowGainDb : 0);
    if (db > maxDb) maxDb = db;
    bands[b] = { hz: centre, db, peakDb: linearGainToDb(peak) };
  }
  if (maxDb < RTA_BAND_GATE_DB) {
    const silence = linearGainToDb(0);
    return bands.map((x) => ({ hz: x.hz, db: silence, peakDb: silence }));
  }
  return bands;
}
