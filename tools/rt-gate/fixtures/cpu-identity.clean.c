/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> */
/* An LV2 run asks which CPU it is on (sched_getcpu), class `cpu-identity`, green. */
// abi: lv2
// expect: 0
#define _GNU_SOURCE
#include <sched.h>
#include "lv2-mini.h"
struct plug { int cpu; float *out; };
static void connect_port(LV2_Handle h, uint32_t port, void *d) { (void)port; ((struct plug *)h)->out = d; }
static void run(LV2_Handle h, uint32_t n) { struct plug *p = h; (void)n; p->cpu = sched_getcpu(); }
FIXTURE_DESCRIPTOR(run, connect_port, 0)
