// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * A TWIN NOBODY CONSUMES IS NOT KEPT: the kernels below have a C implementation, a golden or oracle that pins it, and
 * no importer in any Open-Mixer repository (git grep of every checkout's origin/main, 2026-11-19), so a second
 * implementation in TypeScript is a copy that drifts. The package does not export them and its source holds no file
 * for them. The twins the console's own tests still import stay listed as KEPT, so deleting one is a decision.
 */
import { describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { existsSync } from 'node:fs';
import * as dsp from '../dist/index.js';

const GONE = [
  'hrpPlaceBands', 'hrpPlaceInRange', 'hrpStepToward',
  'fbsCents', 'fbsGainCap', 'FbsPlace', 'sameRingFreq', 'FBS_ACTIONS', 'FBS_HOLD_NEVER', 'FBS_PLACE_SLOTS',
  'gaincompDbShifted', 'gateKnee',
  'meterClip', 'meterDecimate', 'meterUnitOf', 'meterGrStep', 'meterGrView', 'METER_GR_EMPTY', 'METER_GR_HOLD_MS', 'METER_WORDS',
  'matchedPair', 'sectionMagnitudeDb', 'octavesToQ',
];
const KEPT = ['gaincompDb', 'lowpassHandover', 'meterFill', 'meterFillDb', 'meterMarker', 'meterStep', 'meterBarStep', 'METER_MARKER_EMPTY'];

describe('the package exports no twin that nothing consumes', () => {
  for (const name of GONE) {
    it(`${name} is not exported`, () => {
      assert.equal(name in dsp, false);
    });
  }
  for (const name of KEPT) {
    it(`${name} is still exported: the console's tests import it`, () => {
      assert.equal(typeof dsp[name] !== 'undefined', true);
    });
  }
  it('the source holds no file for the placement twins', () => {
    for (const f of ['hrp-place.ts', 'fbs-place.ts']) assert.equal(existsSync(new URL(`../src/${f}`, import.meta.url)), false, f);
  });
});
