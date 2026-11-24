// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * The trim kernel through the generic face (fx/omx_fx_face.h, omx_trim_desc()) against the trim face's
 * own golden digests, test/golden/trim_instance.sha256, unchanged: the render of
 * test/fx/trim_instance_golden.test.c with the trim's three calls made through the face, the eq face
 * after it as there. A generic face that is not the trim face bit for bit moves them.
 *
 *   make test-fx                                compare with test/golden/trim_instance.sha256
 */
#include <stdint.h>

#include <omxdsp/fx/omx_eq_instance.h>
#include <omxdsp/fx/omx_fx_face.h>
#include <omxdsp/omx_trim.h>

#include "golden.h"
#include "strip_input_script.h"

#define BLOCK STRIP_INPUT_BLOCK

static void render(float sr, float *out) {
  struct omx_fx_face trim;
  struct omx_trim_kernel k;
  OmxEqInstance eq;
  omx_fx_face_init(&trim, omx_trim_desc(), &k, sr);
  omx_eq_instance_init(&eq, sr);
  int type[OMX_EQ_INSTANCE_BANDS], on[OMX_EQ_INSTANCE_BANDS];
  float freq[OMX_EQ_INSTANCE_BANDS], gain[OMX_EQ_INSTANCE_BANDS], q[OMX_EQ_INSTANCE_BANDS];
  for (int i = 0; i < OMX_EQ_INSTANCE_BANDS; i++)
    type[i] = (int)OMX_EQ_BAND_TYPES_DEFAULT, freq[i] = omx_eq_lv2_band_freq_default((uint32_t)i), gain[i] = 0.0f,
    q[i] = OMX_EQ_LV2_BAND_Q_DEFAULT, on[i] = 0;
  uint32_t lcg = 0x1234567u;
  float l[BLOCK], r[BLOCK], ol[BLOCK], orr[BLOCK];
  for (int o = 0; o < OMX_FX_GOLDEN_FRAMES; o += BLOCK) {
    const struct strip_input_words w = strip_input_words_at(o / BLOCK);
    const float v[1] = {w.trim_db};
    omx_fx_face_resolve(&trim, w.bypass, v);
    omx_eq_instance_resolve(&eq, w.bypass, w.hpf_on, w.hpf_freq, w.hpf_slope, w.lpf_on, w.lpf_freq, w.lpf_slope, type,
                            freq, gain, q, on);
    for (int i = 0; i < BLOCK; i++) omx_fx_golden_stimulus(o + i, &lcg, &l[i], &r[i]);
    omx_fx_face_run(&trim, l, r, ol, orr, BLOCK);
    omx_eq_instance_run(&eq, ol, orr, ol, orr, BLOCK);
    for (int i = 0; i < BLOCK; i++) { out[2 * (o + i)] = ol[i]; out[2 * (o + i) + 1] = orr[i]; }
  }
}

int main(int argc, char **argv) { return omx_fx_golden_main(argc, argv, "face_trim", render); }
