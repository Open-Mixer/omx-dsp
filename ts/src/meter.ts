// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * The meter view law, the twin of mix/omx_meter_view.h: how a measured value is drawn, the same on every surface.
 * The fill of a dB scale from the floor to 0 dBFS, the peak marker that takes a louder value at once and falls by
 * elapsed time (the same seconds give the same fall at any draw rate, and it falls on silence), the clip word that
 * stays lit until cleared, and the stepped count of an LED ladder with its hysteresis (a bar has no release). Only
 * what a surface of the console draws with is kept in TypeScript; the clip word, the decimation, the unit and the
 * gain-reduction reading are the kernel's alone, with no twin. The numbers are METER_VIEW, declared in
 * data/primitives/meter.json. Pinned by
 * test/meter.test.mjs: against the kernel through the oracle at the nine oracle rates, and against
 * vectors/meter-view.json, the file every copy of the law reads.
 */
import { METER_VIEW } from './definitions.js';

/** `x` held to 0..1; NaN reads 0. */
const unit = (x: number): number => (x > 0 ? (x < 1 ? x : 1) : 0);

/** The fill of a dB level, linear in dB from `floorDb` (0) to 0 dBFS (1); NaN and -Infinity read 0. Twin of omx_meter_view_fill_db. */
export function meterFillDb(db: number, floorDb: number = METER_VIEW.floorDb): number {
  if (!(db > floorDb)) return 0;
  if (db >= 0) return 1;
  return (db - floorDb) / -floorDb;
}

/** The fill of a linear peak (1 is 0 dBFS) on the same dB scale. Twin of omx_meter_view_fill. */
export function meterFill(level: number, floorDb: number = METER_VIEW.floorDb): number {
  if (!(level > 0)) return 0;
  if (level >= 1) return 1;
  return meterFillDb(20 * Math.log10(level), floorDb);
}

/** One meter's peak marker between draw ticks, the state of struct omx_meter_view. */
export interface MeterMarker {
  /** The fill the marker last took. */
  readonly peak: number;
  /** Seconds of draw ticks since it took it. */
  readonly elapsedS: number;
  /** The live fill of the last tick. */
  readonly live: number;
  /** The marker drawn at the last tick. */
  readonly marker: number;
}

/** A fresh marker: empty. */
export const METER_MARKER_EMPTY: MeterMarker = { peak: 0, elapsedS: 0, live: 0, marker: 0 };

/**
 * One draw tick of the peak marker: a fill at or above it is taken at once; below it the marker falls
 * `fallDbPerSecond` for each second elapsed since it took its value, never under the live fill. `dtS` is the time
 * since the last tick (negative or NaN counts as 0). Twin of omx_meter_view_step.
 */
export function meterMarker(prev: MeterMarker | undefined, fill: number, dtS: number, fallDbPerSecond: number = METER_VIEW.fallDbPerSecond, floorDb: number = METER_VIEW.floorDb): MeterMarker {
  const p = prev ?? METER_MARKER_EMPTY;
  const f = unit(fill);
  const elapsedS = p.elapsedS + (dtS > 0 ? dtS : 0);
  const m = p.peak - (fallDbPerSecond * elapsedS) / -floorDb;
  if (!(m > f)) return { peak: f, elapsedS: 0, live: f, marker: f };
  return { peak: p.peak, elapsedS, live: f, marker: m };
}

/**
 * A stepped display's count (an LED ladder of `steps` segments) with hysteresis against the count it showed last: it
 * holds while the unrounded value stays within `0.5 + hysteresis` steps of it. `prevStep` undefined (or negative) is a
 * fresh or reset strip: plain rounding. Twin of omx_meter_view_ladder.
 */
export function meterStep(prevStep: number | undefined, fill: number, steps: number, hysteresis: number = METER_VIEW.stepHysteresis): number {
  const raw = unit(fill) * steps;
  if (prevStep !== undefined && prevStep >= 0 && Math.abs(raw - prevStep) <= 0.5 + hysteresis) return prevStep;
  return Math.floor(raw + 0.5);
}

/** A stepped BAR for one tick: a linear peak to its fill, then its count against the last one. No release. Twin of omx_meter_view_bar_step. */
export function meterBarStep(prevStep: number | undefined, level: number, steps: number, hysteresis: number = METER_VIEW.stepHysteresis): number {
  return meterStep(prevStep, meterFill(level), steps, hysteresis);
}
