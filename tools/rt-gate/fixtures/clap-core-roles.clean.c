/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> */
/* omx-clap-core's LV2 adapter on the audio thread calls what its embedder provides: the audio-role predicate
   (is_audio) and the log sink (log.sink), audio by omx-clap-core.abi: green. A field no row declares is red
   (clap-core-roles.c). */
// abi: lv2 omx-clap-core
// expect: 0
#include "lv2-mini.h"
struct lv2_audio_role { int (*is_audio)(void *ctx); void *ctx; };
struct log_options { void (*sink)(void *ctx, const char *type, const char *line); void *ctx; };
struct plug { struct lv2_audio_role role; struct { struct log_options log; } o; int on; float *out; };
static void connect_port(LV2_Handle h, uint32_t port, void *d) { (void)port; ((struct plug *)h)->out = d; }
static void run(LV2_Handle h, uint32_t n) {
  struct plug *p = h; (void)n;
  const struct lv2_audio_role r = p->role;
  p->on = r.is_audio ? r.is_audio(r.ctx) : -1;
  if (p->o.log.sink) p->o.log.sink(p->o.log.ctx, 0, "line");
}
FIXTURE_DESCRIPTOR(run, connect_port, 0)
