/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> */
/* syscall in an LV2 run: red unless it is a futex wake spelled at the call (futex.clean.c). A
   wait, a wake of another kind, an operation held in a variable, another system call: each red. */
// abi: lv2
// expect: 1
// expect-red: RT-BLOCK syscall at fixtures/futex.c:20:3 via run[audio]
// expect-red: RT-BLOCK syscall at fixtures/futex.c:21:3 via run[audio]
// expect-red: RT-BLOCK syscall at fixtures/futex.c:22:3 via run[audio]
// expect-red: RT-BLOCK syscall at fixtures/futex.c:23:3 via run[audio]
#define _GNU_SOURCE
#include <linux/futex.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "lv2-mini.h"
struct plug { uint32_t w; int op; float *out; };
static void connect_port(LV2_Handle h, uint32_t port, void *d) { (void)port; ((struct plug *)h)->out = d; }
static void run(LV2_Handle h, uint32_t n) { struct plug *p = h; (void)n;
  syscall(SYS_futex, &p->w, FUTEX_WAIT_PRIVATE, 0, NULL, NULL, 0);
  syscall(SYS_futex, &p->w, FUTEX_WAKE_OP_PRIVATE, 1, NULL, &p->w, 0);
  syscall(SYS_futex, &p->w, p->op, 1);
  syscall(SYS_gettid);
}
FIXTURE_DESCRIPTOR(run, connect_port, 0)
