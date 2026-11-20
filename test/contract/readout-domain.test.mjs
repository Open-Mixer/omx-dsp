// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * Sorting pass, row V22: READOUT_RATE_HZ, the rate the kernels publish a readout at (50 Hz, the rate they decimate to),
 * is the `readout` domain of this library (contract.d/readout/readout.json), named after its item's prefix, no longer an
 * item of omx-contract's data/primitives.json. The read is omx-contract's domain reader (Domain.scalar) over this
 * tree's contract.d; a temporary directory decides the answer: another value is read, no item is refused by name.
 */
import assert from 'node:assert/strict';
import { cpSync, readFileSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';
import { test } from 'node:test';
import { pathToFileURL } from 'node:url';
import { mkdtempSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

// no helpers.mjs: its union reads every kernel, whose refs to this item resolve only where it is declared
const HERE = join(dirname(fileURLToPath(import.meta.url)), '../..');
const OC = process.env.OMX_CONTRACT;
const DATA = join(OC, 'data');
const { openDomainAt } = await import(pathToFileURL(join(OC, 'lib/domain-reader.mjs')).href);
const GOLDEN = { value: 50, unit: 'Hz' };
const doc = () => JSON.parse(readFileSync(join(HERE, 'contract.d/readout/readout.json'), 'utf8'));

test('the readout domain opens from this library and READOUT_RATE_HZ is 50 Hz (golden, frozen from omx-contract data/primitives.json)', () => {
  assert.deepEqual({ ...openDomainAt(join(HERE, 'contract.d'), 'readout').scalar('READOUT_RATE_HZ') }, GOLDEN);
});

test('the domain is this library\'s, named after its item\'s prefix, and holds that one item', () => {
  const d = doc();
  assert.equal(d.$id, 'https://openmixer.org/contract/readout');
  assert.equal(d.info['x-owner'], 'omx-dsp');
  assert.deepEqual(Object.keys(d.components.schemas), ['READOUT_RATE_HZ']);
});

test('omx-contract no longer holds READOUT_RATE_HZ (the old home)', () => {
  const prim = JSON.parse(readFileSync(join(DATA, 'primitives.json'), 'utf8'));
  assert.equal('READOUT_RATE_HZ' in prim, false, 'omx-contract data/primitives.json still holds it (build the chain from its sorting-pass branch)');
});

test('sabotage: a temporary directory with another value gives that value, and one without the item is refused by name', (t) => {
  const dir = mkdtempSync(join(tmpdir(), 'omx-dsp-readout-'));
  t.after(() => rmSync(dir, { recursive: true, force: true }));
  cpSync(join(HERE, 'contract.d/readout'), join(dir, 'readout'), { recursive: true });
  const f = join(dir, 'readout/readout.json');
  const d = doc();
  d.components.schemas.READOUT_RATE_HZ['x-value'] = 25;
  writeFileSync(f, JSON.stringify(d, null, 2));
  assert.equal(openDomainAt(dir, 'readout').scalar('READOUT_RATE_HZ').value, 25);
  delete d.components.schemas.READOUT_RATE_HZ;
  d.components.schemas.READOUT_OTHER = { description: 'x', 'x-since': '1.0.0', 'x-kind': 'scalar', 'x-unit': '', 'x-value': 1 };
  writeFileSync(f, JSON.stringify(d, null, 2));
  assert.throws(() => openDomainAt(dir, 'readout').scalar('READOUT_RATE_HZ'), /READOUT_RATE_HZ/);
});
