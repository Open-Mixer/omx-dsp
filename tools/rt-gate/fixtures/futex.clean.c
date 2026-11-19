/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> */
/* A futex wake through syscall from an LV2 run, its operation spelled at the call (FUTEX_WAKE,
   FUTEX_WAKE_PRIVATE, across lines too): class `wake`, green. Every other operation is red (futex.c). */
// abi: lv2
// expect: 0
#define _GNU_SOURCE
#include <linux/futex.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "lv2-mini.h"
struct plug { uint32_t w; float *out; };
static void connect_port(LV2_Handle h, uint32_t port, void *d) { (void)port; ((struct plug *)h)->out = d; }
static void run(LV2_Handle h, uint32_t n) {
  struct plug *p = h; (void)n;
  syscall(SYS_futex, &p->w, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
  syscall(SYS_futex, &p->w, FUTEX_WAKE, 1);
  syscall(SYS_futex,
          &p->w,
          FUTEX_WAKE_PRIVATE, 1);
}
FIXTURE_DESCRIPTOR(run, connect_port, 0)
