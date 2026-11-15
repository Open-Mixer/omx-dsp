/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> */
/* strnlen in an LV2 run, bounded by its buffer: class `bulk-memory`, green. strlen, unbounded, has no row. */
// abi: lv2
// expect: 0
#include <string.h>
#include "lv2-mini.h"
struct plug { char line[64]; size_t n; float *out; };
static void connect_port(LV2_Handle h, uint32_t port, void *d) { (void)port; ((struct plug *)h)->out = d; }
static void run(LV2_Handle h, uint32_t n) { struct plug *p = h; (void)n; p->n = strnlen(p->line, sizeof(p->line)); }
FIXTURE_DESCRIPTOR(run, connect_port, 0)
