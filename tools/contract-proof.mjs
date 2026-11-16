#!/usr/bin/env node
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * Completeness: every kernel of data/kernels/ has every artifact omx-contract's recipes/kernel.recipe.json lists,
 * checked in a scratch tree of this tree's data/ and recipes/answers/ and their renders, with omx-contract's own
 * reader, renderer and recipe (the checkout named on the command line).
 *
 *   node tools/contract-proof.mjs <omx-contract checkout>
 *
 * Exit 0 every kernel is complete, 1 a gap, 2 usage.
 */
import { cpSync, existsSync, mkdirSync, mkdtempSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const HERE = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const OC = process.argv[2];
if (!OC || !existsSync(join(OC, 'lib/data.mjs'))) {
  console.error('usage: node tools/contract-proof.mjs <omx-contract checkout>');
  process.exit(2);
}
const oc = (p) => import(pathToFileURL(join(resolve(OC), p)).href);
const { loadData, resolveData, packageVersion } = await oc('lib/data.mjs');
const { renderC } = await oc('render/c.mjs');
const { renderJson } = await oc('render/json.mjs');
const { renderTs } = await oc('render/ts.mjs');
const { completeness } = await oc('tools/kernel-recipe.mjs');

/** A scratch tree of omx-contract's schema and recipe, this tree's data and answers, and their renders. */
function completenessTree() {
  const dir = mkdtempSync(join(tmpdir(), 'omx-dsp-contract-proof-'));
  for (const p of ['schema', 'recipes', 'package.json']) cpSync(join(resolve(OC), p), join(dir, p), { recursive: true });
  rmSync(join(dir, 'recipes/answers'), { recursive: true, force: true });
  cpSync(join(resolve(OC), 'data'), join(dir, 'data'), { recursive: true });
  cpSync(join(HERE, 'data'), join(dir, 'data'), { recursive: true });
  cpSync(join(HERE, 'recipes/answers'), join(dir, 'recipes/answers'), { recursive: true });
  const resolved = resolveData(loadData(join(dir, 'data')));
  const files = [
    ...renderC(resolved).map((f) => ['include', f]),
    ...renderJson(resolved, packageVersion(resolve(OC))).map((f) => ['share/omx-contract', f]),
    ...renderTs(resolved).map((f) => ['test/golden/ts', f]),
  ];
  for (const [d, f] of files) {
    mkdirSync(dirname(join(dir, d, f.path)), { recursive: true });
    writeFileSync(join(dir, d, f.path), f.text);
  }
  return dir;
}

const tree = completenessTree();
let v;
try {
  v = completeness(tree);
} finally {
  rmSync(tree, { recursive: true, force: true });
}
for (const l of v.lines) console.log(l);
for (const g of v.gaps) console.log(`  GAP ${g}`);
console.log(v.ok ? 'completeness: every kernel has every artifact of the recipe' : `completeness: ${v.gaps.length} gap(s)`);
process.exit(v.ok ? 0 : 1);
