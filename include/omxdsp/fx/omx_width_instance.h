/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * omx_width_instance.h — stereo width and pseudo-stereo as a plugin instance (FX-11): a mono keyboard
 * or playback made wide on the PA, a monitor mix narrowed. It adds no DSP to the omx_width word.
 *
 *   RESOLVE. omx_width_instance_resolve() takes the width kernel's contract controls in their
 *   declared order (data/kernels/width.json), a travel clamped into its range and a non-finite word
 *   reading as its default, a choice outside its set reading as its default. widthPct/100 scales the
 *   side; pseudoMs becomes the delay in samples (omx_width_delay); bassMonoHz is the LR4 corner.
 *   The engage edge (bypass off, or the word leaving its come-up values) clears the state; a change
 *   of pseudo-stereo mode or delay clears the ring; bass mono switched on clears the crossover.
 *   RUN. The word over both legs, in to out. At its come-up values (100 %, no pseudo-stereo, no bass
 *   mono) the face is the input bit for bit.
 *
 * LATENCY IS ZERO: the Haas delay and the comb are the effect, the dry mid is frame-aligned.
 * Nothing is allocated: the word's ring is inline in {@link OmxWidthInstance}.
 */
/**
 * @file omx_width_instance.h
 * @brief stereo width and pseudo-stereo as a plugin instance (FX-11): a mono keyboard or playback made wide on the PA, a monitor mix narrowed. It adds no DSP to the omx_width word.
 */
#ifndef OMX_WIDTH_INSTANCE_H
#define OMX_WIDTH_INSTANCE_H

#include <stdint.h>
#include <string.h>

#include <omxdsp/fx/omx_instance_io.h>
#include <omxdsp/omx_param.h>
#include <omxdsp/omx_width.h>
#include <omxdsp/omxdsp_definitions.h>

#define OMX_WIDTH_INSTANCE_CHANNELS 2

typedef struct {
  float sr;
  struct omx_width p;
  struct omx_width_state st;
  float wpct;  /**< widthPct as resolved */
  int pmode;   /**< pseudo as resolved, an OMX_WIDTH_PSEUDOS_* member */
  float pms;   /**< pseudoMs as resolved */
  int bmode;   /**< bassMono as resolved, an OMX_WIDTH_BASS_MONOS_* member */
  float bhz;   /**< bassMonoHz as resolved */
  int enabled;
  int ready;
} OmxWidthInstance;

static inline void omx_width_instance_resolve(OmxWidthInstance *s, int bypass, float width_pct, int pseudo, float pseudo_ms,
                                              int bass_mono, float bass_mono_hz);

/** Bind an instance to its rate. Returns 1 when usable, 0 when not: the rate must be one of the oracle
 * set. The face comes up at its declared defaults, the identity. Allocates nothing.
 * @param s The instance; every word of it is written.
 * @param sr The sample rate, Hz, one of the oracle set.
 * @return 1 when usable, 0 when not: a rate the console does not declare (the oracle set, 32 to 192 kHz) is refused, `ready` stays clear and run() is the identity.
 * @thread control */
static inline int omx_width_instance_init(OmxWidthInstance *s, float sr) {
  if (!omx_instance_bind(s, sizeof(*s), sr)) return 0;
  s->sr = sr;
  s->ready = 1;
  omx_width_instance_resolve(s, 0, (float)OMX_WIDTH_WIDTH_PCT_RANGE_DEFAULT, (int)OMX_WIDTH_PSEUDOS_DEFAULT,
                             (float)OMX_WIDTH_PSEUDO_MS_RANGE_DEFAULT, (int)OMX_WIDTH_BASS_MONOS_DEFAULT,
                             (float)OMX_WIDTH_BASS_MONO_HZ_RANGE_DEFAULT);
  return 1;
}

#define OMX_CONTRACT_STAGE "width/instance-resolve"
/**
 * Resolve the host's control values for one cycle. `bypass` non-zero makes run() the identity. The
 * arguments are the width kernel's contract controls in their declared order: the width (percent),
 * the pseudo-stereo (a WIDTH_PSEUDOS index), its delay (ms), the bass mono (a WIDTH_BASS_MONOS index)
 * and its corner (Hz).
 * @param s The instance; null or not ready leaves it as it is.
 * @param bypass Non-zero makes run() the identity.
 * @param width_pct widthPct, 0 to 200 \%, default 100: The stereo width, percent: the side (L - R)/2 times widthPct/100
 *        over the mid (L + R)/2.
 * @param pseudo pseudo, the place in off, comb, haas (0 to 2), default off: A pseudo-stereo for a mono source, in the
 *        order of the integer the kernel reads: off; comb, the complementary comb L = M + g M[n - d], R = M - g M[n -
 *        d] with g WIDTH_COMB_GAIN, whose mono sum is the mid exactly; haas, the right leg delayed by pseudoMs.
 * @param pseudo_ms pseudoMs, 0 to OMX_WIDTH_MAX_DELAY_MS ms, default 12: The pseudo-stereo's delay d, ms, rounded to
 *        the nearest sample.
 * @param bass_mono bassMono, the place in off, on (0 to 1), default off: Whether the low end is summed to mono: on
 *        splits the side with a Linkwitz-Riley crossover at bassMonoHz (omx_xover, LR4) and keeps its high band only,
 *        the mid through the crossover's all-pass so the bands stay in phase.
 * @param bass_mono_hz bassMonoHz, 20 to 300 Hz, default 120: Where the low end goes mono, Hz: the crossover's corner.
 * @thread audio
 */
static inline void omx_width_instance_resolve(OmxWidthInstance *s, int bypass, float width_pct, int pseudo, float pseudo_ms,
                                              int bass_mono, float bass_mono_hz) {
  if (!s || !s->ready) return;
  s->wpct = omx_clamp_or(width_pct, (float)OMX_WIDTH_WIDTH_PCT_RANGE_MIN, (float)OMX_WIDTH_WIDTH_PCT_RANGE_MAX,
                         (float)OMX_WIDTH_WIDTH_PCT_RANGE_DEFAULT);
  s->pmode = pseudo >= 0 && pseudo < (int)OMX_WIDTH_PSEUDOS_COUNT ? pseudo : (int)OMX_WIDTH_PSEUDOS_DEFAULT;
  s->pms = omx_clamp_or(pseudo_ms, (float)OMX_WIDTH_PSEUDO_MS_RANGE_MIN, (float)OMX_WIDTH_PSEUDO_MS_RANGE_MAX,
                        (float)OMX_WIDTH_PSEUDO_MS_RANGE_DEFAULT);
  s->bmode = bass_mono == OMX_WIDTH_BASS_MONOS_ON ? 1 : 0;
  s->bhz = omx_clamp_or(bass_mono_hz, (float)OMX_WIDTH_BASS_MONO_HZ_RANGE_MIN, (float)OMX_WIDTH_BASS_MONO_HZ_RANGE_MAX,
                        (float)OMX_WIDTH_BASS_MONO_HZ_RANGE_DEFAULT);
  const int was_bass = s->p.bass_mono, was_pseudo = s->p.pseudo, was_live = s->enabled && s->p.active;
  const uint32_t was_d = s->p.d;
  omx_width_design(&s->p, s->wpct * 0.01f, s->pmode, omx_width_delay(s->pms, s->sr), s->bmode, (double)s->bhz, (double)s->sr);
  s->enabled = bypass ? 0 : 1;
  /* The engage edge starts the word from silence; a new pseudo-stereo mode or delay starts its ring
   * from silence, so a ring left from minutes earlier, or the other mode's signal, is never read. */
  if (s->enabled && s->p.active && !was_live) {
    memset(&s->st, 0, sizeof s->st);
  } else {
    if (s->p.bass_mono && !was_bass) {
      memset(&s->st.xs, 0, sizeof s->st.xs);
      memset(&s->st.ap, 0, sizeof s->st.ap);
    }
    if (s->p.pseudo != was_pseudo || s->p.d != was_d) {
      memset(s->st.ring, 0, sizeof s->st.ring);
      s->st.pos = 0u;
    }
  }
}
#undef OMX_CONTRACT_STAGE

/**
 * THE AUDIO CALLBACK'S WHOLE SHARE: the word over both legs, in to out. `in_*` and `out_*` may alias.
 * Not ready, bypassed or at the come-up values is the identity and moves no state.
 * @param s The instance; null, not ready or bypassed leaves `out_*` holding `in_*`.
 * @param in_l The left input, `n` frames.
 * @param in_r The right input, `n` frames.
 * @param out_l The left output, `n` frames, may be `in_l`.
 * @param out_r The right output, `n` frames, may be `in_r`.
 * @param n The block length, frames; a null port or 0 touches nothing.
 * @thread audio
 */
static inline void omx_width_instance_run(OmxWidthInstance *s, const float *in_l, const float *in_r, float *out_l,
                                          float *out_r, uint32_t n) {
  if (!omx_instance_copy_through(in_l, in_r, out_l, out_r, n)) return;
  if (!s || !s->ready || !s->enabled) return;
  omx_width_process(out_l, out_r, n, &s->p, &s->st);
}

/** The frames of latency the instance introduces: none.
 * @param s The instance.
 * @return 0.
 * @thread audio */
static inline uint32_t omx_width_instance_latency(const OmxWidthInstance *s) {
  (void)s;
  return 0u;
}

#endif /* OMX_WIDTH_INSTANCE_H */
