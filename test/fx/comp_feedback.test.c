// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
//
// The feedback arms of the bus compressor's oracles (the ssl-bus-compressor spec §5, O7, O8 and O9), run on the PRESET the
// spec rules (the bus row: feedback, 4:1, 0.1 ms, `auto` release, -10 dB, +2 dB make-up, `detectorOversampling` 4x stored),
// through the strip compressor's instance face, at each of the nine oracle rates (OMX_FX_ORACLE_RATES, 32 to 192 kHz).
// dynamics_feedback.test.c holds the kernel's own O1-O5 and O10 and a raw-atom O7/O8; these are the preset's arms:
//   A  O7: an impulse through the preset is heard at index 0; the instance reports 0 frames of latency; the kernel's factor
//      is 1 with 4x stored, so the detector reads not oversampled. The control: the SAME stored 4x on the feedforward
//      topology reports 72, answers factor 4 and holds index 0 silent, so the probe can tell a latency from none;
//   B  O8: a 0 dBFS burst (the loop and the `auto` word both engaged, read nonzero), then 2 s of silence: every state word
//      of the slot, the loop's, the cascade's, the program release's and the delay lines', is 0 or normal;
//   C  O9: the preset then the colour row (drive: tape, 3 dB, character 0, full band, mix 100, trim 0, auto gain, linked,
//      no roll-off, 4x) as on MAIN: the latency the two report is the drive's 72 (the compressor adds none), the loudest
//      sample of an impulse lands at index 72 and the output is exactly 0 before the drive's ringing begins, and at 0 dBFS
//      (a full-scale sine and a full-scale impulse) the output never exceeds make-up + trim + 0.01 dB.
#define OMX_CONTRACT_STORAGE 1
#include <omxdsp/fx/omx_comp_instance.h>
#include <omxdsp/fx/omx_drive_instance.h>

#include "instance_oracle.h"

#include <stdlib.h>

#define BLK 256u
#define LATENCY_4X 72u /* the spec's L6: omx_oversampler_latency_for(4) */

/* The undeclared oracle rates (32, 64, 128 kHz) record `rate-is-declared` on every block, more than the ledger holds, so it is
 * drained after each call and only what the rate does not explain is kept (omx_fx_drain_ledger). */
static uint32_t g_unexplained = 0u;
static void drain(void) { g_unexplained += omx_fx_drain_ledger(g_sr); }
/* `auto`'s word and the 4x path make poles per sample, so a rate the declaration lacks logs several records per sample: the run is
 * cut into 8-frame pieces, which fit the ledger (OMX_CONTRACT_MAX), and drained between them. */
static void comp_run(OmxCompInstance *c, float *l, float *r, uint32_t n) {
  for (uint32_t at = 0; at < n; at += 8u) {
    omx_comp_instance_run(c, l + at, r + at, l + at, r + at, n - at < 8u ? n - at : 8u);
    drain();
  }
}
static void drive_run(OmxDriveInstance *d, float *l, float *r, uint32_t n) {
  for (uint32_t at = 0; at < n; at += 8u) {
    omx_drive_instance_run(d, l + at, r + at, l + at, r + at, n - at < 8u ? n - at : 8u);
    drain();
  }
}
static void arm_clean(void) {
  drain();
  ok(g_unexplained == 0u, "no contract violation in this arm", (double)g_unexplained, 0.0);
  g_unexplained = 0u;
}

/* Silent blocks until the slot's factor handover (a cold slot starts on the base path) has run out. */
static void settle(OmxCompInstance *c) {
  float l[BLK] = {0}, r[BLK] = {0};
  for (int i = 0; i < 8; i++) {
    comp_run(c, l, r, BLK);
    drain();
  }
}

#define SSL_BUS_MAKEUP_DB 2.0f
#define SSL_BUS_COLOUR_TRIM_DB 0.0f

/* The bus row, resolved on a fresh instance. An arm moves the topology, and arm B the release (the row's own is `auto`). */
static OmxCompInstance *bus_with(int topology, int release_mode, float release_ms) {
  OmxCompInstance *s = calloc(1, sizeof *s);
  omx_comp_instance_init(s, g_sr);
  omx_comp_instance_resolve(s, 0, -10.0f, 4.0f, 0.0f, 0.1f, release_ms, SSL_BUS_MAKEUP_DB, 100.0f, (int)OMX_COMP_KINDS_COMP,
                            (int)OMX_DETECTOR_OVERSAMPLINGS_4X, topology, (int)OMX_COMP_KEY_SOURCES_DEFAULT,
                            (float)OMX_COMP_KEY_HP_HZ_RANGE_DEFAULT, (float)OMX_COMP_KEY_LP_HZ_RANGE_DEFAULT,
                            (float)OMX_COMP_KEY_TILT_DB_RANGE_DEFAULT, (int)OMX_COMP_KEY_LISTENS_DEFAULT,
                            (float)OMX_COMP_HOLD_MS_RANGE_DEFAULT, (int)OMX_COMP_DETECT_LINKS_DEFAULT,
                            (int)OMX_COMP_DUCKS_DEFAULT, (float)OMX_COMP_RANGE_DB_RANGE_DEFAULT,
                            release_mode);
  drain();
  return s;
}
static OmxCompInstance *bus(int topology) { return bus_with(topology, (int)OMX_COMP_RELEASE_MODES_AUTO, 100.0f); }

/* The colour row on a fresh drive instance. */
static OmxDriveInstance *colour(void) {
  OmxDriveInstance *d = calloc(1, sizeof *d);
  omx_drive_instance_init(d, g_sr);
  omx_drive_instance_resolve(d, 0, 3.0f, 0.0f, 2000.0f, 100.0f, SSL_BUS_COLOUR_TRIM_DB, OMX_DRIVE_TAPE, OMX_DRIVE_BAND_FULL, 1, 1,
                             OMX_DRIVE_HF_ROLLOFFS_OFF, OMX_DRIVE_OVERSAMPLES_4);
  drain();
  return d;
}

static uint32_t first_nonzero(const float *x, uint32_t n) {
  for (uint32_t i = 0; i < n; i++)
    if (x[i] != 0.0f) return i;
  return n;
}

/* ---- A: O7 --------------------------------------------------------------------------------- */
static void arm_zero_latency(void) {
  g_arm = "A";
  OmxCompInstance *fb = bus((int)OMX_DETECTOR_TOPOLOGY_FEEDBACK);
  float l[BLK] = {0}, r[BLK] = {0};
  l[0] = r[0] = 0.5f;
  settle(fb);
  comp_run(fb, l, r, BLK);
  ok(l[0] != 0.0f && r[0] != 0.0f, "O7: an impulse through the bus preset is heard at index 0", (double)l[0], 0.0);
  ok(omx_comp_instance_latency(fb) == 0.0f, "O7: the bus preset reports 0 frames of latency", (double)omx_comp_instance_latency(fb), 0.0);
  ok(omx_dyn_oversample_factor(&fb->atom) == 1u, "O7: feedback answers factor 1 with 4x stored (detectorOversampled reads false)",
     (double)omx_dyn_oversample_factor(&fb->atom), 1.0);
  ok(fb->atom.ovs_mode == OMX_DYN_OVS_X4, "O7: the stored choice is still 4x", (double)fb->atom.ovs_mode, (double)OMX_DYN_OVS_X4);

  /* The control: the same stored 4x, feedforward, is the 72-frame path. */
  OmxCompInstance *ff = bus((int)OMX_DETECTOR_TOPOLOGY_FEEDFORWARD);
  float m[BLK] = {0}, n[BLK] = {0};
  m[0] = n[0] = 0.5f;
  settle(ff);
  comp_run(ff, m, n, BLK);
  ok(omx_comp_instance_latency(ff) == (float)LATENCY_4X, "O7 control: feedforward 4x reports 72", (double)omx_comp_instance_latency(ff),
     (double)LATENCY_4X);
  ok(omx_dyn_oversample_factor(&ff->atom) == 4u, "O7 control: feedforward 4x answers factor 4", (double)omx_dyn_oversample_factor(&ff->atom), 4.0);
  ok(first_nonzero(m, BLK) == LATENCY_4X, "O7 control: feedforward 4x holds the impulse back 72 frames", (double)first_nonzero(m, BLK),
     (double)LATENCY_4X);
  arm_clean();
  free(fb);
  free(ff);
}

/* ---- B: O8 --------------------------------------------------------------------------------- */
static int normal_or_zero(float v) {
  const int c = fpclassify(v);
  return c == FP_ZERO || c == FP_NORMAL;
}
static int words_ok(const struct omx_dyn_state *st) {
  int good = 1;
  for (uint32_t i = 0; i < OMX_DYN_ENV_STAGES; i++) good &= normal_or_zero(st->env.stage[i]);
  const float w[] = {st->fb_env, st->fb_yl, st->fb_yr, st->fb_xh, st->prs.fast, st->prs.slow, st->prs.mem, st->hyst_credit_db,
                     st->gr_min_db, st->last_gain, st->xfade_gain};
  for (unsigned i = 0; i < sizeof w / sizeof *w; i++) good &= normal_or_zero(w[i]);
  for (uint32_t i = 0; i < OMX_OVS_LATENCY_4X + 1u; i++) good &= normal_or_zero(st->delay_l[i]) & normal_or_zero(st->delay_r[i]);
  return good;
}
/* A 0 dBFS burst of noise, 50 ms, then 2 s of silence, through the bus row with the given release; the words after it. */
static void burst_then_silence(OmxCompInstance *s, int engaged_word) {
  float l[BLK], r[BLK];
  const uint32_t burst = (uint32_t)(0.05f * g_sr);
  for (uint32_t done = 0; done < burst; done += BLK) {
    stimulus(l, r, BLK, 0x9e3779b9u + done, 0);
    for (uint32_t i = 0; i < BLK; i++) { l[i] *= 2.0f; r[i] *= 4.0f; }
    comp_run(s, l, r, BLK);
  }
  ok(s->st.fb_ran != 0 && s->st.fb_env > 0.0f && (!engaged_word || (s->st.prs.slow < 0.0f && s->st.prs.mem > 0.0f)),
     "O8 control: the burst engaged the loop (and the auto word), their states read nonzero", (double)s->st.fb_env, 0.0);
  const uint32_t silence = (uint32_t)(2.0f * g_sr);
  for (uint32_t done = 0; done < silence; done += BLK) {
    memset(l, 0, sizeof l);
    memset(r, 0, sizeof r);
    comp_run(s, l, r, BLK);
  }
}
static void arm_no_denormal(void) {
  g_arm = "B";
  /* The row as ruled: the `auto` word holds the envelope at the level of its own slow tail, which is still a normal number
   * 2 s on, so this is the spec's figure, exact. */
  OmxCompInstance *s = bus((int)OMX_DETECTOR_TOPOLOGY_FEEDBACK);
  burst_then_silence(s, 1);
  ok(words_ok(&s->st), "O8: every state word is 0 or normal after 2 s of silence (the bus row, auto)", 0.0, 0.0);
  free(s);
  /* The same loop with the shortest release the row allows: the envelope and the held outputs run the whole way down through
   * the denormal range inside the 2 s, which the auto tail never does, so a write that does not flush is left standing. */
  s = bus_with((int)OMX_DETECTOR_TOPOLOGY_FEEDBACK, (int)OMX_COMP_RELEASE_MODES_FIXED, OMX_COMP_RELEASE_MS_MIN);
  burst_then_silence(s, 0);
  ok(words_ok(&s->st), "O8: every state word is 0 or normal after 2 s of silence (the loop at the shortest release)", 0.0, 0.0);
  arm_clean();
  free(s);
}

/* ---- C: O9 --------------------------------------------------------------------------------- */
static void chain(OmxCompInstance *c, OmxDriveInstance *d, float *l, float *r) {
  comp_run(c, l, r, BLK);
  drive_run(d, l, r, BLK);
  drain();
}
static void arm_combination(void) {
  g_arm = "C";
  OmxCompInstance *c = bus((int)OMX_DETECTOR_TOPOLOGY_FEEDBACK);
  OmxDriveInstance *d = colour();
  const uint32_t reported = (uint32_t)omx_comp_instance_latency(c) + omx_drive_instance_latency(d);
  ok(reported == LATENCY_4X, "O9: the combination reports the drive's 72 (the compressor adds none)", (double)reported, (double)LATENCY_4X);
  ok(omx_drive_instance_latency(d) == LATENCY_4X, "O9: the drive row's own latency is 72", (double)omx_drive_instance_latency(d),
     (double)LATENCY_4X);
  float l[BLK] = {0}, r[BLK] = {0};
  l[0] = r[0] = 0.5f;
  chain(c, d, l, r);
  uint32_t at = 0;
  for (uint32_t i = 1; i < BLK; i++)
    if (fabsf(l[i]) > fabsf(l[at])) at = i;
  ok(at == LATENCY_4X, "O9: the audio is 72 samples late (the impulse peaks at index 72)", (double)at, (double)LATENCY_4X);
  const uint32_t lead = first_nonzero(l, BLK);
  /* The oversampler's linear-phase FIR rings ahead of its peak, so the first non-zero sample is earlier than 72; but it is
   * not index 0, the impulse is not passed through undelayed. */
  ok(lead > 0u && lead <= at, "O9: the impulse is not passed through undelayed (index 0 holds nothing)", (double)lead, (double)at);
  arm_clean();
  free(c);
  free(d);

  /* The bound: 0 dBFS in, never more than make-up + trim + 0.01 dB out. */
  const double bound = pow(10.0, (SSL_BUS_MAKEUP_DB + SSL_BUS_COLOUR_TRIM_DB + 0.01) / 20.0);
  double worst = 0.0, worst_c = 0.0;
  c = bus((int)OMX_DETECTOR_TOPOLOGY_FEEDBACK);
  d = colour();
  const uint32_t total = (uint32_t)(0.5f * g_sr);
  double ph = 0.0;
  const double inc = 2.0 * M_PI * 1000.0 / (double)g_sr;
  for (uint32_t done = 0; done < total; done += BLK) {
    for (uint32_t i = 0; i < BLK; i++, ph += inc) l[i] = r[i] = (float)sin(ph);
    if (done == 0u) l[0] = r[0] = 1.0f; /* a full-scale first sample, before the loop has heard anything */
    comp_run(c, l, r, BLK);
    for (uint32_t i = 0; i < BLK; i++) {
      const double al = fabs((double)l[i]), ar = fabs((double)r[i]);
      if (al > worst_c) worst_c = al;
      if (ar > worst_c) worst_c = ar;
    }
    drive_run(d, l, r, BLK);
    for (uint32_t i = 0; i < BLK; i++) {
      const double al = fabs((double)l[i]), ar = fabs((double)r[i]);
      if (al > worst) worst = al;
      if (ar > worst) worst = ar;
    }
  }
  /* The compressor alone: the full-scale first sample is heard by no loop yet, so it leaves at exactly the make-up (the probe
   * reaches the bound's edge) and nothing after it leaves louder. */
  const double makeup_lin = pow(10.0, SSL_BUS_MAKEUP_DB / 20.0);
  ok(worst_c <= bound && worst_c >= makeup_lin * 0.9999, "O9: the compressor alone never exceeds make-up + 0.01 dB, and reaches it", worst_c, bound);
  ok(worst > 0.0 && worst <= bound, "O9: at 0 dBFS the output never exceeds make-up + trim + 0.01 dB", worst, bound);
  free(c);
  free(d);
  c = bus((int)OMX_DETECTOR_TOPOLOGY_FEEDBACK);
  d = colour();
  worst = 0.0;
  for (uint32_t done = 0; done < total; done += BLK) {
    for (uint32_t i = 0; i < BLK; i++) l[i] = r[i] = (done + i) % 997u == 0u ? 1.0f : 0.0f; /* full-scale impulses */
    chain(c, d, l, r);
    for (uint32_t i = 0; i < BLK; i++) {
      const double al = fabs((double)l[i]), ar = fabs((double)r[i]);
      if (al > worst) worst = al;
      if (ar > worst) worst = ar;
    }
  }
  ok(worst > 0.0 && worst <= bound, "O9: a 0 dBFS impulse train never exceeds make-up + trim + 0.01 dB", worst, bound);
  arm_clean();
  free(c);
  free(d);
}

int main(void) {
  omx_fx_require_rate_floor();
  omx_fx_require_full_rates_cover_declared();
  (void)expect_clean;
  (void)sine;
  (void)rms;
  for (unsigned k = 0; k < OMX_FX_ORACLE_RATE_COUNT; k++) {
    g_sr = OMX_FX_ORACLE_RATES[k];
    arm_zero_latency();
    arm_no_denormal();
    arm_combination();
  }
  return instance_oracle_end("comp_feedback");
}
