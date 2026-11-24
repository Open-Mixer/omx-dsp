/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * FBS feedback detection: one magnitude spectrum per push in, the confirmed rings out.
 *
 * Moved unchanged from the OpenMixer engine's fbs_detect.h (omx-dsp#15): the engine keeps the rings and the control-thread
 * tick that feed it a spectrum, and plants what it confirms in the strip's own EQ bands; the detection itself lives here.
 *
 * The gates are core's declaration (`DEFAULT_THRESHOLDS`, `thresholdsFor`), read here from the
 * generated omxdsp_definitions.h or handed in per detector; this file restates none of them.
 * NEVER called from the audio callback: the detector allocates nothing, takes every buffer from
 * the caller, holds no global mutable state and does no I/O.
 */
/**
 * @file omx_fbs_detect.h
 * @brief FBS feedback detection: one magnitude spectrum per push in, the confirmed rings out.
 */
#ifndef OMX_FBS_DETECT_H
#define OMX_FBS_DETECT_H

#include <omxdsp/omxdsp_definitions.h>
#include <stdint.h>

/* Every constant this engine reads is declared once in core's feedback declaration and reaches
 * it through the generated omxdsp_definitions.h: the gates (OMX_DEFAULT_THRESHOLDS_*,
 * OMX_GROWTH_WINDOW_FRAMES, OMX_DEEP_PLATEAU_FRAMES, OMX_SUBHARMONIC_RADIUS_BINS), the ratio
 * neighbourhoods (OMX_FBS_PAPR_GUARD_BINS, OMX_FBS_PNPR_RADIUS_BINS, OMX_FBS_PNPR_GUARD_BINS,
 * OMX_FBS_PHPR_HARMONICS, OMX_FBS_PSHR_SELF_SKIP_BINS), the dB floor (OMX_FBS_FLOOR_LIN_DOUBLE),
 * the depth law and the notch Q (OMX_FBS_MAX_NOTCH_DEPTH_DB, OMX_FBS_DEFAULT_NOTCH_Q) and the
 * console's per-channel pool (OMX_FBS_TRACK_SLOTS). */

/** A detector pool: ring-out finds (`fixed`) or show finds (`live`). */
typedef enum { OMX_FBS_POOL_FIXED = 0, OMX_FBS_POOL_LIVE = 1 } OmxFbsPool;

/** Every gate a push confirms on — core's `DetectThresholds`, field for field. The three
 * opt-in routes are off while their `*_on` flag is 0. */
typedef struct {
  /** dB: a candidate's level over the spectrum average outside its guard band must reach this. */
  double papr_db;
  /** dB: a candidate's level over the average of its near neighbours (guard excluded) must reach this. */
  double pnpr_db;
  /** dB: a candidate's level over its strongest harmonic must reach this. */
  double phpr_db;
  /** Frames: the ramp route needs a streak this long; the plateau route checks this many latest levels. */
  uint32_t persist_frames;
  /** Pushes in a row omx_fbs_is_growing() must hold for the ramp route. */
  uint32_t growth_confirm_frames;
  /** Pushes in a row the bin must pass `pshr_min_db` for the ramp and plateau routes. */
  uint32_t sub_clean_frames;
  /** dB: omx_fbs_is_growing() needs the newest level minus the oldest to reach this. */
  double growth_min_total_rise_db;
  /** Fraction: omx_fbs_is_growing() fails when one frame-to-frame rise exceeds this share of the total rise. */
  double growth_max_single_frame_share;
  /** dB per frame: omx_fbs_is_growing() needs the least-squares slope of the history to reach this. */
  double growth_min_db_per_frame;
  /** dB: the largest frame-to-frame drop the ramp, plateau and trend routes allow; the deep plateau allows half. */
  double growth_max_dip_db;
  /** dB: a push whose omx_fbs_pshr() reaches this counts toward the slot's `sub_clean_run`. */
  double pshr_min_db;
  /** Nonzero enables the plateau route; 0 (the default) leaves it off. */
  int plateau_on;
  /** dB: the plateau route needs each of the last `persist_frames` levels at or above this. */
  double plateau_floor_db;
  /** Nonzero enables the deep-plateau route; 0 (the default) leaves it off. */
  int deep_plateau_on;
  /** dB: the deep-plateau route needs each of the last OMX_DEEP_PLATEAU_FRAMES levels at or above this. */
  double deep_plateau_floor_db;
  /** Nonzero enables the trend route; 0 (the default) leaves it off. */
  int trend_on;
  /** dB: the trend route needs the deep history's newest level minus its oldest to reach this. */
  double trend_min_rise_db;
} OmxFbsThresholds;

/** One confirmed ring — core's `NotchCandidate` less its pool, which is the detector's. */
typedef struct {
  uint32_t bin;          /**< the spectrum bin */
  double freq_hz;        /**< Hz: bin * sample_rate / fft_size */
  double depth_db;       /**< dB: minus the bin's papr rounded half up, capped at OMX_FBS_MAX_NOTCH_DEPTH_DB */
  double q;              /**< the notch Q: always OMX_FBS_DEFAULT_NOTCH_Q */
  uint32_t persistence;  /**< frames: the slot's streak when this candidate was built */
  double level_db;       /**< dB: the bin's level on this frame */
} OmxFbsCandidate;

/** One tracked candidate bin's memory. Caller-owned; the detector lays nothing out itself. */
typedef struct {
  int32_t bin;            /**< the bin this slot tracks, or -1 when the slot is free */
  uint32_t streak;        /**< frames the bin has been a candidate since the slot was taken */
  uint32_t growing_run;   /**< consecutive pushes omx_fbs_is_growing() held over `hist`; 0 on a miss */
  uint32_t sub_clean_run; /**< consecutive pushes the bin passed the pshr gate; 0 on a miss */
  uint32_t n_hist;        /**< filled entries of `hist` */
  uint32_t n_deep;        /**< filled entries of `deep` */
  /** dB: the bin's latest levels, oldest first; the ramp and plateau routes read it. */
  double hist[OMX_GROWTH_WINDOW_FRAMES];
  /** dB: the bin's latest levels, oldest first; the deep-plateau and trend routes read it. */
  double deep[OMX_DEEP_PLATEAU_FRAMES];
  int confirmed;          /**< 1 once the bin has confirmed; it confirms once per slot */
  uint64_t confirm_seq;   /**< the detector's `seq` at confirmation; orders the snapshot */
  /** The detector `frame` on which the bin was last a candidate; a push frees every slot not seen on its frame. */
  uint64_t seen_frame;
} OmxFbsSlot;

/** One channel's detector. Every pointer is caller-owned memory of the stated size. */
typedef struct {
  OmxFbsThresholds t;  /**< the gates, set by the caller before omx_fbs_init() */
  OmxFbsPool pool;     /**< the pool this detector finds for, set by the caller; omx_fbs_push() does not read it */
  double sample_rate;  /**< Hz, > 0: turns a bin into `freq_hz` */
  uint32_t fft_size;   /**< the FFT length in samples, > 0: turns a bin into `freq_hz` */
  int32_t *bin_slot;  /**< nbins_cap entries: the slot tracking each bin, or −1 */
  uint32_t nbins_cap;  /**< entries of `bin_slot`; omx_fbs_push() scores at most this many bins */
  OmxFbsSlot *slots;  /**< n_slots entries */
  uint32_t *free_stack; /**< n_slots entries */
  uint32_t n_slots;    /**< entries of `slots` and `free_stack`: how many bins may be tracked at once */
  uint32_t n_free;     /**< free slots left on `free_stack`; omx_fbs_init() sets it to n_slots */
  uint64_t frame;      /**< pushes since omx_fbs_init(); omx_fbs_push() counts it up first */
  uint64_t seq;        /**< confirmations since omx_fbs_init(); each confirmation takes the next value */
} OmxFbsDetector;

/** What one push concluded. Every array is caller-owned with room for the detector's n_slots. */
typedef struct {
  OmxFbsCandidate *confirmed; /**< newly confirmed on this frame, ascending bin */
  uint32_t n_confirmed;       /**< entries filled in `confirmed` */
  OmxFbsCandidate *snapshot;  /**< every confirmed bin still ringing, in confirmation order */
  uint32_t n_snapshot;        /**< entries filled in `snapshot` */
  uint32_t *active;           /**< every candidate bin this frame, ascending */
  uint32_t n_active;          /**< entries filled in `active` */
  uint32_t untracked;         /**< candidate bins this frame that found no free slot */
} OmxFbsResult;

/**
 * @brief The corpus-tuned default gates, read from the generated header.
 * @return core's `DEFAULT_THRESHOLDS`: every opt-in route off.
 * @thread worker
 */
OmxFbsThresholds omx_fbs_thresholds_default(void);

/**
 * @brief Initialise a detector over caller-owned memory.
 * @param d The detector, its configuration already filled: `t`, `pool`, `sample_rate` (Hz, > 0),
 *          `fft_size` (> 0), `bin_slot` (`nbins_cap` entries, the widest frame a push may carry),
 *          `slots` and `free_stack` (`n_slots` entries each, how many bins may be tracked at once).
 *          Every other field is reset.
 * @thread worker
 */
void omx_fbs_init(OmxFbsDetector *d);

/**
 * @brief Score one spectrum frame.
 * @param d The detector.
 * @param mags Linear magnitudes, `nbins` of them.
 * @param nbins Bin count, ≤ the detector's nbins_cap.
 * @param out Receives the frame's decisions.
 * @note Allocation-free, lock-free, no I/O.
 * @thread control
 */
void omx_fbs_push(OmxFbsDetector *d, const double *mags, uint32_t nbins, OmxFbsResult *out);

/**
 * @brief The ramp gate over a dB history, oldest first.
 * @param levels_db The history.
 * @param n Its length.
 * @param t The gates.
 * @return 1 when the history is a monotone, distributed, linear-in-dB run-away.
 * @thread worker
 */
int omx_fbs_is_growing(const double *levels_db, uint32_t n, const OmxFbsThresholds *t);

/**
 * @brief Peak-to-subharmonic ratio at `bin`: its level minus the strongest line near bin/2 or
 *        bin/3.
 * @param mags Linear magnitudes.
 * @param nbins Their count.
 * @param bin The candidate bin, < nbins.
 * @return dB; +INFINITY when there is no line below to measure.
 * @thread worker
 */
double omx_fbs_pshr(const double *mags, uint32_t nbins, uint32_t bin);

#endif /* OMX_FBS_DETECT_H */
