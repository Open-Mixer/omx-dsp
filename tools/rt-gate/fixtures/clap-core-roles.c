/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> */
/* A call through a field omx-clap-core.abi does not declare stays red, with the ABI bound (clap-core-roles.clean.c). */
// abi: lv2 omx-clap-core
// expect: 1
// expect-red: RT-INDIRECT r.is_worker at fixtures/clap-core-roles.c:13:13 via run[audio]
#include "lv2-mini.h"
struct role { int (*is_worker)(void *ctx); void *ctx; };
struct plug { struct role role; int on; float *out; };
static void connect_port(LV2_Handle h, uint32_t port, void *d) { (void)port; ((struct plug *)h)->out = d; }
static void run(LV2_Handle h, uint32_t n) {
  struct plug *p = h; const struct role r = p->role; (void)n;
  p->on = r.is_worker ? r.is_worker(r.ctx) : -1;
}
FIXTURE_DESCRIPTOR(run, connect_port, 0)
