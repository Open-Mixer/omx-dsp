// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>

/* ---- the drive's transfer curve (omx_curve.h, family `drive`): a steady level in, the kernel's level out, in dB --
 *
 * A transfer_curve is dB in to dB out on one axis on every surface, the drive's too: x a steady input level, y the
 * level the drive's static path gives it. The curve is that path, so the oracle is the kernel itself: an instance at
 * 1x, on the full band, with no roll-off and no auto-gain, runs the steady sample of each x (10^(x/20)) as a block,
 * and every output sample, in dB (omx_lin_to_db), must be the curve's y. That holds exactly where nothing that is not
 * a function of one sample runs: at tone -1 the bias is zero and the DC blocker is off. With a bias the kernel's DC
 * blocker runs, which a steady level's curve does not draw, so there the oracle is the kernel's own shaper block
 * (omx_drive_shape_block(), what omx_drive_process() runs at 1x) under the instance's own atom, mixed and trimmed as
 * omx_drive_leg_chunk() and omx_drive_process() do. */

#include <omxdsp/fx/omx_drive_instance.h>
#include <omxdsp/omx_curve.h>

#define CD_POINTS 129u
/* The shaper block may run libmvec's vector tanhf where the curve runs the scalar one: a few float ULPs, in dB. */
#define CD_TOL 1e-5

static uint32_t cd_draw(int model, double amount, double tone, double mix, double output, int with_tone, int with_mix,
                        int with_output, double *x, double *y, int *code) {
  struct omx_curve_term t[5];
  uint32_t n = 0;
  t[n++] = (struct omx_curve_term){OMX_CURVE_ROLES_DRIVE, 0, amount, OMX_CURVE_UNITS_DB, OMX_CURVE_NO_SHAPE, OMX_CURVE_ORIGINS_USER};
  t[n++] = (struct omx_curve_term){OMX_CURVE_ROLES_MODEL, 0, (double)model, OMX_CURVE_UNITS_NONE, OMX_CURVE_NO_SHAPE, OMX_CURVE_ORIGINS_USER};
  if (with_tone) t[n++] = (struct omx_curve_term){OMX_CURVE_ROLES_TONE, 0, tone, OMX_CURVE_UNITS_NONE, OMX_CURVE_NO_SHAPE, OMX_CURVE_ORIGINS_USER};
  if (with_mix) t[n++] = (struct omx_curve_term){OMX_CURVE_ROLES_MIX, 0, mix, OMX_CURVE_UNITS_PC, OMX_CURVE_NO_SHAPE, OMX_CURVE_ORIGINS_USER};
  if (with_output) t[n++] = (struct omx_curve_term){OMX_CURVE_ROLES_OUTPUT, 0, output, OMX_CURVE_UNITS_DB, OMX_CURVE_NO_SHAPE, OMX_CURVE_ORIGINS_USER};
  const struct omx_curve_request rq = {OMX_CURVE_DISPLAYS_TRANSFER_CURVE, OMX_CURVE_FAMILIES_DRIVE, 48000.0, CD_POINTS, 0, 0.0, 0.0,
                                       t, n, 0};
  *code = -1;
  return omx_curve_points(&rq, x, y, OMX_CURVE_POINTS_MAX, code);
}

/* The kernel's answer at those words: an instance resolved at 1x, full band, no roll-off, no auto-gain, the
 * block run once after a silent block (so the gain ramp starts settled). */
static void cd_kernel(int model, float amount, float tone, float mix, float output, const double *x, float *out) {
  static OmxDriveInstance s;
  memset(&s, 0, sizeof s);
  omx_drive_instance_init(&s, 48000.0f);
  omx_drive_instance_resolve(&s, 0, amount, tone, OMX_DRIVE_INSTANCE_BAND_FREQ_DEFAULT, mix, output, model,
                             OMX_DRIVE_BAND_FULL, 0, 1, OMX_DRIVE_HF_ROLLOFFS_OFF, OMX_DRIVE_OVERSAMPLES_1);
  float l[CD_POINTS], r[CD_POINTS];
  memset(l, 0, sizeof l);
  memset(r, 0, sizeof r);
  omx_drive_instance_run(&s, l, r, l, r, CD_POINTS);
  for (uint32_t i = 0; i < CD_POINTS; i++) l[i] = r[i] = omx_db_to_lin((float)x[i]);
  omx_drive_instance_run(&s, l, r, l, r, CD_POINTS);
  for (uint32_t i = 0; i < CD_POINTS; i++) out[i] = omx_lin_to_db(l[i]);
}

/* The kernel's shaper block under the instance's own atom, mixed and trimmed as the leg and the process do. */
static void cd_shaper(int model, float amount, float tone, float mix, float output, const double *x, float *out) {
  struct omx_drive o;
  memset(&o, 0, sizeof o);
  omx_drive_instance_controls(&o, amount, tone, mix, output, model);
  const struct omx_drive_bias b = omx_drive_bias_for(o.curve, o.even_w);
  float wet[CD_POINTS];
  for (uint32_t i = 0; i < CD_POINTS; i++) wet[i] = omx_db_to_lin((float)x[i]);
  omx_drive_shape_block(o.curve, &b, o.drive_lin, wet, CD_POINTS);
  for (uint32_t i = 0; i < CD_POINTS; i++) {
    const float dry = omx_db_to_lin((float)x[i]), delta = o.curve == OMX_DRIVE_EXCITER ? wet[i] : wet[i] - dry;
    out[i] = omx_lin_to_db((dry + o.mix * delta) * o.trim_lin);
  }
}

static void arm_curve_drive(void) {
  g_arm = "curve-drive";
  double x[OMX_CURVE_POINTS_MAX], y[OMX_CURVE_POINTS_MAX], y2[OMX_CURVE_POINTS_MAX];
  float k[CD_POINTS];
  int code;
  const int models[] = {OMX_DRIVE_SOFT, OMX_DRIVE_TAPE, OMX_DRIVE_TUBE, OMX_DRIVE_EXCITER};
  const float amounts[] = {0.0f, 6.0f, 18.0f, 36.0f}, mixes[] = {100.0f, 50.0f, 0.0f}, outputs[] = {0.0f, -6.0f, 4.5f};
  const float tones[] = {-0.5f, 0.0f, 1.0f};

  /* 1. Against the kernel run, at tone -1: every shaper, drive, mix and trim, sample for sample. */
  double worst = 0.0;
  uint32_t cells = 0;
  for (uint32_t m = 0; m < 4u; m++)
    for (uint32_t a = 0; a < 4u; a++)
      for (uint32_t w = 0; w < 3u; w++)
        for (uint32_t t = 0; t < 3u; t++) {
          const uint32_t n = cd_draw(models[m], amounts[a], -1.0, mixes[w], outputs[t], 1, 1, 1, x, y, &code);
          ok(n == CD_POINTS && code == OMX_CURVE_OK, "a drive curve draws", code, 0);
          if (n != CD_POINTS) continue;
          cd_kernel(models[m], amounts[a], -1.0f, mixes[w], outputs[t], x, k);
          for (uint32_t i = 0; i < n; i++) {
            const double e = fabs(y[i] - (double)k[i]);
            if (e > worst) worst = e;
          }
          cells++;
        }
  ok(cells == 4u * 4u * 3u * 3u && worst <= CD_TOL, "the drive curve is the kernel run on its x (tone -1, 144 cells)", worst, CD_TOL);
  printf("  curve-drive: worst |curve - kernel run| %.3g dB over %u cells of %u levels (tone -1)\n", worst, cells, CD_POINTS);

  /* 2. With a bias: against the kernel's shaper block under the instance's atom. */
  worst = 0.0;
  for (uint32_t m = 0; m < 4u; m++)
    for (uint32_t a = 0; a < 4u; a++)
      for (uint32_t c = 0; c < 3u; c++) {
        const uint32_t n = cd_draw(models[m], amounts[a], tones[c], 70.0, -3.0, 1, 1, 1, x, y, &code);
        if (n != CD_POINTS) { ok(0, "a biased drive curve draws", code, 0); continue; }
        cd_shaper(models[m], amounts[a], tones[c], 70.0f, -3.0f, x, k);
        for (uint32_t i = 0; i < n; i++) {
          const double e = fabs(y[i] - (double)k[i]);
          if (e > worst) worst = e;
        }
      }
  ok(worst <= CD_TOL, "a biased drive curve is the kernel's shaper block (48 cells)", worst, CD_TOL);
  printf("  curve-drive: worst |curve - kernel shaper block| %.3g dB over 48 biased cells\n", worst);

  /* 3. The axis: dB in, dB out. The default grid is driveLevelMinDb..driveLevelMaxDb, both edges; the level out
   * never falls as the level in rises; low in the grid the soft shaper alone is its drive (unity small-signal
   * slope): -60 dB in at +18 dB drive is -42 dB out; a bias moves the curve. */
  cd_draw(OMX_DRIVE_TUBE, 12.0, 1.0, 100.0, 0.0, 1, 1, 1, x, y, &code);
  ok(x[0] == OMX_CURVE_EXTENTS_DRIVE_LEVEL_MIN_DB && x[CD_POINTS - 1u] == OMX_CURVE_EXTENTS_DRIVE_LEVEL_MAX_DB,
     "the default extents are driveLevelMinDb..driveLevelMaxDb", x[0], x[CD_POINTS - 1u]);
  int rising = 1;
  for (uint32_t i = 1; i < CD_POINTS; i++) rising &= y[i] >= y[i - 1u];
  ok(rising, "the level out never falls as the level in rises", 0, 0);
  cd_draw(OMX_DRIVE_TUBE, 12.0, -1.0, 100.0, 0.0, 1, 1, 1, x, y2, &code);
  ok(y[CD_POINTS - 1u] != y2[CD_POINTS - 1u], "a bias moves the curve", y[CD_POINTS - 1u], y2[CD_POINTS - 1u]);
  cd_draw(OMX_DRIVE_SOFT, 18.0, -1.0, 100.0, 0.0, 1, 1, 1, x, y, &code);
  ok(fabs(y[0] - (x[0] + 18.0)) < 0.01, "low in the grid the soft drive is its gain: -60 dB in, -42 dB out", y[0], x[0] + 18.0);
  ok(y[CD_POINTS - 1u] < 0.0, "a soft shaper's output stays under full scale at 0 dB in, +18 dB drive", y[CD_POINTS - 1u], 0.0);

  /* 4. The defaults that are not guesses: no tone is -1, no mix 100 %, no output 0 dB (a card's picture). */
  cd_draw(OMX_DRIVE_TAPE, 18.0, 0.0, 0.0, 0.0, 0, 0, 0, x, y, &code);
  cd_draw(OMX_DRIVE_TAPE, 18.0, -1.0, 100.0, 0.0, 1, 1, 1, x, y2, &code);
  int same = 1;
  for (uint32_t i = 0; i < CD_POINTS; i++) same &= y[i] == y2[i];
  ok(same, "a drive curve with no tone, mix or output is tone -1, mix 100 %, output 0 dB", 0, 0);
  /* a model that is no shaper's index reads as the instance's default, as resolve() reads it */
  cd_draw(9, 18.0, 0.0, 0.0, 0.0, 0, 0, 0, x, y, &code);
  cd_draw(OMX_DRIVE_INSTANCE_CURVE_DEFAULT, 18.0, 0.0, 0.0, 0.0, 0, 0, 0, x, y2, &code);
  same = code == OMX_CURVE_OK;
  for (uint32_t i = 0; i < CD_POINTS; i++) same &= y[i] == y2[i];
  ok(same, "a model past the shapers reads as the instance's default shaper", 0, 0);
  /* the four shapers are four curves */
  for (uint32_t m = 1; m < 4u; m++) {
    cd_draw(models[m - 1u], 18.0, 0.0, 0.0, 0.0, 0, 0, 0, x, y, &code);
    cd_draw(models[m], 18.0, 0.0, 0.0, 0.0, 0, 0, 0, x, y2, &code);
    ok(y[CD_POINTS - 1u] != y2[CD_POINTS - 1u], "each shaper draws its own curve", y[CD_POINTS - 1u], y2[CD_POINTS - 1u]);
  }

  /* 5. The codes: drive and model are needed, each unit is its role's. */
  {
    const struct omx_curve_term only_drive[] = {{OMX_CURVE_ROLES_DRIVE, 0, 12.0, OMX_CURVE_UNITS_DB, OMX_CURVE_NO_SHAPE, OMX_CURVE_ORIGINS_USER}};
    const struct omx_curve_term only_model[] = {{OMX_CURVE_ROLES_MODEL, 0, 1.0, OMX_CURVE_UNITS_NONE, OMX_CURVE_NO_SHAPE, OMX_CURVE_ORIGINS_USER}};
    const struct omx_curve_term mix_in_db[] = {{OMX_CURVE_ROLES_DRIVE, 0, 12.0, OMX_CURVE_UNITS_DB, OMX_CURVE_NO_SHAPE, OMX_CURVE_ORIGINS_USER},
                                               {OMX_CURVE_ROLES_MODEL, 0, 1.0, OMX_CURVE_UNITS_NONE, OMX_CURVE_NO_SHAPE, OMX_CURVE_ORIGINS_USER},
                                               {OMX_CURVE_ROLES_MIX, 0, 50.0, OMX_CURVE_UNITS_DB, OMX_CURVE_NO_SHAPE, OMX_CURVE_ORIGINS_USER}};
    struct omx_curve_request rq = {OMX_CURVE_DISPLAYS_TRANSFER_CURVE, OMX_CURVE_FAMILIES_DRIVE, 48000.0, 33u, 0, 0.0, 0.0, only_drive, 1u, 0};
    ok(omx_curve_points(&rq, x, y, OMX_CURVE_POINTS_MAX, &code) == 0u && code == OMX_CURVE_TERM_MISSING, "no model: term missing", code, 0);
    rq.terms = only_model;
    ok(omx_curve_points(&rq, x, y, OMX_CURVE_POINTS_MAX, &code) == 0u && code == OMX_CURVE_TERM_MISSING, "no drive: term missing", code, 0);
    rq.terms = mix_in_db;
    rq.nterms = 3u;
    ok(omx_curve_points(&rq, x, y, OMX_CURVE_POINTS_MAX, &code) == 0u && code == OMX_CURVE_UNIT_REFUSED, "a mix in dB: unit refused", code, 0);
    rq.kind = OMX_CURVE_DISPLAYS_EQ_RESPONSE;
    ok(omx_curve_points(&rq, x, y, OMX_CURVE_POINTS_MAX, &code) == 0u && code == OMX_CURVE_KIND_UNKNOWN, "a drive eq_response: kind unknown", code, 0);
  }
  ok(omx_curve_word(OMX_CURVE_FAMILY_WORDS, OMX_CURVE_FAMILIES_COUNT, "drive") == OMX_CURVE_FAMILIES_DRIVE &&
     omx_curve_word(OMX_CURVE_ROLE_WORDS, OMX_CURVE_ROLES_COUNT, "tone") == OMX_CURVE_ROLES_TONE &&
     omx_curve_word(OMX_CURVE_UNIT_WORDS, OMX_CURVE_UNITS_COUNT, "pc") == OMX_CURVE_UNITS_PC,
     "the drive's words are the description's", 0, 0);
  expect_clean();
}
