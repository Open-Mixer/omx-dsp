// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * The clipper kernel through the generic face (fx/omx_fx_face.h, omx_clipper_desc()) against the clipper
 * face's own golden digests, test/golden/clipper_instance.sha256, unchanged: the render of
 * test/fx/clipper_instance_golden.test.c with its calls made through the face. A generic face that is not
 * the clipper face bit for bit moves them.
 *
 *   make test-fx                                compare with test/golden/clipper_instance.sha256
 */
#include <stdint.h>

#define OMX_FX_GOLDEN_ORACLE_RATES 1
#include <omxdsp/fx/omx_fx_face.h>
#include <omxdsp/omx_clipper.h>

#include "golden.h"

#define BLOCK 128

static void render(float sr, float *out) {
  static struct omx_fx_face f;
  static struct omx_clipper_kernel k;
  omx_fx_face_init(&f, omx_clipper_desc(), &k, sr);
  uint32_t lcg = 0x1234567u;
  float l[BLOCK], r[BLOCK], ol[BLOCK], orr[BLOCK];
  for (int o = 0; o < OMX_FX_GOLDEN_FRAMES; o += BLOCK) {
    const float u = (float)o / (float)OMX_FX_GOLDEN_FRAMES;
    const float v[3] = {24.0f * u, -12.0f * u, u < 0.5f ? 0.0f : 60.0f};
    omx_fx_face_resolve(&f, 0, v);
    for (int i = 0; i < BLOCK; i++) omx_fx_golden_stimulus(o + i, &lcg, &l[i], &r[i]);
    omx_fx_face_run(&f, l, r, ol, orr, BLOCK);
    for (int i = 0; i < BLOCK; i++) { out[2 * (o + i)] = ol[i]; out[2 * (o + i) + 1] = orr[i]; }
  }
}

int main(int argc, char **argv) { return omx_fx_golden_main(argc, argv, "face_clipper", render); }
