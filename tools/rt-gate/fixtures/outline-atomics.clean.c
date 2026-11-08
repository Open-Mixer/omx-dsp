/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> */
/* An atomic add in an LV2 run, as gcc emits it on aarch64 (-moutline-atomics, the default there): a call
 * to libgcc's __aarch64_ldadd<size>_<order> helper, an LSE add or an exclusive load and store loop, one
 * word, lock-free. Class `word-atomic`, green. The calls are written out so every target's compiler emits
 * them: on x86_64 the same add is inline and no call reaches the gate at all. */
// abi: lv2
// expect: 0
#include <stdint.h>
#include "lv2-mini.h"
extern int __aarch64_ldadd4_relax(int, int *);
extern int __aarch64_ldadd4_rel(int, int *);
extern long __aarch64_ldadd8_rel(long, long *);
struct plug { int count; int notes; long fed; float *out; };
static void connect_port(LV2_Handle h, uint32_t port, void *d) { (void)port; ((struct plug *)h)->out = d; }
static void run(LV2_Handle h, uint32_t n) {
  struct plug *p = h;
  __aarch64_ldadd4_relax(1, &p->notes);
  __aarch64_ldadd4_rel((int)n, &p->count);
  __aarch64_ldadd8_rel((long)n, &p->fed);
}
FIXTURE_DESCRIPTOR(run, connect_port, 0)
