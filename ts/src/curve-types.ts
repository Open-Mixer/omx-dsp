// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * The documents of omx_curve_points_json (curve-io.schema.json) as TypeScript, and the words of the sets they name.
 * Types only: the arithmetic is the C, loaded as WebAssembly by wasm.ts. The interfaces below MOVED here from
 * curve.ts; they were not added by this change. Their types are to come from the owner schema as
 * `FromSchema<typeof schema>` (json-schema-to-ts), not from a rendered .d.ts, per omx-contract's runtime-data spec
 * (docs/runtime-data.md, merged in PR #104): no render writes this file.
 */
import { CURVE_DISPLAYS, CURVE_FAMILIES, CURVE_LEGS, CURVE_ORIGINS, CURVE_ROLE_PREFIXES, CURVE_ROLES, CURVE_SHAPES, CURVE_UNITS, CURVE_CODES } from './definitions.js';

/** A set id as the description spells it: `eqResponse` is `eq_response`; the unshaped band is `-`. */
const wordOf = (id: string): string => (id === 'unshaped' ? '-' : id.replace(/[A-Z]/g, (c) => `_${c.toLowerCase()}`));

/** The wire words of each set, in value order. */
export const CURVE_WORDS = {
  displays: CURVE_DISPLAYS.map(wordOf),
  families: CURVE_FAMILIES.map(wordOf),
  roles: CURVE_ROLES.map(wordOf),
  units: [...CURVE_UNITS], // the PARAM_UNITS ids themselves, never re-spelt (#122)
  shapes: CURVE_SHAPES.map(wordOf),
  legs: CURVE_LEGS.map(wordOf),
  /** fbs-fixed and fbs-live: the origins are spelt with a hyphen */
  origins: CURVE_ORIGINS.map((id) => id.replace(/[A-Z]/g, (c) => `-${c.toLowerCase()}`)),
} as const;

/** What curvePoints answers when it draws nothing: a CURVE_CODES id other than `ok`. */
/**
 * Whether a role word is an indexed band role's: it starts with CURVE_ROLE_PREFIXES `band` (`band_freq` does), as
 * omx_curve_role_is_band() answers in C. A consumer asks this, never a prefix of its own (#40).
 */
export const curveRoleIsBand = (word: string): boolean => word.startsWith(CURVE_ROLE_PREFIXES.band);

export type CurveCode = Exclude<(typeof CURVE_CODES)[number], 'ok'>;

/** One bound term: a role of the description, its value now, in the unit the description gives it. */
export interface CurveTerm {
  /** The role word: `band_freq`, `threshold`, `knee_start`… */
  readonly role: string;
  /** The band's index for a `band_*` role; 0 (or absent) for any other. */
  readonly index?: number;
  /** The control's plain value. */
  readonly value: number;
  /** The unit token: `hz`, `khz`, `db`, `coef`, `oct`, `none`… */
  readonly unit: string;
  /** For `band_type`, the shape its value selects (from the role's `shapes`); for `band_freq`, the fixed shape. */
  readonly shape?: string;
  /** Where the band comes from: `user` (the default), `fbs-fixed` or `fbs-live`. A band's is its band_freq term's. */
  readonly origin?: string;
}

/** A curve request: one display of the description, bound. */
export interface CurveRequest {
  /** The display's kind: `eq_response` or `transfer_curve`. */
  readonly kind: string;
  /** The family: `eq`, `dynamics`, `limiter`, `gate` or `deesser`. */
  readonly family: string;
  /** The instance's rate, Hz. */
  readonly sampleRate: number;
  /** The grid points, 2 to CURVE_POINTS_MAX (an EQ's anchors on top). */
  readonly points: number;
  /** The x extents; CURVE_EXTENTS when absent. */
  readonly extents?: { readonly min: number; readonly max: number };
  /** The terms; a later term of one role and index replaces an earlier. */
  readonly terms: readonly CurveTerm[];
  /** Also answer the curve over the `user` bands alone, at the same x. */
  readonly userOnly?: boolean;
  /** The host's tempo, BPM, for a synced delay; absent when the host hands none (a synced delay is then termMissing). */
  readonly tempoBpm?: number;
}

/** One tap of a delay's time_response: its time (ms), its level (dB), its brightness (0..1) and the leg it lands on. */
export interface CurveStem {
  readonly x: number;
  readonly y: number;
  readonly shade: number;
  readonly leg: string;
}

/** A band whose origin is not `user`: its centre (Hz), its own magnitude there (dB), its Q and its origin. */
export interface CurveMark {
  readonly freq: number;
  readonly depthDb: number;
  readonly q: number;
  readonly origin: string;
}

/** One point: Hz and dB for an EQ, input dB and output dB for a transfer curve. */
export interface CurvePoint {
  readonly x: number;
  readonly y: number;
}

/** The answer: the points, ascending in x, or the code of what is missing or refused. */
export type CurveAnswer =
  | {
      readonly points: readonly CurvePoint[];
      /** One per drawn band whose origin is not `user`, in band order; none for a transfer curve. */
      readonly marks: readonly CurveMark[];
      /** With `userOnly`: the curve over the user bands (and the head filters) alone, at the same x. */
      readonly userPoints?: readonly CurvePoint[];
      /** A delay's tap train (a time_response); absent for every other display. */
      readonly stems?: readonly CurveStem[];
    }
  | { readonly code: CurveCode };

