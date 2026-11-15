/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * omx_hrp_place.h — the harmonic resonance processor's PLACEMENT and STEPPING in C: which bands a voice's
 * per-harmonic gains become, and how far one move takes a band toward its depth.
 *
 * The console placed HRP bands in its own TypeScript (core's harmonicBands, the server planter's stepToward)
 * while this library's HRP insert placed them inline in its analysis pass. They now share this placement: the
 * console's server calls it through its native addon, and omx_hrp_lv2.h calls it from its pass.
 * ts/src/hrp-place.ts is its TypeScript twin, and test/analysis/fixtures/hrp_place.vectors is the one vector
 * file the C test and the twin test both read.
 *
 * The rules, each the console's:
 *  - REFUSED WHOLE: an f0 that is not finite or not above zero, a Q that is not finite, a cut or boost bound that
 *    is not finite or below zero, or a band count that is not finite or below one places nothing.
 *  - HARMONIC k+1 AT (k+1) x f0: gains[k] is harmonic k+1, so gains[0] is the fundamental. The caller decides
 *    whether H1 is ever cut (the console's excess law never cuts it); this placement only places.
 *  - DROP, NEVER CLAMP: a band outside EQ_FREQ_RANGE (both bounds kept) is dropped here, because every door
 *    downstream CLAMPS a frequency into range, and a clamped partial is a real cut on a frequency nobody asked
 *    about ({@link omx_hrp_place_in_range}).
 *  - THE BOUNDS: a gain is clamped into [-max_cut_db, max_boost_db]; a gain that is not finite is skipped.
 *  - SPARSE: a zero gain (asked for, or reached by the clamp) is no band.
 *  - THE BELL'S Q: every band is a bell at q clamped into EQ_Q_RANGE, as the console's store clamps it.
 *  - THE CAP: over max_bands, the LARGEST |gain| are kept, ties to the lower harmonic; the kept bands come back
 *    in harmonic order. max_bands is a free-slot count (a number; a fraction keeps its whole part).
 *  - ONE STEP: a band moves toward its depth by at most HRP_GAIN_STEP_DB per move ({@link omx_hrp_step_toward}).
 *
 * Doubles throughout, as the console computes: the console's server calls this with its own numbers, and a
 * float here would move a step by a rounding the console never had.
 *
 * Worker thread or the console's server: no allocation, no lock, no I/O, one pass over the gains plus the cap's.
 */
#ifndef OMX_HRP_PLACE_H
#define OMX_HRP_PLACE_H

#include <stdint.h>

#include <omxdsp/omxdsp_definitions.h>

#ifdef __cplusplus
extern "C" {
#endif

/** The authority one placement is given, by the CALLER (the console reads its declared values; the insert its
 * ports). Doubles, so a non-finite value is refused rather than converted. */
typedef struct OmxHrpPlaceLimits {
  double max_cut_db;   /**< the largest cut, a positive magnitude */
  double max_boost_db; /**< the largest boost, a positive magnitude (0: cuts only) */
  double max_bands;    /**< how many bands may be placed (a free-slot count) */
} OmxHrpPlaceLimits;

/** One placed band: a bell at freq_hz, gain_db (negative is a cut), q. */
typedef struct OmxHrpPlacedBand {
  uint32_t harmonic; /**< the harmonic it was placed for (1 is the fundamental) */
  double freq_hz;
  double gain_db;
  double q;
} OmxHrpPlacedBand;

/**
 * @brief The bands one voice's per-harmonic gains become.
 * @param f0_hz The voice's measured fundamental.
 * @param gains_db gains_db[k] is harmonic k+1's requested gain (negative cuts).
 * @param n_gains How many gains (the ladder's length).
 * @param q The Q every band is placed at, before the bell's clamp.
 * @param limits The cut, boost and band bounds.
 * @param out Room for n_gains bands; the first (return value) are written, in harmonic order.
 * @return How many bands were placed (0 when the inputs are refused).
 * @thread worker
 */
uint32_t omx_hrp_place_bands(double f0_hz, const double *gains_db, uint32_t n_gains, double q,
                             const OmxHrpPlaceLimits *limits, OmxHrpPlacedBand *out);

/**
 * @brief Is a band at freq_hz placeable (inside EQ_FREQ_RANGE, both bounds kept)? A NaN is not.
 * @thread worker
 */
int omx_hrp_place_in_range(double freq_hz);

/**
 * @brief One bounded move of `current` toward `target`: the target when it is within HRP_GAIN_STEP_DB, else one
 * step toward it. A NaN in gives a NaN out, as the console's arithmetic does.
 * @thread worker
 */
double omx_hrp_step_toward(double current, double target);

#ifdef __cplusplus
}
#endif

#endif /* OMX_HRP_PLACE_H */
