/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * omx_place_clamp.h — the console's clamp, shared by the placements that port the console's rules
 * (omx_fbs_place.c, omx_hrp_place.c): below lo is lo, above hi is hi, anything else passes through, a NaN too.
 * Doubles, as the console computes. One copy, so the two placements cannot drift apart on it.
 */
/**
 * @file omx_place_clamp.h
 * @brief the console's clamp, shared by the placements that port the console's rules (omx_fbs_place.c, omx_hrp_place.c): below lo is lo, above hi is hi, anything else passes through, a NaN too.
 */
#ifndef OMX_PLACE_CLAMP_H
#define OMX_PLACE_CLAMP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The console's clamp of `n` into [lo, hi]; a NaN passes through.
 * @param n The value.
 * @param lo The lower bound.
 * @param hi The upper bound.
 * @return lo when n < lo, hi when n > hi, else n (a NaN included).
 * @thread worker
 */
static inline double omx_place_clamp(double n, double lo, double hi) { return n < lo ? lo : (n > hi ? hi : n); }

#ifdef __cplusplus
}
#endif

#endif /* OMX_PLACE_CLAMP_H */
