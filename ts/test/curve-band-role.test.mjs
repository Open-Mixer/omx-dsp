// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * The band roles are told from the others by one declared prefix (CURVE_ROLE_PREFIXES, data/primitives/curve.json,
 * #40), never by a `band_` literal of the consumer's: exactly the five indexed band roles carry it, and
 * curveRoleIsBand answers on the role word as omx_curve_role_is_band() does in C. A renamed prefix or role goes red.
 */
import { describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { CURVE_ROLE_PREFIXES, CURVE_WORDS, curveRoleIsBand } from '../dist/index.js';

describe('the band role prefix', () => {
  it('is carried by exactly the indexed band roles, in their order', () => {
    const band = CURVE_WORDS.roles.filter((w) => w.startsWith(CURVE_ROLE_PREFIXES.band));
    assert.deepEqual(band, ['band_type', 'band_freq', 'band_gain', 'band_q', 'band_on']);
    assert.deepEqual(CURVE_WORDS.roles.slice(0, band.length), band);
  });
  it('is what curveRoleIsBand reads', () => {
    for (const w of CURVE_WORDS.roles) assert.equal(curveRoleIsBand(w), w.startsWith(CURVE_ROLE_PREFIXES.band), w);
    assert.equal(curveRoleIsBand('band'), false);
    assert.equal(curveRoleIsBand('hpf_freq'), false);
  });
});
