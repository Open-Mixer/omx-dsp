// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
//
// The comp slot's key, at the kernel (omx_key_build feeding omx_dynamics_keyed, the two calls the
// engine's lane walk makes at its COMP step), at each of the nine oracle rates (OMX_FX_ORACLE_RATES).
// Ported from the engine's mix_ducker.test.c, whose other arms comp_key.test.c already holds:
//   K  the key feeds the detector and never the audio: silent music under a full-scale key stays
//      exactly zero, and a loud key at unity ratio leaves the music bit for bit (the detector's copy
//      is not the output's);
//   T  four threads, each on its own key, state and block, produce the single-thread bytes.
// Sabotage: test/fx/comp_key_isolation-perturb.sh.
#define OMX_CONTRACT_STORAGE 1
#include <omxdsp/omxdsp.h>
#include <omxdsp/omx_dyn.h>
#include <omxdsp/mix/omx_key.h>

#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fx_rates.h"

#define BLOCK 256u
#define FRAMES 32768u
#define NTHREADS 4
#define THRESH_DB (-30.0f)
#define RATIO 20.0f

static int g_fail = 0, g_checks = 0;
static const char *g_arm = "?";
static float g_sr_now = 0.0f;

static void check(int cond, const char *what) {
  g_checks++;
  if (!cond) {
    g_fail++;
    printf("FAIL [%s @ %.0f Hz] %s\n", g_arm, (double)g_sr_now, what);
  }
}

/* The catalogue ducker row's timing: ratio 20, attack 1 ms, release 300 ms, as the engine's oracle. */
static struct omx_dyn duck_slot(float sr) {
  struct omx_dyn p;
  memset(&p, 0, sizeof p);
  p.enabled = 1;
  p.gc.mode = OMX_DYN_ABOVE;
  p.detect = OMX_DETECT_PEAK;
  p.gc.thresh_db = THRESH_DB;
  p.gc.ratio = RATIO;
  p.gc.knee_db = 0.0f;
  p.gc.range_db = 0.0f;
  p.gc.makeup_lin = 1.0f;
  p.attack_ms = 1.0f;
  p.ovs_mode = OMX_DYN_OVS_OFF;
  p.attack_coeff = omx_pole_from_time_ms(1.0f, sr);
  p.release_coeff = omx_pole_from_time_ms(300.0f, sr);
  return p;
}

/* One mono comp step as the lane walks it: build the key from `src`, apply the keyed slot. */
static void render(float sr, float *x, const float *src, uint32_t n, float ratio) {
  struct omx_dyn p = duck_slot(sr);
  p.gc.ratio = ratio;
  struct omx_dyn_state st;
  omx_dyn_state_init(&st, 1u);
  float scratch[BLOCK], hp[4] = {0}, lp[4] = {0};
  struct omx_key key;
  memset(&key, 0, sizeof key);
  key.buf = scratch;
  key.to_detector = 1;
  for (uint32_t off = 0; off < n; off += BLOCK) {
    const uint32_t len = n - off < BLOCK ? n - off : BLOCK;
    key.src = src + off;
    const float *k = omx_key_build(&key, x + off, NULL, len, hp, lp);
    omx_dynamics_keyed(x + off, NULL, k, len, &p, &st);
  }
}

static float g_music[FRAMES], g_key[FRAMES], g_ref[FRAMES];

static void music(float *b, uint32_t n, float sr) {
  uint32_t s = 12345u;
  for (uint32_t i = 0; i < n; i++) {
    s = s * 1664525u + 1013904223u;
    const float hiss = ((float)(s >> 8) / 16777216.0f - 0.5f) * 0.05f;
    b[i] = 0.3f * sinf(2.0f * (float)M_PI * 220.0f * (float)i / sr) +
           0.2f * sinf(2.0f * (float)M_PI * 1375.0f * (float)i / sr) + hiss;
  }
}

static void arm_key_never_in_the_audio(float sr) {
  g_arm = "K";
  memset(g_music, 0, sizeof g_music);
  for (uint32_t i = 0; i < FRAMES; i++) g_key[i] = 1.0f;
  render(sr, g_music, g_key, FRAMES, RATIO);
  int zeros = 1;
  for (uint32_t i = 0; i < FRAMES; i++) zeros &= g_music[i] == 0.0f;
  check(zeros, "silent music under a full-scale key stays exactly zero");

  /* Ratio 1 is no reduction whatever the key: the music is untouched, so no key sample reached it. */
  music(g_ref, FRAMES, sr);
  music(g_music, FRAMES, sr);
  render(sr, g_music, g_key, FRAMES, 1.0f);
  check(memcmp(g_music, g_ref, sizeof g_music) == 0, "a unity-ratio keyed comp leaves the music bit for bit");
}

static float g_tkey[FRAMES];
struct job {
  float sr;
  float out[FRAMES];
};
static float g_single[FRAMES];

static void *thread_main(void *arg) {
  struct job *j = arg;
  music(j->out, FRAMES, j->sr);
  render(j->sr, j->out, g_tkey, FRAMES, RATIO);
  return NULL;
}

static void arm_n_thread_identity(float sr) {
  g_arm = "T";
  for (uint32_t i = 0; i < FRAMES; i++) g_tkey[i] = (i / 4096u) % 2u ? 0.5f : 0.0f;
  music(g_single, FRAMES, sr);
  render(sr, g_single, g_tkey, FRAMES, RATIO);
  static struct job jobs[NTHREADS];
  pthread_t th[NTHREADS];
  for (int t = 0; t < NTHREADS; t++) {
    jobs[t].sr = sr;
    pthread_create(&th[t], NULL, thread_main, &jobs[t]);
  }
  for (int t = 0; t < NTHREADS; t++) pthread_join(th[t], NULL);
  for (int t = 0; t < NTHREADS; t++)
    check(memcmp(jobs[t].out, g_single, sizeof g_single) == 0, "thread output == single-thread bytes");
  /* The reference must have ducked: a key that never moved the gain would pass the identity vacuously. */
  music(g_ref, FRAMES, sr);
  check(memcmp(g_single, g_ref, sizeof g_single) != 0, "the keyed comp changed the music (the identity is not vacuous)");
}

int main(void) {
  omx_fx_require_rate_floor();
  omx_fx_require_full_rates_cover_declared();
  for (unsigned ri = 0; ri < OMX_FX_ORACLE_RATE_COUNT; ri++) {
    g_sr_now = OMX_FX_ORACLE_RATES[ri];
    arm_key_never_in_the_audio(g_sr_now);
    arm_n_thread_identity(g_sr_now);
  }
  g_arm = "L";
  check(omx_contract_log.count == 0u, "no contract violation recorded");
  printf("comp_key_isolation: %d/%d checks passed\n", g_checks - g_fail, g_checks);
  return g_fail ? 1 : 0;
}
