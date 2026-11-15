// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * The twin of omx_hrp_place.h (src/omx_hrp_place.c): the harmonic resonance processor's placement and stepping.
 * The C placement and this one read the same vector file (test/analysis/fixtures/hrp_place.vectors) in
 * test/analysis/hrp_place.test.c and test/hrp-place.test.mjs, so the two are held to the same values, exactly.
 *
 * The rules are the header's, one function each, in the same order and arithmetic: the console's harmonicBands
 * and the HRP planter's stepToward, term for term.
 */
import { EQ_FREQ_RANGE, EQ_Q_RANGE, HRP_GAIN_STEP_DB } from './definitions.js';

/** The authority one placement is given, by the caller. */
export interface HrpPlaceLimits {
  /** The largest cut, a positive magnitude. */
  readonly maxCutDb: number;
  /** The largest boost, a positive magnitude (0: cuts only). */
  readonly maxBoostDb: number;
  /** How many bands may be placed (a free-slot count). */
  readonly maxBands: number;
}

/** One placed band: a bell at freqHz, gainDb (negative is a cut), q, for one harmonic (1 is the fundamental). */
export interface HrpPlacedBand {
  readonly harmonic: number;
  readonly freqHz: number;
  readonly gainDb: number;
  readonly q: number;
}

/** A number that may take part in the arithmetic. */
const usable = (v: number): boolean => Number.isFinite(v);

/** Below lo is lo, above hi is hi, anything else (a NaN too) passes through. */
const clamp = (v: number, lo: number, hi: number): number => (v < lo ? lo : v > hi ? hi : v);

/** REFUSED WHOLE: an input the placement cannot honour places nothing. */
function refused(f0Hz: number, q: number, l: HrpPlaceLimits): boolean {
  if (!usable(f0Hz) || f0Hz <= 0) return true;
  if (!usable(q)) return true;
  if (!usable(l.maxCutDb) || l.maxCutDb < 0) return true;
  if (!usable(l.maxBoostDb) || l.maxBoostDb < 0) return true;
  if (!usable(l.maxBands) || l.maxBands < 1) return true;
  return false;
}

/** The bands one voice's per-harmonic gains become (`gainsDb[k]` is harmonic k+1), in harmonic order. */
export function hrpPlaceBands(
  f0Hz: number,
  gainsDb: readonly number[],
  q: number,
  limits: HrpPlaceLimits,
): HrpPlacedBand[] {
  if (refused(f0Hz, q, limits)) return [];
  const bandQ = clamp(q, EQ_Q_RANGE.min, EQ_Q_RANGE.max);
  const placed: HrpPlacedBand[] = [];
  for (let k = 0; k < gainsDb.length; k++) {
    const requested = gainsDb[k]!;
    if (!usable(requested)) continue;
    const harmonic = k + 1;
    const freqHz = harmonic * f0Hz;
    if (!hrpPlaceInRange(freqHz)) continue;
    const gainDb = clamp(requested, -limits.maxCutDb, limits.maxBoostDb);
    if (gainDb === 0) continue; // SPARSE: -0 is the same fact
    placed.push({ harmonic, freqHz, gainDb, q: bandQ });
  }
  if (placed.length <= limits.maxBands) return placed;
  // THE CAP: the largest |gain| kept, ties to the lower harmonic, then back in harmonic order
  return [...placed]
    .sort((a, b) => Math.abs(b.gainDb) - Math.abs(a.gainDb) || a.harmonic - b.harmonic)
    .slice(0, limits.maxBands)
    .sort((a, b) => a.harmonic - b.harmonic);
}

/** Is a band at freqHz placeable (inside EQ_FREQ_RANGE, both bounds kept)? A NaN is not. */
export function hrpPlaceInRange(freqHz: number): boolean {
  return !(freqHz < EQ_FREQ_RANGE.min || freqHz > EQ_FREQ_RANGE.max || Number.isNaN(freqHz));
}

/** One bounded move of `current` toward `target`, never further than HRP_GAIN_STEP_DB. */
export function hrpStepToward(current: number, target: number): number {
  const delta = target - current;
  if (Math.abs(delta) <= HRP_GAIN_STEP_DB) return target;
  return current + Math.sign(delta) * HRP_GAIN_STEP_DB;
}
