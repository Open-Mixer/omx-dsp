#!/usr/bin/env node
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
//
// filter-responses-check.mjs — the section kinds the EQ design builds have one home: omx-contract's PARAM_FILTER_RESPONSES
// (its param domain, contract.d/param/param.json; Open-Mixer/omx-contract#208, omx-dsp#170). omx-dsp types the list
// nowhere: enum omx_eq_kind takes each code from the item's render, the X-macro OMX_PARAM_FILTER_RESPONSES(X) of
// <omxcontract/omx_vocabulary.h>, and no item of data/ holds the words again.
//
//   node tools/filter-responses-check.mjs <omx-contract checkout> [<data dir> <eq design header>]
//
// Findings, each named:
//   - an item of data/ (a set's ids, a list's values, a words item's words, at any depth) holding every word of the
//     item: the list typed again (BIQUAD_KINDS was one);
//   - an OMX_BIQUAD_KINDS_ name in the header: the copy's render, read again;
//   - an OMX_EQ_ kind of enum omx_eq_kind set to a number, or to anything but its OMX_FILTER_RESPONSE_ code;
//   - a header that does not expand OMX_PARAM_FILTER_RESPONSES(X), or misses a kind's member.
// Exit 0 clean; 1 a finding; 2 usage or environment (no checkout, no item), never a pass.
import { readFileSync, readdirSync, statSync } from 'node:fs';
import { join, relative, resolve } from 'node:path';

const HERE = resolve(import.meta.dirname, '..');
const argv = process.argv.slice(2);
if (argv.length !== 1 && argv.length !== 3) {
  console.error('usage: filter-responses-check.mjs <omx-contract checkout> [<data dir> <eq design header>]');
  process.exit(2);
}
const [oc, dataDir = join(HERE, 'data'), header = join(HERE, 'include/omxdsp/omx_eq_design.h')] = argv;

let words;
try {
  const item = JSON.parse(readFileSync(join(oc, 'contract.d/param/param.json'), 'utf8')).components?.schemas?.PARAM_FILTER_RESPONSES;
  words = item?.oneOf?.map((w) => w.const);
} catch (e) {
  console.error(`filter-responses-check: no omx-contract param domain at ${oc}: ${e.message}`);
  process.exit(2);
}
if (!Array.isArray(words) || words.length === 0) {
  console.error(`filter-responses-check: ${oc}/contract.d/param/param.json has no PARAM_FILTER_RESPONSES words item (omx-contract#208)`);
  process.exit(2);
}

const findings = [];
const holdsAll = (list) => words.every((w) => list.includes(w));

/** Every array of strings and every object's keys under `v`, with its JSON pointer. */
function scan(rel, v, at) {
  if (Array.isArray(v)) {
    if (v.every((x) => typeof x === 'string') && holdsAll(v)) findings.push(`${rel}${at}: holds every word of PARAM_FILTER_RESPONSES again (${v.join(', ')}): name omx-contract's item, never type it`);
    v.forEach((x, i) => scan(rel, x, `${at}/${i}`));
  } else if (v && typeof v === 'object') {
    if (at.endsWith('/words') && holdsAll(Object.keys(v))) findings.push(`${rel}${at}: words every word of PARAM_FILTER_RESPONSES again: name omx-contract's item, never type it`);
    for (const [k, x] of Object.entries(v)) scan(rel, x, `${at}/${k}`);
  }
}

function walk(dir) {
  for (const name of readdirSync(dir).sort()) {
    const p = join(dir, name);
    if (statSync(p).isDirectory()) walk(p);
    else if (name.endsWith('.json')) scan(relative(HERE, p) || p, JSON.parse(readFileSync(p, 'utf8')), '#');
  }
}
walk(dataDir);

const text = readFileSync(header, 'utf8');
const hrel = relative(HERE, header) || header;
if (/OMX_BIQUAD_KINDS_/.test(text)) findings.push(`${hrel}: reads OMX_BIQUAD_KINDS_, the render of a copy of PARAM_FILTER_RESPONSES`);
if (!/OMX_PARAM_FILTER_RESPONSES\(/.test(text)) findings.push(`${hrel}: does not expand OMX_PARAM_FILTER_RESPONSES(X), the render of omx-contract's item`);
const body = /enum omx_eq_kind \{([^}]*)\}/.exec(text)?.[1];
if (body === undefined) findings.push(`${hrel}: has no enum omx_eq_kind`);
else {
  const members = [...body.matchAll(/\b(OMX_EQ_[A-Z0-9_]+)\s*=\s*([^,/\n]+)/g)].map((m) => [m[1], m[2].trim()]);
  for (const [name, value] of members) {
    if (!/^OMX_FILTER_RESPONSE_[A-Z0-9_]+$/.test(value)) findings.push(`${hrel}: ${name} = ${value}: a kind takes its code from omx-contract's item (OMX_FILTER_RESPONSE_<NAME>), never a number or a copy`);
  }
  const snake = (w) => w.replace(/([a-z0-9])([A-Z])/g, '$1_$2').toUpperCase();
  const used = new Set(members.map(([, v]) => v));
  for (const w of words) if (!used.has(`OMX_FILTER_RESPONSE_${snake(w)}`)) findings.push(`${hrel}: enum omx_eq_kind has no kind for ${w} (OMX_FILTER_RESPONSE_${snake(w)})`);
}

for (const f of findings) console.error(`filter-responses-check: ${f}`);
if (findings.length) process.exit(1);
console.log(`filter-responses-check: ${words.length} words of PARAM_FILTER_RESPONSES, typed nowhere in ${relative(HERE, dataDir) || dataDir}, read by ${hrel}`);
