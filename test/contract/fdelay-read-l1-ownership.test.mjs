// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * Sorting pass, row V30 (ruling R2: this library owns every kernel with a closed-form oracle and the primitives those
 * kernels call). omx-contract's data/ holds no kernel or primitive item any more (V22, V25 and V26 moved theirs); the
 * one piece of this library's it still computed was the fractional read's L1 norm: FDELAY_READ_L1_NORM was
 * `{ derive: lagrangeReadL1 }`, the Lagrange read's worst Σ|c| evaluated by omx-contract's code. The value is this
 * library's, a plain number held by its own kernel (test/kernels/fixtures/fdelay_read_l1.h, the suite's fdelay arm).
 * The golden is the value omx-contract's derivation gave, frozen from omx-contract at 3794409 (lagrangeReadL1(3)).
 *
 * The pins: no item of this library's data/ derives its value through a derivation that is not generic arithmetic,
 * no ref of its data/ lands on an omx-contract item that is not a standard word, and no C source of this library
 * names an omx-contract identifier outside those standard words.
 */
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';
import { cpSync, readFileSync, readdirSync, writeFileSync } from 'node:fs';
import { join, relative } from 'node:path';
import { test } from 'node:test';
import { OC, loadData } from './oc.mjs';
import { HERE, UNION, codeOnly, editData, relOf, scratchData, scratchDir } from './helpers.mjs';

const GOLDEN = JSON.parse(readFileSync(join(HERE, 'test/contract/golden/fdelay-read-l1.json'), 'utf8'));
const FILE = 'data/primitives/fdelay-read-l1-norm.json';

/** omx-contract's standard words this library may read (ruled standard: PARAM_* V21, the two rate lists V25 Q5). */
const STANDARD_ITEMS = ['STANDARD_SAMPLE_RATES', 'STANDARD_QUANTA', 'PARAM_SCALES', 'PARAM_UNITS', 'PARAM_UNIT_DIMENSIONS',
  'PARAM_UNIT_SCOPES', 'PARAM_UNITS_CLAP_ENCODED', 'PARAM_MEANINGS'];
/** Their C spellings: each item's OMX_ stem, and OMX_DECLARED_RATES, omx-contract's render of STANDARD_SAMPLE_RATES. */
const STANDARD_C = new RegExp(`^OMX_(${[...STANDARD_ITEMS, 'DECLARED_RATE'].join('|')})(_[A-Z0-9_]*)?S?$|_H$`);
/** The derivations that are arithmetic on this library's own values, not a kernel's closed form. */
const GENERIC_DERIVATIONS = ['count', 'sqrt', 'ceilSum'];

const walk = (o, fn) => {
  if (Array.isArray(o)) for (const x of o) walk(x, fn);
  else if (o && typeof o === 'object') { fn(o); for (const v of Object.values(o)) walk(v, fn); }
};
const dspFiles = (dataDir) => {
  const out = [];
  for (const sub of ['kernels', 'primitives']) {
    for (const f of readdirSync(join(dataDir, sub)).sort()) if (f.endsWith('.json')) out.push(`data/${sub}/${f}`);
  }
  return out;
};

/** Each derivation of this library's data/ that is not generic arithmetic, as `<file>: <derive>`. */
function kernelDerivations(dataDir) {
  const found = [];
  for (const rel of dspFiles(dataDir)) {
    walk(JSON.parse(readFileSync(join(dataDir, '..', rel), 'utf8')), (o) => {
      if (typeof o.derive === 'string' && !GENERIC_DERIVATIONS.includes(o.derive)) found.push(`${rel}: ${o.derive}`);
    });
  }
  return found;
}

/** Each ref of this library's data/ that lands on an item outside its own files and outside the standard words. */
function strayRefs(dataDir) {
  const mine = new Set(dspFiles(dataDir));
  const items = loadData(dataDir).items;
  const found = [];
  for (const rel of mine) {
    walk(JSON.parse(readFileSync(join(dataDir, '..', rel), 'utf8')), (o) => {
      if (typeof o.ref !== 'string') return;
      const name = o.ref.split('.')[0];
      const at = items.get(name)?.rel;
      if (at && !mine.has(at) && !STANDARD_ITEMS.includes(name)) found.push(`${rel}: ${o.ref} (${at})`);
    });
  }
  return found;
}

/** omx-contract's C identifiers (its installed headers, comments left out). */
const ocIdentifiers = () => {
  const dir = join(OC, 'include/omxcontract');
  const ids = new Set();
  for (const f of readdirSync(dir)) {
    if (f.endsWith('.h')) for (const m of codeOnly(readFileSync(join(dir, f), 'utf8')).matchAll(/\b(?:OMX|omx)_[A-Za-z0-9_]+\b/g)) ids.add(m[0]);
  }
  return ids;
};

/** This library's C sources (include/, src/, test/, tools/, fuzz/), as [rel, text]. */
const cSources = () => {
  const out = [];
  const visit = (dir) => {
    for (const e of readdirSync(dir, { withFileTypes: true })) {
      const p = join(dir, e.name);
      if (e.isDirectory()) visit(p);
      else if (/\.[ch]$/.test(e.name)) out.push([relative(HERE, p), readFileSync(p, 'utf8')]);
    }
  };
  for (const d of ['include', 'src', 'test', 'tools', 'fuzz']) visit(join(HERE, d));
  return out;
};

/** Each omx-contract identifier a C source names (code, not comments) that is not a standard word's. */
function strayIdentifiers(sources) {
  const oc = ocIdentifiers();
  const found = [];
  for (const [rel, text] of sources) {
    for (const m of codeOnly(text).matchAll(/\b(?:OMX|omx)_[A-Za-z0-9_]+\b/g)) {
      if (oc.has(m[0]) && !STANDARD_C.test(m[0]) && !/^omx_declared_rate/.test(m[0])) found.push(`${rel}: ${m[0]}`);
    }
  }
  return [...new Set(found)];
}

/**
 * The C read of this library's data (a data/ of kernels/ and primitives/): omxdsp_definitions.h rendered from it as
 * tools/definitions-render.sh renders the committed one (both packages installed into one contract directory, then
 * omx-contract's command renders omx-dsp's items), compiled with the kernel; the declared Λ and the kernel's own
 * worst L1 at the declared read order.
 */
function compiledRead(dspData) {
  const dir = scratchDir('omx-dsp-l1-');
  const ocVersion = JSON.parse(readFileSync(join(OC, 'package.json'), 'utf8')).version;
  const major = JSON.parse(readFileSync(join(OC, 'schema/omx-contract.schema.json'), 'utf8'))['x-omx-schema-version'].split('.')[0];
  const version = execFileSync('make', ['-s', '-C', HERE, 'version'], { encoding: 'utf8' }).trim();
  const install = (...a) => execFileSync('sh', [join(OC, 'tools/install-contract-dir.sh'), ...a], { stdio: ['ignore', 'ignore', 'inherit'] });
  install('-x', 'scale-vectors.json', 'omx-contract', ocVersion, major, join(OC, 'data'), join(dir, 'dir'), 'data');
  install('omx-dsp', version, major, dspData, join(dir, 'dir'), 'data');
  execFileSync('node', [join(OC, 'bin/omx-contract.mjs'), 'render', '--target', 'c', '--package', 'omx-dsp', '--contract-dir', join(dir, 'dir'),
    '--out', join(dir, 'include')], { cwd: OC, stdio: ['ignore', 'ignore', 'inherit'] });
  const src = join(dir, 'read.c');
  writeFileSync(src, `#include <stdio.h>
#include <omxdsp/omxdsp_definitions.h>
#include "fixtures/fdelay_read_l1.h"
int main(void) {
  printf("%d %.9g %.9g\\n", OMX_FDELAY_MOD_READ_ORDER, OMX_FDELAY_READ_L1_NORM_DOUBLE, omx_test_fdelay_read_l1(OMX_FDELAY_MOD_READ_ORDER));
  return 0;
}
`);
  execFileSync('cc', ['-std=c11', '-Wall', '-Werror', `-I${join(dir, 'include')}`, `-I${join(HERE, 'include')}`, `-I${join(HERE, 'test/kernels')}`,
    `-I${join(OC, 'include')}`, '-o', join(dir, 'read'), src, '-lm']);
  const [order, declared, oracle] = execFileSync(join(dir, 'read'), { encoding: 'utf8' }).trim().split(' ').map(Number);
  return { order, declared, oracle };
}

/** A scratch copy of this library's data/, to perturb. */
function scratchDsp() {
  const dir = join(scratchDir('omx-dsp-data-'), 'data');
  cpSync(join(HERE, 'data'), dir, { recursive: true });
  return dir;
}

test('the owner is this library: FDELAY_READ_L1_NORM is a plain number in its data, the value omx-contract derived (golden)', () => {
  assert.equal(relOf('FDELAY_READ_L1_NORM'), FILE);
  const item = JSON.parse(readFileSync(join(HERE, FILE), 'utf8')).FDELAY_READ_L1_NORM;
  assert.equal(typeof item.value, 'number', `${FILE} derives the value (${JSON.stringify(item.value)}): omx-contract's code computes it`);
  assert.equal(item.value, GOLDEN.FDELAY_READ_L1_NORM);
});

test('the consumer read and the oracle: the compiled header gives the golden, and it IS the kernel\'s worst L1 at the read order', () => {
  const r = compiledRead(join(HERE, 'data'));
  assert.deepEqual(r, { order: GOLDEN.FDELAY_MOD_READ_ORDER, declared: GOLDEN.FDELAY_READ_L1_NORM, oracle: GOLDEN.FDELAY_READ_L1_NORM });
});

test('pin: no item of this library\'s data derives a value through omx-contract\'s code beyond generic arithmetic', () => {
  assert.deepEqual(kernelDerivations(UNION), []);
});

test('pin: every ref of this library\'s data lands in its own files or on a standard word of omx-contract', () => {
  assert.deepEqual(strayRefs(UNION), []);
});

test('pin: this library\'s C names no omx-contract identifier but the standard words\'', () => {
  assert.deepEqual(strayIdentifiers(cSources()), []);
});

test('sabotage: the value moved in this library\'s data -> the readers follow, the oracle and the golden go red', () => {
  const dir = scratchDsp();
  editData(dir, 'primitives/fdelay-read-l1-norm.json', (d) => { d.FDELAY_READ_L1_NORM.value = 1.3; });
  const r = compiledRead(dir);
  assert.equal(r.declared, 1.3, 'the compiled read follows the moved value');
  assert.notEqual(r.declared, r.oracle, 'the oracle refuses the moved value');
  assert.notEqual(r.declared, GOLDEN.FDELAY_READ_L1_NORM, 'the golden refuses the moved value');
});

test('sabotage: each pin finds what it pins', () => {
  const dir = scratchData();
  editData(dir, 'primitives/fdelay-read-l1-norm.json', (d) => { d.FDELAY_READ_L1_NORM.value = { derive: 'lagrangeReadL1', order: { ref: 'FDELAY_MOD_READ_ORDER' } }; });
  assert.deepEqual(kernelDerivations(dir), ['data/primitives/fdelay-read-l1-norm.json: lagrangeReadL1']);
  editData(dir, 'primitives/fdelay-read-l1-norm.json', (d) => { d.FDELAY_READ_L1_NORM.value = { ref: 'RESOURCE_HINTS' }; });
  assert.deepEqual(strayRefs(dir), [`data/primitives/fdelay-read-l1-norm.json: RESOURCE_HINTS (${relOf('RESOURCE_HINTS')})`]);
  const stray = [...ocIdentifiers()].find((id) => /^OMX_RESOURCE_/.test(id) && !STANDARD_C.test(id));
  assert.ok(stray, 'omx-contract renders a resource word to plant');
  assert.deepEqual(strayIdentifiers([['planted.c', `/* ${stray} in a comment is no read */ int x = ${stray};`]]), [`planted.c: ${stray}`]);
});
