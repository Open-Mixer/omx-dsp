// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
//
// The generic face (fx/omx_fx_face.h) over each kernel it serves (fx/omx_fx_kernels.h) against that kernel's
// hand-written face, at each of the nine oracle rates:
//   A  init answers as the hand-written face at every rate, the oracle set's and refused ones alike;
//   B  bit for bit: a script of host values, block by block, the travels walked past both ends and
//      through non-finite words, the choices through fractional, negative and out-of-set indices, the
//      bypass switched on and off, in place and out of place: the output, every meter after each block
//      and the latency equal the hand-written face's;
//   C  a choice word that is no number, or past any index, reads as the declared default, as the host's
//      default index does on the hand-written face;
//   D  the refusals: a null face, descriptor, state or port, an empty block, a face not ready;
//   E  clear: a trim lands its ramp, a width ring is silenced, a clipper's meter reads 0;
//   F  the served list: every kernel is found by its name and holds a hand-written twin here, nothing
//      allocates and the ledger is clean.
#define OMX_CONTRACT_STORAGE 1
#include <omxdsp/fx/omx_clipper_instance.h>
#include <omxdsp/fx/omx_fx_face.h>
#include <omxdsp/fx/omx_fx_kernels.h>
#include <omxdsp/fx/omx_trim_instance.h>
#include <omxdsp/fx/omx_width_instance.h>

#include "instance_oracle.h"
#include "instance_no_alloc.h"

#include <stdlib.h>

#define MAXN 512u
#define BLOCKS 96
static float g_l[MAXN], g_r[MAXN], g_hl[MAXN], g_hr[MAXN], g_fl[MAXN], g_fr[MAXN];

/* ---- each kernel's hand-written face, behind one shape ------------------------------------------- */

/* The host's integer for a choice word, as a plugin binding passes it ((int)lrintf), for a word the
 * conversion is defined for. */
static int idx(float v) { return (int)lrintf(v); }

static OmxTrimInstance h_trim;
static OmxWidthInstance h_width;
static OmxClipperInstance h_clipper;

struct twin {
  const char *name;
  void *hand;
  int (*init)(void *h, float sr);
  void (*resolve)(void *h, int bypass, const float *v);
  void (*run)(void *h, const float *il, const float *ir, float *ol, float *orr, uint32_t n);
  uint32_t (*latency)(const void *h);
  float (*meter)(const void *h, uint32_t i);
};

static int trim_init(void *h, float sr) { return omx_trim_instance_init(h, sr); }
static void trim_resolve(void *h, int bypass, const float *v) { omx_trim_instance_resolve(h, bypass, v[0]); }
static void trim_run(void *h, const float *il, const float *ir, float *ol, float *orr, uint32_t n) {
  omx_trim_instance_run(h, il, ir, ol, orr, n);
}
static uint32_t trim_latency(const void *h) { return omx_trim_instance_latency(h); }

static int width_init(void *h, float sr) { return omx_width_instance_init(h, sr); }
static void width_resolve(void *h, int bypass, const float *v) {
  omx_width_instance_resolve(h, bypass, v[0], idx(v[1]), v[2], idx(v[3]), v[4]);
}
static void width_run(void *h, const float *il, const float *ir, float *ol, float *orr, uint32_t n) {
  omx_width_instance_run(h, il, ir, ol, orr, n);
}
static uint32_t width_latency(const void *h) { return omx_width_instance_latency(h); }

static int clipper_init(void *h, float sr) { return omx_clipper_instance_init(h, sr); }
static void clipper_resolve(void *h, int bypass, const float *v) { omx_clipper_instance_resolve(h, bypass, v[0], v[1], v[2]); }
static void clipper_run(void *h, const float *il, const float *ir, float *ol, float *orr, uint32_t n) {
  omx_clipper_instance_run(h, il, ir, ol, orr, n);
}
static uint32_t clipper_latency(const void *h) { return omx_clipper_instance_latency(h); }
static float clipper_meter(const void *h, uint32_t i) { return i == 0u ? omx_clipper_instance_gain_reduction_db(h) : 0.0f; }

static const struct twin TWINS[] = {
    {"trim", &h_trim, trim_init, trim_resolve, trim_run, trim_latency, NULL},
    {"width", &h_width, width_init, width_resolve, width_run, width_latency, NULL},
    {"clipper", &h_clipper, clipper_init, clipper_resolve, clipper_run, clipper_latency, clipper_meter},
};
#define NTWINS (sizeof TWINS / sizeof TWINS[0])

/* The generic face and the kernel state it runs over: the largest kernel's, aligned for any of them. */
static struct omx_fx_face g_f;
static union {
  struct omx_trim_kernel trim;
  struct omx_width_kernel width;
  struct omx_clipper_kernel clipper;
} g_k;

/* ---- the script ------------------------------------------------------------------------------------ */

/* A travel's word for block `b`: inside, at and past both ends, and a non-finite word now and then. */
static float travel_word(const struct omx_param_desc *p, int b, int i) {
  static const float WILD[] = {NAN, INFINITY, -INFINITY, 1e30f, -1e30f};
  const int k = (b * 7 + i * 3) % 23;
  if (k == 22) return WILD[(b + i) % 5];
  const float span = p->max - p->min;
  return p->min - 0.25f * span + 1.5f * span * (float)k / 21.0f;
}

/* A choice's word for block `b`: every index, halves, negatives and indices past the last. */
static float choice_word(const struct omx_param_desc *p, int b, int i) {
  const int k = (b * 5 + i) % 11;
  return -2.0f + (p->max + 4.0f) * (float)k / 10.0f;
}

static void words(const struct omx_kernel_desc *d, int b, float *v) {
  for (uint32_t i = 0; i < d->n_params; i++)
    v[i] = d->params[i].kind == OMX_PARAM_CHOICE ? choice_word(&d->params[i], b, (int)i) : travel_word(&d->params[i], b, (int)i);
}

static int same(const float *a, const float *b, uint32_t n) { return memcmp(a, b, (size_t)n * sizeof(float)) == 0; }

/* ---- the arms ---------------------------------------------------------------------------------------- */

static void arm_init(const struct twin *t, const struct omx_kernel_desc *d) {
  g_arm = "A";
  static const float RATES[] = {22050.0f, 0.0f, -48000.0f, NAN, INFINITY, 1.0f};
  for (unsigned i = 0; i < sizeof RATES / sizeof RATES[0]; i++) {
    const int h = t->init(t->hand, RATES[i]), f = omx_fx_face_init(&g_f, d, &g_k, RATES[i]);
    ok(h == f, "init answers as the hand-written face at a rate outside the set", f, h);
  }
  ok(t->init(t->hand, g_sr) == 1 && omx_fx_face_init(&g_f, d, &g_k, g_sr) == 1, "an oracle rate is taken by both", 0, 1);
  drain();
}

static void arm_bits(const struct twin *t, const struct omx_kernel_desc *d) {
  g_arm = "B";
  t->init(t->hand, g_sr);
  omx_fx_face_init(&g_f, d, &g_k, g_sr);
  float v[OMX_KERNEL_PARAMS_MAX];
  uint32_t moved = 0u, meters = 0u, lat = 0u;
  g_counting = 1;
  for (int b = 0; b < BLOCKS; b++) {
    const uint32_t n = 32u + (uint32_t)(b * 37) % (MAXN - 32u);
    const int bypass = (b % 13) >= 10;
    words(d, b, v);
    stimulus(g_l, g_r, n, 0x1234567u + (uint32_t)b, 64u);
    for (uint32_t i = 0; i < n; i++) g_l[i] *= 1.0f + (float)(b % 5), g_r[i] *= 1.0f + (float)(b % 3);
    t->resolve(t->hand, bypass, v);
    omx_fx_face_resolve(&g_f, bypass, v);
    t->run(t->hand, g_l, g_r, g_hl, g_hr, n);
    if (b % 2) {
      memcpy(g_fl, g_l, n * sizeof(float));
      memcpy(g_fr, g_r, n * sizeof(float));
      omx_fx_face_run(&g_f, g_fl, g_fr, g_fl, g_fr, n);
    } else {
      omx_fx_face_run(&g_f, g_l, g_r, g_fl, g_fr, n);
    }
    moved += !same(g_hl, g_fl, n) || !same(g_hr, g_fr, n);
    for (uint32_t m = 0; m < d->n_meters; m++) {
      const float hm = t->meter ? t->meter(t->hand, m) : 0.0f, fm = omx_fx_face_meter(&g_f, m);
      meters += memcmp(&hm, &fm, sizeof hm) != 0;
    }
    lat += t->latency(t->hand) != omx_fx_face_latency(&g_f);
    drain();
  }
  g_counting = 0;
  ok(moved == 0u, "the output is the hand-written face's, bit for bit, block by block", moved, 0);
  ok(meters == 0u, "every meter reads the hand-written face's after each block", meters, 0);
  ok(lat == 0u, "the latency is the hand-written face's", lat, 0);
  ok(g_allocs == 0, "nothing allocates", (double)g_allocs, 0);
  clean();
}

static void arm_choice_defaults(const struct twin *t, const struct omx_kernel_desc *d) {
  g_arm = "C";
  static const float ODD[] = {NAN, INFINITY, -INFINITY, 1e30f, -1e30f, 2147483648.0f};
  float hv[OMX_KERNEL_PARAMS_MAX], fv[OMX_KERNEL_PARAMS_MAX];
  for (unsigned w = 0; w < sizeof ODD / sizeof ODD[0]; w++) {
    for (uint32_t i = 0; i < d->n_params; i++) {
      hv[i] = d->params[i].def;
      fv[i] = d->params[i].kind == OMX_PARAM_CHOICE ? ODD[w] : d->params[i].def;
    }
    t->init(t->hand, g_sr);
    omx_fx_face_init(&g_f, d, &g_k, g_sr);
    t->resolve(t->hand, 0, hv);
    omx_fx_face_resolve(&g_f, 0, fv);
    stimulus(g_l, g_r, 256u, 0x55u + w, 64u);
    t->run(t->hand, g_l, g_r, g_hl, g_hr, 256u);
    omx_fx_face_run(&g_f, g_l, g_r, g_fl, g_fr, 256u);
    ok(same(g_hl, g_fl, 256u) && same(g_hr, g_fr, 256u), "a choice word that is no index reads as its default", w, 0);
  }
  clean();
}

static void arm_refusals(const struct omx_kernel_desc *d) {
  g_arm = "D";
  ok(omx_fx_face_init(NULL, d, &g_k, g_sr) == 0, "a null face is refused", 1, 0);
  ok(omx_fx_face_init(&g_f, NULL, &g_k, g_sr) == 0 && !g_f.ready, "a null descriptor is refused", 1, 0);
  ok(omx_fx_face_init(&g_f, d, NULL, g_sr) == 0 && !g_f.ready, "a null state is refused", 1, 0);
  struct omx_kernel_desc lame = *d;
  lame.latency = NULL;
  ok(omx_fx_face_init(&g_f, &lame, &g_k, g_sr) == 0 && !g_f.ready, "a descriptor without its latency is refused", 1, 0);
  lame = *d;
  lame.n_params = OMX_KERNEL_PARAMS_MAX + 1u;
  ok(omx_fx_face_init(&g_f, &lame, &g_k, g_sr) == 0 && !g_f.ready, "a descriptor of more controls than the face holds is refused", 1, 0);
  stimulus(g_l, g_r, 128u, 0x99u, 0u);
  for (uint32_t i = 0; i < 128u; i++) g_fl[i] = 7.0f, g_fr[i] = 7.0f;
  omx_fx_face_run(&g_f, g_l, g_r, g_fl, g_fr, 128u);
  ok(same(g_fl, g_l, 128u) && same(g_fr, g_r, 128u), "a face not ready leaves out holding in", 1, 0);
  omx_fx_face_resolve(&g_f, 0, NULL);
  omx_fx_face_clear(&g_f);
  ok(omx_fx_face_latency(&g_f) == 0u && omx_fx_face_meter(&g_f, 0u) == 0.0f, "a face not ready reads no latency and no meter", 1, 0);
  omx_fx_face_init(&g_f, d, &g_k, g_sr);
  for (uint32_t i = 0; i < 128u; i++) g_fl[i] = 7.0f, g_fr[i] = 7.0f;
  omx_fx_face_run(&g_f, g_l, g_r, g_fl, g_fr, 0u);
  omx_fx_face_run(&g_f, NULL, g_r, g_fl, g_fr, 128u);
  omx_fx_face_run(&g_f, g_l, g_r, g_fl, NULL, 128u);
  ok(g_fl[0] == 7.0f && g_fl[127] == 7.0f && g_fr[0] == 7.0f, "an empty block or a null port touches nothing", g_fl[0], 7);
  omx_fx_face_run(NULL, g_l, g_r, g_fl, g_fr, 128u);
  ok(same(g_fl, g_l, 128u) && same(g_fr, g_r, 128u), "a null face copies in to out and runs nothing", 1, 0);
  ok(omx_fx_face_meter(&g_f, d->n_meters) == 0.0f, "a meter past the last reads 0", 1, 0);
  /* null values: every default, as the host's defaults do */
  float v[OMX_KERNEL_PARAMS_MAX];
  for (uint32_t i = 0; i < d->n_params; i++) v[i] = d->params[i].def;
  omx_fx_face_init(&g_f, d, &g_k, g_sr);
  omx_fx_face_resolve(&g_f, 0, NULL);
  stimulus(g_l, g_r, 128u, 0x42u, 0u);
  omx_fx_face_run(&g_f, g_l, g_r, g_hl, g_hr, 128u);
  omx_fx_face_init(&g_f, d, &g_k, g_sr);
  omx_fx_face_resolve(&g_f, 0, v);
  omx_fx_face_run(&g_f, g_l, g_r, g_fl, g_fr, 128u);
  ok(same(g_hl, g_fl, 128u) && same(g_hr, g_fr, 128u), "null values resolve every default", 1, 0);
  clean();
}

static void arm_clear(void) {
  g_arm = "E";
  const uint32_t n = 256u;
  /* trim: clear lands the ramp, the next block is the gain itself from its first sample */
  omx_fx_face_init(&g_f, omx_trim_desc(), &g_k, g_sr);
  const float six[1] = {6.0f};
  omx_fx_face_resolve(&g_f, 0, six);
  omx_fx_face_clear(&g_f);
  stimulus(g_l, g_r, n, 0x31u, 0u);
  omx_fx_face_run(&g_f, g_l, g_r, g_fl, g_fr, n);
  const float g = omx_db_to_lin(6.0f);
  uint32_t off = 0u;
  for (uint32_t i = 0; i < n; i++) off += g_fl[i] != g_l[i] * g || g_fr[i] != g_r[i] * g;
  ok(off == 0u, "a cleared trim runs at its resolved gain from the first sample", off, 0);
  /* width: Haas at 10 ms, an impulse, clear: the delayed leg never sounds */
  omx_fx_face_init(&g_f, omx_width_desc(), &g_k, g_sr);
  const float haas[5] = {100.0f, (float)OMX_WIDTH_PSEUDOS_HAAS, 10.0f, 0.0f, 120.0f};
  omx_fx_face_resolve(&g_f, 0, haas);
  memset(g_l, 0, sizeof g_l);
  memset(g_r, 0, sizeof g_r);
  g_l[0] = 1.0f;
  omx_fx_face_run(&g_f, g_l, g_r, g_fl, g_fr, 64u);
  omx_fx_face_clear(&g_f);
  g_l[0] = 0.0f;
  float peak = 0.0f;
  for (int b = 0; b < 40; b++) {
    omx_fx_face_run(&g_f, g_l, g_r, g_fl, g_fr, MAXN);
    for (uint32_t i = 0; i < MAXN; i++) peak = fmaxf(peak, fmaxf(fabsf(g_fl[i]), fabsf(g_fr[i])));
  }
  ok(peak == 0.0f, "a cleared width ring is silent", peak, 0);
  /* clipper: clear rests the meter */
  omx_fx_face_init(&g_f, omx_clipper_desc(), &g_k, g_sr);
  const float hot[3] = {24.0f, -12.0f, 0.0f};
  omx_fx_face_resolve(&g_f, 0, hot);
  sine(g_l, n, 997.0, g_sr, 0.9);
  memcpy(g_r, g_l, n * sizeof(float));
  omx_fx_face_run(&g_f, g_l, g_r, g_fl, g_fr, n);
  ok(omx_fx_face_meter(&g_f, 0u) < 0.0f, "a driven clipper reads a reduction", omx_fx_face_meter(&g_f, 0u), 0);
  omx_fx_face_clear(&g_f);
  ok(omx_fx_face_meter(&g_f, 0u) == 0.0f, "a cleared clipper's meter reads 0", omx_fx_face_meter(&g_f, 0u), 0);
  clean();
}

static void arm_list(void) {
  g_arm = "F";
  uint32_t n = 0u, twinned = 0u;
  for (const struct omx_kernel_desc *d; (d = omx_fx_kernel_at(n)) != NULL; n++) {
    ok(omx_fx_kernel(d->name) == d, "a served kernel is found by its name", n, 0);
    for (unsigned t = 0; t < NTWINS; t++) twinned += strcmp(TWINS[t].name, d->name) == 0;
  }
  ok(n == NTWINS && twinned == NTWINS, "every served kernel has its hand-written twin here, and no other", n, NTWINS);
  ok(omx_fx_kernel("no-such-kernel") == NULL && omx_fx_kernel(NULL) == NULL, "an unknown name finds nothing", 1, 0);
  clean();
}

static void run_all(void) {
  for (unsigned t = 0; t < NTWINS; t++) {
    const struct omx_kernel_desc *d = omx_fx_kernel(TWINS[t].name);
    ok(d != NULL, "the twin's kernel is served", t, 0);
    if (!d) continue;
    arm_init(&TWINS[t], d);
    arm_bits(&TWINS[t], d);
    arm_choice_defaults(&TWINS[t], d);
    arm_refusals(d);
  }
  arm_clear();
  arm_list();
}

OMX_INSTANCE_ORACLE_MAIN("face", run_all();)
