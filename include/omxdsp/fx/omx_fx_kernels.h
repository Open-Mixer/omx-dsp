// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * @file omx_fx_kernels.h
 * @brief Every kernel the generic face (omx_fx_face.h) serves, by index and by name: the one list of them.
 *
 * A kernel joins the generic face by its descriptor's entry here; tools/face-descriptor-check.sh holds each
 * descriptor to its kernel file and test/fx/face.test.c holds the face over it to its hand-written face while
 * that face stays (docs/generic-face.md).
 */
#ifndef OMX_FX_KERNELS_H
#define OMX_FX_KERNELS_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <omxdsp/omx_clipper.h>
#include <omxdsp/omx_kernel_desc.h>
#include <omxdsp/omx_trim.h>
#include <omxdsp/omx_width.h>

/**
 * @brief The served kernel at index `i`, in name order.
 * @param i The index, from 0.
 * @return The kernel's descriptor; null past the last.
 * @thread control
 */
static inline const struct omx_kernel_desc *omx_fx_kernel_at(uint32_t i) {
  switch (i) {
  case 0: return omx_clipper_desc();
  case 1: return omx_trim_desc();
  case 2: return omx_width_desc();
  default: return NULL;
  }
}

/**
 * @brief The served kernel of a name.
 * @param name The kernel's name, its kernel file's (`width`).
 * @return The kernel's descriptor; null for a null name or a kernel the face does not serve.
 * @thread control
 */
static inline const struct omx_kernel_desc *omx_fx_kernel(const char *name) {
  const struct omx_kernel_desc *d;
  if (!name) return NULL;
  for (uint32_t i = 0; (d = omx_fx_kernel_at(i)) != NULL; i++)
    if (strcmp(d->name, name) == 0) return d;
  return NULL;
}

#endif /* OMX_FX_KERNELS_H */
