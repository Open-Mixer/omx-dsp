// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * face-descriptor-dump: every kernel the generic face serves (omx_fx_kernel_at()), its descriptor printed
 * one row per line, the form tools/face-descriptor-check.sh renders the kernel's data/kernels/<k>.json to:
 *
 *   kernel <name>
 *   param <name> travel|choice <min> <max> <default> <PARAM_UNITS id>
 *   meter <name> <PARAM_UNITS id> <min> <max>
 *
 * Numbers in %.7g, a float's digits, the form the check prints the kernel file's numbers in.
 */
#include <stdio.h>

#include <omxdsp/fx/omx_fx_kernels.h>

int main(void) {
  const struct omx_kernel_desc *d;
  for (uint32_t k = 0; (d = omx_fx_kernel_at(k)) != NULL; k++) {
    printf("kernel %s\n", d->name);
    for (uint32_t i = 0; i < d->n_params; i++) {
      const struct omx_param_desc *p = &d->params[i];
      printf("param %s %s %.7g %.7g %.7g %s\n", p->name, p->kind == OMX_PARAM_CHOICE ? "choice" : "travel", (double)p->min,
             (double)p->max, (double)p->def, p->unit);
    }
    for (uint32_t i = 0; i < d->n_meters; i++) {
      const struct omx_meter_desc *m = &d->meters[i];
      printf("meter %s %s %.7g %.7g\n", m->name, m->unit, (double)m->min, (double)m->max);
    }
  }
  return 0;
}
