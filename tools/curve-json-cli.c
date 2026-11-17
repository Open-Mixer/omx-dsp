// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * curve-json-cli — omx_curve_points_json() over standard input and output: one request document in, its answer
 * out, exit 0 for an answer, 1 for a malformed request and 2 for an answer that did not fit. It is the native door
 * of test/curve-vectors.mjs (`make curve-vectors`), which reads vectors/curve-points.json once and puts each row to
 * this binary and to the WebAssembly build.
 */
#include <stdio.h>

#include <omxdsp/omx_curve_json.h>

int main(void) {
  static char in[OMX_CURVE_JSON_OUT_MAX], out[OMX_CURVE_JSON_OUT_MAX];
  const size_t n = fread(in, 1, sizeof in - 1u, stdin);
  in[n] = '\0';
  const int w = omx_curve_points_json(in, out, sizeof out);
  if (w < 0) return w == OMX_CURVE_JSON_MALFORMED ? 1 : 2;
  fputs(out, stdout);
  return 0;
}
