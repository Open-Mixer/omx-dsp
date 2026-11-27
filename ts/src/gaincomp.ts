// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * The gain computer twin: omx_gaincomp_db (omx_gaincomp.h), kept for the console's tests that draw against it;
 * omx_gaincomp_db_shifted and omx_gate_knee have no twin,
 * the static characteristic a dynamics curve draws. The kernel is float; each step here is rounded to float by
 * Math.fround as the kernel rounds it (every operation is one IEEE single operation, and a double result rounded
 * once to single is the single result for +, -, *, / and sqrt), so the twin is the kernel's value, bit for bit,
 * where the kernel is built without contraction (-ffp-contract=off, as omxdsp.pc says). Pinned by
 * test/agreement.test.mjs.
 */

const f = Math.fround;

/** The gain computer's mode: act ABOVE the threshold (comp, limiter), BELOW it (gate, expander), or raise the
 *  signal below it (UPWARD). omx_gaincomp.h's OMX_DYN_ABOVE, OMX_DYN_BELOW and OMX_DYN_UPWARD. */
export type GaincompMode = 'above' | 'below' | 'upward';

/** The gain computer's parameters, omx_gaincomp_params: each value is narrowed to float as the kernel stores it. */
export interface GaincompParams {
  readonly mode: GaincompMode;
  /** The threshold, dB. */
  readonly threshDb: number;
  /** The ratio, at least the mode's minimum. */
  readonly ratio: number;
  /** The knee width, dB, not negative; 0 is a hard knee. */
  readonly kneeDb: number;
  /** BELOW: the attenuation floor, dB, at most 0. UPWARD: the largest boost, dB. Not read in ABOVE. */
  readonly rangeDb: number;
}

/**
 * The static gain, dB, at detector level `levelDb`: the twin of omx_gaincomp_db (its per-sample form).
 * @param p The parameters.
 * @param levelDb The detector level, dB; finite.
 */
export function gaincompDb(p: GaincompParams, levelDb: number): number {
  const thresh = f(p.threshDb), ratio = f(p.ratio), knee = f(p.kneeDb), range = f(p.rangeDb);
  const x = f(f(levelDb) - thresh);
  const half = f(0.5 * knee);
  const soft = knee > 0;
  const k2 = soft ? f(2 * knee) : 1;
  const eb = f(x - half);
  const kb = f(f(f(f(1 - ratio) * eb) * eb) / k2);
  let gb = soft && x > -half ? kb : f(f(ratio - 1) * x);
  gb = x >= half ? 0 : gb;
  gb = gb < range ? range : gb;
  const slope = f(f(1 / ratio) - 1);
  const ea = f(x + half);
  const ka = f(f(f(slope * ea) * ea) / k2);
  let ga = soft && x < half ? ka : f(slope * x);
  ga = x <= -half ? 0 : ga;
  const eu = f(half - x);
  const ku = f(f(f(slope * eu) * eu) / k2);
  let gu = soft && -x < half ? ku : f(slope * -x);
  gu = -x <= -half ? 0 : -gu;
  gu = gu > range ? range : gu;
  return p.mode === 'below' ? gb : p.mode === 'upward' ? gu : ga;
}
