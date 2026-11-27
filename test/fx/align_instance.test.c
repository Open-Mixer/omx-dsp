// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * align_instance.test.c — the input alignment instance face (omx_align_instance.h), every arm at every
 * rate of the nine oracle rates (OMX_FX_ORACLE_RATES, 32 to 192 kHz; the six declared ones are among them):
 *   A  a rate outside the nine is refused and the face is the identity; the published latency is zero;
 *   B  bypassed is the identity byte for byte, in place and out of place;
 *   C  engaged, the instance IS the kernel: bit-identical to omx_align_apply over caller rings at
 *      omx_align_target() of the same delay, steady and across moves, at no delay, a fraction, a whole
 *      number of samples and the ceiling;
 *   D  the closed form: at 10 ms (a whole sr / 100 samples at all nine rates), once the slew has landed
 *      (OMX_ALIGN_SLEW per sample, so D x 32 samples), a unit impulse on each leg arrives exactly D
 *      samples late, at exactly 1, and nothing arrives anywhere else; the readout is D;
 *   E  every hostile host value lands inside INPUT_ALIGN_DELAY_RANGE (a non-finite word at no delay),
 *      the target is omx_align_target() of where it landed, and the output is finite;
 *   F  a bypass->engaged edge clears the rings: the re-engaged instance is a fresh one;
 *   G  in == out (an aliased port) is the out-of-place answer, and an instance run between two others
 *      leaves each the instance alone (no shared state);
 *   H  init, resolve (re-engage included) and run allocate nothing: malloc, calloc, realloc and free are
 *      wrapped at link time and counted across the instance's whole life.
 *   make test-fx
 */
#include <stdlib.h>

#define OMX_CONTRACT_STORAGE 1

#include <omxdsp/fx/omx_align_instance.h>

#include "instance_harness.h"

/* Outside the console's declaration each call below records its kernels' `rate-is-declared`; the ledger is taken after every call. */
#define omx_align_instance_run(...) OMX_FX_CHECKPOINTED(omx_align_instance_run, __VA_ARGS__)
#define omx_align_instance_init(...) OMX_FX_CHECKPOINTED_INT(omx_align_instance_init, __VA_ARGS__)
#define omx_align_instance_resolve(...) OMX_FX_CHECKPOINTED(omx_align_instance_resolve, __VA_ARGS__)
#define omx_align_apply(...) OMX_FX_CHECKPOINTED(omx_align_apply, __VA_ARGS__)

#define BLK 256u
#define NBLK 12

/* the allocation counter: fx_harness.h (linked with --wrap=malloc,calloc,realloc,free) */
#include "fx_harness.h"

/* Heap-held: an instance carries its two alignment rings. */
static OmxAlignInstance *inst(void) { return calloc(1, sizeof(OmxAlignInstance)); }

static void arm_refused(void) {
  OmxAlignInstance *s = inst();
  ok(omx_align_instance_init(s, 0.0f) == 0, "A: rate 0 refused");
  ok(omx_align_instance_init(s, NAN) == 0, "A: NaN rate refused");
  ok(omx_align_instance_init(s, g_sr + 1.0f) == 0, "A: a rate outside the nine refused");
  ok(omx_align_instance_init(s, 22050.0f) == 0, "A: 22.05 kHz, below the nine, refused");
  ok(omx_align_instance_init(s, 384000.0f) == 0, "A: 384 kHz, above the nine, refused");
  ok(omx_align_instance_init(NULL, g_sr) == 0, "A: a null instance refused");
  omx_align_instance_resolve(s, 0, 3.0f);
  float l[BLK], r[BLK], ol[BLK], or_[BLK];
  programme(l, r, BLK, 0);
  omx_align_instance_run(s, l, r, ol, or_, BLK);
  ok(same_bytes(l, ol, BLK) && same_bytes(r, or_, BLK), "A: a refused instance is the identity");
  ok(omx_align_instance_target(s) == 0.0f, "A: a refused instance reads no target");
  ok(omx_align_instance_init(s, g_sr) == 1, "A: the rate is accepted");
  ok(omx_align_instance_latency(s) == 0u && OMX_ALIGN_INSTANCE_LATENCY_FRAMES == 0.0f,
     "A: the published latency is zero frames");
  free(s);
  drain_violations("A: no contract broken");
}

static void arm_bypass(void) {
  OmxAlignInstance *s = inst();
  omx_align_instance_init(s, g_sr);
  float l[BLK], r[BLK], ol[BLK], or_[BLK], l0[BLK], r0[BLK];
  int same = 1;
  for (int b = 0; b < NBLK; b++) {
    programme(l, r, BLK, (uint32_t)b * BLK);
    memcpy(l0, l, sizeof l); memcpy(r0, r, sizeof r);
    omx_align_instance_resolve(s, 1, 2.5f);
    omx_align_instance_run(s, l, r, ol, or_, BLK);
    same &= same_bytes(ol, l0, BLK) && same_bytes(or_, r0, BLK);
    omx_align_instance_run(s, l, r, l, r, BLK);
    same &= same_bytes(l, l0, BLK) && same_bytes(r, r0, BLK);
  }
  ok(same, "B: bypassed is the identity, in place and out of place");
  free(s);
  drain_violations("B: no contract broken");
}

static float g_kl[OMX_ALIGN_CAP], g_kr[OMX_ALIGN_CAP];

static void arm_is_the_kernel(void) {
  /* each a pair of delays, ms: the first half of the run at one, the second at the other (a move) */
  static const float MS[][2] = {{0.0f, 0.0f}, {0.0123f, 0.0123f}, {1.2345f, 0.37f}, {10.0f, 10.0f},
                                {2.0f, 0.0f},  {100.0f, 100.0f},  {250.0f, 1.0f}};
  for (int k = 0; k < (int)(sizeof MS / sizeof MS[0]); k++) {
    OmxAlignInstance *s = inst();
    omx_align_instance_init(s, g_sr);
    struct omx_align_state ks;
    memset(&ks, 0, sizeof ks);
    memset(g_kl, 0, sizeof g_kl);
    memset(g_kr, 0, sizeof g_kr);
    ok(omx_align_arm(&ks, g_kl, g_kr) == 1, "C: the console's lines arm over caller rings");
    float l[BLK], r[BLK], ol[BLK], or_[BLK], l0[BLK];
    int same = 1, wet = 0, target = 1;
    for (int b = 0; b < NBLK; b++) {
      const float ms = MS[k][b < NBLK / 2 ? 0 : 1];
      const float want = omx_align_target(ms, g_sr);
      programme(l, r, BLK, (uint32_t)b * BLK);
      memcpy(l0, l, sizeof l);
      omx_align_instance_resolve(s, 0, ms);
      target &= omx_align_instance_target(s) == want;
      omx_align_instance_run(s, l, r, ol, or_, BLK);
      omx_align_apply(&ks, l, r, BLK, want);
      same &= same_bytes(ol, l, BLK) && same_bytes(or_, r, BLK);
      wet |= !same_bytes(ol, l0, BLK);
    }
    ok(target, "C: the face's target is omx_align_target of the delay, the kernel's own conversion");
    ok(wet || omx_align_target(MS[k][0], g_sr) + omx_align_target(MS[k][1], g_sr) == 0.0f, "C: a non-zero target is not a wire (the comparison is not of two passthroughs)");
    ok(same, "C: the engaged instance is omx_align_apply at omx_align_target, bit for bit");
    free(s);
  }
  drain_violations("C: no contract broken");
}

#define IMP_N 70000u
static float g_il[IMP_N], g_ir[IMP_N];

static void arm_closed_form(void) {
  OmxAlignInstance *s = inst();
  omx_align_instance_init(s, g_sr);
  const uint32_t D = (uint32_t)lrintf(g_sr / 100.0f); /* 10 ms, whole at all nine rates */
  ok((float)D * 100.0f == g_sr, "D: 10 ms is a whole number of samples at this rate");
  omx_align_instance_resolve(s, 0, 10.0f);
  ok(omx_align_instance_target(s) == (float)D, "D: the target is sr / 100 samples, exactly");
  /* the slew lands after D / OMX_ALIGN_SLEW = 32 D samples of silence */
  const uint32_t settle = D * 32u;
  ok(settle + 2u * D < IMP_N, "D: the buffer holds the slew and the impulse's arrival");
  memset(g_il, 0, sizeof g_il);
  memset(g_ir, 0, sizeof g_ir);
  g_il[settle] = 1.0f;
  g_ir[settle] = -1.0f;
  for (uint32_t o = 0; o < IMP_N; o += 4096u)
    omx_align_instance_run(s, g_il + o, g_ir + o, g_il + o, g_ir + o, IMP_N - o < 4096u ? IMP_N - o : 4096u);
  ok(s->state.cur == (float)D, "D: the slew has landed exactly on the target");
  ok(g_il[settle + D] == 1.0f && g_ir[settle + D] == -1.0f, "D: the impulse arrives exactly D samples late, at exactly 1, both legs");
  double rest = 0.0;
  for (uint32_t i = 0; i < IMP_N; i++)
    if (i != settle + D) rest += fabs((double)g_il[i]) + fabs((double)g_ir[i]);
  ok(rest == 0.0, "D: nothing arrives anywhere else");
  free(s);
  drain_violations("D: no contract broken");
}

static void arm_clamps(void) {
  static const float EXTRA[] = {50.0f, 100.0f, 100.5f, 1e-7f, -1e-7f};
  float l[BLK], r[BLK];
  for (int h = 0; h < HOSTILE_COUNT + (int)(sizeof EXTRA / sizeof EXTRA[0]); h++) {
    const float x = h < HOSTILE_COUNT ? HOSTILE[h] : EXTRA[h - HOSTILE_COUNT];
    OmxAlignInstance *s = inst();
    omx_align_instance_init(s, g_sr);
    omx_align_instance_resolve(s, 0, x);
    const float want = land(x, (float)OMX_INPUT_ALIGN_DELAY_RANGE_MIN, (float)OMX_INPUT_ALIGN_DELAY_RANGE_MAX,
                            OMX_ALIGN_INSTANCE_DELAY_DEFAULT);
    char what[128];
    snprintf(what, sizeof what, "E: inputAlignDelay at %g lands at %g ms, at omx_align_target of it", (double)x,
             (double)want);
    ok(s->ms == want && omx_align_instance_target(s) == omx_align_target(want, g_sr), what);
    programme(l, r, BLK, (uint32_t)h * BLK);
    omx_align_instance_run(s, l, r, l, r, BLK);
    ok(all_finite(l, BLK) && all_finite(r, BLK), "E: hostile controls leave the output finite");
    free(s);
  }
  drain_violations("E: no contract broken");
}

static void arm_reengage_clears(void) {
  OmxAlignInstance *s = inst(), *fresh = inst();
  omx_align_instance_init(s, g_sr);
  omx_align_instance_init(fresh, g_sr);
  float l[BLK], r[BLK], fl[BLK], fr[BLK];
  /* fill the rings past their length, so a ring left uncleared holds signal where the re-engaged line reads */
  for (int b = 0; b < (int)(OMX_ALIGN_CAP / BLK) + 2; b++) {
    programme(l, r, BLK, (uint32_t)b * BLK);
    omx_align_instance_resolve(s, 0, 1.5f);
    omx_align_instance_run(s, l, r, l, r, BLK);
  }
  omx_align_instance_resolve(s, 1, 1.5f);
  int same = 1;
  for (int b = 0; b < NBLK; b++) {
    programme(l, r, BLK, (uint32_t)(b + 40) * BLK);
    memcpy(fl, l, sizeof l); memcpy(fr, r, sizeof r);
    omx_align_instance_resolve(s, 0, 1.5f);
    omx_align_instance_resolve(fresh, 0, 1.5f);
    omx_align_instance_run(s, l, r, l, r, BLK);
    omx_align_instance_run(fresh, fl, fr, fl, fr, BLK);
    same &= same_bytes(l, fl, BLK) && same_bytes(r, fr, BLK);
  }
  ok(same, "F: a re-engaged alignment is a fresh one, its rings silent");
  free(s); free(fresh);
  drain_violations("F: no contract broken");
}

static void arm_alias_and_independence(void) {
  OmxAlignInstance *a = inst(), *b = inst(), *c = inst(), *alone = inst();
  omx_align_instance_init(a, g_sr);
  omx_align_instance_init(b, g_sr);
  omx_align_instance_init(c, g_sr);
  omx_align_instance_init(alone, g_sr);
  float l[BLK], r[BLK], al[BLK], ar[BLK], bl[BLK], br[BLK], cl[BLK], cr[BLK], xl[BLK], xr[BLK];
  int alias = 1, indep = 1;
  for (int k = 0; k < NBLK; k++) {
    programme(l, r, BLK, (uint32_t)k * BLK);
    omx_align_instance_resolve(a, 0, 0.7f);
    omx_align_instance_resolve(b, 0, 0.7f);
    omx_align_instance_resolve(alone, 0, 0.7f);
    omx_align_instance_resolve(c, 0, 3.3f);
    omx_align_instance_run(a, l, r, al, ar, BLK);
    programme(cl, cr, BLK, (uint32_t)(k + 77) * BLK);
    omx_align_instance_run(c, cl, cr, cl, cr, BLK); /* a different one between */
    memcpy(bl, l, sizeof l); memcpy(br, r, sizeof r);
    omx_align_instance_run(b, bl, br, bl, br, BLK);
    omx_align_instance_run(alone, l, r, xl, xr, BLK);
    alias &= same_bytes(al, bl, BLK) && same_bytes(ar, br, BLK);
    indep &= same_bytes(al, xl, BLK) && same_bytes(ar, xr, BLK);
  }
  ok(alias, "G: in place is the out-of-place answer");
  ok(indep, "G: an instance between two others leaves them each the instance alone");
  free(a); free(b); free(c); free(alone);
  drain_violations("G: no contract broken");
}

static void arm_no_allocation(void) {
  static OmxAlignInstance s;
  float l[BLK], r[BLK];
  g_allocs = 0;
  g_counting = 1;
  const int ready = omx_align_instance_init(&s, g_sr);
  for (int b = 0; b < NBLK; b++) {
    programme(l, r, BLK, (uint32_t)b * BLK);
    omx_align_instance_resolve(&s, b == 5, b < 6 ? 0.4f : 80.0f);
    omx_align_instance_run(&s, l, r, l, r, BLK);
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
  return finish("align_instance");
}
