// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * idelay_instance.test.c — the output delay instance face (omx_idelay_instance.h), every arm at every
 * rate of the nine oracle rates (OMX_FX_ORACLE_RATES, 32 to 192 kHz; the six declared ones are among them):
 *   A  a rate outside the nine is refused and the face is the identity; the published latency is zero;
 *   B  bypassed is the identity byte for byte, in place and out of place;
 *   C  engaged, the instance IS the kernel: bit-identical to omx_idelay_pair_apply over caller rings at
 *      omx_idelay_samples() of the same delay, steady and across moves, at no delay, a sub-sample delay,
 *      a whole number of samples and the ceiling;
 *   D  the closed form: at 10 ms (a whole sr / 100 samples at all nine rates), a fresh instance's first
 *      block is the cross-fade from tap 0 to tap D written out here, (1 - (i + 1) / n) x in[i] while the
 *      new tap still reads silence; after it a unit impulse on each leg arrives exactly D samples late, at
 *      exactly 1, and nothing arrives anywhere else; the readout is D;
 *   E  every hostile host value lands inside OUTPUT_DELAY_RANGE (a non-finite word at no delay), the
 *      delay is omx_idelay_samples() of where it landed, and the output is finite;
 *   F  a bypass->engaged edge clears the rings: the re-engaged instance is a fresh one;
 *   G  in == out (an aliased port) is the out-of-place answer, and an instance run between two others
 *      leaves each the instance alone (no shared state);
 *   H  init, resolve (re-engage included) and run allocate nothing: malloc, calloc, realloc and free are
 *      wrapped at link time and counted across the instance's whole life.
 *   make test-fx
 */
#include <stdlib.h>

#define OMX_CONTRACT_STORAGE 1

#include <omxdsp/fx/omx_idelay_instance.h>

#include "instance_harness.h"

/* Outside the console's declaration each call below records its kernels' `rate-is-declared`; the ledger is taken after every call. */
#define omx_idelay_instance_run(...) OMX_FX_CHECKPOINTED(omx_idelay_instance_run, __VA_ARGS__)
#define omx_idelay_instance_init(...) OMX_FX_CHECKPOINTED_INT(omx_idelay_instance_init, __VA_ARGS__)
#define omx_idelay_instance_resolve(...) OMX_FX_CHECKPOINTED(omx_idelay_instance_resolve, __VA_ARGS__)
#define omx_idelay_pair_apply(...) OMX_FX_CHECKPOINTED(omx_idelay_pair_apply, __VA_ARGS__)

/* omx_idelay_samples() records `rate-is-declared` off the declaration too: its answer passed through. */
static uint32_t samples_at(float ms) {
  const uint32_t d = omx_idelay_samples(ms, g_sr, (uint32_t)OMX_IDELAY_CAP);
  ledger_checkpoint();
  return d;
}

#define BLK 256u
#define NBLK 12

/* the allocation counter: fx_harness.h (linked with --wrap=malloc,calloc,realloc,free) */
#include "fx_harness.h"

/* Heap-held: an instance carries its two 1 s rings. */
static OmxIdelayInstance *inst(void) { return calloc(1, sizeof(OmxIdelayInstance)); }

static void arm_refused(void) {
  OmxIdelayInstance *s = inst();
  ok(omx_idelay_instance_init(s, 0.0f) == 0, "A: rate 0 refused");
  ok(omx_idelay_instance_init(s, NAN) == 0, "A: NaN rate refused");
  ok(omx_idelay_instance_init(s, g_sr + 1.0f) == 0, "A: a rate outside the nine refused");
  ok(omx_idelay_instance_init(s, 22050.0f) == 0, "A: 22.05 kHz, below the nine, refused");
  ok(omx_idelay_instance_init(s, 384000.0f) == 0, "A: 384 kHz, above the nine, refused");
  ok(omx_idelay_instance_init(NULL, g_sr) == 0, "A: a null instance refused");
  omx_idelay_instance_resolve(s, 0, 3.0f);
  float l[BLK], r[BLK], ol[BLK], or_[BLK];
  programme(l, r, BLK, 0);
  omx_idelay_instance_run(s, l, r, ol, or_, BLK);
  ok(same_bytes(l, ol, BLK) && same_bytes(r, or_, BLK), "A: a refused instance is the identity");
  ok(omx_idelay_instance_samples(s) == 0u, "A: a refused instance reads no delay");
  ok(omx_idelay_instance_init(s, g_sr) == 1, "A: the rate is accepted");
  ok(omx_idelay_instance_latency(s) == 0u && OMX_IDELAY_INSTANCE_LATENCY_FRAMES == 0.0f,
     "A: the published latency is zero frames");
  free(s);
  drain_violations("A: no contract broken");
}

static void arm_bypass(void) {
  OmxIdelayInstance *s = inst();
  omx_idelay_instance_init(s, g_sr);
  float l[BLK], r[BLK], ol[BLK], or_[BLK], l0[BLK], r0[BLK];
  int same = 1;
  for (int b = 0; b < NBLK; b++) {
    programme(l, r, BLK, (uint32_t)b * BLK);
    memcpy(l0, l, sizeof l); memcpy(r0, r, sizeof r);
    omx_idelay_instance_resolve(s, 1, 2.5f);
    omx_idelay_instance_run(s, l, r, ol, or_, BLK);
    same &= same_bytes(ol, l0, BLK) && same_bytes(or_, r0, BLK);
    omx_idelay_instance_run(s, l, r, l, r, BLK);
    same &= same_bytes(l, l0, BLK) && same_bytes(r, r0, BLK);
  }
  ok(same, "B: bypassed is the identity, in place and out of place");
  free(s);
  drain_violations("B: no contract broken");
}

static void arm_is_the_kernel(void) {
  /* each a pair of delays, ms: the first half of the run at one, the second at the other (a move) */
  static const float MS[][2] = {{0.0f, 0.0f}, {0.004f, 0.004f}, {1.2345f, 0.37f}, {10.0f, 10.0f},
                                {2.0f, 0.0f},  {1000.0f, 1000.0f}, {5000.0f, 1.0f}};
  float *rl = calloc(OMX_IDELAY_CAP, sizeof(float)), *rr = calloc(OMX_IDELAY_CAP, sizeof(float));
  for (int k = 0; k < (int)(sizeof MS / sizeof MS[0]); k++) {
    OmxIdelayInstance *s = inst();
    omx_idelay_instance_init(s, g_sr);
    memset(rl, 0, OMX_IDELAY_CAP * sizeof(float));
    memset(rr, 0, OMX_IDELAY_CAP * sizeof(float));
    struct omx_idelay_pair kp = {rl, rr, (uint32_t)OMX_IDELAY_CAP, 0u, 0u, 0u, 0u};
    float l[BLK], r[BLK], ol[BLK], or_[BLK], l0[BLK];
    int same = 1, wet = 0, delay = 1;
    for (int b = 0; b < NBLK; b++) {
      const float ms = MS[k][b < NBLK / 2 ? 0 : 1];
      const uint32_t want = samples_at(ms > 1000.0f ? 1000.0f : ms);
      programme(l, r, BLK, (uint32_t)b * BLK);
      memcpy(l0, l, sizeof l);
      omx_idelay_instance_resolve(s, 0, ms);
      delay &= omx_idelay_instance_samples(s) == want;
      omx_idelay_instance_run(s, l, r, ol, or_, BLK);
      omx_idelay_pair_apply(l, r, BLK, &kp, want, want);
      same &= same_bytes(ol, l, BLK) && same_bytes(or_, r, BLK);
      wet |= !same_bytes(ol, l0, BLK);
    }
    ok(delay, "C: the face's delay is omx_idelay_samples of the delay, the kernel's own conversion");
    ok(wet || samples_at(MS[k][0]) + samples_at(MS[k][1]) == 0u,
       "C: a delay of a sample or more is not a wire (the comparison is not of two passthroughs)");
    ok(same, "C: the engaged instance is omx_idelay_pair_apply at omx_idelay_samples, bit for bit");
    free(s);
  }
  free(rl); free(rr);
  drain_violations("C: no contract broken");
}

static void arm_closed_form(void) {
  OmxIdelayInstance *s = inst();
  omx_idelay_instance_init(s, g_sr);
  const uint32_t D = (uint32_t)lrintf(g_sr / 100.0f); /* 10 ms, whole at all nine rates */
  ok((float)D * 100.0f == g_sr && D >= BLK, "D: 10 ms is a whole number of samples at this rate, a block or more");
  omx_idelay_instance_resolve(s, 0, 10.0f);
  ok(omx_idelay_instance_samples(s) == D, "D: the delay is sr / 100 samples, exactly");
  /* the first block cross-fades from tap 0 (the sample just written) to tap D (still silence) */
  float l[BLK], r[BLK], ol[BLK], or_[BLK];
  programme(l, r, BLK, 0);
  omx_idelay_instance_run(s, l, r, ol, or_, BLK);
  int fade = 1;
  for (uint32_t i = 0; i < BLK; i++) {
    const float t = (float)(i + 1) / (float)BLK;
    fade &= ol[i] == l[i] * (1.0f - t) + 0.0f * t && or_[i] == r[i] * (1.0f - t) + 0.0f * t;
  }
  ok(fade, "D: a fresh instance's first block is the cross-fade from tap 0 to tap D, written out");
  /* then a steady D: an impulse after the fade's block arrives D samples late, exactly */
  const uint32_t n = 2u * D + BLK;
  float *il = calloc(n, sizeof(float)), *ir = calloc(n, sizeof(float));
  il[BLK] = 1.0f;
  ir[BLK] = -1.0f;
  for (uint32_t o = 0; o < n; o += BLK) omx_idelay_instance_run(s, il + o, ir + o, il + o, ir + o, n - o < BLK ? n - o : BLK);
  ok(il[BLK + D] == 1.0f && ir[BLK + D] == -1.0f, "D: the impulse arrives exactly D samples late, at exactly 1, both legs");
  /* index j of this run is frame BLK + j of the instance's life: the fade block (frames 0 .. BLK - 1) arrives at
   * j = D - BLK .. D - 1, the impulse (frame 2 BLK) at j = BLK + D, and nothing else is in the line */
  double rest = 0.0;
  for (uint32_t i = 0; i < n; i++)
    if (i != BLK + D && !(i + BLK >= D && i < D)) rest += fabs((double)il[i]) + fabs((double)ir[i]);
  ok(rest == 0.0, "D: nothing arrives anywhere else but the first block, D samples late");
  int echo = 1;
  for (uint32_t i = 0; i < BLK; i++) echo &= il[D - BLK + i] == l[i] && ir[D - BLK + i] == r[i];
  ok(echo, "D: and the first block arrives D samples late, bit for bit");
  free(il); free(ir);
  free(s);
  drain_violations("D: no contract broken");
}

static void arm_clamps(void) {
  static const float EXTRA[] = {500.0f, 1000.0f, 1000.5f, 1e-7f, -1e-7f};
  float l[BLK], r[BLK];
  for (int h = 0; h < HOSTILE_COUNT + (int)(sizeof EXTRA / sizeof EXTRA[0]); h++) {
    const float x = h < HOSTILE_COUNT ? HOSTILE[h] : EXTRA[h - HOSTILE_COUNT];
    OmxIdelayInstance *s = inst();
    omx_idelay_instance_init(s, g_sr);
    omx_idelay_instance_resolve(s, 0, x);
    const float want = land(x, (float)OMX_OUTPUT_DELAY_RANGE_MIN, (float)OMX_OUTPUT_DELAY_RANGE_MAX,
                            OMX_IDELAY_INSTANCE_DELAY_DEFAULT);
    char what[128];
    snprintf(what, sizeof what, "E: outputDelay at %g lands at %g ms, at omx_idelay_samples of it", (double)x,
             (double)want);
    ok(s->ms == want && omx_idelay_instance_samples(s) == samples_at(want), what);
    programme(l, r, BLK, (uint32_t)h * BLK);
    omx_idelay_instance_run(s, l, r, l, r, BLK);
    ok(all_finite(l, BLK) && all_finite(r, BLK), "E: hostile controls leave the output finite");
    free(s);
  }
  drain_violations("E: no contract broken");
}

static void arm_reengage_clears(void) {
  OmxIdelayInstance *s = inst(), *fresh = inst();
  omx_idelay_instance_init(s, g_sr);
  omx_idelay_instance_init(fresh, g_sr);
  float l[BLK], r[BLK], fl[BLK], fr[BLK];
  /* fill the rings past their length, so a ring left uncleared holds signal where the re-engaged line reads */
  for (int b = 0; b < (int)(OMX_IDELAY_CAP / BLK) + 2; b++) {
    programme(l, r, BLK, (uint32_t)b * BLK);
    omx_idelay_instance_resolve(s, 0, 1.5f);
    omx_idelay_instance_run(s, l, r, l, r, BLK);
  }
  omx_idelay_instance_resolve(s, 1, 1.5f);
  int same = 1;
  for (int b = 0; b < NBLK; b++) {
    programme(l, r, BLK, (uint32_t)(b + 40) * BLK);
    memcpy(fl, l, sizeof l); memcpy(fr, r, sizeof r);
    omx_idelay_instance_resolve(s, 0, 1.5f);
    omx_idelay_instance_resolve(fresh, 0, 1.5f);
    omx_idelay_instance_run(s, l, r, l, r, BLK);
    omx_idelay_instance_run(fresh, fl, fr, fl, fr, BLK);
    same &= same_bytes(l, fl, BLK) && same_bytes(r, fr, BLK);
  }
  ok(same, "F: a re-engaged output delay is a fresh one, its rings silent");
  free(s); free(fresh);
  drain_violations("F: no contract broken");
}

static void arm_alias_and_independence(void) {
  OmxIdelayInstance *a = inst(), *b = inst(), *c = inst(), *alone = inst();
  omx_idelay_instance_init(a, g_sr);
  omx_idelay_instance_init(b, g_sr);
  omx_idelay_instance_init(c, g_sr);
  omx_idelay_instance_init(alone, g_sr);
  float l[BLK], r[BLK], al[BLK], ar[BLK], bl[BLK], br[BLK], cl[BLK], cr[BLK], xl[BLK], xr[BLK];
  int alias = 1, indep = 1;
  for (int k = 0; k < NBLK; k++) {
    programme(l, r, BLK, (uint32_t)k * BLK);
    omx_idelay_instance_resolve(a, 0, 0.7f);
    omx_idelay_instance_resolve(b, 0, 0.7f);
    omx_idelay_instance_resolve(alone, 0, 0.7f);
    omx_idelay_instance_resolve(c, 0, 3.3f);
    omx_idelay_instance_run(a, l, r, al, ar, BLK);
    programme(cl, cr, BLK, (uint32_t)(k + 77) * BLK);
    omx_idelay_instance_run(c, cl, cr, cl, cr, BLK); /* a different one between */
    memcpy(bl, l, sizeof l); memcpy(br, r, sizeof r);
    omx_idelay_instance_run(b, bl, br, bl, br, BLK);
    omx_idelay_instance_run(alone, l, r, xl, xr, BLK);
    alias &= same_bytes(al, bl, BLK) && same_bytes(ar, br, BLK);
    indep &= same_bytes(al, xl, BLK) && same_bytes(ar, xr, BLK);
  }
  ok(alias, "G: in place is the out-of-place answer");
  ok(indep, "G: an instance between two others leaves them each the instance alone");
  free(a); free(b); free(c); free(alone);
  drain_violations("G: no contract broken");
}

static void arm_no_allocation(void) {
  static OmxIdelayInstance s;
  float l[BLK], r[BLK];
  g_allocs = 0;
  g_counting = 1;
  const int ready = omx_idelay_instance_init(&s, g_sr);
  for (int b = 0; b < NBLK; b++) {
    programme(l, r, BLK, (uint32_t)b * BLK);
    omx_idelay_instance_resolve(&s, b == 5, b < 6 ? 0.4f : 800.0f);
    omx_idelay_instance_run(&s, l, r, l, r, BLK);
  }
  g_counting = 0;
  ok(ready, "H: the instance came up");
  ok(g_allocs == 0, "H: init, resolve, a re-engage and run allocate nothing");
  /* The counter is live: an allocation inside the window is counted. Called through volatile
   * pointers, so the compiler cannot fold the pair away. */
  void *(*volatile alloc)(size_t) = malloc;
  void (*volatile release)(void *) = free;
  g_counting = 1;
  release(alloc(16));
  g_counting = 0;
  ok(g_allocs == 2, "H: the allocation counter counts (a malloc and a free inside the window)");
  drain_violations("H: no contract broken");
}

int main(void) {
  omx_fx_require_rate_floor();
  omx_fx_require_full_rates_cover_declared();
  for (int ri = 0; ri < (int)OMX_FX_ORACLE_RATE_COUNT; ri++) {
    g_sr = OMX_FX_ORACLE_RATES[ri];
    arm_refused();
    arm_bypass();
    arm_is_the_kernel();
    arm_closed_form();
    arm_clamps();
    arm_reengage_clears();
    arm_alias_and_independence();
    arm_no_allocation();
  }
  return finish("idelay_instance");
}
