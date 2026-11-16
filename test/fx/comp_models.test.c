// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
//
// The compressor models' detector oracles (the compressor-models spec §5, O5, O8 and O9) at each of the
// nine oracle rates (OMX_FX_ORACLE_RATES, 32 to 192 kHz). The detector is the kernel's own cascade
// (omx_env_step) made from the same poles the comp instance makes (omx_pole_from_time_ms), in RMS, so
// its state is the power domain:
//   A  O5: a step up reaches 1 - S(4) = 56.65 % of its target at attackMs, a step down falls to
//      S(4) = 43.35 % at releaseMs (the Erlang-4 survivor S(x) = e^-x sum_{k<4} x^k/k! at x = 4 t / tau),
//      within 5 %;
//   B  O8: the dbx-style release is not a constant rate and the deviation is pinned: after a step down
//      the power falls 20 dB in 2.5113 tau and from -10 to -30 dB in 1.5954 tau, so the fitted release
//      (66.4 ms) takes 166.7 ms and 105.9 ms, and the landed one (100 ms) 251.1 ms and 159.5 ms, within
//      2 %: a change to the cascade that "fixed" or worsened the deviation reads red;
//   C  O9: the time the gain reduction takes to reach 63.2 % of its settled value for a step of 10, 20
//      and 30 dB over the threshold at a 15 ms attack against a double-precision run of the same cascade
//      and gain computer (1 %), the residual against the unit's 15, 5 and 3 ms printed.
#define OMX_CONTRACT_STORAGE 1
#include <omxdsp/fx/omx_comp_instance.h>

#include "instance_oracle.h"

#define STAGES 4
/* The Erlang-4 survivor, double. */
static double surv(double x) { return exp(-x) * (1.0 + x + x * x / 2.0 + x * x * x / 6.0); }
/* The x at which surv falls to v, by bisection. */
static double solve(double v) {
  double lo = 0.0, hi = 80.0;
  for (int k = 0; k < 200; k++) {
    const double m = 0.5 * (lo + hi);
    if (surv(m) > v) lo = m;
    else hi = m;
  }
  return 0.5 * (lo + hi);
}

struct det {
  struct omx_env env;
  struct omx_env_params p;
  float ac, rc;
};
static void det_make(struct det *d, double att_ms, double rel_ms) {
  memset(d, 0, sizeof *d);
  d->p.attack_pole = omx_pole_from_time_ms((float)att_ms, g_sr);
  d->p.release_pole = omx_pole_from_time_ms((float)rel_ms, g_sr);
  d->p.detect = OMX_DETECT_RMS;
  omx_env_stage_poles(&d->p, 1u, &d->ac, &d->rc);
}
static float det_step(struct det *d, float x) { return omx_env_step(&d->env, &d->p, x, d->ac, d->rc); }

/* ---- A: O5 --------------------------------------------------------------------------------- */
static void arm_constants(void) {
  g_arm = "A";
  static const double MS[] = {5.0, 15.0, 100.0};
  for (int k = 0; k < 3; k++) {
    struct det d;
    const double ms = MS[k];
    det_make(&d, ms, ms);
    const uint32_t n = (uint32_t)(3.0 * ms * 1e-3 * g_sr);
    double t_up = -1.0, t_dn = -1.0;
    for (uint32_t i = 0; i < n && t_up < 0.0; i++) {
      det_step(&d, 1.0f);
      if ((double)d.env.stage[STAGES - 1] >= 1.0 - surv(4.0)) t_up = (double)(i + 1u) * 1e3 / (double)g_sr;
    }
    for (int s = 0; s < STAGES; s++) d.env.stage[s] = 1.0f;
    for (uint32_t i = 0; i < n && t_dn < 0.0; i++) {
      det_step(&d, 0.0f);
      if ((double)d.env.stage[STAGES - 1] <= surv(4.0)) t_dn = (double)(i + 1u) * 1e3 / (double)g_sr;
    }
    ok(fabs(t_up - ms) <= 0.05 * ms, "O5: a step up reaches 56.65 % at attackMs (5 %)", t_up, ms);
    ok(fabs(t_dn - ms) <= 0.05 * ms, "O5: a step down falls to 43.35 % at releaseMs (5 %)", t_dn, ms);
  }
}

/* ---- B: O8 --------------------------------------------------------------------------------- */
static void arm_release_deviation(void) {
  g_arm = "B";
  const double X20 = solve(0.01) / 4.0, X1030 = (solve(0.001) - solve(0.1)) / 4.0; /* in tau */
  ok(fabs(X20 - 2.5113) < 1e-3 && fabs(X1030 - 1.5954) < 1e-3, "O8: the survivor's closed form is the spec's 2.511 and 1.596", X20, 2.5113);
  static const double REL[] = {66.4, 100.0};
  static const double WANT20[] = {166.7, 251.1}, WANT1030[] = {105.9, 159.5};
  for (int k = 0; k < 2; k++) {
    struct det d;
    det_make(&d, 5.0, REL[k]);
    for (int s = 0; s < STAGES; s++) d.env.stage[s] = 1.0f;
    const uint32_t n = (uint32_t)(0.6 * g_sr);
    double t10 = -1.0, t20 = -1.0, t30 = -1.0;
    for (uint32_t i = 0; i < n && t30 < 0.0; i++) {
      det_step(&d, 0.0f);
      const double db = 10.0 * log10((double)d.env.stage[STAGES - 1]), t = (double)(i + 1u) * 1e3 / (double)g_sr;
      if (t10 < 0.0 && db <= -10.0) t10 = t;
      if (t20 < 0.0 && db <= -20.0) t20 = t;
      if (db <= -30.0) t30 = t;
    }
    ok(fabs(t20 - WANT20[k]) <= 0.02 * WANT20[k], "O8: the first 20 dB take the pinned time (2 %)", t20, WANT20[k]);
    ok(fabs((t30 - t10) - WANT1030[k]) <= 0.02 * WANT1030[k], "O8: -10 to -30 dB take the pinned time (2 %)", t30 - t10, WANT1030[k]);
    ok(fabs(t20 - X20 * REL[k]) <= 0.02 * t20, "O8: the first 20 dB are 2.511 tau", t20, X20 * REL[k]);
    if (k == 1) printf("  comp_models @ %.0f Hz: releaseMs 100 falls %.1f dB/s over the first 20 dB, %.1f dB/s from -10 to -30\n",
                       (double)g_sr, 20000.0 / t20, 20000.0 / (t30 - t10));
  }
}

/* ---- C: O9 --------------------------------------------------------------------------------- */
static void arm_attack_fit(void) {
  g_arm = "C";
  static const double OVER[] = {10.0, 20.0, 30.0}, UNIT_MS[] = {15.0, 5.0, 3.0};
  const double thr = -20.0, ratio = 4.0, att = 15.0;
  struct omx_gaincomp_params gc = {OMX_DYN_ABOVE, (float)thr, (float)ratio, 0.0f, 0.0f, 1.0f};
  for (int k = 0; k < 3; k++) {
    struct det d;
    det_make(&d, att, 100.0);
    const double amp = pow(10.0, (thr + OVER[k]) / 20.0);
    const double settled = OVER[k] * (1.0 - 1.0 / ratio), goal = 0.632120558828557678 * settled;
    /* The reference: the same four poles in double, the same hard-knee characteristic. */
    const double pa = pow((double)d.p.attack_pole, 4.0);
    double ref[STAGES] = {0, 0, 0, 0};
    double t_f = -1.0, t_d = -1.0;
    const uint32_t n = (uint32_t)(0.5 * g_sr);
    for (uint32_t i = 0; i < n && (t_f < 0.0 || t_d < 0.0); i++) {
      const float lv = det_step(&d, (float)(amp * amp));
      const double gf = -omx_gaincomp_db(&gc, omx_lin_to_db(lv));
      double in = amp * amp;
      for (int s = 0; s < STAGES; s++) { ref[s] = in + pa * (ref[s] - in); in = ref[s]; }
      const double ld = 10.0 * log10(ref[STAGES - 1]), gd = ld > thr ? (ld - thr) * (1.0 - 1.0 / ratio) : 0.0;
      const double t = (double)(i + 1u) * 1e3 / (double)g_sr;
      /* the kernel's reduction is the gain computer's (negative); the magnitude is compared. */
      if (t_f < 0.0 && gf >= goal) t_f = t;
      if (t_d < 0.0 && gd >= goal) t_d = t;
    }
    ok(t_f > 0.0 && t_d > 0.0 && fabs(t_f - t_d) <= 0.01 * t_d + 1e3 / (double)g_sr, "O9: the kernel's reduction reaches 63 % when the closed-form cascade does (1 %)", t_f, t_d);
    if (k == 0 || g_sr == 48000.0f) printf("  comp_models @ %.0f Hz: +%.0f dB over reaches 63 %% in %.2f ms (the unit's %.0f ms)\n", (double)g_sr, OVER[k], t_f, UNIT_MS[k]);
  }
}

int main(void) {
  (void)stimulus;
  (void)sine;
  (void)rms;
  (void)expect_clean;
  omx_fx_require_rate_floor();
  omx_fx_require_full_rates_cover_declared();
  for (unsigned ri = 0; ri < OMX_FX_ORACLE_RATE_COUNT; ri++) {
    g_sr = OMX_FX_ORACLE_RATES[ri];
    arm_constants();
    arm_release_deviation();
    arm_attack_fit();
    expect_clean();
  }
  return instance_oracle_end("comp_models");
}
