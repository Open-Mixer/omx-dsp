// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * omx_curve_wasm.c: omx_curve_points_json (omx_curve_json.h) as a WebAssembly module, the one body every JavaScript
 * runtime loads (the typed-contracts spec, section 4.3). Built by tools/wasm-build.sh (`make wasm`); the module exports
 * four names and imports only the WASI functions wasi-libc's stdio is linked against (clock_time_get, fd_close,
 * fd_seek, fd_write, random_get: snprintf's FILE machinery), none of which a call
 * reaches, so a host's stand-ins THROW if one is ever called (tools/wasm-check.mjs, ts/src/wasm.ts) and no WASI is
 * implemented:
 *
 *   omx_curve_wasm_request()   the address of the request buffer, OMX_CURVE_JSON_OUT_MAX bytes: write the document here
 *   omx_curve_wasm_answer()    the address of the answer buffer, the same size
 *   omx_curve_wasm_capacity()  the size of both
 *   omx_curve_wasm_call(len)   answer the `len`-byte request: the answer's length, or omx_curve_points_json()'s
 *                              negative return (OMX_CURVE_JSON_MALFORMED, OMX_CURVE_JSON_CAP)
 *
 * The buffers are static: one call at a time, which a JavaScript thread makes anyway.
 */
#include <omxdsp/omx_curve_json.h>

#define OMX_WASM_EXPORT(name) __attribute__((export_name(#name))) name

static char g_request[OMX_CURVE_JSON_OUT_MAX], g_answer[OMX_CURVE_JSON_OUT_MAX];

char *OMX_WASM_EXPORT(omx_curve_wasm_request)(void) { return g_request; }
char *OMX_WASM_EXPORT(omx_curve_wasm_answer)(void) { return g_answer; }
uint32_t OMX_WASM_EXPORT(omx_curve_wasm_capacity)(void) { return OMX_CURVE_JSON_OUT_MAX; }

int OMX_WASM_EXPORT(omx_curve_wasm_call)(uint32_t len) {
  if (len >= sizeof g_request) return OMX_CURVE_JSON_MALFORMED;
  g_request[len] = '\0';
  return omx_curve_points_json(g_request, g_answer, sizeof g_answer);
}
