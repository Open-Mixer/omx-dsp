// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * @file omx_fx_face.h
 * @brief The generic face: one plugin instance surface for every kernel, from its descriptor
 *        (omx_kernel_desc.h), in place of a hand-written instance face (omx_width_instance.h and its kin). No format in it.
 *
 * Five calls: init, resolve, run, clear and latency, and the meter read. The face resolves the host's
 * values the way every hand-written face does, from the descriptor's rows: a travel clamped into its
 * range, a non-finite word reading as its default; a choice rounded to its index, a word that is no
 * index of it reading as its default. It then hands the kernel the resolved values in the declared
 * order. A run copies in to out where they differ and, engaged, runs the kernel in place on out.
 *
 * The cost over a hand-written face is one call through a function pointer per block and one per
 * resolve. No allocation, no lock: the face and the kernel's state are the caller's
 * (docs/generic-face.md).
 */
#ifndef OMX_FX_FACE_H
#define OMX_FX_FACE_H

#include <math.h>
#include <stdint.h>
#include <string.h>

#include <omxdsp/fx/omx_instance_io.h>
#include <omxdsp/omx_kernel_desc.h>
#include <omxdsp/omx_param.h>

/** @brief One instance of the generic face: the kernel it serves, the caller's state for it, and its two switches. */
struct omx_fx_face {
  const struct omx_kernel_desc *desc; /**< The kernel served; init sets it. */
  void *state;                        /**< The caller's kernel state, `desc->state_size` bytes. */
  int ready;                          /**< Non-zero once init bound the kernel to a usable rate; while zero, resolve does nothing and run leaves out holding in. */
  int bypass;                         /**< Non-zero while bypassed, as the last resolve set it; run then leaves out holding in. */
};

/**
 * @brief A host's word for one control, resolved: a travel clamped into its range, a non-finite word its
 *        default; a choice rounded to the nearest index, a word that rounds to no index its default.
 * @param p The control's row.
 * @param v The host's word, any float.
 * @return The resolved value, inside [p->min, p->max]; a choice's an integral float.
 * @thread audio
 */
static inline float omx_fx_face_word(const struct omx_param_desc *p, float v) {
  if (p->kind != OMX_PARAM_CHOICE) return omx_clamp_or(v, p->min, p->max, p->def);
  if (!(v >= p->min - 0.5f && v <= p->max + 0.5f)) return p->def;
  const float i = (float)lrintf(v);
  return i >= p->min && i <= p->max ? i : p->def;
}

/**
 * @brief Resolve the host's values for one cycle and hand them to the kernel. `bypass` non-zero makes run()
 *        the identity.
 * @param f The face; null or not ready leaves it as it is.
 * @param bypass Non-zero makes run() the identity.
 * @param values The kernel's controls in their declared order and user units, `desc->n_params` of them; null
 *        reads every control's default.
 * @thread audio
 */
static inline void omx_fx_face_resolve(struct omx_fx_face *f, int bypass, const float *values) {
  if (!f || !f->ready) return;
  const struct omx_kernel_desc *d = f->desc;
  float v[OMX_KERNEL_PARAMS_MAX];
  for (uint32_t i = 0; i < d->n_params; i++) v[i] = omx_fx_face_word(&d->params[i], values ? values[i] : d->params[i].def);
  f->bypass = bypass ? 1 : 0;
  d->set(f->state, v, f->bypass);
}

/**
 * @brief Bind a face to its kernel and a rate: the state zeroed and bound, every control at its default,
 *        engaged. Allocates nothing.
 * @param f The face; every word of it is written.
 * @param desc The kernel to serve; a descriptor without init, set, process or latency, or with more than
 *        OMX_KERNEL_PARAMS_MAX controls, is refused.
 * @param state The caller's kernel state, `desc->state_size` bytes; every byte of it is written.
 * @param sr The sample rate, Hz; the kernel says which it takes.
 * @return 1 when usable, 0 when refused (a null argument, a descriptor the face cannot serve, or a rate the
 *         kernel refuses): `ready` stays clear and run() leaves out holding in.
 * @thread control
 */
static inline int omx_fx_face_init(struct omx_fx_face *f, const struct omx_kernel_desc *desc, void *state, float sr) {
  if (!f) return 0;
  memset(f, 0, sizeof(*f));
  if (!desc || !state || !desc->init || !desc->set || !desc->process || !desc->latency ||
      desc->n_params > OMX_KERNEL_PARAMS_MAX) return 0;
  f->desc = desc;
  f->state = state;
  memset(state, 0, desc->state_size);
  if (!desc->init(state, sr)) return 0;
  f->ready = 1;
  omx_fx_face_resolve(f, 0, NULL);
  return 1;
}

/**
 * @brief THE AUDIO CALLBACK'S WHOLE SHARE: copy in to out where they differ, then, engaged, the kernel over
 *        both legs in place on out: one call through the descriptor per block. `in_*` and `out_*` may alias.
 * @param f The face; null, not ready or bypassed leaves `out_*` holding `in_*`.
 * @param in_l The left input, `n` frames.
 * @param in_r The right input, `n` frames.
 * @param out_l The left output, `n` frames, may be `in_l`.
 * @param out_r The right output, `n` frames, may be `in_r`.
 * @param n The block length, frames; a null port or 0 touches nothing.
 * @thread audio
 */
static inline void omx_fx_face_run(struct omx_fx_face *f, const float *in_l, const float *in_r, float *out_l, float *out_r,
                                   uint32_t n) {
  if (!omx_instance_copy_through(in_l, in_r, out_l, out_r, n)) return;
  if (!f || !f->ready || f->bypass) return;
  f->desc->process(f->state, out_l, out_r, n);
}

/**
 * @brief Silence the kernel's memory (its rings, filters and ramps), keeping its resolved values: the
 *        re-enable recipe. Bounded work, no allocation.
 * @param f The face; null, not ready, or a kernel with no memory is ignored.
 * @thread audio
 */
static inline void omx_fx_face_clear(struct omx_fx_face *f) {
  if (!f || !f->ready || !f->desc->clear) return;
  f->desc->clear(f->state);
}

/**
 * @brief The frames of latency the kernel introduces at its resolved values.
 * @param f The face.
 * @return The kernel's latency; 0 for a null or not-ready face.
 * @thread audio
 */
static inline uint32_t omx_fx_face_latency(const struct omx_fx_face *f) {
  if (!f || !f->ready) return 0u;
  return f->desc->latency(f->state);
}

/**
 * @brief One of the kernel's readouts, as its last block left it.
 * @param f The face.
 * @param i The readout's index in the descriptor's `meters`.
 * @return The readout, in its unit; 0 for a null or not-ready face or an index past the last.
 * @thread audio
 */
static inline float omx_fx_face_meter(const struct omx_fx_face *f, uint32_t i) {
  if (!f || !f->ready || i >= f->desc->n_meters || !f->desc->meter) return 0.0f;
  return f->desc->meter(f->state, i);
}

#endif /* OMX_FX_FACE_H */
