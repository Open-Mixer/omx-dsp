/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * omx_idelay_instance.h — the per-destination output delay as a plugin instance: a whole-sample delay
 * that aligns a loudspeaker, with no format in it. The core is omx_idelay.h's `omx_idelay_pair_apply`
 * at the delay `omx_idelay_samples` gives, THE SAME INLINES the console runs on a route leg or a bus
 * output, and this file adds no DSP to them. It adds what a host's port model needs and the kernel
 * leaves to its caller:
 *
 *   1. RINGS. `struct omx_idelay_pair` reads two rings the kernel leaves to its caller; the instance
 *      OWNS them, inline, {@link OMX_IDELAY_CAP} floats each (the 1 s ceiling at 192 kHz, plus one), so
 *      the full travel is reachable at every rate of the oracle grid and nothing is allocated, at init
 *      or after. An instance is initialised where it lives and is never copied after init (the pair
 *      points into its own rings).
 *   2. RESOLVE. omx_idelay_instance_resolve() takes the idelay kernel's one contract control,
 *      `outputDelay` (OUTPUT_DELAY_RANGE, ms), clamped into its declared travel, a non-finite word
 *      reading as no delay (the travel declares no default; 0 ms is its floor). omx_idelay_samples()
 *      then turns it into whole samples, rounded to the nearest: the kernel's own conversion, never a
 *      second one. Both legs take the one delay.
 *   3. IN -> OUT. The kernel works IN PLACE on each leg; run() copies in to out where they differ (they
 *      may alias) and runs it on out. Identity when bypassed or not ready.
 *
 * A delay change CROSS-FADES from the old tap to the new one across one block, the kernel's re-point, so
 * a fresh instance at a dialled delay fades there from 0 in its first block, as the console's fresh pair
 * does. Re-engaging from bypass CLEARS the rings and the cursors, the re-enable recipe of every face with
 * a ring: a re-engaged instance is a fresh one, silent until its delay has filled.
 *
 * LATENCY IS ZERO, AND PUBLISHED: the output delay IS the effect, the delay a host must not compensate
 * away (a compensating host would undo the alignment).
 *
 * The face omx-dsp#185 requires (tools/face-conformance.sh REQUIRED): the console runs the kernel per
 * output, and its plugin is generated from this face.
 */
/**
 * @file omx_idelay_instance.h
 * @brief the per-destination output delay as a plugin instance: the shell's core, with no format in it. The core is `omx_idelay.h`'s `omx_idelay_pair_apply` — THE SAME INLINE the console runs per destination — and this file adds no DSP to it.
 */
#ifndef OMX_IDELAY_INSTANCE_H
#define OMX_IDELAY_INSTANCE_H

#include <stdint.h>
#include <string.h>

#include <omxdsp/omxdsp_definitions.h>
#include <omxdsp/fx/omx_instance_io.h>
#include <omxdsp/omx_idelay.h>
#include <omxdsp/omx_param.h>

/** @brief Audio legs in and out: TWO, a stereo pair at one delay. */
#define OMX_IDELAY_INSTANCE_CHANNELS 2
/** @brief The latency the instance reports to its host, frames: none, the delay is the effect. */
#define OMX_IDELAY_INSTANCE_LATENCY_FRAMES 0.0f

/** @brief The delay a non-finite word reads as, ms: OUTPUT_DELAY_RANGE declares no default, so its floor, no delay. */
#define OMX_IDELAY_INSTANCE_DELAY_DEFAULT ((float)OMX_OUTPUT_DELAY_RANGE_MIN)

/** One instance: the kernel's pair over the instance's own rings, and the delay the last resolve set. */
typedef struct {
  float sr;                        /**< The sample rate the instance is bound to, Hz. */
  struct omx_idelay_pair pair;     /**< The kernel's pair: the rings, the cursors, the applied delays. */
  uint32_t tgt;                    /**< The delay, whole samples, as the last resolve set it (omx_idelay_samples()). */
  float ms;                        /**< The delay the last resolve delivered, ms, inside OUTPUT_DELAY_RANGE. */
  float ring_l[OMX_IDELAY_CAP];    /**< The left delay line, the instance's own. */
  float ring_r[OMX_IDELAY_CAP];    /**< The right delay line, the instance's own. */
  int bypass;                      /**< Non-zero while bypassed, as the last resolve set it; run() then copies the input. */
  int was_engaged;                 /**< The previous cycle's engaged flag, so a bypass->engaged edge can clear the rings. */
  int ready;                       /**< 1 after a successful omx_idelay_instance_init(); 0 makes run() the identity. */
} OmxIdelayInstance;

/**
 * @brief Bind an instance to its rate, its pair over its own rings, at no delay. Allocates nothing.
 * @param s The instance; every word of it is written.
 * @param sr The sample rate, Hz, one of the oracle grid's.
 * @return 1 when usable, 0 when not: a null instance or a rate off the grid is refused, `ready` stays clear
 *         and run() is the identity.
 * @thread control
 */
static inline int omx_idelay_instance_init(OmxIdelayInstance *s, float sr) {
  if (!omx_instance_bind(s, sizeof(*s), sr)) return 0;
  s->sr = sr;
  s->pair.ring_l = s->ring_l;
  s->pair.ring_r = s->ring_r;
  s->pair.cap = (uint32_t)OMX_IDELAY_CAP;
  s->tgt = 0u;
  s->ms = OMX_IDELAY_INSTANCE_DELAY_DEFAULT;
  s->was_engaged = 1;
  s->ready = 1;
  return 1;
}

#define OMX_CONTRACT_STAGE "idelay-instance/clear"
/** @brief The engage-edge clear: both rings back to silence, the write cursors and the applied delays back at
 * 0, as init left them. RT-safe: two bounded memsets over the inline rings.
 * @param s The instance; null or not ready is ignored.
 * @post `cleared-pair-is-fresh`: the rings read 0 at their ends and every cursor is back at 0.
 * @thread audio */
static inline void omx_idelay_instance_clear(OmxIdelayInstance *s) {
  if (!s || !s->ready) return;
  omx_instance_rings_clear(s->ring_l, s->ring_r, (uint32_t)OMX_IDELAY_CAP);
  s->pair.wpos_l = s->pair.wpos_r = 0u;
  s->pair.cur_l = s->pair.cur_r = 0u;
  OMX_POST(s->ring_l[0] == 0.0f && s->ring_r[OMX_IDELAY_CAP - 1] == 0.0f && s->pair.wpos_l == 0u &&
               s->pair.cur_r == 0u,
           "cleared-pair-is-fresh");
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "idelay-instance/resolve"
/**
 * @brief Resolve the host's control value for one cycle. The argument is the idelay kernel's contract control,
 * in its user unit: `output_delay` (ms), clamped into OUTPUT_DELAY_RANGE, a non-finite word reading as no
 * delay; omx_idelay_samples() turns it into whole samples. A null or not-ready instance is refused and nothing
 * changes.
 * @param s The instance.
 * @param bypass Non-zero makes run() the identity; the next engaged resolve clears the rings.
 * @param output_delay outputDelay, 0 to 1000 ms: the delay both legs are aligned with.
 * @post `the-delay-is-inside-the-ring`: the delay is a tap strictly inside the ring.
 * @thread audio
 */
static inline void omx_idelay_instance_resolve(OmxIdelayInstance *s, int bypass, float output_delay) {
  if (!s || !s->ready) return;
  const int engaged = bypass ? 0 : 1;
  if (engaged && !s->was_engaged) omx_idelay_instance_clear(s);
  s->was_engaged = engaged;
  s->bypass = engaged ? 0 : 1;
  s->ms = omx_clamp_or(output_delay, (float)OMX_OUTPUT_DELAY_RANGE_MIN, (float)OMX_OUTPUT_DELAY_RANGE_MAX,
                       OMX_IDELAY_INSTANCE_DELAY_DEFAULT);
  s->tgt = omx_idelay_samples(s->ms, s->sr, s->pair.cap);
  OMX_POST(s->tgt < s->pair.cap, "the-delay-is-inside-the-ring");
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "idelay-instance/run"
/**
 * @brief THE AUDIO CALLBACK'S WHOLE SHARE: copy in to out where they differ, then delay both legs in place on
 * out by the resolved delay, through the console's kernel. `in_*` and `out_*` may alias. Not ready, or
 * bypassed, is the identity byte for byte.
 * @param s The instance; null, not ready or bypassed leaves `out_*` holding `in_*`.
 * @param in_l The left input, `n` frames.
 * @param in_r The right input, `n` frames.
 * @param out_l The left output, `n` frames, may be `in_l`.
 * @param out_r The right output, `n` frames, may be `in_r`.
 * @param n The block length, frames; a null port or 0 touches nothing.
 * @thread audio
 */
static inline void omx_idelay_instance_run(OmxIdelayInstance *s, const float *in_l, const float *in_r, float *out_l,
                                           float *out_r, uint32_t n) {
  if (!omx_instance_copy_through(in_l, in_r, out_l, out_r, n)) return;
  if (!s || !s->ready || s->bypass) return;
  omx_idelay_pair_apply(out_l, out_r, n, &s->pair, s->tgt, s->tgt);
}
#undef OMX_CONTRACT_STAGE

/** @brief The frames of latency the instance introduces: none, at every rate (the delay is the effect).
 * @param s The instance, unread.
 * @return 0.
 * @thread audio */
static inline uint32_t omx_idelay_instance_latency(const OmxIdelayInstance *s) {
  (void)s;
  return 0u;
}

/** @brief The delay the pair applies once settled, whole samples, as the last resolve set it: the number a
 * host's readback or a plugin's delay readout publishes (omx_idelay_samples() of the delivered ms).
 * @param s The instance; null or not ready reads 0.
 * @return The delay, samples.
 * @thread audio */
static inline uint32_t omx_idelay_instance_samples(const OmxIdelayInstance *s) { return s && s->ready ? s->tgt : 0u; }

#endif /* OMX_IDELAY_INSTANCE_H */
