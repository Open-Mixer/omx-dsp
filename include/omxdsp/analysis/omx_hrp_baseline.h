/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * HRP per-note harmonic baseline: what THIS instrument's harmonics normally are, at THIS pitch.
 *
 * Moved unchanged from the OpenMixer engine's hrp_baseline.h (omx-dsp#15). The engine keeps the spectrum rings, the control-thread tick and the strip's EQ
 * bands that apply a correction; the analysis itself lives here.
 *
 * ## Note-relative, never channel-absolute (§20)
 *
 * The model is `baseline[midi note][harmonic]`: the same instrument produces different harmonic
 * relationships at different pitches, so one absolute-frequency profile per channel would blur
 * every note it has ever played into one shape and call the blur "normal". A voice that changes
 * note reads (and learns) a different row.
 *
 * ## A robust estimator, not a mean (§22)
 *
 * Each accepted measurement moves the stored level toward itself by AT MOST `step_db`. A bounded
 * step is a poor man's median: one 20 dB outlier — a mic bump, a collision the attribution
 * missed — moves the baseline by half a decibel, where an arithmetic mean would carry it for the
 * rest of the show. The first accepted measurement seeds the row outright (there is nothing to
 * be robust against yet).
 *
 * ## Ripeness — absence is a fact
 *
 * A row answers a baseline only after `ripe_updates` accepted measurements. Before that the
 * honest answer is NO ANSWER, and the deviation tier does nothing (§54: a missed correction
 * beats a wrong cut). Ripeness is per (note, harmonic): a note the instrument has never held
 * long enough has no baseline, however long its neighbours have.
 *
 * ## What is deliberately NOT here
 *
 * The acceptance GATES (§21 — stable voice, unambiguous present partial) are the caller's:
 * they are facts about the tracker and the attribution, and this module must stay testable by
 * feeding it numbers. Allocation-free, no globals, caller owns the memory. Never called from
 * the RT callback.
 */
/**
 * @file omx_hrp_baseline.h
 * @brief HRP per-note harmonic baseline: what THIS instrument's harmonics normally are, at THIS pitch.
 */
#ifndef OMX_HRP_BASELINE_H
#define OMX_HRP_BASELINE_H

#include <stdint.h>
#include <string.h>

#include <omxdsp/analysis/omx_hrp_attribute.h>

/** @brief MIDI note range the model spans — the full byte, so no playable note falls outside it. */
#define OMX_HRP_BASELINE_NOTES 128

/** @brief One channel's learned model: a level and an update count per (note, harmonic). */
typedef struct {
  /** The learned level per [MIDI note][harmonic - 1], dB, in the unit of the `level_db` values
   *  omx_hrp_baseline_learn() is given; seeded by the first one, then moved by at most `step_db` each. */
  float base_db[OMX_HRP_BASELINE_NOTES][OMX_HRP_HARMONICS];
  /** Accepted measurements per [MIDI note][harmonic - 1], saturating at UINT16_MAX; 0 is nothing learned,
   *  and the row answers once it reaches `ripe_updates`. */
  uint16_t updates[OMX_HRP_BASELINE_NOTES][OMX_HRP_HARMONICS];
} OmxHrpBaseline;

/** @brief How the baseline learns: the bound on one step and the count that makes a row ripe. */
typedef struct {
  /** The most one accepted measurement may move a learned level, in dB. */
  float step_db;
  /** Accepted measurements before a (note, harmonic) row answers at all. */
  uint16_t ripe_updates;
} OmxHrpBaselineConfig;

/** @brief The default config, step 0.5 dB / ripe 24: at the ~100 ms analysis tick, a note must hold (stable, confident,
 *  unambiguous) for ~2.4 s in total before its baseline speaks — soundcheck listening, not a
 *  snap judgement — and after that a full outlier costs half a decibel.
 * @return `step_db` 0.5, `ripe_updates` 24.
 * @thread worker */
static inline OmxHrpBaselineConfig omx_hrp_baseline_config_default(void) {
  OmxHrpBaselineConfig c;
  c.step_db = 0.5f;
  c.ripe_updates = 24;
  return c;
}

/** @brief Empty model: nothing learned, every row unripe. Also the §51 reset.
 * @param b The model to clear; non-NULL.
 * @thread worker */
static inline void omx_hrp_baseline_init(OmxHrpBaseline *b) { memset(b, 0, sizeof(*b)); }

/** @brief How many (note, harmonic) rows currently answer — the "what has it learned" readout.
 * @param b The model; non-NULL.
 * @param cfg The config whose `ripe_updates` decides ripeness; non-NULL.
 * @return The number of rows whose `updates` count is at least `ripe_updates`.
 * @thread worker */
static inline uint32_t omx_hrp_baseline_ripe_rows(const OmxHrpBaseline *b,
                                                  const OmxHrpBaselineConfig *cfg) {
  uint32_t n = 0;
  for (int note = 0; note < OMX_HRP_BASELINE_NOTES; note++)
    for (uint32_t k = 0; k < OMX_HRP_HARMONICS; k++)
      if (b->updates[note][k] >= cfg->ripe_updates) n++;
  return n;
}

/**
 * @brief Accept one gated measurement of harmonic `harmonic` (1-based; harmonic 1 IS the fundamental)
 * of the note `midi_note`, at `level_db`. The caller has already applied §21's gates; a note or
 * harmonic outside the model is ignored rather than clamped — clamping would file a piccolo's
 * overflow under the highest note the model holds, which is a different note.
 * @param b The model; non-NULL.
 * @param midi_note The note, 0..OMX_HRP_BASELINE_NOTES-1; outside it the call does nothing.
 * @param harmonic The harmonic, 1..OMX_HRP_HARMONICS; outside it the call does nothing.
 * @param level_db The measured level, dB: the row's value on its first update, else a target moved toward.
 * @param cfg The config whose `step_db` bounds the move; non-NULL.
 * @thread worker
 */
static inline void omx_hrp_baseline_learn(OmxHrpBaseline *b, int midi_note, uint32_t harmonic,
                                          float level_db, const OmxHrpBaselineConfig *cfg) {
  if (midi_note < 0 || midi_note >= OMX_HRP_BASELINE_NOTES) return;
  if (harmonic < 1 || harmonic > OMX_HRP_HARMONICS) return;
  const uint32_t k = harmonic - 1;
  uint16_t *n = &b->updates[midi_note][k];
  float *base = &b->base_db[midi_note][k];
  if (*n == 0) {
    *base = level_db;
  } else {
    float step = level_db - *base;
    if (step > cfg->step_db) step = cfg->step_db;
    if (step < -cfg->step_db) step = -cfg->step_db;
    *base += step;
  }
  if (*n < UINT16_MAX) (*n)++;
}

/**
 * @brief The learned level for (note, harmonic), when it is RIPE. Returns 1 and fills `out_db`, or
 * returns 0 — no baseline, and the caller does nothing (§54).
 * @param b The model; non-NULL.
 * @param midi_note The note, 0..OMX_HRP_BASELINE_NOTES-1; outside it there is no baseline.
 * @param harmonic The harmonic, 1..OMX_HRP_HARMONICS; outside it there is no baseline.
 * @param cfg The config whose `ripe_updates` decides ripeness; non-NULL.
 * @param out_db The learned level, dB; written only when the call returns 1.
 * @return 1 when the row is ripe and `out_db` was written, else 0.
 * @thread worker
 */
static inline int omx_hrp_baseline_read(const OmxHrpBaseline *b, int midi_note, uint32_t harmonic,
                                        const OmxHrpBaselineConfig *cfg, float *out_db) {
  if (midi_note < 0 || midi_note >= OMX_HRP_BASELINE_NOTES) return 0;
  if (harmonic < 1 || harmonic > OMX_HRP_HARMONICS) return 0;
  const uint32_t k = harmonic - 1;
  if (b->updates[midi_note][k] < cfg->ripe_updates) return 0;
  *out_db = b->base_db[midi_note][k];
  return 1;
}

#endif /* OMX_HRP_BASELINE_H */
