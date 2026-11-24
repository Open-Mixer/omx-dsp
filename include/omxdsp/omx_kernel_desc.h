// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * @file omx_kernel_desc.h
 * @brief A kernel's descriptor: its controls, readouts and state as data, and the functions the generic face
 *        (fx/omx_fx_face.h) calls through.
 *
 * A kernel publishes one, built from its kernel file (data/kernels/width.json for the width) through the render
 * (omxdsp_definitions.h): each control's name, kind, range and default, its unit as a PARAM_UNITS id
 * (omxcontract/omx_vocabulary.h), each readout, and the kernel's init, set, process, clear, latency and
 * meter functions. tools/face-descriptor-check.sh holds every served descriptor to its kernel file
 * (docs/generic-face.md).
 */
#ifndef OMX_KERNEL_DESC_H
#define OMX_KERNEL_DESC_H

#include <stddef.h>
#include <stdint.h>

/** @brief The most controls a descriptor may carry: the face resolves them into a buffer of this many on the stack. */
#define OMX_KERNEL_PARAMS_MAX 32u

/** @brief A control's kind, as its kernel file declares it. */
enum omx_param_kind {
  OMX_PARAM_TRAVEL = 0, /**< A number in a user unit, clamped into [min, max]. */
  OMX_PARAM_CHOICE = 1  /**< An index into the control's ids, 0 to max. */
};

/** @brief One control: the kernel file's `controls[i]`, in the declared order. */
struct omx_param_desc {
  const char *name;         /**< The contract name, `pseudoMs`. */
  enum omx_param_kind kind; /**< Travel or choice. */
  float min;                /**< A travel's least value; a choice's first index, 0. */
  float max;                /**< A travel's greatest value; a choice's last index. */
  float def;                /**< The come-up value; a choice's default index. */
  const char *unit;         /**< The PARAM_UNITS id, `OMX_PARAM_UNITS_PERCENT`; `OMX_PARAM_UNITS_NONE` for a choice. */
};

/** @brief One readout: the kernel file's `readouts[i]`, in the declared order. */
struct omx_meter_desc {
  const char *name; /**< The readout's name, `gainReductionDb`. */
  const char *unit; /**< The PARAM_UNITS id. */
  float min;        /**< The least value it reads. */
  float max;        /**< The greatest value it reads. */
};

/** @brief A kernel as the generic face serves it: its controls and readouts as data, its state's size and its functions. */
struct omx_kernel_desc {
  const char *name;                     /**< The kernel's name, its kernel file's: `width` for data/kernels/width.json. */
  size_t state_size;                    /**< The bytes of the kernel's state, which the caller owns. */
  uint32_t n_params;                    /**< Rows in `params`, at most OMX_KERNEL_PARAMS_MAX. */
  const struct omx_param_desc *params;  /**< The controls, in the declared order. */
  uint32_t n_meters;                    /**< Rows in `meters`. */
  const struct omx_meter_desc *meters;  /**< The readouts, in the declared order. */
  int (*init)(void *state, float sr);   /**< Binds a zeroed state to a rate: 1 when usable, 0 when the rate is refused. */
  void (*set)(void *state, const float *values, int bypass); /**< Takes the resolved values, in the declared order, and the bypass. */
  void (*process)(void *state, float *l, float *r, uint32_t n); /**< One engaged block, both legs in place. */
  void (*clear)(void *state);           /**< Silences the kernel's memory, keeping its values; null when it has none. */
  uint32_t (*latency)(const void *state); /**< The frames the kernel delays at its resolved values. */
  float (*meter)(const void *state, uint32_t i); /**< Readout `i`; null when the kernel has none. */
};

#endif /* OMX_KERNEL_DESC_H */
