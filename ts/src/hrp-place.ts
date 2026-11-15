// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * The twin of omx_hrp_place.h (src/omx_hrp_place.c): the harmonic resonance processor's placement and stepping.
 * The C placement and this one read the same vector file (test/analysis/fixtures/hrp_place.vectors) in
 * test/analysis/hrp_place.test.c and test/hrp-place.test.mjs, so the two are held to the same values: exact
 * everywhere but a Q clamped onto the bell's lower bound, which the contract renders as 0.3f into C.
 *
 * NOT YET: places nothing and jumps to the target, so the twin's test reads the shared vectors red first.
 */

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

/** The bands one voice's per-harmonic gains become (`gainsDb[k]` is harmonic k+1), in harmonic order. */
export function hrpPlaceBands(
  f0Hz: number,
  gainsDb: readonly number[],
  q: number,
  limits: HrpPlaceLimits,
): HrpPlacedBand[] {
  void f0Hz, gainsDb, q, limits;
  return [];
}

/** Is a band at freqHz placeable (inside EQ_FREQ_RANGE, both bounds kept)? A NaN is not. */
export function hrpPlaceInRange(freqHz: number): boolean {
  void freqHz;
  return true;
}

/** One bounded move of `current` toward `target`, never further than HRP_GAIN_STEP_DB. */
export function hrpStepToward(current: number, target: number): number {
  void current;
  return target;
}
