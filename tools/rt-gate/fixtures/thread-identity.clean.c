/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> */
/* An LV2 run asks who it is: pthread_self, pthread_equal and (glibc 2.42 and later) pthread_gettid_np,
   class `thread-identity`, green. */
// abi: lv2
// expect: 0
#define _GNU_SOURCE
#include <pthread.h>
#include <sys/types.h>
#include "lv2-mini.h"
struct plug { pthread_t owner; pid_t tid; float *out; };
static void connect_port(LV2_Handle h, uint32_t port, void *d) { (void)port; ((struct plug *)h)->out = d; }
static void run(LV2_Handle h, uint32_t n) {
  struct plug *p = h; (void)n;
  if (!pthread_equal(p->owner, pthread_self())) {
    p->owner = pthread_self();
#if defined(__GLIBC__) && __GLIBC_PREREQ(2, 42)
    p->tid = pthread_gettid_np(p->owner);
#endif
  }
}
FIXTURE_DESCRIPTOR(run, connect_port, 0)
