// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * An item whose name says it is a rate in dB per second, or a slope in dB per octave, states that unit's PARAM_UNITS
 * id (C2, omx-contract docs/units.md: `dB/s` is `dBPerS`, `dB/oct` is `dBPerOct`), in data/ and in recipes/answers/.
 *
 * Before: the leveler's and the balancer's `speedDbPerS` travelled in `dB`, a level, so the one owner of a gain
 * change per second never used `dBPerS` and omx-contract's live-owner proof read the unit as used by nothing
 * (chain 5, sort-2026-11-24-c5a, line 73: `a unit nothing uses is in the enum`, actual ['dBPerS']).
 */
import assert from 'node:assert/strict';
import { readdirSync, readFileSync } from 'node:fs';
import { join } from 'node:path';
import { test } from 'node:test';
import { HERE } from './helpers.mjs';

/** The name's spelling of the rate -> the unit id it measures in. */
const RATES = [
  [/DbPerS$|_DB_PER_S(_|$)/, 'dBPerS'],
  [/Db(Per)?Oct$|_DB_(PER_)?OCT(_|$)/, 'dBPerOct'],
];

const files = ['data/kernels', 'data/primitives', 'recipes/answers'].flatMap((d) =>
  readdirSync(join(HERE, d)).filter((f) => f.endsWith('.json')).map((f) => `${d}/${f}`));

/** Every [file, name, unit] where a named item, or a keyed table, carries a unit. */
const units = () => {
  const out = [];
  const walk = (o, rel) => {
    if (Array.isArray(o)) return o.forEach((x) => walk(x, rel));
    if (!o || typeof o !== 'object') return;
    if (typeof o.name === 'string') {
      const body = [o.travel, o.value, o].find((b) => b && typeof b === 'object' && typeof b.unit === 'string');
      if (body) out.push([rel, o.name, body.unit]);
    }
    for (const [k, v] of Object.entries(o)) {
      if (v && typeof v === 'object' && !Array.isArray(v) && typeof v.unit === 'string' && typeof v.name !== 'string') out.push([rel, k, v.unit]);
      walk(v, rel);
    }
  };
  for (const rel of files) walk(JSON.parse(readFileSync(join(HERE, rel), 'utf8')), rel);
  return out;
};

test('an item named for a rate in dB per second or a slope in dB per octave states that unit id', () => {
  const named = units().filter(([, name]) => RATES.some(([re]) => re.test(name)));
  // Positive control: the leveler's and the balancer's speed, in data/ and in recipes/answers/, and the balancer's slope.
  for (const want of ['data/kernels/loudlevel.json speedDbPerS', 'data/kernels/balancer.json speedDbPerS', 'recipes/answers/loudlevel.json LOUDLEVEL_SPEED_DB_PER_S_RANGE', 'data/kernels/balancer.json slopeDbOct']) {
    assert.ok(named.some(([rel, name]) => `${rel} ${name}` === want), `the probe sees ${want}`);
  }
  const wrong = named.filter(([, name, unit]) => unit !== RATES.find(([re]) => re.test(name))[1]).map(([rel, name, unit]) => `${rel} ${name}: ${unit}`);
  assert.deepEqual(wrong, [], 'a rate item states another unit than the one its name measures');
});
