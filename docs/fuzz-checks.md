<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> -->

# The checks of the fuzz targets

Every property the harnesses of `fuzz/` hold the library to, read one by one: 104 checks, each an
`if (...) abort();` with its comment above it, and the stops that are not properties. This is the reading OSS Verify's
enrollment review asks for (items F5 and F8 of its `review.md`): does each check prove what its comment says, in both
directions, and does its expected value come from the harness rather than from the library under test.

How to read a row:

- **Check**: the file and line of the `abort()`.
- **Property**: the check's comment, less its records: the claim.
- **Expected value**: what the library's answer is compared with, and where the harness gets it. A value the
  harness works out itself, a constant of the contract, or the caller's own input are independent of the code under
  test. Where a check compares the library with itself (a determinism or aliasing property), the row says so.
- **Breakage**: the record in the check's comment: a one-line change of the library (`file:line`, the text before and
  after) and the seed of `fuzz/corpus/<harness>/` that makes this check abort with the change made. Each record is
  run alone: the library is changed, the harness rebuilt, and the seed must stop at this `abort()`; without the
  change every seed passes.
- **Verdict**: OK when the check tests what the comment states, its expected value is right and independent, and its
  breakage makes it fail. WEAK would mean a wrong library can pass it; WRONG, that the comment or the value is wrong.

## Verdicts

All 104 checks are OK. Five were WEAK before this reading, and are fixed in the harnesses as they stand now:

- `fuzz_fx`: a refused init was held to run() as the identity, but which rates an init refuses was the library's own
  answer. A face bound to the grid that took every rate, or a dynamics face that took only the grid, passed every
  check. A new check holds each init to the rates its face owes, by the harness's `rate_owed()`.
- `fuzz_eq`: a section was held to its band only by its gain at DC, which is 1 for a peaking and a high-shelf band
  whatever their gain. A bell designed flat, or with its gain negated, passed. Two new checks read a bell's gain at its
  centre and a high shelf's at Nyquist, by the harness's own evaluation of the section.
- `fuzz_src`, two checks: the bounds on the outputs of a take were read from `omx_src_out_frames()` and
  `omx_src_out_capacity()`, the library's own counts. The harness now works both out from the reduced ratio.
- `fuzz_note_map`: the held table was compared with the harness's account by its count of keys only. It is now
  compared key by key.

Each of these has a record of a breakage that the old checks let through and the new one stops: the ones in the rows
of `fuzz_fx.c` (the init check), `fuzz_eq.c` (the bell and the shelf) and `fuzz_note_map.c` (the table, `key ^ 1`).

## Shared by every harness: `fuzz/fuzz.h`

| Check | Property | Expected value | Breakage | Verdict |
|---|---|---|---|---|
| `fuzz.h:167` | No contract the library states is broken. | the library's own PRE and POST laws, read from the ledger; the claim is that none is broken, not a value | `omx_onepole.h:124` `pole >= 0.0f && pole < 1.0f` → `pole < 0.0f`, on `onepole`<br>`omx_dyn.h:875` `gain[i] = g * p->gc.makeup_lin;` → `gain[i] = g * p->gc.makeup_lin * (prog && done + i >= 64u && !omx_rate_is_declared(p->prp[0].sr) ? 2.0f : 1.0f);`, on `comp-feedback-opto-64000` | OK |

## `fuzz_fx`: the instance faces

| Check | Property | Expected value | Breakage | Verdict |
|---|---|---|---|---|
| `fuzz_fx.c:319` | Two inits at one rate answer the same. | the face's own second answer: a determinism property | `fx/omx_trim_instance.h:71` `return 1;` → `{ static int calls; return calls++ % 2; }`, on `trim-neutral` | OK |
| `fuzz_fx.c:329` | An init takes exactly the rates its face owes a yes: a face bound to the grid refuses every rate off it, and a face that takes any rate above its floor refuses none of them. | the harness's `rate_owed()`: the nine grid rates from the contract, or the face's own floor (above 0; twice the tremolo's fastest rate, the phaser's top corner, the graphic EQ's 20 kHz ISO centre) | `fx/omx_instance_io.h:114` `return omx_rate_is_oracle_full(sr);` → `return sr > 0.0f;`, on `refused-rate`<br>`fx/omx_instance_io.h:129` `return sr > 0.0f;` → `return omx_rate_is_oracle_full(sr);`, on `comp-22050` | OK |
| `fuzz_fx.c:358` | Aliasing is safe: running in place gives the samples of running out of place, bit for bit. | instance B, run out of place on a copy of the block | `fx/omx_trim_instance.h:98` `n)) return;` → `n)) return; if (out_l == in_l) out_l[0] += 1.0f;`, on `trim-neutral` | OK |
| `fuzz_fx.c:366` | A refused init leaves run() the identity, bit for bit, whatever the ports say. | the harness's own copy of the block | `fx/omx_instance_io.h:156` `!(s)->ready) return;` → `!(s)->ready) { (out_l)[0] = 0.5f; return; }`, on `refused-rate` | OK |
| `fuzz_fx.c:374` | An instance that took finite samples gives finite samples back, at any port value. | `fuzz_all_finite()`, the harness's finiteness test | `fx/omx_trim_instance.h:101` `out_l[i] *= omx_ramp_at(g, i)` → `out_l[i] = NAN`, on `trim-neutral` | OK |

## `fuzz_laws`: the scalar laws

| Check | Property | Expected value | Breakage | Verdict |
|---|---|---|---|---|
| `fuzz_laws.c:69` | A clamp lands inside its travel, whatever the word. | the travel [lo, hi] the harness passed | `omx_param.h:38` `(v <= hi ? v : hi)` → `v`, on `clamp` | OK |
| `fuzz_laws.c:77` | A word already inside the travel comes back unchanged. | the word itself | `omx_param.h:38` `(v <= hi ? v : hi)` → `(v <= hi ? lo : hi)`, on `clamp` | OK |
| `fuzz_laws.c:88` | omx_clamp_or lands inside the travel too. | the travel the harness passed | `omx_param.h:58` `omx_clampf(v, lo, hi)` → `v`, on `clamp` | OK |
| `fuzz_laws.c:96` | A word that is not finite reads as the declared default. | the default the harness passed | `omx_param.h:58` `: dflt` → `: lo`, on `clamp` | OK |
| `fuzz_laws.c:105` | omx_unit lands in [0, 1]. | the constant travel [0, 1] | `omx_param.h:70` `omx_clampf(v, 0.0f, 1.0f)` → `v`, on `clamp` | OK |
| `fuzz_laws.c:117` | dB to linear is never negative. | 0, by the definition 10^(dB/20) | `omx_units.h:40` `powf(10.0f, db * 0.05f)` → `-powf(10.0f, db * 0.05f)`, on `units` | OK |
| `fuzz_laws.c:126` | Inside +-150 dB, dB to linear is finite and above 0. | 10^(dB/20) is finite and above 0 there | `omx_units.h:40` `powf(10.0f, db * 0.05f)` → `0.0f * powf(10.0f, db * 0.05f)`, on `units` | OK |
| `fuzz_laws.c:134` | Linear to dB brings a gain back to its dB, to 0.01 dB. | the dB the harness started from; the two directions share no code (powf against log10f) | `omx_units.h:53` `20.0f * log10f` → `21.0f * log10f`, on `units` | OK |
| `fuzz_laws.c:142` | The polynomial dB to linear is finite and above 0 inside +-150 dB. | finite and above 0, the documented range of 10^(dB/20) | `omx_units.h:159` `p * scale` → `p * scale * NAN`, on `units` | OK |
| `fuzz_laws.c:153` | Linear to dB keeps its floor and stays finite for any finite word: 0 is the floor, not -Inf. | the documented floor OMX_LIN_TO_DB_FLOOR_DB | `omx_units.h:53` `lin < floor_lin ? floor_lin : lin` → `lin`, on `units` | OK |
| `fuzz_laws.c:163` | The polynomial linear to dB is finite for any finite word. | finite, the documented POST | `omx_units.h:124` `8.685889638065037f` → `NAN`, on `units` | OK |
| `fuzz_laws.c:176` | The pan law is constant power: the squares of the two legs add up to 1. | cos^2 + sin^2 = 1, constant power | `omx_fader_law.h:59` `sinf(theta)` → `cosf(theta)`, on `pan-fader` | OK |
| `fuzz_laws.c:186` | The balance law never boosts: both legs in [0, 1], and one of them is 1. | [0, 1] with one leg at 1, the documented law | `omx_balance_law.h:49` `1.0f - p` → `1.0f + p`, on `pan-fader` | OK |
| `fuzz_laws.c:204` | The fader law's coefficient is finite and not negative. | finite and not below 0 | `omx_fader_law.h:87` `fader * pan` → `-fader * pan`, on `pan-fader` | OK |
| `fuzz_laws.c:212` | A muted strip or DCA has a coefficient of 0. | 0 when muted, the documented law | `omx_fader_law.h:85` `p->mute ? 0.0f : 1.0f` → `p->mute ? 1.0f : 1.0f`, on `pan-fader` | OK |
| `fuzz_laws.c:234` | A ramp starts at the applied value, 0 for one that is not finite. | the start the harness passed (0 for a non-finite one) | `omx_ramp.h:58` `r.cur + r.step` → `r.tgt + r.step`, on `ramp` | OK |
| `fuzz_laws.c:243` | No sample of a ramp leaves the span of its two ends. | the span of the two ends the harness passed | `omx_ramp.h:58` `r.step * (float)i` → `2.0f * r.step * (float)i`, on `ramp` | OK |
| `fuzz_laws.c:252` | A ramp ends on its target. | the target the harness passed (0 for a non-finite one) | `omx_ramp.h:68` `*cur = r.tgt` → `*cur = r.cur`, on `ramp` | OK |
| `fuzz_laws.c:268` | A wet/dry mix of finite samples lies between the dry one and the wet one. | the two samples the harness passed | `omx_wetdry.h:36` `dry * xl` → `2.0f * dry * xl`, on `wetdry` | OK |
| `fuzz_laws.c:283` | Each sample of a cross-fade lies between the two bodies. | the two bodies the harness passed | `omx_wetdry.h:82` `(step * (float)i)` → `(2.0f * step * (float)i)`, on `wetdry` | OK |
| `fuzz_laws.c:296` | A time constant gives a pole in [0, 1), however long the time. | the constant interval [0, 1) | `omx_onepole.h:123` `fminf(expf(-1.0f / (ms * 0.001f * sr)), OMX_POLE_MAX)` → `expf(-1.0f / (ms * 0.001f * sr))`, on `onepole` | OK |
| `fuzz_laws.c:303` | A corner gives a pole in [0, 1), however low the corner. | the constant interval [0, 1) | `omx_onepole.h:143` `fminf(expf(-2.0f * (float)M_PI * hz / sr), OMX_POLE_MAX)` → `expf(-2.0f * (float)M_PI * hz / sr)`, on `onepole` | OK |
| `fuzz_laws.c:312` | A rescaled pole stays in [0, 1]. | the constant interval [0, 1] | `omx_onepole.h:169` `powf(p, ref / sr)` → `2.0f`, on `onepole` | OK |
| `fuzz_laws.c:322` | One step of the filter lies between the input and the state, and the state is what it returns. | the input and state the harness passed | `omx_onepole.h:52` `x * (1.0f - pole)` → `2.0f * x * (1.0f - pole)`, on `onepole` | OK |
| `fuzz_laws.c:334` | A step is one sample's travel at most, and never none. | the constant interval (0, 1] | `mix/omx_slew.h:49` `s < 1.0f ? (s > FLT_MIN ? s : FLT_MIN) : 1.0f` → `s`, on `slew` | OK |
| `fuzz_laws.c:351` | The cursor stays between where it was and its target, and is exactly the target once the walk fits the block. | the start and target the harness passed | `mix/omx_slew.h:67` `up < tgt ? up : tgt` → `up`, on `slew` | OK |
| `fuzz_laws.c:362` | The hold's way back to open waits for a carried sample: over silence the position stays. | the position the harness passed | `mix/omx_slew.h:127` `h < t \|\| xl != 0.0f \|\| xr != 0.0f` → `1`, on `slew` | OK |

## `fuzz_eq`: the EQ design and run path

| Check | Property | Expected value | Breakage | Verdict |
|---|---|---|---|---|
| `fuzz_eq.c:108` | The head filters write at most their limit of sections. | OMX_EQ_PASS_MAX_SECTIONS, the documented limit | `omx_eq_design.h:554` `return n;` → `return n + 9u;`, on `bank-0` | OK |
| `fuzz_eq.c:116` | Each section of the head filters is stable and finite, by the harness's own reading of its coefficients. | the harness's `stable()`, the Jury conditions | `omx_eq_design.h:552` `coeffs[i]);` → `coeffs[i]), coeffs[i][4] = 2.0;`, on `bank-0` | OK |
| `fuzz_eq.c:140` | The bank has at most as many sections as bands. | the band count the harness built | `omx_eq_design.h:497` `return n;` → `return nbands + 1u;`, on `bank-0` | OK |
| `fuzz_eq.c:147` | The bank has a section for each band that is on, and none with the EQ off. | the harness's count of bands that are on | `omx_eq_design.h:491` `if (!b->on) continue;` → ``, on `bank-off-band` | OK |
| `fuzz_eq.c:157` | Each section of the bank is stable, by the harness's own reading of its coefficients. | the harness's `stable()`, the Jury conditions | `omx_eq_design.h:492` `coeffs[n]);` → `coeffs[n]), coeffs[n][4] = 2.0;`, on `bank-0` | OK |
| `fuzz_eq.c:165` | Each coefficient of the bank is finite. | finite | `omx_eq_design.h:492` `coeffs[n]);` → `coeffs[n]), coeffs[n][0] = NAN;`, on `bank-0` | OK |
| `fuzz_eq.c:175` | A section is parked exactly when its band is a peaking or shelf band at 0 dB. | the harness's `parked()`: a peaking or shelf band at 0 dB | `omx_eq_design.h:493` `? 0u : 1u` → `? 1u : 1u`, on `parked-and-live` | OK |
| `fuzz_eq.c:185` | A section has the gain at DC of the i-th band that is on. The denominator is positive in a stable section; the tolerance leaves room for its rounding at the lowest corner (about 1e-5). | the harness's `dc_gain()`: 10^(g/20) for a low shelf, 0 for high-pass and band-pass, else 1 | `omx_eq_design.h:492` `b->kind, b->freq_hz, b->q, b->gain_db` → `bands->kind, bands->freq_hz, bands->q, bands->gain_db`, on `shelf-then-shelf` | OK |
| `fuzz_eq.c:200` | A peaking section has its band's gain at the band's centre, on its side of 0 dB: a boost boosts and a cut cuts, by at least the gain asked, to 0.1 % (the prototype bell is exactly the gain there; the matched design is that or more near Nyquist, never less). For a bell a console sets: from 20 Hz, no narrower than Q 100; below that the bell is narrower than its double coefficients resolve. | the band's gain in dB, against the harness's own evaluation of \|H\| at the clamped centre | `omx_eq_design.h:164` `omx_eq_matched_pair(w0, 1.0 / (2.0 * A * qq), p);` → `omx_eq_matched_pair(w0, A / (2.0 * qq), p);`, on `peak-then-shelf`<br>`omx_eq_design.h:160` `exp(gain_db * (M_LN10 / 40.0))` → `exp(-gain_db * (M_LN10 / 40.0))`, on `peak-then-shelf` | OK |
| `fuzz_eq.c:212` | A high shelf whose corner is at most an eighth of the rate has at Nyquist, three octaves or more above it, at least a quarter of its band's gain, on its side of 0 dB: past its midpoint a shelf of any Q has gone at least half way to its gain, and the check asks half of that. | a quarter of the band's gain, against the harness's own evaluation of \|H\| at Nyquist | `omx_eq_design.h:180` `omx_eq_matched_pair(w0 * sa, 1.0 / (2.0 * qq), p);` → `omx_eq_matched_pair(w0 / sa, 1.0 / (2.0 * qq), p);`, on `peak-then-shelf` | OK |
| `fuzz_eq.c:227` | The bank's magnitude is finite at any frequency. | finite | `omx_magnitude.h:129` `db_out[i] = db;` → `db_out[i] = NAN;`, on `bank-0` | OK |
| `fuzz_eq.c:235` | A bank with no live section is 0 dB. | 0 dB, with `live` counted by the harness's `parked()` | `omx_magnitude.h:125` `double db = 0.0;` → `double db = 1.0;`, on `bank-3` | OK |
| `fuzz_eq.c:258` | The cascade gives finite samples for finite samples. | `fuzz_all_finite()` | `omx_biquad.h:196` `enabled, state, NULL);` → `enabled, state, NULL); buf[0] = NAN;`, on `bank-2` | OK |
| `fuzz_eq.c:266` | A bank with no live section leaves the block as it was, bit for bit. | the harness's copy of the block | `omx_biquad.h:196` `enabled, state, NULL);` → `enabled, state, NULL); buf[0] = 0.5f;`, on `eq-off-mono` | OK |
| `fuzz_eq.c:273` | In a stereo run, a bank with no live section leaves the right leg as it was too. | the harness's copy of the right leg | `omx_biquad.h:170` `OMX_POST(!r \|\|` → `if (r && !nl) r[0] = 0.5f; OMX_POST(!r \|\|`, on `bank-3` | OK |
| `fuzz_eq.c:287` | The width to Q conversion is finite and not negative. | finite and not below 0 | `omx_eq_design.h:319` `1.0 / (2.0 *` → `-1.0 / (2.0 *`, on `bank-0` | OK |
| `fuzz_eq.c:296` | The octaves to Q conversion is finite and above 0. | finite and above 0 | `omx_eq_design.h:420` `if (q < OMX_EQ_Q_MIN) q = OMX_EQ_Q_MIN;` → `q = -q;`, on `bank-0` | OK |

## `fuzz_src`: the sample-rate converter

| Check | Property | Expected value | Breakage | Verdict |
|---|---|---|---|---|
| `fuzz_src.c:91` | The plan refuses only what it cannot size: a zero rate, or an interpolation factor past the limit. | the harness's `gcd()` and OMX_SRC_MAX_L | `omx_src.h:91` `l > OMX_SRC_MAX_L` → `l > 1u`, on `44100-48000` | OK |
| `fuzz_src.c:100` | A plan taken is one that can be sized: neither rate is 0, and the factor is inside the limit. | the harness's `gcd()` and OMX_SRC_MAX_L | `omx_src.h:91` `l > OMX_SRC_MAX_L` → `l > 2u * OMX_SRC_MAX_L`, on `one-to-1025` | OK |
| `fuzz_src.c:107` | The plan is reduced, by the harness's own gcd. | the harness's `gcd()` | `omx_src.h:88` `omx_src_gcd(out_rate, in_rate)` → `1u`, on `six-to-four` | OK |
| `fuzz_src.c:114` | The plan is the ratio of the two rates. | the two rates of the input, l * in = m * out | `omx_src.h:90` `in_rate / g` → `in_rate / g + 2u`, on `44100-48000` | OK |
| `fuzz_src.c:157` | A take is the same however it is cut into blocks, bit for bit. | the same take run whole, a second conversion | `omx_src.h:345` `i < in_n` → `i + 1u < in_n`, on `44100-48000` | OK |
| `fuzz_src.c:171` | Without a flush, a take makes no more than a whole file's frames and the half length of the filter. | n * l / m by the harness's arithmetic, plus half the taps | `omx_src.h:322` `s->acc += (uint64_t)s->m` → `s->acc += (uint64_t)s->m / 2u`, on `96000-48000` | OK |
| `fuzz_src.c:179` | A take makes no more samples than the capacity of what went in, n * l / m + 2, by the harness's arithmetic. | n * l / m + 2 by the harness's arithmetic | `omx_src.h:322` `s->acc += (uint64_t)s->m` → `s->acc += (uint64_t)s->m / 2u`, on `48000-44100` | OK |
| `fuzz_src.c:187` | Each output is finite, and inside the input's peak times the filter's worst gain. | the input's peak times the sum of the absolute coefficients of the worst phase, summed by the harness | `omx_src.h:292` `c[k] * h[k]` → `1000.0f * c[k] * h[k]`, on `44100-48000` | OK |
| `fuzz_src.c:196` | An identity plan returns the take unchanged. | the harness's copy of the take | `omx_src.h:175` `coef[0] = 1.0f` → `coef[0] = 0.5f`, on `48000-48000` | OK |
| `fuzz_src.c:207` | The coefficients of each phase sum to 1, the rule of the table in omx_src.h. The bound on the outputs above takes its gain from this table, so this is what holds the table's scale. | 1, the rule of the table in omx_src.h, summed by the harness | `omx_src.h:208` `*= g;` → `*= 2.0f * g;`, on `44100-48000` | OK |

## `fuzz_delay`: the delay lines

| Check | Property | Expected value | Breakage | Verdict |
|---|---|---|---|---|
| `fuzz_delay.c:104` | A ring takes a line exactly when it is longer than the kernel's reach: the lookbehind and two. | the harness's `fits`: an order of 3 or 5 and a ring longer than the kernel's reach | `omx_fdelay.h:147` `omx_fdelay_max_delay(cap, order) <= 0.0f` → `cap < 2000u`, on `fractional-3` | OK |
| `fuzz_delay.c:112` | An order the kernel does not have is answered by the code that says so. | OMX_FDELAY_BAD_ORDER, the documented code | `omx_fdelay.h:146` `return OMX_FDELAY_BAD_ORDER` → `return OMX_FDELAY_RING_TOO_SMALL`, on `fractional-refused-order` | OK |
| `fuzz_delay.c:122` | A ring that cannot hold the kernel is answered by the code that says so. | OMX_FDELAY_RING_TOO_SMALL, the documented code | `omx_fdelay.h:147` `return OMX_FDELAY_RING_TOO_SMALL` → `return OMX_FDELAY_BAD_ORDER`, on `fractional-tiny-ring` | OK |
| `fuzz_delay.c:132` | A refused line is left unarmed. | order 0, no ring, size 0 | `omx_fdelay.h:145` `l->order = 0;` → `l->order = order;`, on `fractional-refused-order` | OK |
| `fuzz_delay.c:155` | A line that is not armed delays nothing. | 0 | `omx_fdelay.h:241` `return 0.0f` → `return 1.0f`, on `fractional-tiny-ring` | OK |
| `fuzz_delay.c:164` | A line that is not armed leaves the block as it was, bit for bit. | the harness's copy of the block | `omx_fdelay.h:509` `return;` → `{ if (buf) buf[0] = 0.0f; return; }`, on `fractional-refused-order` | OK |
| `fuzz_delay.c:175` | A delay is delivered inside the ring, and finite. | the harness's `longest()` | `omx_fdelay.h:247` `d = maxd;` → `;`, on `fractional-3` | OK |
| `fuzz_delay.c:183` | A delay that is not finite reads as 0. | 0 | `omx_fdelay.h:242` `: 0.0f` → `: 1.0f`, on `fractional-3` | OK |
| `fuzz_delay.c:191` | A line delivers what its comment says it owes a request. | the harness's `owed()`, the rule in the comment of omx_fdelay_latency() | `omx_fdelay.h:246` `d = mind;` → `;`, on `fractional-3` | OK |
| `fuzz_delay.c:207` | The line gives finite samples for finite samples. | finite | `omx_fdelay.h:517` `omx_fdelay_read_at(l, id, whole ? 0 : c)` → `NAN`, on `fractional-3` | OK |
| `fuzz_delay.c:214` | A sample stays inside the sum of the absolute Lagrange weights times the take's peak. | the harness's `weights()` times the take's peak | `omx_fdelay.h:517` `omx_fdelay_read_at(l, id, whole ? 0 : c)` → `8.0f * omx_fdelay_read_at(l, id, whole ? 0 : c)`, on `fractional-3` | OK |
| `fuzz_delay.c:225` | A whole-sample delay is the take delayed, bit for bit. | the harness's copy of the take, delayed by the whole part of `owed()` | `omx_fdelay.h:517` `omx_fdelay_read_at(l, id, whole` → `omx_fdelay_read_at(l, id + 1u, whole`, on `fractional-3`<br>`omx_fdelay.h:267` `*id = (uint32_t)d;` → `*id = (uint32_t)d + 1u;`, on `fractional-3` | OK |
| `fuzz_delay.c:237` | A fractional delay is the Lagrange sum over the take, to float rounding. | the Lagrange sum of the harness's `weights()` over its copy of the take | `omx_fdelay.h:269` `*kf = 1.0f - fr;` → `*kf = fr;`, on `fractional-3` | OK |
| `fuzz_delay.c:266` | A delay in samples lands inside the ring for any finite ms and rate. | the ring size the harness passed | `omx_idelay.h:68` `d = d >= cap ? cap - 1 : d;` → `d = d >= cap && s < 1.0e7f ? cap - 1 : d;`, on `integer-any-ms-and-rate` | OK |
| `fuzz_delay.c:273` | It is the rounded delay, saturated at the ring's last slot, however large the request. | the harness's `samples_owed()` | `omx_idelay.h:67` `s >= (float)cap ? cap : (uint32_t)s` → `s >= (float)cap ? 0u : (uint32_t)s`, on `integer-any-ms-and-rate` | OK |
| `fuzz_delay.c:285` | A delay in samples lands inside the ring, on both legs. | the ring size the harness passed | `omx_idelay.h:68` `cap - 1 : d` → `d : d`, on `integer-pair` | OK |
| `fuzz_delay.c:293` | A delay of no length is 0 samples. | 0 | `omx_idelay.h:64` `return 0;` → `return 1;`, on `integer-pair` | OK |
| `fuzz_delay.c:301` | A delay in samples is the rounded delay, on both legs. | the harness's `samples_owed()` | `omx_idelay.h:65` `* 0.001f` → `* 0.002f`, on `integer-pair` | OK |
| `fuzz_delay.c:325` | A steady delay on the left is the take delayed, bit for bit. | the harness's copy of the left take, delayed | `omx_idelay.h:109` `omx_lookahead_back(w, tgt, cap)` → `omx_lookahead_back(w, tgt + 1u, cap)`, on `integer-pair`<br>`omx_idelay.h:122` `*cur = tgt;` → ``, on `integer-pair` | OK |
| `fuzz_delay.c:334` | A delay that moves on the left stays between the two taps it fades. | the harness's old and new taps of the left take | `omx_idelay.h:119` `ring[r0] *` → `2.0f * ring[r0] *`, on `integer-pair` | OK |
| `fuzz_delay.c:346` | A steady delay on the right is the take delayed, bit for bit. | the harness's copy of the right take, delayed | `omx_idelay.h:172` `omx_idelay_apply(r, n, d->ring_r, d->cap, &d->wpos_r, &d->cur_r, tgt_r);` → `{ omx_idelay_apply(r, n, d->ring_r, d->cap, &d->wpos_r, &d->cur_r, tgt_r); r[n - 1] += 1.0f; }`, on `integer-right-steady` | OK |
| `fuzz_delay.c:355` | A delay that moves on the right stays between the two taps it fades. | the harness's old and new taps of the right take | `omx_idelay.h:172` `omx_idelay_apply(r, n, d->ring_r, d->cap, &d->wpos_r, &d->cur_r, tgt_r);` → `{ omx_idelay_apply(r, n, d->ring_r, d->cap, &d->wpos_r, &d->cur_r, tgt_r); r[n - 1] += 1.0f; }`, on `integer-pair` | OK |

## `fuzz_note_map`: the MIDI note map

| Check | Property | Expected value | Breakage | Verdict |
|---|---|---|---|---|
| `fuzz_note_map.c:102` | A call makes no more messages than it has room for. | the room the harness gave | `omx_note_map.h:327` `return n;` → `return n + 9;`, on `omni-linear` | OK |
| `fuzz_note_map.c:111` | Each message delivered is a channel message with data bytes of 7 bits. | the MIDI 1.0 channel message ranges | `omx_note_map.h:309` `0x90 \| ch` → `0xF0 \| ch`, on `omni-linear` | OK |
| `fuzz_note_map.c:119` | A note on delivered has a velocity of 1 or more. | 1, MIDI 1.0: velocity 0 is a note off | `omx_note_map.h:311` `omx_velocity_curve_apply(m->leg.velocity_curve, in.data2, m->leg.velocity_fixed)` → `0`, on `omni-linear` | OK |
| `fuzz_note_map.c:128` | A note on is delivered only on a channel the leg takes. | the harness's own copy of the leg in force | `omx_note_map.h:297` `!omx_note_map_takes_channel(m, ch) \|\| ` → ``, on `channel-1-up-an-octave` | OK |
| `fuzz_note_map.c:136` | A note off delivered is for a key the model has sounding on that channel. | the harness's model of what sounds downstream | `omx_note_map.h:292` `m->out_key[ch][key]` → `m->out_key[ch][key] + 1`, on `omni-linear` | OK |
| `fuzz_note_map.c:161` | The leg the harness builds is one the library takes. | valid by the harness's construction | `omx_note_map.h:168` `l->velocity_fixed <= 127` → `l->velocity_fixed <= 0`, on `omni-linear` | OK |
| `fuzz_note_map.c:192` | A note on that the leg takes and whose transposed key is a key is delivered. | the harness's copy of the leg: channel, key range and transposed key | `omx_note_map.h:297` `if (!omx_note_map_takes_channel` → `if (1 \|\| !omx_note_map_takes_channel`, on `omni-linear` | OK |
| `fuzz_note_map.c:201` | A note on is delivered only inside the leg's key range, and its output key is a key. | the harness's copy of the leg | `omx_note_map.h:297` ` \|\| key > m->leg.key_high` → ``, on `channel-1-up-an-octave` | OK |
| `fuzz_note_map.c:210` | A note on makes one message, or two when its key was held, and the last sounds the transposed key. | the harness's account of held keys, and key + transpose from its copy of the leg | `omx_note_map.h:310` `(uint8_t)to` → `(uint8_t)key`, on `channel-1-up-an-octave`<br>`omx_note_map.h:204` `omx_note_leg_valid(leg) ? *leg : omx_note_leg_default()` → `omx_note_leg_default()`, on `channel-1-up-an-octave` | OK |
| `fuzz_note_map.c:220` | out_of_range counts the note ons the leg takes whose transposed key is outside 0..127. | the harness's own count | `omx_note_map.h:300` `m->out_of_range++` → `m->out_of_range--`, on `out-of-range-twice`<br>`omx_note_map.h:300` `m->out_of_range++` → ``, on `out-of-range-twice` | OK |
| `fuzz_note_map.c:246` | set_leg takes a leg exactly when it is valid, by the harness's own reading of a leg. | the harness's `leg_valid()` | `omx_note_map.h:222` `if (!omx_note_leg_valid(leg)) return 0;` → ``, on `transposed-below-zero-and-change-leg` | OK |
| `fuzz_note_map.c:253` | A leg taken is the policy now, and a leg refused leaves the old one. | the leg the harness passed, or the one before it | `omx_note_map.h:223` `m->leg = *leg;` → ``, on `transposed-below-zero-and-change-leg` | OK |
| `fuzz_note_map.c:266` | release_all writes no more than its cap. | the cap the harness passed | `omx_note_map.h:351` `return n;` → `return n + cap + 1;`, on `channel-1-up-an-octave` | OK |
| `fuzz_note_map.c:275` | release_all writes only note offs. | 0x80, a note off | `omx_note_map.h:345` `0x80 \| c` → `0x90 \| c`, on `channel-1-up-an-octave` | OK |
| `fuzz_note_map.c:287` | After release_all, nothing is held. | 0 | `omx_note_map.h:349` `omx_note_map_mark(m, c, k, 0);` → ``, on `channel-1-up-an-octave` | OK |
| `fuzz_note_map.c:296` | After release_all, the model has nothing sounding. | the harness's model: nothing sounding | `omx_note_map.h:348` `n++;` → ``, on `channel-1-up-an-octave` | OK |
| `fuzz_note_map.c:304` | held_count is the population of the held table. | the population of the map's own table (an invariant between two fields; the next check holds the table to the harness) | `omx_note_map.h:253` `m->held_count++;` → `m->held_count += 2;`, on `omni-linear` | OK |
| `fuzz_note_map.c:314` | The table holds the keys the harness's own account of what was played holds, key by key. | the harness's own table of held keys | `omx_note_map.h:314` `omx_note_map_mark(m, ch, key, 1);` → ``, on `omni-linear`<br>`omx_note_map.h:314` `omx_note_map_mark(m, ch, key, 1);` → `omx_note_map_mark(m, ch, key ^ 1, 1);`, on `omni-linear` | OK |
| `fuzz_note_map.c:328` | The release at the end of the input leaves nothing held. | 0 | `omx_note_map.h:349` `omx_note_map_mark(m, c, k, 0);` → ``, on `held-at-the-end` | OK |
| `fuzz_note_map.c:337` | No note sticks: after the release at the end, the model has nothing sounding. | the harness's model: nothing sounding | `omx_note_map.h:348` `n++;` → ``, on `held-at-the-end` | OK |
| `fuzz_note_map.c:348` | Each curve keeps a velocity in 1..127. | 1..127, MIDI 1.0 note-on velocities | `omx_note_map.h:133` `if (out > 127) out = 127;` → `out = 128;`, on `leg-only` | OK |
| `fuzz_note_map.c:356` | The linear curve is the identity. | the velocity itself | `omx_note_map.h:130` `out = v` → `out = 1`, on `leg-only` | OK |

## Stops that are not properties

| Stop | Reason |
|---|---|
| `fuzz_fx.c:296` | malloc failed: an allocation of the harness's own, before the library is called |
| `fuzz_src.c:119` | malloc failed: an allocation of the harness's own, before the library is called |
| `fuzz_src.c:147` | malloc failed: an allocation of the harness's own, before the library is called |
| `fuzz_delay.c:93` | calloc failed: an allocation of the harness's own, before the library is called |
| `fuzz_delay.c:249` | calloc failed: an allocation of the harness's own, before the library is called |
