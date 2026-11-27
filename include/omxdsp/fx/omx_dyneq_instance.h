/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * omx_dyneq_instance.h — the dynamic EQ as a plugin instance: six EQ bands, each with its own
 * dynamics, and no format in it. It mints no DSP: every band's static section is the EQ face's own
 * (fx/omx_eq_instance.h: its design, its clamps, its parking, its bank and its state), and every
 * band's dynamics are the band-dynamics word (omx_band_dyn.h), the word the de-esser runs.
 *
 *   1. RESOLVE. omx_dyneq_instance_resolve() takes the dyneq kernel's contract controls in
 *      their declared order (data/kernels/dyneq.json), one array per band control and the mix. The
 *      static half (switch, type, centre, gain, Q) goes to an EQ face whose first six bands are these
 *      and whose other bands and pass filters are off, so its bank is exactly the six bands that are
 *      on, in order. The dynamic half resolves to one band-dynamics atom per band: above the
 *      threshold for `down`, below it for `up`, the signed range, the band knee BAND_DYN_KNEE_DB, peak
 *      detection, no make-up; and `B`, designed by omx_band_dyn_design() at the band's clamped centre
 *      and Q only when the shape, centre or Q moved. Every travel is clamped into its declared range,
 *      a non-finite word reading as the declared default; a choice outside its ids reads as its
 *      default.
 *   2. RUN. In chunks of OMX_DYNEQ_INSTANCE_CHUNK frames, band by band: the band's static section
 *      over both legs (omx_biquad_cascade() on the EQ face's bank entry and state), then its dynamics
 *      over both legs, one detector for the two (omx_band_dyn_process()). The detector hears the band
 *      after its static section. With every band's dynamics off the output is the EQ face's at the
 *      same bands, bit for bit: the cascade runs the same sections in the same order on the same
 *      state, and a band whose dynamics are off touches no sample.
 *   3. LISTEN. While a band listens, the output is its `B` over the signal reaching the band, through
 *      a history of its own; with several bands listening, their sum. Listen ignores the mix.
 *   4. MIX. Below 100 % the output is the convex blend of the input and the processed signal
 *      (omx_wetdry_mix()); at 100 % it is the processed signal itself.
 *
 * READOUTS. omx_dyneq_instance_band_gain_db() is the dyneq kernel's band<n>GainDb: the band's
 * gain computer re-run on the level its detector holds after the last block, signed.
 *
 * Bypass is the identity and keeps every state word. LATENCY IS ZERO. Nothing is allocated; the
 * whole state is inline in {@link OmxDyneqInstance}.
 */
/**
 * @file omx_dyneq_instance.h
 * @brief the dynamic EQ as a plugin instance: six EQ bands, each with its own dynamics, and no format in it. It mints no DSP: every band's static section is the EQ face's own (fx/omx_eq_instance.h: its design, its clamps, its parking, its bank and its state), and every band's dynamics are the band-dynamics word (omx_band_dyn.h), the word the de-esser runs.
 */
#ifndef OMX_DYNEQ_INSTANCE_H
#define OMX_DYNEQ_INSTANCE_H

#include <stdint.h>
#include <string.h>

#include <omxdsp/fx/omx_eq_instance.h>
#include <omxdsp/omx_band_dyn.h>
#include <omxdsp/fx/omx_instance_io.h>
#include <omxdsp/omx_param.h>
#include <omxdsp/omx_wetdry.h>

/** @brief Audio legs in and out: TWO, a stereo pair. */
#define OMX_DYNEQ_INSTANCE_CHANNELS 2

/** The bands, the dyneq kernel's DYNEQ_BANDS. */
#define OMX_DYNEQ_INSTANCE_BANDS OMX_DYNEQ_BANDS

/** Frames per pass through the scratch; nothing observable depends on it. */
#define OMX_DYNEQ_INSTANCE_CHUNK 256u

_Static_assert(OMX_DYNEQ_INSTANCE_BANDS <= OMX_EQ_INSTANCE_BANDS, "the EQ face carries every band of the dynamic EQ");
_Static_assert((int)OMX_DYNEQ_BAND_TYPES_BELL == (int)OMX_EQ_BAND_TYPES_BELL &&
                   (int)OMX_DYNEQ_BAND_TYPES_LOW_SHELF == (int)OMX_EQ_BAND_TYPES_LOW_SHELF &&
                   (int)OMX_DYNEQ_BAND_TYPES_HIGH_SHELF == (int)OMX_EQ_BAND_TYPES_HIGH_SHELF,
               "a dynamic band's type is the EQ band type of the same index");
_Static_assert((int)OMX_DYNEQ_BAND_ONS_DEFAULT == (int)OMX_EQ_BAND_ONS_DEFAULT &&
                   (int)OMX_DYNEQ_BAND_ONS_ON == (int)OMX_EQ_BAND_ONS_ON,
               "a dynamic band switches and comes up as an EQ band does");

/** One instance: the EQ face that runs the static sections, one band-dynamics atom and state per band,
 * the listen histories and the scratch. */
typedef struct {
  /** The sample rate init bound the instance to, Hz. */
  float sr;
  /** The EQ face that holds the static sections: its first OMX_DYNEQ_INSTANCE_BANDS bands are these, its other bands
   * and its pass filters off; resolve() resolves it and run() runs its bank entries. */
  OmxEqInstance eq;
  /** Each band's band-dynamics atom: resolve() writes its dynamics and `B`; `enabled` while the band is on and its
   * dynMode is not off. */
  struct omx_band_dyn band[OMX_DYNEQ_INSTANCE_BANDS];
  /** Each band's band-dynamics state, its detector's envelope; init clears it and omx_band_dyn_process() carries it. */
  struct omx_band_dyn_state st[OMX_DYNEQ_INSTANCE_BANDS];
  /** Band i's entry in the EQ face's bank while it is on. */
  uint32_t entry[OMX_DYNEQ_INSTANCE_BANDS];
  /** Each band's switch as resolve() last resolved it, non-zero while on; run() skips a band that is off. */
  int on[OMX_DYNEQ_INSTANCE_BANDS];
  /** Each band's listen switch as resolve() last resolved it; it counts in run() only while the band is on. */
  int listen[OMX_DYNEQ_INSTANCE_BANDS];
  /** The shape `B` was last designed from, an enum omx_eq_kind value; `designed` 0 until the first. */
  int kind[OMX_DYNEQ_INSTANCE_BANDS];
  /** @var OmxDyneqInstance::q
   * The Q `B` was last designed from: the band's Q as the EQ face holds it, after its clamps. */
  /** The centre `B` was last designed from, Hz: the band's centre as the EQ face holds it, after its clamps. */
  float freq[OMX_DYNEQ_INSTANCE_BANDS], q[OMX_DYNEQ_INSTANCE_BANDS];
  /** Non-zero once resolve() has designed the band's `B`; init leaves it 0, so the first resolve designs every band. */
  int designed[OMX_DYNEQ_INSTANCE_BANDS];
  /** @var OmxDyneqInstance::listen_r
   * Each listening band's own history of `B`, right leg; cleared when the band starts listening. */
  /** Each listening band's own history of `B`, left leg; cleared when the band starts listening. */
  float listen_l[OMX_DYNEQ_INSTANCE_BANDS][4], listen_r[OMX_DYNEQ_INSTANCE_BANDS][4];
  /** The processed share, 0 to 1: mixPct as resolve() clamped it, over 100; init sets 1. */
  float mix;
  /** Non-zero while the effect is engaged, as the last resolve() set it; while zero, run() leaves the output as the input. */
  int enabled;
  /** Non-zero once init bound the instance to a usable rate; while zero, resolve() does nothing and run() leaves the output as the input. */
  int ready;
  /** @var OmxDyneqInstance::dry_r
   * Scratch: the chunk's right input, kept for the mix while the mix is under 1 and no band listens. */
  /** Scratch: the chunk's left input, kept for the mix while the mix is under 1 and no band listens. */
  float dry_l[OMX_DYNEQ_INSTANCE_CHUNK], dry_r[OMX_DYNEQ_INSTANCE_CHUNK];
  /** @var OmxDyneqInstance::sum_r
   * Scratch: the listening bands' sum over the chunk, right leg; the chunk's output while any band listens. */
  /** Scratch: the listening bands' sum over the chunk, left leg; the chunk's output while any band listens. */
  float sum_l[OMX_DYNEQ_INSTANCE_CHUNK], sum_r[OMX_DYNEQ_INSTANCE_CHUNK];
  /** @var OmxDyneqInstance::tap_r
   * Scratch: one listening band's `B` over the chunk, right leg, before it is added to `sum_r`. */
  /** Scratch: one listening band's `B` over the chunk, left leg, before it is added to `sum_l`. */
  float tap_l[OMX_DYNEQ_INSTANCE_CHUNK], tap_r[OMX_DYNEQ_INSTANCE_CHUNK];
} OmxDyneqInstance;

/** A choice word: inside [0, count), else the declared default.
 * @param v The host's word.
 * @param count The choice's member count.
 * @param dflt The choice's declared default id.
 * @return `v` when it is inside [0, count), else `dflt`.
 * @thread audio */
static inline int omx_dyneq_instance_choice(int v, int count, int dflt) { return v >= 0 && v < count ? v : dflt; }

/** The EQ design kind of a dynamic band's type.
 * @param type The band's type, a DYNEQ_BAND_TYPES id.
 * @return OMX_EQ_LOWSHELF for lowShelf, OMX_EQ_HIGHSHELF for highShelf, OMX_EQ_PEAKING for anything else.
 * @thread audio */
static inline enum omx_eq_kind omx_dyneq_instance_kind(int type) {
  return type == OMX_DYNEQ_BAND_TYPES_LOW_SHELF ? OMX_EQ_LOWSHELF
         : type == OMX_DYNEQ_BAND_TYPES_HIGH_SHELF ? OMX_EQ_HIGHSHELF
                                                   : OMX_EQ_PEAKING;
}

/**
 * Bind an instance to its rate. Returns 1 when usable, 0 when not (a refused init leaves `ready`
 * clear and run() is the identity): the rate must be one of the oracle set. Every band comes up off,
 * so a fresh instance is a wire. Allocates nothing.
 * @param s The instance; every word of it is written.
 * @param sr The sample rate, Hz, one of the full oracle set.
 * @return 1 when usable, 0 when not: a null instance, a rate outside the oracle set or one the EQ core refuses leaves `ready` clear and run() the identity.
 * @thread control
 */
static inline int omx_dyneq_instance_init(OmxDyneqInstance *s, float sr) {
  if (!s) return 0;
  memset(s, 0, sizeof(*s));
  if (!omx_rate_is_oracle_full(sr) || !omx_eq_instance_init(&s->eq, sr)) return 0;
  for (uint32_t i = 0; i < OMX_DYNEQ_INSTANCE_BANDS; i++) omx_band_dyn_state_init(&s->st[i]);
  s->sr = sr;
  s->mix = 1.0f;
  s->enabled = 1;
  s->ready = 1;
  return 1;
}

#define OMX_CONTRACT_STAGE "dyneq/instance-resolve"
/**
 * Resolve the host's control values for one cycle. `bypass` non-zero makes run() the identity. The
 * arguments are the dyneq kernel's contract controls in their declared order: per band (arrays of
 * OMX_DYNEQ_INSTANCE_BANDS) the switch (EQ_BAND_ONS index), the type (bell, lowShelf, highShelf), the
 * centre (Hz), the static gain (dB), the Q, the dynMode (off, down, up), the threshold (dB), the
 * ratio, the signed range (dB), the attack and the release (ms) and the listen switch; then the
 * mix (percent).
 * @param s The instance; null or not ready leaves it as it is.
 * @param bypass Non-zero makes run() the identity.
 * @param band_on Per band, bandOn, the place in off, on (0 to 1), default off: A band's switch.
 * @param band_type Per band, bandType, the place in bell, lowShelf, highShelf (0 to 2), default bell: The band's shape,
 *        in the order of the integer the kernel reads, which is the order of the first three EQ_BAND_TYPES: a bell, a
 *        low shelf, a high shelf.
 * @param freq Per band, freq, EQ_FREQ_RANGE.min to EQ_FREQ_RANGE.max Hz: The band's centre or corner, the EQ band's
 *        travel by reference.
 * @param gain Per band, gain, EQ_GAIN_RANGE.min to EQ_GAIN_RANGE.max dB, default 0: The band's static gain, the EQ
 *        band's travel by reference: the curve the band draws with its dynamics at rest.
 * @param q Per band, q, EQ_Q_RANGE.min to EQ_Q_RANGE.max, default 1: The bell's Q, the EQ band's travel by reference;
 *        the dynamics' bandpass takes the same Q.
 * @param dyn_mode Per band, dynMode, the place in off, down, up (0 to 2), default off: Whether and when the band's
 *        dynamics act, in the order of the integer the kernel reads: off, the band is its static section alone; down,
 *        it acts above its threshold, when the band is loud (band_dyn's above); up, it acts below its threshold, when
 *        the band is quiet (band_dyn's below).
 * @param threshold Per band, threshold, DEESS_THRESHOLD_RANGE.min to DEESS_THRESHOLD_RANGE.max dB, default
 *        DEESS_THRESHOLD_RANGE.default: Where the band's dynamics start, in dB of the band's own level after its static
 *        section: band_dyn's threshold travel by reference.
 * @param ratio Per band, ratio, DEESS_RATIO_RANGE.min to DEESS_RATIO_RANGE.max, default 2: How hard the band is held
 *        past its threshold: band_dyn's ratio travel by reference.
 * @param range Per band, range, DEESS_RANGE_RANGE.min to 12 dB, default -6: The most the dynamics add to the band's
 *        static gain, signed: below zero the deepest cut, above zero the largest boost.
 * @param attack Per band, attack, OMX_DETECTOR_OVERSAMPLE_AUTO_MS to COMP_LIMITS.attackMs.max ms, default 5: The band
 *        detector's attack, in ms: band_dyn's attack travel by reference, which stops at the oversampling floor; a step
 *        reaches 56.65 \% at the attack time (the 4-stage cascade).
 * @param release Per band, release, DEESS_RELEASE_RANGE.min to COMP_LIMITS.releaseMs.max ms, default 100: The band
 *        detector's release, in ms: band_dyn's release travel by reference; a step falls to 43.35 \% at the release
 *        time (the 4-stage cascade).
 * @param listen Per band, listen, the place in off, on (0 to 1), default off: While on, the output is what the band's
 *        detector hears, its section B over the signal reaching the band, in place of the EQ's output; with more than
 *        one band listening, their sum.
 * @param mix_pct mixPct, 0 to 100 \%, default 100: The share of the processed output, in percent: 100 is the dynamic EQ
 *        alone, bit for bit, 0 the input.
 * @post `one-bank-entry-per-band-that-is-on`: each leg's EQ bank holds one section per band that is on;
 *       `mix-in-unit-range`: the mix inside [0, 1].
 * @thread audio
 */
static inline void omx_dyneq_instance_resolve(OmxDyneqInstance *s, int bypass, const int band_on[OMX_DYNEQ_INSTANCE_BANDS],
                                              const int band_type[OMX_DYNEQ_INSTANCE_BANDS],
                                              const float freq[OMX_DYNEQ_INSTANCE_BANDS],
                                              const float gain[OMX_DYNEQ_INSTANCE_BANDS],
                                              const float q[OMX_DYNEQ_INSTANCE_BANDS],
                                              const int dyn_mode[OMX_DYNEQ_INSTANCE_BANDS],
                                              const float threshold[OMX_DYNEQ_INSTANCE_BANDS],
                                              const float ratio[OMX_DYNEQ_INSTANCE_BANDS],
                                              const float range[OMX_DYNEQ_INSTANCE_BANDS],
                                              const float attack[OMX_DYNEQ_INSTANCE_BANDS],
                                              const float release[OMX_DYNEQ_INSTANCE_BANDS],
                                              const int listen[OMX_DYNEQ_INSTANCE_BANDS], float mix_pct) {
  if (!s || !s->ready || !band_on || !band_type || !freq || !gain || !q || !dyn_mode || !threshold || !ratio || !range ||
      !attack || !release || !listen)
    return;
  int eq_type[OMX_EQ_INSTANCE_BANDS], eq_on[OMX_EQ_INSTANCE_BANDS], modes[OMX_DYNEQ_INSTANCE_BANDS];
  float eq_freq[OMX_EQ_INSTANCE_BANDS], eq_gain[OMX_EQ_INSTANCE_BANDS], eq_q[OMX_EQ_INSTANCE_BANDS];
  for (uint32_t i = OMX_DYNEQ_INSTANCE_BANDS; i < (uint32_t)OMX_EQ_INSTANCE_BANDS; i++) {
    eq_on[i] = 0;
    eq_type[i] = (int)OMX_EQ_BAND_TYPES_DEFAULT;
    eq_freq[i] = (float)OMX_EQ_FREQ_RANGE_MIN;
    eq_gain[i] = 0.0f;
    eq_q[i] = OMX_EQ_LV2_BAND_Q_DEFAULT;
  }
  float centres[OMX_DYNEQ_INSTANCE_BANDS];
  const int spread = omx_eq_default_centres(OMX_DYNEQ_INSTANCE_BANDS, centres) == OMX_DYNEQ_INSTANCE_BANDS;
  for (uint32_t i = 0; i < OMX_DYNEQ_INSTANCE_BANDS; i++) {
    const int sw = omx_dyneq_instance_choice(band_on[i], (int)OMX_DYNEQ_BAND_ONS_COUNT, (int)OMX_DYNEQ_BAND_ONS_DEFAULT);
    const int type = omx_dyneq_instance_choice(band_type[i], (int)OMX_DYNEQ_BAND_TYPES_COUNT, (int)OMX_DYNEQ_BAND_TYPES_DEFAULT);
    /* A non-finite centre is the band's come-up centre in a bank of six (omx_eq_default_centres). */
    const float fc = freq[i] - freq[i] == 0.0f
                         ? omx_clampf(freq[i], (float)OMX_DYNEQ_FREQ_RANGE_MIN, (float)OMX_DYNEQ_FREQ_RANGE_MAX)
                     : spread ? centres[i]
                              : (float)OMX_DYNEQ_FREQ_RANGE_MIN;
    const float g = omx_clamp_or(gain[i], (float)OMX_DYNEQ_GAIN_RANGE_MIN, (float)OMX_DYNEQ_GAIN_RANGE_MAX,
                                 (float)OMX_DYNEQ_GAIN_RANGE_DEFAULT);
    const float bq = omx_clamp_or(q[i], (float)OMX_DYNEQ_Q_RANGE_MIN, (float)OMX_DYNEQ_Q_RANGE_MAX,
                                  (float)OMX_DYNEQ_Q_RANGE_DEFAULT);
    const int dyn_mode_i = omx_dyneq_instance_choice(dyn_mode[i], (int)OMX_DYNEQ_DYN_MODES_COUNT,
                                                     (int)OMX_DYNEQ_DYN_MODES_DEFAULT);
    const int lis = omx_dyneq_instance_choice(listen[i], (int)OMX_DYNEQ_LISTENS_COUNT, (int)OMX_DYNEQ_LISTENS_DEFAULT);
    const float thr = omx_clamp_or(threshold[i], (float)OMX_DYNEQ_THRESHOLD_RANGE_MIN, (float)OMX_DYNEQ_THRESHOLD_RANGE_MAX,
                                   (float)OMX_DYNEQ_THRESHOLD_RANGE_DEFAULT);
    const float rat = omx_clamp_or(ratio[i], (float)OMX_DYNEQ_RATIO_RANGE_MIN, (float)OMX_DYNEQ_RATIO_RANGE_MAX,
                                   (float)OMX_DYNEQ_RATIO_RANGE_DEFAULT);
    const float rng = omx_clamp_or(range[i], (float)OMX_DYNEQ_RANGE_RANGE_MIN, (float)OMX_DYNEQ_RANGE_RANGE_MAX,
                                   (float)OMX_DYNEQ_RANGE_RANGE_DEFAULT);
    const float att = omx_clamp_or(attack[i], (float)OMX_DYNEQ_ATTACK_RANGE_MIN, (float)OMX_DYNEQ_ATTACK_RANGE_MAX,
                                   (float)OMX_DYNEQ_ATTACK_RANGE_DEFAULT);
    const float rel = omx_clamp_or(release[i], (float)OMX_DYNEQ_RELEASE_RANGE_MIN, (float)OMX_DYNEQ_RELEASE_RANGE_MAX,
                                   (float)OMX_DYNEQ_RELEASE_RANGE_DEFAULT);
    eq_on[i] = sw;
    eq_type[i] = type;
    eq_freq[i] = fc;
    eq_gain[i] = g;
    eq_q[i] = bq;
    modes[i] = dyn_mode_i;
    if (lis && !s->listen[i]) {
      memset(s->listen_l[i], 0, sizeof s->listen_l[i]);
      memset(s->listen_r[i], 0, sizeof s->listen_r[i]);
    }
    s->listen[i] = lis;
    struct omx_band_dyn *b = &s->band[i];
    b->dyn.enabled = 1;
    b->dyn.gc.mode = dyn_mode_i == OMX_DYNEQ_DYN_MODES_UP ? OMX_DYN_BELOW : OMX_DYN_ABOVE;
    b->dyn.gc.thresh_db = thr;
    b->dyn.gc.ratio = rat;
    b->dyn.gc.knee_db = (float)OMX_BAND_DYN_KNEE_DB;
    b->dyn.gc.range_db = rng;
    b->dyn.gc.makeup_lin = 1.0f;
    b->dyn.detect = OMX_DETECT_PEAK;
    b->dyn.ovs_mode = OMX_DYN_OVS_OFF;
    b->dyn.attack_ms = att;
    b->dyn.attack_coeff = omx_pole_from_time_ms(att, s->sr);
    b->dyn.release_coeff = omx_pole_from_time_ms(rel, s->sr);
  }
  omx_eq_instance_resolve(&s->eq, 0, 0, OMX_EQ_LV2_HPF_FREQ_DEFAULT, (int)OMX_FILTER_SLOPES_DEFAULT, 0,
                          OMX_EQ_LV2_LPF_FREQ_DEFAULT, (int)OMX_FILTER_SLOPES_DEFAULT, eq_type, eq_freq, eq_gain, eq_q,
                          eq_on);
  omx_eq_lv2_publish(&s->eq.leg[0]);
  omx_eq_lv2_publish(&s->eq.leg[1]);
  uint32_t k = 0;
  for (uint32_t i = 0; i < OMX_DYNEQ_INSTANCE_BANDS; i++) {
    /* The band as the EQ face holds it (its clamps applied), the band B is designed from. */
    const struct omx_eq_lv2_band *eb = &s->eq.leg[0].band[i];
    s->on[i] = eq_on[i];
    s->entry[i] = k;
    k += (uint32_t)s->on[i];
    struct omx_band_dyn *b = &s->band[i];
    b->enabled = s->on[i] && modes[i] != OMX_DYNEQ_DYN_MODES_OFF;
    const int kind = (int)omx_dyneq_instance_kind(eb->type);
    if (!s->designed[i] || kind != s->kind[i] || eb->freq_hz != s->freq[i] || eb->q != s->q[i]) {
      omx_band_dyn_design((enum omx_eq_kind)kind, (double)eb->freq_hz, (double)eb->q, (double)s->sr, b->b_c);
      s->kind[i] = kind;
      s->freq[i] = eb->freq_hz;
      s->q[i] = eb->q;
      s->designed[i] = 1;
    }
  }
  const float mix_share = omx_clamp_or(mix_pct, (float)OMX_DYNEQ_MIX_PCT_RANGE_MIN, (float)OMX_DYNEQ_MIX_PCT_RANGE_MAX,
                                       (float)OMX_DYNEQ_MIX_PCT_RANGE_DEFAULT) /
                          100.0f;
  s->mix = mix_share;
  s->enabled = bypass ? 0 : 1;
  OMX_POST(k == s->eq.leg[0].count && k == s->eq.leg[1].count, "one-bank-entry-per-band-that-is-on");
  OMX_POST(s->mix >= 0.0f && s->mix <= 1.0f, "mix-in-unit-range");
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "dyneq/instance-chunk"
/** One chunk, in place over `l` and `r`: band by band, the static section and the dynamics; the
 * listened bands' sum, or the mix.
 * @param s The instance, ready.
 * @param l The left leg, `m` frames, processed in place.
 * @param r The right leg, `m` frames, processed in place.
 * @param m The chunk length, frames.
 * @post `finite-out`: both legs' `m` frames are finite.
 * @thread audio */
static inline void omx_dyneq_instance_chunk(OmxDyneqInstance *s, float *l, float *r, uint32_t m) {
  struct omx_eq_lv2 *el = &s->eq.leg[0], *er = &s->eq.leg[1];
  int listening = 0;
  for (uint32_t i = 0; i < OMX_DYNEQ_INSTANCE_BANDS; i++) listening |= s->on[i] && s->listen[i];
  if (s->mix < 1.0f && !listening) {
    memcpy(s->dry_l, l, m * sizeof(float));
    memcpy(s->dry_r, r, m * sizeof(float));
  }
  if (listening) {
    memset(s->sum_l, 0, m * sizeof(float));
    memset(s->sum_r, 0, m * sizeof(float));
  }
  for (uint32_t i = 0; i < OMX_DYNEQ_INSTANCE_BANDS; i++) {
    if (!s->on[i]) continue;
    const uint32_t k = s->entry[i];
    omx_biquad_cascade(l, m, 1u, (const float(*)[5]) & el->coeffs[k], &el->enabled[k], &el->state[k]);
    omx_biquad_cascade(r, m, 1u, (const float(*)[5]) & er->coeffs[k], &er->enabled[k], &er->state[k]);
    if (s->listen[i]) {
      for (uint32_t j = 0; j < m; j++) {
        s->tap_l[j] = omx_biquad(l[j], s->band[i].b_c, s->listen_l[i]);
        s->tap_r[j] = omx_biquad(r[j], s->band[i].b_c, s->listen_r[i]);
      }
      for (uint32_t j = 0; j < m; j++) {
        s->sum_l[j] += s->tap_l[j];
        s->sum_r[j] += s->tap_r[j];
      }
    }
    omx_band_dyn_process(l, r, m, &s->band[i], &s->st[i]);
  }
  if (listening) {
    memcpy(l, s->sum_l, m * sizeof(float));
    memcpy(r, s->sum_r, m * sizeof(float));
  } else if (s->mix < 1.0f) {
    const float dry = 1.0f - s->mix;
    for (uint32_t j = 0; j < m; j++) omx_wetdry_mix(&l[j], &r[j], s->dry_l[j], s->dry_r[j], l[j], r[j], dry, s->mix);
  }
  OMX_POST(omx_block_pair_finite(l, r, m), "finite-out");
}
#undef OMX_CONTRACT_STAGE

#define OMX_CONTRACT_STAGE "dyneq/instance-run"
/**
 * THE AUDIO CALLBACK'S WHOLE SHARE: copy in to out where they differ, run the bands in place on out.
 * `in_*` and `out_*` may alias. Not ready, or bypassed, is the identity and moves no state word.
 * @param s The instance; null, not ready or bypassed leaves `out_*` holding `in_*`.
 * @param in_l The left input, `n` frames.
 * @param in_r The right input, `n` frames.
 * @param out_l The left output, `n` frames, may be `in_l`.
 * @param out_r The right output, `n` frames, may be `in_r`.
 * @param n The block length, frames; a null port or 0 touches nothing.
 * @pre `finite-in`: the input, copied through to the output legs, is finite.
 * @thread audio
 */
static inline void omx_dyneq_instance_run(OmxDyneqInstance *s, const float *in_l, const float *in_r, float *out_l,
                                          float *out_r, uint32_t n) {
  if (!omx_instance_copy_through(in_l, in_r, out_l, out_r, n)) return;
  if (!s || !s->ready || !s->enabled) return;
  OMX_PRE(omx_block_pair_finite(out_l, out_r, n), "finite-in");
  for (uint32_t o = 0; o < n; o += OMX_DYNEQ_INSTANCE_CHUNK) {
    const uint32_t m = n - o < OMX_DYNEQ_INSTANCE_CHUNK ? n - o : OMX_DYNEQ_INSTANCE_CHUNK;
    omx_dyneq_instance_chunk(s, out_l + o, out_r + o, m);
  }
}
#undef OMX_CONTRACT_STAGE

/**
 * Band `i`'s readout, band<i+1>GainDb: the gain its dynamics apply now, dB, signed (a cut negative, a
 * boost positive), inside the range travel. 0 for a band that is off, a band whose dynamics are off,
 * and a refused instance.
 * @param s The instance; null or not ready reads 0.
 * @param i The band, 0 to OMX_DYNEQ_INSTANCE_BANDS - 1; any other reads 0.
 * @return The band's dynamic gain now, dB, signed; 0 when the band or its dynamics are off.
 * @thread audio
 */
static inline float omx_dyneq_instance_band_gain_db(const OmxDyneqInstance *s, uint32_t i) {
  if (!s || !s->ready || i >= OMX_DYNEQ_INSTANCE_BANDS || !s->band[i].enabled) return 0.0f;
  const struct omx_band_dyn_block bk = omx_band_dyn_begin(&s->band[i].dyn);
  return omx_band_dyn_offset_db(&s->band[i].dyn.gc, omx_env_level(&s->st[i].env, &bk.ep));
}

/** The frames of latency the instance introduces: none.
 * @param s The instance.
 * @return 0.
 * @thread audio */
static inline uint32_t omx_dyneq_instance_latency(const OmxDyneqInstance *s) {
  (void)s;
  return 0u;
}

#endif /* OMX_DYNEQ_INSTANCE_H */
