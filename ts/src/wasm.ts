// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * The loader of omx-dsp's curve function: the C of omx_curve_points_json (omx_curve_json.h) compiled to WebAssembly
 * (wasm/omx_curve_wasm.c, tools/wasm-build.sh), the one implementation every JavaScript runtime runs. `curvePoints` keeps
 * the signature the TypeScript twin had; the arithmetic is no longer here.
 *
 * The module imports five WASI functions of wasi-libc's stdio that no call reaches. They are stood in for by functions
 * that THROW, so a call that did reach one would be an error naming it, never a silent wrong answer, and no WASI is
 * implemented. The exported `__wasm_call_ctors` and the module's memory are not touched but through the four exports.
 */
import type { CurveAnswer, CurveRequest } from './curve-types.js';

export { CURVE_WORDS, curveRoleIsBand } from './curve-types.js';
export type { CurveAnswer, CurveCode, CurveMark, CurvePoint, CurveRequest, CurveStem, CurveTerm } from './curve-types.js';

/** The WASI functions the module imports and no call reaches. */
export const WASI_IMPORTS = ['clock_time_get', 'fd_close', 'fd_seek', 'fd_write', 'random_get'] as const;

interface CurveExports {
  readonly memory: WebAssembly.Memory;
  omx_curve_wasm_request(): number;
  omx_curve_wasm_answer(): number;
  omx_curve_wasm_capacity(): number;
  omx_curve_wasm_call(len: number): number;
}

/** A loaded curve module. */
export interface CurveModule {
  /** omx_curve_points_json: a `CurveRequest` document in, a `CurveReply` or `CurveRefusal` document out. Throws on a
   *  document that is no request (the C returns -1) or that does not fit the buffers (-2). */
  pointsJson(request: string): string;
  /** The twin's signature: a request object in, its answer object out; the C's own refusal codes as `{ code }`. */
  curvePoints(rq: CurveRequest): CurveAnswer;
  /** The module's size in bytes, as loaded. */
  readonly bytes: number;
}

/** A request as JSON text. JSON has no NaN and no Infinity: a number that is not finite is written 1e999 (-1e999), which
 *  is a JSON number the C reads as infinity, so the C's own `valueNotFinite` refusal answers it, in the C's own order. */
const requestJson = (rq: CurveRequest): string =>
  JSON.stringify(rq, (_k, v: unknown) => (typeof v === 'number' && !Number.isFinite(v) ? `\u0000nf${v < 0 ? '-' : ''}` : v)).replace(
    /"\\u0000nf(-?)"/g,
    (_m, sign: string) => `${sign}1e999`,
  );

const standIn = (name: string) => (): never => {
  throw new Error(`omx-dsp.wasm called WASI ${name}, which no call is meant to reach`);
};

/** The import object the module is instantiated with: every WASI function a stand-in that throws. */
export const wasiStandIns = (): { wasi_snapshot_preview1: Record<string, () => never> } => ({
  wasi_snapshot_preview1: Object.fromEntries(WASI_IMPORTS.map((n) => [n, standIn(n)])),
});

/**
 * Load the module.
 * @param source The module's bytes, or an already compiled module; `ts/wasm/omx-dsp.wasm` in the package.
 * @returns The loaded module, synchronous to use.
 */
export async function loadCurve(source: BufferSource | WebAssembly.Module): Promise<CurveModule> {
  const mod = source instanceof WebAssembly.Module ? source : await WebAssembly.compile(source);
  const x = (await WebAssembly.instantiate(mod, wasiStandIns())).exports as unknown as CurveExports;
  const enc = new TextEncoder(), dec = new TextDecoder();
  const capacity = x.omx_curve_wasm_capacity();
  const pointsJson = (request: string): string => {
    const bytes = enc.encode(request);
    if (bytes.length >= capacity) throw new Error(`omx_curve_points_json: a request of ${bytes.length} bytes does not fit ${capacity}`);
    new Uint8Array(x.memory.buffer, x.omx_curve_wasm_request(), bytes.length).set(bytes);
    const n = x.omx_curve_wasm_call(bytes.length);
    if (n === -1) throw new Error('omx_curve_points_json: the document is no curve request');
    if (n < 0) throw new Error('omx_curve_points_json: the answer does not fit the buffer');
    return dec.decode(new Uint8Array(x.memory.buffer, x.omx_curve_wasm_answer(), n));
  };
  const curvePoints = (rq: CurveRequest): CurveAnswer => JSON.parse(pointsJson(requestJson(rq))) as CurveAnswer;
  return { pointsJson, curvePoints, bytes: source instanceof WebAssembly.Module ? 0 : source.byteLength };
}
