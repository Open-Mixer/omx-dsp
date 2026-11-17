// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * wasm-check.mjs <omx-dsp.wasm>: the module tools/wasm-build.sh builds from wasm/omx_curve_wasm.c is what the loader
 * expects, before any vector runs through it: it imports only the five WASI functions of wasi-libc's stdio (stand-ins that THROW here, so a call that reaches one is red), exports exactly the
 * four names of wasm/omx_curve_wasm.c plus its memory and `__wasm_call_ctors` (exported so the linker wraps nothing; a loader never calls it), and answers one request and one refusal.
 */
import { readFileSync } from 'node:fs';

const file = process.argv[2];
if (!file) { console.error('usage: wasm-check.mjs <omx-dsp.wasm>'); process.exit(2); }
const mod = new WebAssembly.Module(readFileSync(file));
const imports = WebAssembly.Module.imports(mod);
const ALLOWED = ['clock_time_get', 'fd_close', 'fd_seek', 'fd_write', 'random_get'];
const exports = WebAssembly.Module.exports(mod).map((e) => e.name).sort();
const want = ['__wasm_call_ctors', 'memory', 'omx_curve_wasm_answer', 'omx_curve_wasm_call', 'omx_curve_wasm_capacity', 'omx_curve_wasm_request'];
const fail = [];
const stray = imports.filter((i) => i.module !== 'wasi_snapshot_preview1' || !ALLOWED.includes(i.name));
if (stray.length) fail.push(`imports ${stray.map((i) => `${i.module}.${i.name}`).join(', ')}: only ${ALLOWED.join(', ')} of wasi_snapshot_preview1 are allowed`);
if (JSON.stringify(exports) !== JSON.stringify(want)) fail.push(`exports ${exports.join(', ')}, want ${want.join(', ')}`);
if (!fail.length) {
  const x = new WebAssembly.Instance(mod, { wasi_snapshot_preview1: Object.fromEntries(ALLOWED.map((n) => [n, () => { throw new Error(`the module called WASI ${n}`); }])) }).exports;
  const call = (doc) => {
    const bytes = new TextEncoder().encode(doc);
    new Uint8Array(x.memory.buffer, x.omx_curve_wasm_request(), bytes.length).set(bytes);
    const n = x.omx_curve_wasm_call(bytes.length);
    return n < 0 ? n : new TextDecoder().decode(new Uint8Array(x.memory.buffer, x.omx_curve_wasm_answer(), n));
  };
  const dyn = '{"kind":"transfer_curve","family":"dynamics","sampleRate":48000,"points":3,"extents":{"min":-40,"max":0},"terms":[{"role":"threshold","value":-20,"unit":"db"},{"role":"ratio","value":4,"unit":"ratio"}]}';
  const got = call(dyn);
  if (got !== '{"points":[{"x":-40,"y":-40},{"x":-20,"y":-20},{"x":0,"y":-15}],"marks":[]}') fail.push(`a request answered ${got}`);
  if (call('{"kind":"transfer_curve"}') !== -1) fail.push('a malformed request was not refused as -1');
  if (call('{"kind":"transfer_curve","family":"dynamics","sampleRate":48000,"points":3,"terms":[]}') !== '{"code":"termMissing"}') fail.push('a missing threshold was not refused as termMissing');
  if (x.omx_curve_wasm_capacity() !== 65536) fail.push(`capacity ${x.omx_curve_wasm_capacity()}`);
}
for (const f of fail) console.error(`wasm-check: ${f}`);
if (!fail.length) console.log(`wasm-check: ${file}: ${readFileSync(file).length} bytes, only the stdio WASI imports, the four exports, a request and two refusals answered`);
process.exit(fail.length ? 1 : 0);
