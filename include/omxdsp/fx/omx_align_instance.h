/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * omx_align_instance.h — the input alignment delay as a plugin instance: one microphone pushed back to
 * meet another, with no format in it. The core is omx_align.h's `omx_align_apply` over the target
 * `omx_align_target` gives, THE SAME INLINES the console's strip runs beside trim and polarity, and
 * this file adds no DSP to them. It adds what a host's port model needs and the kernel leaves to its
 * caller:
 *
 *   1. RINGS. `struct omx_align_state` reads two rings the kernel leaves to its caller; the instance
 *      OWNS them, inline, {@link OMX_ALIGN_CAP} floats each (the 100 ms ceiling at 192 kHz plus the
 *      order-5 kernel's reach), so the full travel is reachable at every rate of the oracle grid and
 *      nothing is allocated, at init or after. An instance is initialised where it lives and is never
 *      copied after init (the state points into its own rings).
 *   2. RESOLVE. omx_align_instance_resolve() takes the align kernel's one contract control,
 *      `inputAlignDelay` (INPUT_ALIGN_DELAY_RANGE, ms), clamped into its declared travel, a non-finite
 *      word reading as no delay (the travel declares no default; 0 ms is the travel's floor and what
 *      the kernel itself reads a non-finite request as). omx_align_target() then turns it into the
 *      target in fractional samples: the kernel's own conversion, never a second one.
 *   3. IN -> OUT. The kernel works IN PLACE on both legs, one target for the pair; run() copies in to
 *      out where they differ (they may alias) and runs it on out. Identity when bypassed or not ready.
 *
 * A move SLEWS the read point by OMX_ALIGN_SLEW per sample and lands exactly, the console's re-point,
 * so a fresh instance at a dialled delay glides there from 0 as the console's freshly armed line does.
 * Re-engaging from bypass CLEARS the rings and re-arms the lines, the re-enable recipe of every face
 * with a ring: a re-engaged instance is a fresh one, silent until its delay has filled.
 *
 * LATENCY IS ZERO, AND PUBLISHED: the alignment delay IS the effect, the delay a host must not
 * compensate away (a compensating host would undo the alignment).
 *
 * The face omx-dsp#185 requires (tools/face-conformance.sh REQUIRED): the console runs the kernel per
 * strip, and its plugin is generated from this face.
 */
/**
 * @file omx_align_instance.h
 * @brief the input alignment delay as a plugin instance: the shell's core, with no format in it. The core is `omx_align.h`'s `omx_align_apply` — THE SAME INLINE the console's strip runs — and this file adds no DSP to it.
 */
#ifndef OMX_ALIGN_INSTANCE_H
#define OMX_ALIGN_INSTANCE_H

#include <stdint.h>
#include <string.h>

#include <omxdsp/omxdsp_definitions.h>
#include <omxdsp/fx/omx_instance_io.h>
#include <omxdsp/omx_align.h>
#include <omxdsp/omx_param.h>

/** @brief Audio legs in and out: TWO, a stereo pair over one target. */
#define OMX_ALIGN_INSTANCE_CHANNELS 2
/** @brief The latency the instance reports to its host, frames: none, the delay is the effect. */
#define OMX_ALIGN_INSTANCE_LATENCY_FRAMES 0.0f

/** @brief The delay a non-finite word reads as, ms: INPUT_ALIGN_DELAY_RANGE declares no default, so its floor,
 *  no delay, which omx_align_target() reads a non-finite request as. */
#define OMX_ALIGN_INSTANCE_DELAY_DEFAULT ((float)OMX_INPUT_ALIGN_DELAY_RANGE_MIN)

/** One instance: the kernel's state over the instance's own rings, and the target the last resolve set. */
typedef struct {
  float sr;                        /**< The sample rate the instance is bound to, Hz. */
  struct omx_align_state state;    /**< The kernel's state, armed over the instance's rings. */
  float tgt;                       /**< The target, fractional samples, as the last resolve set it (omx_align_target()). */
  float ms;                        /**< The delay the last resolve delivered, ms, inside INPUT_ALIGN_DELAY_RANGE. */
  float ring_l[OMX_ALIGN_CAP];     /**< The left alignment line, the instance's own. */
  float ring_r[OMX_ALIGN_CAP];     /**< The right alignment line, the instance's own. */
  int bypass;                      /**< Non-zero while bypassed, as the last resolve set it; run() then copies the input. */
  int was_engaged;                 /**< The previous cycle's engaged flag, so a bypass->engaged edge can clear the rings. */
  int ready;                       /**< 1 after a successful omx_align_instance_init(); 0 makes run() the identity. */
} OmxAlignInstance;

/**
 * @brief Bind an instance to its rate and arm its two lines over its own rings, at no delay. Allocates nothing.
 * @param s The instance; every word of it is written.
 * @param sr The sample rate, Hz, one of the oracle grid's.
 * @return 1 when usable, 0 when not: a null instance or a rate off the grid is refused, `ready` stays clear
 *         and run() is the identity.
 * @thread control
 */
static inline int omx_align_instance_init(OmxAlignInstance *s, float sr) {
  if (!omx_instance_bind(s, sizeof(*s), sr)) return 0;
  s->sr = sr;
  if (!omx_align_arm(&s->state, s->ring_l, s->ring_r)) return 0;
  s->tgt = 0.0f;
  s->ms = OMX_ALIGN_INSTANCE_DELAY_DEFAULT;
  s->was_engaged = 1;
  s->ready = 1;
  return 1;
}

#define OMX_CONTRACT_STAGE "align-instance/clear"
/** @brief The engage-edge clear: both rings back to silence and the lines re-armed at no delay, as init left
 * them. RT-safe: two bounded memsets over the inline rings.
 * @param s The instance; null or not ready is ignored.
 * @post `cleared-line-is-fresh`: the rings read 0 at their ends and the applied delay is back at 0.
 * @thread audio */
static inline void omx_align_instance_clear(OmxAlignInstance *s) {
  if (!s || !s->ready) return;
  omx_instance_rings_clear(s->ring_l, s->ring_r, (uint32_t)OMX_ALIGN_CAP);
  (void)omx_align_arm(&s->state, s->ring_l, s->ring_r);
  OMX_POST(s->ring_l[0] == 0.0f && s->ring_r[OMX_ALIGN_CAP - 1] == 0.0f && s->state.cur == 0.0f &&
               s->state.line_l.wpos == 0u,
           "cleared-line-is-fresh");
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "align-instance/resolve"
/**
 * @brief Resolve the host's control value for one cycle. The argument is the align kernel's contract control,
 * in its user unit: `input_align_delay` (ms), clamped into INPUT_ALIGN_DELAY_RANGE, a non-finite word reading
 * as no delay; omx_align_target() turns it into the target. A null or not-ready instance is refused and
 * nothing changes.
 * @param s The instance.
 * @param bypass Non-zero makes run() the identity; the next engaged resolve clears the rings.
 * @param input_align_delay inputAlignDelay, 0 to 100 ms: how far both legs are pushed back to meet another
 *        microphone on the same source.
 * @post `the-target-is-the-kernels`: the target is omx_align_target() of the delivered delay, inside the ring.
 * @thread audio
 */
static inline void omx_align_instance_resolve(OmxAlignInstance *s, int bypass, float input_align_delay) {
  if (!s || !s->ready) return;
  const int engaged = bypass ? 0 : 1;
  if (engaged && !s->was_engaged) omx_align_instance_clear(s);
  s->was_engaged = engaged;
  s->bypass = engaged ? 0 : 1;
  s->ms = omx_clamp_or(input_align_delay, (float)OMX_INPUT_ALIGN_DELAY_RANGE_MIN, (float)OMX_INPUT_ALIGN_DELAY_RANGE_MAX,
                       OMX_ALIGN_INSTANCE_DELAY_DEFAULT);
  s->tgt = omx_align_target(s->ms, s->sr);
  OMX_POST(s->tgt >= 0.0f && s->tgt <= omx_fdelay_max_delay(s->state.cap, OMX_ALIGN_ORDER), "the-target-is-the-kernels");
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "align-instance/run"
/**
 * @brief THE AUDIO CALLBACK'S WHOLE SHARE: copy in to out where they differ, then delay both legs in place on
 * out toward the resolved target, through the console's kernel. `in_*` and `out_*` may alias. Not ready, or
 * bypassed, is the identity byte for byte.
 * @param s The instance; null, not ready or bypassed leaves `out_*` holding `in_*`.
 * @param in_l The left input, `n` frames.
 * @param in_r The right input, `n` frames.
 * @param out_l The left output, `n` frames, may be `in_l`.
 * @param out_r The right output, `n` frames, may be `in_r`.
 * @param n The block length, frames; a null port or 0 touches nothing.
 * @thread audio
 */
static inline void omx_align_instance_run(OmxAlignInstance *s, const float *in_l, const float *in_r, float *out_l,
                                          float *out_r, uint32_t n) {
  if (!omx_instance_copy_through(in_l, in_r, out_l, out_r, n)) return;
  if (!s || !s->ready || s->bypass) return;
  omx_align_apply(&s->state, out_l, out_r, n, s->tgt);
}
#undef OMX_CONTRACT_STAGE

/** @brief The frames of latency the instance introduces: none, at every rate (the alignment is the effect).
 * @param s The instance, unread.
 * @return 0.
 * @thread audio */
static inline uint32_t omx_align_instance_latency(const OmxAlignInstance *s) {
  (void)s;
  return 0u;
}

/** @brief The delay the line settles at, fractional samples, as the last resolve set it: the number a host's
 * readback or a plugin's delay readout publishes (omx_align_target() of the delivered ms).
 * @param s The instance; null or not ready reads 0.
 * @return The target, samples.
 * @thread audio */
static inline float omx_align_instance_target(const OmxAlignInstance *s) { return s && s->ready ? s->tgt : 0.0f; }

#endif /* OMX_ALIGN_INSTANCE_H */
