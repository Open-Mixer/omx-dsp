// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * The EQ section design's shared data (#86): the biquad kinds, omx-contract's PARAM_FILTER_RESPONSES (#170: omx-dsp
 * types them nowhere), and the low-pass hand-over table LOWPASS_HANDOVER (eq.json). omx-dsp holds both today in include/omxdsp/omx_eq_design.h
 * (enum omx_eq_kind, :24-33, and OMX_EQ_LOWPASS_HANDOVER_*, :44-60) and the console engine's core holds the same
 * in eq.ts (BiquadKind, :425-434, and the hand-over constants and band ends, :496-511). The values below are the
 * ones both sides hold; a drift in the data turns these red, and a moved value moves every render and turns
 * `render --check` red.
 */
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { readFileSync } from 'node:fs';
import { join } from 'node:path';
import { OC, loadData, resolveData } from './oc.mjs';
import { renderC } from './oc.mjs';
import { renderJson } from './oc.mjs';
import { UNION, withoutMemberDocs } from './helpers.mjs';

const json = (dir = UNION) => JSON.parse(renderJson(resolveData(loadData(dir)), '0.0.0')[0].text);
const header = (dir = UNION) => withoutMemberDocs(renderC(resolveData(loadData(dir)))[0].text);

// The nine kinds in the order enum omx_eq_kind numbers them, peaking 0 through allpass2 8: core's BiquadKind order
// (eq.ts), held here as written so a reorder or an insertion in omx-contract's item turns red here first. The enum
// takes each code from the contract's X-macro, so nothing else in this tree would notice a renumbering.
const KINDS = ['peaking', 'lowShelf', 'highShelf', 'highpass', 'lowpass', 'notch', 'bandpass', 'allpass1', 'allpass2'];
/** The codes a words item gives, word by word: what enum omx_eq_kind becomes when it reads that item. */
const codesOf = (item) => Object.fromEntries((item?.oneOf ?? []).map((w) => [w.const, w['x-code']]));
/** The codes this library was built on: each kind its place in KINDS. */
const PINNED = Object.fromEntries(KINDS.map((k, i) => [k, i]));
// omx-contract's PARAM_FILTER_RESPONSES, the item enum omx_eq_kind reads its codes from
const RESPONSES = JSON.parse(readFileSync(join(OC, 'contract.d/param/param.json'), 'utf8')).components.schemas.PARAM_FILTER_RESPONSES;
// omx-dsp's OMX_EQ_LOWPASS_HANDOVER_* as written there, parsed as doubles.
const HANDOVER = {
  qHi: 0.70710678118654752, qLo: 0.54119610014619698, qPeak: 0.9,
  loAtHiQ: 0.6, hiAtHiQ: 0.85, loAtLoQ: 0.8, hiAtLoQ: 0.9, hiAtPeakQ: 0.8,
};

test('the biquad kinds are omx-contract\'s PARAM_FILTER_RESPONSES, each code its place, and no render of omx-dsp holds them again', () => {
  assert.ok(Array.isArray(RESPONSES?.oneOf) && RESPONSES.oneOf.length > 0, `${OC} has the PARAM_FILTER_RESPONSES words item`);
  RESPONSES.oneOf.forEach((w, i) => assert.equal(w['x-code'], i, `${w.const}: code ${w['x-code']}, place ${i}`));
  const words = RESPONSES.oneOf.map((w) => w.const);
  for (const [name, it] of Object.entries(json().items)) {
    const ids = Array.isArray(it.value) ? it.value : [];
    assert.ok(!words.every((w) => ids.includes(w)), `${name} holds the words of PARAM_FILTER_RESPONSES again`);
  }
  assert.doesNotMatch(header(), /OMX_BIQUAD_KINDS_/);
});

test('the kinds keep their codes: peaking 0 through allpass2 8, in the order omx-dsp has always numbered them', () => {
  assert.deepEqual(RESPONSES.oneOf.map((w) => w.const), KINDS);
  assert.deepEqual(codesOf(RESPONSES), PINNED);
  assert.equal(codesOf(RESPONSES).peaking, 0);
  assert.equal(codesOf(RESPONSES).allpass2, 8);
});

test('the pin refuses a moved item: two words swapped, a word inserted, a word dropped, each renumbered as the contract would', () => {
  const renumbered = (words) => ({ oneOf: words.map((w, i) => ({ const: w, 'x-code': i })) });
  const swapped = KINDS.slice(); [swapped[0], swapped[1]] = [swapped[1], swapped[0]];
  const inserted = [...KINDS.slice(0, 5), 'tilt', ...KINDS.slice(5)];
  const dropped = KINDS.filter((k) => k !== 'notch');
  for (const [what, words] of [['swapped', swapped], ['inserted', inserted], ['dropped', dropped]]) {
    // each sabotaged item is one the place check above accepts: its codes are its places
    renumbered(words).oneOf.forEach((w, i) => assert.equal(w['x-code'], i));
    assert.throws(() => assert.deepEqual(codesOf(renumbered(words)), PINNED), assert.AssertionError, `${what}: the pin did not refuse it`);
  }
  // a word appended after allpass2 moves no code of the nine, and is refused all the same: the enum has no kind for it
  assert.throws(() => assert.deepEqual(codesOf(renumbered([...KINDS, 'tilt'])), PINNED), assert.AssertionError);
});

test('the hand-over table holds omx-dsp\'s eight numbers, the high Q end being the Butterworth Q', () => {
  const j = json();
  assert.deepEqual(j.items.LOWPASS_HANDOVER.value, HANDOVER);
  assert.equal(j.items.LOWPASS_HANDOVER.value.qHi, j.items.BUTTERWORTH_Q.value);
  assert.match(header(), /^#define OMX_LOWPASS_HANDOVER_Q_LO_DOUBLE 0\.541196100146197$/m);
  assert.match(header(), /^#define OMX_LOWPASS_HANDOVER_HI_AT_PEAK_Q_DOUBLE 0\.8$/m);
});

test('the renders never spell the OMX_EQ_* names omx-dsp defines today', () => {
  assert.doesNotMatch(header(), /OMX_EQ_LOWPASS_HANDOVER_|OMX_EQ_(PEAKING|LOWSHELF|HIGHSHELF|HIGHPASS|LOWPASS|NOTCH|BANDPASS|ALLPASS1|ALLPASS2)\b/);
});
