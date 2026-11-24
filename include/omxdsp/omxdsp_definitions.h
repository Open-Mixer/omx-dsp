/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * This file is part of omx-dsp.
 *
 * omx-dsp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * omx-dsp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with omx-dsp.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 */

#ifndef OMXDSP_DEFINITIONS_H
#define OMXDSP_DEFINITIONS_H
/*
 * GENERATED: DO NOT EDIT BY HAND.
 * Rendered by `omx-contract render --target c --package omx-dsp` from data/, the definitions of the kernels
 * and DSP constants this library implements. The rendered file is kept in this repository so the library
 * builds without node; the values are changed where they are declared and the file is re-rendered, never
 * edited here.
 * It is kept out of the API reference (Doxyfile EXCLUDE): each define is documented by its definition in
 * data/, and the header carries no doc comment of its own for Doxygen to read.
 */

/** @file
 * @brief The definitions omx-dsp implements: every kernel's travels, sets, readouts and constants. */

#include <math.h>
#include <omxcontract/omx_contract_limits.h>

/* Per-parameter numeric travel, one block per `travels` table named <PREFIX>_LIMITS:
 * OMX_<PREFIX>_<FIELD>_MIN/_MAX/_DEFAULT, float literals. */
/** The least value of `thresholdDb`. (dB) */
#define OMX_COMP_THRESHOLD_DB_MIN -60.0f
/** The greatest value of `thresholdDb`. (dB) */
#define OMX_COMP_THRESHOLD_DB_MAX 0.0f
/** The come-up value of `thresholdDb`. (dB) */
#define OMX_COMP_THRESHOLD_DB_DEFAULT -18.0f
/** The least value of `ratio`. (none) */
#define OMX_COMP_RATIO_MIN 1.0f
/** The greatest value of `ratio`. (none) */
#define OMX_COMP_RATIO_MAX 20.0f
/** The come-up value of `ratio`. (none) */
#define OMX_COMP_RATIO_DEFAULT 4.0f
/** The least value of `kneeDb`. (dB) */
#define OMX_COMP_KNEE_DB_MIN 0.0f
/** The greatest value of `kneeDb`. (dB) */
#define OMX_COMP_KNEE_DB_MAX 24.0f
/** The come-up value of `kneeDb`. (dB) */
#define OMX_COMP_KNEE_DB_DEFAULT 6.0f
/** The least value of `attackMs`. (ms) */
#define OMX_COMP_ATTACK_MS_MIN 0.1f
/** The greatest value of `attackMs`. (ms) */
#define OMX_COMP_ATTACK_MS_MAX 100.0f
/** The come-up value of `attackMs`. (ms) */
#define OMX_COMP_ATTACK_MS_DEFAULT 5.0f
/** The least value of `releaseMs`. (ms) */
#define OMX_COMP_RELEASE_MS_MIN 5.0f
/** The greatest value of `releaseMs`. (ms) */
#define OMX_COMP_RELEASE_MS_MAX 3000.0f
/** The come-up value of `releaseMs`. (ms) */
#define OMX_COMP_RELEASE_MS_DEFAULT 200.0f
/** The least value of `makeupDb`. (dB) */
#define OMX_COMP_MAKEUP_DB_MIN 0.0f
/** The greatest value of `makeupDb`. (dB) */
#define OMX_COMP_MAKEUP_DB_MAX 24.0f
/** The come-up value of `makeupDb`. (dB) */
#define OMX_COMP_MAKEUP_DB_DEFAULT 0.0f
/** The least value of `mixPct`. (percent) */
#define OMX_COMP_MIX_PCT_MIN 0.0f
/** The greatest value of `mixPct`. (percent) */
#define OMX_COMP_MIX_PCT_MAX 100.0f
/** The come-up value of `mixPct`. (percent) */
#define OMX_COMP_MIX_PCT_DEFAULT 100.0f

/** The least value of `tap0Gain`. (none) */
#define OMX_DELAY_TAP_TAP0_GAIN_MIN 0.0f
/** The greatest value of `tap0Gain`. (none) */
#define OMX_DELAY_TAP_TAP0_GAIN_MAX 1.0f
/** The come-up value of `tap0Gain`. (none) */
#define OMX_DELAY_TAP_TAP0_GAIN_DEFAULT 1.0f
/** The least value of `tap0Pan`. (none) */
#define OMX_DELAY_TAP_TAP0_PAN_MIN -1.0f
/** The greatest value of `tap0Pan`. (none) */
#define OMX_DELAY_TAP_TAP0_PAN_MAX 1.0f
/** The come-up value of `tap0Pan`. (none) */
#define OMX_DELAY_TAP_TAP0_PAN_DEFAULT 0.0f
/** The least value of `tap1Gain`. (none) */
#define OMX_DELAY_TAP_TAP1_GAIN_MIN 0.0f
/** The greatest value of `tap1Gain`. (none) */
#define OMX_DELAY_TAP_TAP1_GAIN_MAX 1.0f
/** The come-up value of `tap1Gain`. (none) */
#define OMX_DELAY_TAP_TAP1_GAIN_DEFAULT 0.7f
/** The least value of `tap1Pan`. (none) */
#define OMX_DELAY_TAP_TAP1_PAN_MIN -1.0f
/** The greatest value of `tap1Pan`. (none) */
#define OMX_DELAY_TAP_TAP1_PAN_MAX 1.0f
/** The come-up value of `tap1Pan`. (none) */
#define OMX_DELAY_TAP_TAP1_PAN_DEFAULT -0.5f
/** The least value of `tap2Gain`. (none) */
#define OMX_DELAY_TAP_TAP2_GAIN_MIN 0.0f
/** The greatest value of `tap2Gain`. (none) */
#define OMX_DELAY_TAP_TAP2_GAIN_MAX 1.0f
/** The come-up value of `tap2Gain`. (none) */
#define OMX_DELAY_TAP_TAP2_GAIN_DEFAULT 0.5f
/** The least value of `tap2Pan`. (none) */
#define OMX_DELAY_TAP_TAP2_PAN_MIN -1.0f
/** The greatest value of `tap2Pan`. (none) */
#define OMX_DELAY_TAP_TAP2_PAN_MAX 1.0f
/** The come-up value of `tap2Pan`. (none) */
#define OMX_DELAY_TAP_TAP2_PAN_DEFAULT 0.5f
/** The least value of `tap3Gain`. (none) */
#define OMX_DELAY_TAP_TAP3_GAIN_MIN 0.0f
/** The greatest value of `tap3Gain`. (none) */
#define OMX_DELAY_TAP_TAP3_GAIN_MAX 1.0f
/** The come-up value of `tap3Gain`. (none) */
#define OMX_DELAY_TAP_TAP3_GAIN_DEFAULT 0.35f
/** The least value of `tap3Pan`. (none) */
#define OMX_DELAY_TAP_TAP3_PAN_MIN -1.0f
/** The greatest value of `tap3Pan`. (none) */
#define OMX_DELAY_TAP_TAP3_PAN_MAX 1.0f
/** The come-up value of `tap3Pan`. (none) */
#define OMX_DELAY_TAP_TAP3_PAN_DEFAULT 0.0f

/** The least value of `thresholdDb`. (dB) */
#define OMX_GATE_THRESHOLD_DB_MIN -80.0f
/** The greatest value of `thresholdDb`. (dB) */
#define OMX_GATE_THRESHOLD_DB_MAX 0.0f
/** The come-up value of `thresholdDb`. (dB) */
#define OMX_GATE_THRESHOLD_DB_DEFAULT -40.0f
/** The least value of `rangeDb`. (dB) */
#define OMX_GATE_RANGE_DB_MIN -90.0f
/** The greatest value of `rangeDb`. (dB) */
#define OMX_GATE_RANGE_DB_MAX 0.0f
/** The come-up value of `rangeDb`. (dB) */
#define OMX_GATE_RANGE_DB_DEFAULT -90.0f
/** The least value of `kneeStartDb`. (dB) */
#define OMX_GATE_KNEE_START_DB_MIN -80.0f
/** The greatest value of `kneeStartDb`. (dB) */
#define OMX_GATE_KNEE_START_DB_MAX 0.0f
/** The come-up value of `kneeStartDb`. (dB) */
#define OMX_GATE_KNEE_START_DB_DEFAULT -43.0f
/** The least value of `kneeEndDb`. (dB) */
#define OMX_GATE_KNEE_END_DB_MIN -80.0f
/** The greatest value of `kneeEndDb`. (dB) */
#define OMX_GATE_KNEE_END_DB_MAX 0.0f
/** The come-up value of `kneeEndDb`. (dB) */
#define OMX_GATE_KNEE_END_DB_DEFAULT -37.0f
/** The least value of `attackMs`. (ms) */
#define OMX_GATE_ATTACK_MS_MIN 0.0f
/** The greatest value of `attackMs`. (ms) */
#define OMX_GATE_ATTACK_MS_MAX 500.0f
/** The come-up value of `attackMs`. (ms) */
#define OMX_GATE_ATTACK_MS_DEFAULT 1.0f
/** The least value of `holdMs`. (ms) */
#define OMX_GATE_HOLD_MS_MIN 0.0f
/** The greatest value of `holdMs`. (ms) */
#define OMX_GATE_HOLD_MS_MAX 2000.0f
/** The come-up value of `holdMs`. (ms) */
#define OMX_GATE_HOLD_MS_DEFAULT 10.0f
/** The least value of `releaseMs`. (ms) */
#define OMX_GATE_RELEASE_MS_MIN 0.0f
/** The greatest value of `releaseMs`. (ms) */
#define OMX_GATE_RELEASE_MS_MAX 5000.0f
/** The come-up value of `releaseMs`. (ms) */
#define OMX_GATE_RELEASE_MS_DEFAULT 100.0f
/** The least value of `hysteresisDb`. (dB) */
#define OMX_GATE_HYSTERESIS_DB_MIN 0.0f
/** The greatest value of `hysteresisDb`. (dB) */
#define OMX_GATE_HYSTERESIS_DB_MAX 24.0f
/** The come-up value of `hysteresisDb`. (dB) */
#define OMX_GATE_HYSTERESIS_DB_DEFAULT 3.0f
/** The least value of `ratio`. (none) */
#define OMX_GATE_RATIO_MIN 1.0f
/** The greatest value of `ratio`. (none) */
#define OMX_GATE_RATIO_MAX 100.0f
/** The come-up value of `ratio`. (none) */
#define OMX_GATE_RATIO_DEFAULT 16.0f

/** The least value of `hpHz`. (Hz) */
#define OMX_GATE_KEY_HP_HZ_MIN 0.0f
/** The greatest value of `hpHz`. (Hz) */
#define OMX_GATE_KEY_HP_HZ_MAX 20000.0f
/** The least value of `lpHz`. (Hz) */
#define OMX_GATE_KEY_LP_HZ_MIN 0.0f
/** The greatest value of `lpHz`. (Hz) */
#define OMX_GATE_KEY_LP_HZ_MAX 20000.0f

/** The least value of `ceilingDb`. (dBFS) */
#define OMX_LIMITER_CEILING_DB_MIN -12.0f
/** The greatest value of `ceilingDb`. (dBFS) */
#define OMX_LIMITER_CEILING_DB_MAX 0.0f
/** The come-up value of `ceilingDb`. (dBFS) */
#define OMX_LIMITER_CEILING_DB_DEFAULT -1.0f
/** The least value of `lookaheadMs`. (ms) */
#define OMX_LIMITER_LOOKAHEAD_MS_MIN 0.5f
/** The greatest value of `lookaheadMs`. (ms) */
#define OMX_LIMITER_LOOKAHEAD_MS_MAX 5.0f
/** The come-up value of `lookaheadMs`. (ms) */
#define OMX_LIMITER_LOOKAHEAD_MS_DEFAULT 1.5f
/** The least value of `releaseMs`. (ms) */
#define OMX_LIMITER_RELEASE_MS_MIN 1.0f
/** The greatest value of `releaseMs`. (ms) */
#define OMX_LIMITER_RELEASE_MS_MAX 1000.0f
/** The come-up value of `releaseMs`. (ms) */
#define OMX_LIMITER_RELEASE_MS_DEFAULT 50.0f

/** The least value of `semitones`. (st) */
#define OMX_PITCH_SEMITONES_MIN -12.0f
/** The greatest value of `semitones`. (st) */
#define OMX_PITCH_SEMITONES_MAX 12.0f
/** The come-up value of `semitones`. (st) */
#define OMX_PITCH_SEMITONES_DEFAULT 0.0f
/** The least value of `cents`. (cents) */
#define OMX_PITCH_CENTS_MIN -50.0f
/** The greatest value of `cents`. (cents) */
#define OMX_PITCH_CENTS_MAX 50.0f
/** The come-up value of `cents`. (cents) */
#define OMX_PITCH_CENTS_DEFAULT 0.0f
/** The least value of `mix`. (percent) */
#define OMX_PITCH_MIX_MIN 0.0f
/** The greatest value of `mix`. (percent) */
#define OMX_PITCH_MIX_MAX 100.0f
/** The come-up value of `mix`. (percent) */
#define OMX_PITCH_MIX_DEFAULT 100.0f

/** The least value of `attackDb`. (dB) */
#define OMX_TRANSIENT_ATTACK_DB_MIN -24.0f
/** The greatest value of `attackDb`. (dB) */
#define OMX_TRANSIENT_ATTACK_DB_MAX 24.0f
/** The come-up value of `attackDb`. (dB) */
#define OMX_TRANSIENT_ATTACK_DB_DEFAULT 0.0f
/** The least value of `sustainDb`. (dB) */
#define OMX_TRANSIENT_SUSTAIN_DB_MIN -24.0f
/** The greatest value of `sustainDb`. (dB) */
#define OMX_TRANSIENT_SUSTAIN_DB_MAX 24.0f
/** The come-up value of `sustainDb`. (dB) */
#define OMX_TRANSIENT_SUSTAIN_DB_DEFAULT 0.0f
/** The least value of `attackTimeMs`. (ms) */
#define OMX_TRANSIENT_ATTACK_TIME_MS_MIN 2.0f
/** The greatest value of `attackTimeMs`. (ms) */
#define OMX_TRANSIENT_ATTACK_TIME_MS_MAX 50.0f
/** The come-up value of `attackTimeMs`. (ms) */
#define OMX_TRANSIENT_ATTACK_TIME_MS_DEFAULT 10.0f
/** The least value of `sustainTimeMs`. (ms) */
#define OMX_TRANSIENT_SUSTAIN_TIME_MS_MIN 50.0f
/** The greatest value of `sustainTimeMs`. (ms) */
#define OMX_TRANSIENT_SUSTAIN_TIME_MS_MAX 2000.0f
/** The come-up value of `sustainTimeMs`. (ms) */
#define OMX_TRANSIENT_SUSTAIN_TIME_MS_DEFAULT 250.0f
/** The least value of `outputDb`. (dB) */
#define OMX_TRANSIENT_OUTPUT_DB_MIN -24.0f
/** The greatest value of `outputDb`. (dB) */
#define OMX_TRANSIENT_OUTPUT_DB_MAX 12.0f
/** The come-up value of `outputDb`. (dB) */
#define OMX_TRANSIENT_OUTPUT_DB_DEFAULT 0.0f

/* Every scalar, sheet leaf and single travel's numbers, one block sorted by macro. An integer value
 * is an integer literal, any other a float literal AND its OMX_<macro>_DOUBLE spelling. */
/**
 * @name ALLPASS_LIMITS
 * The first-order all-pass section's contract tolerance. `unityTolerance` is the fraction of the larger side within which the block energy identity (the sum of y squared plus the pole times the end state squared equals the sum of x squared plus the pole times the start state squared) must hold, which is what makes the section unity magnitude. A design constant of the primitive, not a control: no row serves it.
 */
/// @{
/** The `unityTolerance` of ALLPASS_LIMITS. */
#define OMX_ALLPASS_UNITY_TOLERANCE 0.0001f
/** OMX_ALLPASS_UNITY_TOLERANCE as a double literal. */
#define OMX_ALLPASS_UNITY_TOLERANCE_DOUBLE 0.0001
/// @}
/**
 * @name BALANCER_AMOUNT_PCT_RANGE
 * How much of each band's distance from the target tone the face takes out, percent. It comes up at 0, the input bit for bit.
 */
/// @{
/** The `default` of BALANCER_AMOUNT_PCT_RANGE. */
#define OMX_BALANCER_AMOUNT_PCT_RANGE_DEFAULT 0
/** The `max` of BALANCER_AMOUNT_PCT_RANGE. */
#define OMX_BALANCER_AMOUNT_PCT_RANGE_MAX 100
/** The `min` of BALANCER_AMOUNT_PCT_RANGE. */
#define OMX_BALANCER_AMOUNT_PCT_RANGE_MIN 0
/** The `step` of BALANCER_AMOUNT_PCT_RANGE. */
#define OMX_BALANCER_AMOUNT_PCT_RANGE_STEP 1
/// @}
/** What a constant-Q band gains per octave on a flat spectrum, dB: its width doubles, 10 log10(2). The target's slope is the spectrum's, so a band's level is read against the slope plus this: on pink noise every band reads the same. (dBPerOct) */
#define OMX_BALANCER_BAND_DB_OCT 3.0103f
/** OMX_BALANCER_BAND_DB_OCT as a double literal. */
#define OMX_BALANCER_BAND_DB_OCT_DOUBLE 3.0103
/** The time constant of the detector's power average, ms: long-term tone, not notes. (ms) */
#define OMX_BALANCER_DETECT_MS 400
/** Under this mean band level, dBFS, the gains hold: a pause has no tone to correct. (dBFS) */
#define OMX_BALANCER_GATE_DBFS -60
/** The Q of each band's detector and of its peaking section: 0.7 is about two octaves, so the bands meet. (none) */
#define OMX_BALANCER_Q 0.7f
/** OMX_BALANCER_Q as a double literal. */
#define OMX_BALANCER_Q_DOUBLE 0.7
/**
 * @name BALANCER_RANGE_DB_RANGE
 * The most any one band moves, dB, either way.
 */
/// @{
/** The `default` of BALANCER_RANGE_DB_RANGE. */
#define OMX_BALANCER_RANGE_DB_RANGE_DEFAULT 3
/** The `max` of BALANCER_RANGE_DB_RANGE. */
#define OMX_BALANCER_RANGE_DB_RANGE_MAX 12
/** The `min` of BALANCER_RANGE_DB_RANGE. */
#define OMX_BALANCER_RANGE_DB_RANGE_MIN 1
/** The `step` of BALANCER_RANGE_DB_RANGE. */
#define OMX_BALANCER_RANGE_DB_RANGE_STEP 0.5f
/** OMX_BALANCER_RANGE_DB_RANGE_STEP as a double literal. */
#define OMX_BALANCER_RANGE_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name BALANCER_SLOPE_DB_OCT_RANGE
 * The target spectrum's slope, dB per octave of density. -3 is pink noise, the long-term average of most programme; 0 is white.
 */
/// @{
/** The `default` of BALANCER_SLOPE_DB_OCT_RANGE. */
#define OMX_BALANCER_SLOPE_DB_OCT_RANGE_DEFAULT -3
/** The `max` of BALANCER_SLOPE_DB_OCT_RANGE. */
#define OMX_BALANCER_SLOPE_DB_OCT_RANGE_MAX 0
/** The `min` of BALANCER_SLOPE_DB_OCT_RANGE. */
#define OMX_BALANCER_SLOPE_DB_OCT_RANGE_MIN -6
/** The `step` of BALANCER_SLOPE_DB_OCT_RANGE. */
#define OMX_BALANCER_SLOPE_DB_OCT_RANGE_STEP 0.5f
/** OMX_BALANCER_SLOPE_DB_OCT_RANGE_STEP as a double literal. */
#define OMX_BALANCER_SLOPE_DB_OCT_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name BALANCER_SPEED_DB_PER_S_RANGE
 * How fast each band walks, dB per second: slow enough that a mix's tone drifts into place and no note is chased.
 */
/// @{
/** The `default` of BALANCER_SPEED_DB_PER_S_RANGE. */
#define OMX_BALANCER_SPEED_DB_PER_S_RANGE_DEFAULT 0.5f
/** OMX_BALANCER_SPEED_DB_PER_S_RANGE_DEFAULT as a double literal. */
#define OMX_BALANCER_SPEED_DB_PER_S_RANGE_DEFAULT_DOUBLE 0.5
/** The `max` of BALANCER_SPEED_DB_PER_S_RANGE. */
#define OMX_BALANCER_SPEED_DB_PER_S_RANGE_MAX 6
/** The `min` of BALANCER_SPEED_DB_PER_S_RANGE. */
#define OMX_BALANCER_SPEED_DB_PER_S_RANGE_MIN 0.1f
/** OMX_BALANCER_SPEED_DB_PER_S_RANGE_MIN as a double literal. */
#define OMX_BALANCER_SPEED_DB_PER_S_RANGE_MIN_DOUBLE 0.1
/** The `step` of BALANCER_SPEED_DB_PER_S_RANGE. */
#define OMX_BALANCER_SPEED_DB_PER_S_RANGE_STEP 0.1f
/** OMX_BALANCER_SPEED_DB_PER_S_RANGE_STEP as a double literal. */
#define OMX_BALANCER_SPEED_DB_PER_S_RANGE_STEP_DOUBLE 0.1
/// @}
/** The soft knee of the band's gain computer, in dB. One knee for the one band-dynamics word, shared by every stage that runs it, the de-esser included: a second spelling of 6 would be a second place where the reduction starts. (dB) */
#define OMX_BAND_DYN_KNEE_DB 6
/** The absolute gating threshold of ITU-R BS.1770-4, Annex 1 (as EBU R 128 adopts it): a 400 ms block at or below -70 LUFS is silence to the gated measures, and the loudness histogram starts at it. The one value the loudness core gates by, the leveler's gate travel starts at and its loudness readout floors at. LUFS is written dB here until omx-contract carries the unit (omx-contract#145). (dB; since 3.0.0) */
#define OMX_BS1770_ABSOLUTE_GATE_LUFS -70
/** The constant of ITU-R BS.1770-4's loudness of a measured block, Annex 1: L = -0.691 + 10 log10 of the weighted mean square of the K-weighted channels. LU is written dB here until omx-contract carries the unit (omx-contract#145). (dB; since 3.0.0) */
#define OMX_BS1770_LOUDNESS_OFFSET_LU -0.691f
/** OMX_BS1770_LOUDNESS_OFFSET_LU as a double literal. */
#define OMX_BS1770_LOUDNESS_OFFSET_LU_DOUBLE -0.691
/** The relative gating threshold of ITU-R BS.1770-4, Annex 1: the integrated loudness keeps the blocks above the absolute gate and no more than 10 LU below their mean. LU is written dB here until omx-contract carries the unit (omx-contract#145). (dB; since 3.0.0) */
#define OMX_BS1770_RELATIVE_GATE_LU -10
/** The input cap a summing bus can be sized to — **the engine's declared input ceiling**, read from the engine's declarations rather than re-typed here. A gig with more than this many inputs needs more than one engine node; below it a bus is sized to `min(N, BUS_INPUT_CAP)`. It was a literal 64 until 2026-09-15, said to mirror the engine's `MAX_BUS_INPUTS` — and it did, but that 64 was the PipeWire builtin mixer's single-tier fan-in (8 × 8), never the engine's bound: the native node has allocated `OMX_MAX_STRIPS` (128) strips all along. Two literals agreeing by hand is how the console came to refuse a 96-channel desk it could carry. the engine's conformance test holds this to the C header; the console allocation design carries the ruling and the measured cost of width. (none) */
#define OMX_BUS_INPUT_CAP 128
/** The Q of a second-order Butterworth section, 1/√2 — maximally flat, the one value every Butterworth biquad in the tree designs at: the LR4 crossover section, the pitch pre-filter and the drive's tilt / band / HF sections, in TS and (as `OMX_BUTTERWORTH_Q`) in C (§7, the scalar door). (none) */
#define OMX_BUTTERWORTH_Q 0.7071067811865476f
/** OMX_BUTTERWORTH_Q as a double literal. */
#define OMX_BUTTERWORTH_Q_DOUBLE 0.7071067811865476
/** The chorus's base delay, ms: the voices swing around it by up to the depth. (ms) */
#define OMX_CHORUS_BASE_MS 10
/**
 * @name CHORUS_DEPTH_RANGE
 * The chorus's peak-to-peak excursion above its declared 10 ms base.
 */
/// @{
/** The `default` of CHORUS_DEPTH_RANGE. */
#define OMX_CHORUS_DEPTH_RANGE_DEFAULT 4
/** The `max` of CHORUS_DEPTH_RANGE. */
#define OMX_CHORUS_DEPTH_RANGE_MAX 12
/** The `min` of CHORUS_DEPTH_RANGE. */
#define OMX_CHORUS_DEPTH_RANGE_MIN 0
/** The `step` of CHORUS_DEPTH_RANGE. */
#define OMX_CHORUS_DEPTH_RANGE_STEP 0.1f
/** OMX_CHORUS_DEPTH_RANGE_STEP as a double literal. */
#define OMX_CHORUS_DEPTH_RANGE_STEP_DOUBLE 0.1
/// @}
/** The longest delay a chorus voice reaches, rounded up to whole ms: the base delay plus the depth maximum. The delay line is sized from it. (ms) */
#define OMX_CHORUS_MAX_MS 22
/**
 * @name CHORUS_MIX_RANGE
 * The chorus's wet/dry, as an instance of DRIVE_MIX_RANGE — the same percent travel, its own come-up value: 35 %, a colour under the dry on a strip. The REVERB_LOWCUT_RANGE-over- REVERB_CUT_RANGE shape: one travel, one declared default per field that reads it.
 */
/// @{
/** The `default` of CHORUS_MIX_RANGE. */
#define OMX_CHORUS_MIX_RANGE_DEFAULT 35
/** The `max` of CHORUS_MIX_RANGE. */
#define OMX_CHORUS_MIX_RANGE_MAX 100
/** The `min` of CHORUS_MIX_RANGE. */
#define OMX_CHORUS_MIX_RANGE_MIN 0
/** The `step` of CHORUS_MIX_RANGE. */
#define OMX_CHORUS_MIX_RANGE_STEP 0.1f
/** OMX_CHORUS_MIX_RANGE_STEP as a double literal. */
#define OMX_CHORUS_MIX_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name CHORUS_RATE_RANGE
 * The chorus sweep's speed. Above ~3 Hz it stops being a chorus and becomes a vibrato.
 */
/// @{
/** The `default` of CHORUS_RATE_RANGE. */
#define OMX_CHORUS_RATE_RANGE_DEFAULT 0.6f
/** OMX_CHORUS_RATE_RANGE_DEFAULT as a double literal. */
#define OMX_CHORUS_RATE_RANGE_DEFAULT_DOUBLE 0.6
/** The `max` of CHORUS_RATE_RANGE. */
#define OMX_CHORUS_RATE_RANGE_MAX 8
/** The `min` of CHORUS_RATE_RANGE. */
#define OMX_CHORUS_RATE_RANGE_MIN 0.05f
/** OMX_CHORUS_RATE_RANGE_MIN as a double literal. */
#define OMX_CHORUS_RATE_RANGE_MIN_DOUBLE 0.05
/** The `step` of CHORUS_RATE_RANGE. */
#define OMX_CHORUS_RATE_RANGE_STEP 0.01f
/** OMX_CHORUS_RATE_RANGE_STEP as a double literal. */
#define OMX_CHORUS_RATE_RANGE_STEP_DOUBLE 0.01
/// @}
/**
 * @name CHORUS_SPREAD_RANGE
 * The chorus's right-leg phase offset, in TURNS of its one oscillator. The right leg's voice k reads the LFO at k/N + spread: 0 is both legs one phase (the stage without the field, bit for bit), 0.25 quadrature, 0.5 the Dimension's opposed sweep. A fraction of a cycle carries no engineering symbol, so the unit is ''.
 */
/// @{
/** The `default` of CHORUS_SPREAD_RANGE. */
#define OMX_CHORUS_SPREAD_RANGE_DEFAULT 0
/** The `max` of CHORUS_SPREAD_RANGE. */
#define OMX_CHORUS_SPREAD_RANGE_MAX 0.5f
/** OMX_CHORUS_SPREAD_RANGE_MAX as a double literal. */
#define OMX_CHORUS_SPREAD_RANGE_MAX_DOUBLE 0.5
/** The `min` of CHORUS_SPREAD_RANGE. */
#define OMX_CHORUS_SPREAD_RANGE_MIN 0
/** The `step` of CHORUS_SPREAD_RANGE. */
#define OMX_CHORUS_SPREAD_RANGE_STEP 0.01f
/** OMX_CHORUS_SPREAD_RANGE_STEP as a double literal. */
#define OMX_CHORUS_SPREAD_RANGE_STEP_DOUBLE 0.01
/// @}
/**
 * @name CHORUS_VOICES_RANGE
 * The chorus's tap count — one oscillator read at k/N.
 */
/// @{
/** The `default` of CHORUS_VOICES_RANGE. */
#define OMX_CHORUS_VOICES_RANGE_DEFAULT 3
/** The `max` of CHORUS_VOICES_RANGE. */
#define OMX_CHORUS_VOICES_RANGE_MAX 4
/** The `min` of CHORUS_VOICES_RANGE. */
#define OMX_CHORUS_VOICES_RANGE_MIN 1
/** The `step` of CHORUS_VOICES_RANGE. */
#define OMX_CHORUS_VOICES_RANGE_STEP 1
/// @}
/**
 * @name CLIPPER_CEILING_DB_RANGE
 * The ceiling, dBFS: no sample leaves above it.
 */
/// @{
/** The `default` of CLIPPER_CEILING_DB_RANGE. */
#define OMX_CLIPPER_CEILING_DB_RANGE_DEFAULT 0
/** The `max` of CLIPPER_CEILING_DB_RANGE. */
#define OMX_CLIPPER_CEILING_DB_RANGE_MAX 0
/** The `min` of CLIPPER_CEILING_DB_RANGE. */
#define OMX_CLIPPER_CEILING_DB_RANGE_MIN -24
/** The `step` of CLIPPER_CEILING_DB_RANGE. */
#define OMX_CLIPPER_CEILING_DB_RANGE_STEP 0.1f
/** OMX_CLIPPER_CEILING_DB_RANGE_STEP as a double literal. */
#define OMX_CLIPPER_CEILING_DB_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name CLIPPER_DRIVE_DB_RANGE
 * The gain before the clip, dB. 0 is no push: a signal under the ceiling goes through bit for bit.
 */
/// @{
/** The `default` of CLIPPER_DRIVE_DB_RANGE. */
#define OMX_CLIPPER_DRIVE_DB_RANGE_DEFAULT 0
/** The `max` of CLIPPER_DRIVE_DB_RANGE. */
#define OMX_CLIPPER_DRIVE_DB_RANGE_MAX 24
/** The `min` of CLIPPER_DRIVE_DB_RANGE. */
#define OMX_CLIPPER_DRIVE_DB_RANGE_MIN 0
/** The `step` of CLIPPER_DRIVE_DB_RANGE. */
#define OMX_CLIPPER_DRIVE_DB_RANGE_STEP 0.1f
/** OMX_CLIPPER_DRIVE_DB_RANGE_STEP as a double literal. */
#define OMX_CLIPPER_DRIVE_DB_RANGE_STEP_DOUBLE 0.1
/// @}
/** The share of the ceiling the bend spans at a softness of 100 %: the knee sits at (1 - span x softness) x ceiling, half the ceiling at full softness, 6 dB under it. (none) */
#define OMX_CLIPPER_KNEE_SPAN 0.5f
/** OMX_CLIPPER_KNEE_SPAN as a double literal. */
#define OMX_CLIPPER_KNEE_SPAN_DOUBLE 0.5
/**
 * @name CLIPPER_SOFTNESS_PCT_RANGE
 * How far under the ceiling the bend starts, percent of CLIPPER_KNEE_SPAN of the ceiling. 0 is a hard clip. Above the knee a sample bends along a hyperbola that leaves the straight line at slope 1 and never reaches the ceiling.
 */
/// @{
/** The `default` of CLIPPER_SOFTNESS_PCT_RANGE. */
#define OMX_CLIPPER_SOFTNESS_PCT_RANGE_DEFAULT 0
/** The `max` of CLIPPER_SOFTNESS_PCT_RANGE. */
#define OMX_CLIPPER_SOFTNESS_PCT_RANGE_MAX 100
/** The `min` of CLIPPER_SOFTNESS_PCT_RANGE. */
#define OMX_CLIPPER_SOFTNESS_PCT_RANGE_MIN 0
/** The `step` of CLIPPER_SOFTNESS_PCT_RANGE. */
#define OMX_CLIPPER_SOFTNESS_PCT_RANGE_STEP 1
/// @}
/**
 * @name COMP_HOLD_MS_RANGE
 * The hold, ms: the detector is frozen this long after its level last rose, so the reduction holds before the release runs. 0 is no hold, the come-up value.
 */
/// @{
/** The `default` of COMP_HOLD_MS_RANGE. */
#define OMX_COMP_HOLD_MS_RANGE_DEFAULT 0
/** The `max` of COMP_HOLD_MS_RANGE. */
#define OMX_COMP_HOLD_MS_RANGE_MAX 2000
/** The `min` of COMP_HOLD_MS_RANGE. */
#define OMX_COMP_HOLD_MS_RANGE_MIN 0
/** The `step` of COMP_HOLD_MS_RANGE. */
#define OMX_COMP_HOLD_MS_RANGE_STEP 5
/// @}
/**
 * @name COMP_KEY_HP_HZ_RANGE
 * The key filter's high-pass corner, Hz: the gate's key filter (one Butterworth section, in double), on what the detector hears and never on the audio. 0 is out, the come-up value, and the compressor is the one without it, bit for bit.
 */
/// @{
/** The `default` of COMP_KEY_HP_HZ_RANGE. */
#define OMX_COMP_KEY_HP_HZ_RANGE_DEFAULT 0
/** The `max` of COMP_KEY_HP_HZ_RANGE. */
#define OMX_COMP_KEY_HP_HZ_RANGE_MAX 2000
/** The `min` of COMP_KEY_HP_HZ_RANGE. */
#define OMX_COMP_KEY_HP_HZ_RANGE_MIN 0
/** The `step` of COMP_KEY_HP_HZ_RANGE. */
#define OMX_COMP_KEY_HP_HZ_RANGE_STEP 1
/// @}
/**
 * @name COMP_KEY_LP_HZ_RANGE
 * The key filter's low-pass corner, Hz, after the high-pass. 0 is out, the come-up value.
 */
/// @{
/** The `default` of COMP_KEY_LP_HZ_RANGE. */
#define OMX_COMP_KEY_LP_HZ_RANGE_DEFAULT 0
/** The `max` of COMP_KEY_LP_HZ_RANGE. */
#define OMX_COMP_KEY_LP_HZ_RANGE_MAX 20000
/** The `min` of COMP_KEY_LP_HZ_RANGE. */
#define OMX_COMP_KEY_LP_HZ_RANGE_MIN 0
/** The `step` of COMP_KEY_LP_HZ_RANGE. */
#define OMX_COMP_KEY_LP_HZ_RANGE_STEP 10
/// @}
/**
 * @name COMP_KEY_TILT_DB_RANGE
 * The key's tilt, in dB per octave at KEY_TILT_PIVOT_HZ: positive brightens what the detector hears, negative darkens it (omx_tilt, a first-order shelf pair about the pivot, after the key filter). 0 is flat, the come-up value, bit for bit.
 */
/// @{
/** The `default` of COMP_KEY_TILT_DB_RANGE. */
#define OMX_COMP_KEY_TILT_DB_RANGE_DEFAULT 0
/** The `max` of COMP_KEY_TILT_DB_RANGE. */
#define OMX_COMP_KEY_TILT_DB_RANGE_MAX 6
/** The `min` of COMP_KEY_TILT_DB_RANGE. */
#define OMX_COMP_KEY_TILT_DB_RANGE_MIN -6
/** The `step` of COMP_KEY_TILT_DB_RANGE. */
#define OMX_COMP_KEY_TILT_DB_RANGE_STEP 0.5f
/** OMX_COMP_KEY_TILT_DB_RANGE_STEP as a double literal. */
#define OMX_COMP_KEY_TILT_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name COMP_RANGE_DB_RANGE
 * The ducker's depth, dB: the deepest the programme goes while the key is above the threshold. 0 ducks nothing. Read only while duck is on.
 */
/// @{
/** The `default` of COMP_RANGE_DB_RANGE. */
#define OMX_COMP_RANGE_DB_RANGE_DEFAULT -12
/** The `max` of COMP_RANGE_DB_RANGE. */
#define OMX_COMP_RANGE_DB_RANGE_MAX 0
/** The `min` of COMP_RANGE_DB_RANGE. */
#define OMX_COMP_RANGE_DB_RANGE_MIN -40
/** The `step` of COMP_RANGE_DB_RANGE. */
#define OMX_COMP_RANGE_DB_RANGE_STEP 1
/// @}
/**
 * @name CURVE_EXTENTS
 * A curve's default extents, where a request gives none: an eq_response from `freqMinHz` to the lower of `freqMaxHz` and `nyquistFraction` of Nyquist, each band's dB floored at `eqFloorDb` so a notch's null stays finite (the console's EQ draws to -120 dB); a transfer_curve's input from `levelMinDb` to `levelMaxDb`, and a drive's (a steady level in, its level out, in dB like every transfer_curve; since 0.32.4) from `driveLevelMinDb` to `driveLevelMaxDb`; a time_response's level floor `timeFloorDb` (dB, below which a tap is not a stem and an envelope ends: the picture's own bottom line, since 0.32.4) (since 3.0.0)
 */
/// @{
/** The `driveLevelMaxDb` of CURVE_EXTENTS. */
#define OMX_CURVE_EXTENTS_DRIVE_LEVEL_MAX_DB 0
/** The `driveLevelMinDb` of CURVE_EXTENTS. */
#define OMX_CURVE_EXTENTS_DRIVE_LEVEL_MIN_DB -60
/** The `eqFloorDb` of CURVE_EXTENTS. */
#define OMX_CURVE_EXTENTS_EQ_FLOOR_DB -120
/** The `freqMaxHz` of CURVE_EXTENTS. */
#define OMX_CURVE_EXTENTS_FREQ_MAX_HZ 20000
/** The `freqMinHz` of CURVE_EXTENTS. */
#define OMX_CURVE_EXTENTS_FREQ_MIN_HZ 20
/** The `levelMaxDb` of CURVE_EXTENTS. */
#define OMX_CURVE_EXTENTS_LEVEL_MAX_DB 0
/** The `levelMinDb` of CURVE_EXTENTS. */
#define OMX_CURVE_EXTENTS_LEVEL_MIN_DB -80
/** The `nyquistFraction` of CURVE_EXTENTS. */
#define OMX_CURVE_EXTENTS_NYQUIST_FRACTION 0.999f
/** OMX_CURVE_EXTENTS_NYQUIST_FRACTION as a double literal. */
#define OMX_CURVE_EXTENTS_NYQUIST_FRACTION_DOUBLE 0.999
/** The `timeFloorDb` of CURVE_EXTENTS. */
#define OMX_CURVE_EXTENTS_TIME_FLOOR_DB -60
/// @}
/** The most points one curve carries, an EQ's band anchors included: a reply of x with four significant digits and y to 0.01 dB, about 13 bytes a point, fits the 4096-byte line a host's reply is built in. Every caller reads it here; it moves with that buffer, in this one place. (none; since 3.0.0) */
#define OMX_CURVE_POINTS_MAX 256
/** The most taps a delay's time_response carries, the dry tap included (since 0.32.4): a runaway feedback near 1 must not make the train unbounded, and 32 taps at the picture's resolution are a ladder the eye reads as one. (none; since 3.0.0) */
#define OMX_CURVE_TAPS_MAX 32
/** The DEEP plateau tier's dwell: ~2 s at the watcher's 10 Hz — the TIME that buys the deeper floor its music safety (see DetectThresholds.deepPlateauFloorDb). (none) */
#define OMX_DEEP_PLATEAU_FRAMES 20
/**
 * @name DEESS_ATTACK_RANGE
 * The detector's ATTACK, in ms. A sibilant's onset is a few milliseconds; 1 ms catches it without the detector chattering on the band's own waveform.
 */
/// @{
/** The `default` of DEESS_ATTACK_RANGE. */
#define OMX_DEESS_ATTACK_RANGE_DEFAULT 1
/** The `max` of DEESS_ATTACK_RANGE. */
#define OMX_DEESS_ATTACK_RANGE_MAX 50
/** The `min` of DEESS_ATTACK_RANGE. */
#define OMX_DEESS_ATTACK_RANGE_MIN 0.1f
/** OMX_DEESS_ATTACK_RANGE_MIN as a double literal. */
#define OMX_DEESS_ATTACK_RANGE_MIN_DOUBLE 0.1
/** The `step` of DEESS_ATTACK_RANGE. */
#define OMX_DEESS_ATTACK_RANGE_STEP 0.1f
/** OMX_DEESS_ATTACK_RANGE_STEP as a double literal. */
#define OMX_DEESS_ATTACK_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name DEESS_FREQ_RANGE
 * The detector band's CENTRE, in Hz. Sibilance lives at 5–10 kHz on most voices; 7 kHz with the default width covers 4 950–9 900 Hz, which is where an "s" and a "t" sit on a close vocal mic. The travel is wider than the come-up on both sides on purpose: a bright female vocal wants the band up near 9 kHz and a chesty male one down at 5, and a surface narrower than the row removes reach the console genuinely has (the same reasoning core's `REVERB_CUT_RANGE` records).
 */
/// @{
/** The `default` of DEESS_FREQ_RANGE. */
#define OMX_DEESS_FREQ_RANGE_DEFAULT 7000
/** The `max` of DEESS_FREQ_RANGE. */
#define OMX_DEESS_FREQ_RANGE_MAX 16000
/** The `min` of DEESS_FREQ_RANGE. */
#define OMX_DEESS_FREQ_RANGE_MIN 2000
/** The `step` of DEESS_FREQ_RANGE. */
#define OMX_DEESS_FREQ_RANGE_STEP 10
/// @}
/** The knee, in dB — a CONSTANT of the stage and not a row field: a de-esser without a soft knee latches audibly on the first sibilant of a phrase, and no other value of this number is a de-esser anyone would dial. The controller pushes it to the node and this is the same number; a second spelling would be a picture that disagrees with the audio about where the reduction starts. (dB) */
#define OMX_DEESS_KNEE_DB 6
/** The highest share of Nyquist the detector band's centre may sit at. The frequency travel tops out at 16 kHz, which is Nyquist at 32 kHz, where a band-pass designed on it has no stable poles; the face designs at min(freq, fraction x sample rate / 2). At 0.91 that is 14.56 kHz at 32 kHz, and above the travel's own 16 kHz at every declared rate (0.91 of 22.05 kHz is 20.06 kHz), so it moves no design there. (none) */
#define OMX_DEESS_NYQUIST_FRACTION 0.91f
/** OMX_DEESS_NYQUIST_FRACTION as a double literal. */
#define OMX_DEESS_NYQUIST_FRACTION_DOUBLE 0.91
/**
 * @name DEESS_RANGE_RANGE
 * The FLOOR on the reduction, in dB and never positive — the deepest cut the stage may reach however loud the sibilant is (`mix_deesser.h`'s clamp). 12 dB tames a hard sibilant; past that a voice starts to lisp, and a de-esser with NO floor turns a shouted "s" into a hole. The ceiling is 0, which switches the reduction off without switching the stage off — a legitimate thing to want while dialling the band in by ear.
 */
/// @{
/** The `default` of DEESS_RANGE_RANGE. */
#define OMX_DEESS_RANGE_RANGE_DEFAULT -12
/** The `max` of DEESS_RANGE_RANGE. */
#define OMX_DEESS_RANGE_RANGE_MAX 0
/** The `min` of DEESS_RANGE_RANGE. */
#define OMX_DEESS_RANGE_RANGE_MIN -24
/** The `step` of DEESS_RANGE_RANGE. */
#define OMX_DEESS_RANGE_RANGE_STEP 0.5f
/** OMX_DEESS_RANGE_RANGE_STEP as a double literal. */
#define OMX_DEESS_RANGE_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name DEESS_RATIO_RANGE
 * How hard the band is held once it is over the threshold. 4:1 is the textbook de-esser ratio: firm on the "s", short of a limiter.
 */
/// @{
/** The `default` of DEESS_RATIO_RANGE. */
#define OMX_DEESS_RATIO_RANGE_DEFAULT 4
/** The `max` of DEESS_RATIO_RANGE. */
#define OMX_DEESS_RATIO_RANGE_MAX 20
/** The `min` of DEESS_RATIO_RANGE. */
#define OMX_DEESS_RATIO_RANGE_MIN 1
/** The `step` of DEESS_RATIO_RANGE. */
#define OMX_DEESS_RATIO_RANGE_STEP 0.1f
/** OMX_DEESS_RATIO_RANGE_STEP as a double literal. */
#define OMX_DEESS_RATIO_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name DEESS_RELEASE_RANGE
 * The detector's RELEASE, in ms. 60 ms is long enough not to modulate inside one "s" and short enough to be open again for the next syllable.
 */
/// @{
/** The `default` of DEESS_RELEASE_RANGE. */
#define OMX_DEESS_RELEASE_RANGE_DEFAULT 60
/** The `max` of DEESS_RELEASE_RANGE. */
#define OMX_DEESS_RELEASE_RANGE_MAX 500
/** The `min` of DEESS_RELEASE_RANGE. */
#define OMX_DEESS_RELEASE_RANGE_MIN 5
/** The `step` of DEESS_RELEASE_RANGE. */
#define OMX_DEESS_RELEASE_RANGE_STEP 1
/// @}
/**
 * @name DEESS_THRESHOLD_RANGE
 * Where reduction starts — the level of the BAND, not of the strip, which is why the come-up is so much lower than the compressor's. A vocal peaking −12 dBFS carries roughly −30 dBFS in a one-octave band at 7 kHz on its sibilants and under −40 on its vowels, so −30 sits between the two and the stage opens on an "s" and stays shut on an "ah".
 */
/// @{
/** The `default` of DEESS_THRESHOLD_RANGE. */
#define OMX_DEESS_THRESHOLD_RANGE_DEFAULT -30
/** The `max` of DEESS_THRESHOLD_RANGE. */
#define OMX_DEESS_THRESHOLD_RANGE_MAX 0
/** The `min` of DEESS_THRESHOLD_RANGE. */
#define OMX_DEESS_THRESHOLD_RANGE_MIN -60
/** The `step` of DEESS_THRESHOLD_RANGE. */
#define OMX_DEESS_THRESHOLD_RANGE_STEP 0.5f
/** OMX_DEESS_THRESHOLD_RANGE_STEP as a double literal. */
#define OMX_DEESS_THRESHOLD_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name DEESS_WIDTH_RANGE
 * The band's WIDTH, in OCTAVES around the centre: the edges are `f / 2^(w/2)` and `f · 2^(w/2)`, which is what makes the centre the geometric middle at every width. Octaves and not a Q, because a Q is a property of one section and this band is TWO (a highpass and a lowpass, `mix_deesser.h`) — an operator asked to dial a Q would be dialling a number neither filter carries. One octave is the classic de-esser band: wide enough for a sibilant that moves with the vowel, narrow enough that a 1 kHz vowel sits 27.8 dB down the skirt.
 */
/// @{
/** The `default` of DEESS_WIDTH_RANGE. */
#define OMX_DEESS_WIDTH_RANGE_DEFAULT 1
/** The `max` of DEESS_WIDTH_RANGE. */
#define OMX_DEESS_WIDTH_RANGE_MAX 4
/** The `min` of DEESS_WIDTH_RANGE. */
#define OMX_DEESS_WIDTH_RANGE_MIN 0.25f
/** OMX_DEESS_WIDTH_RANGE_MIN as a double literal. */
#define OMX_DEESS_WIDTH_RANGE_MIN_DOUBLE 0.25
/** The `step` of DEESS_WIDTH_RANGE. */
#define OMX_DEESS_WIDTH_RANGE_STEP 0.05f
/** OMX_DEESS_WIDTH_RANGE_STEP as a double literal. */
#define OMX_DEESS_WIDTH_RANGE_STEP_DOUBLE 0.05
/// @}
/**
 * @name DEFAULT_THRESHOLDS
 * The corpus-tuned SAFE DEFAULT — today's exact gates, assembled byte-for-byte from the exported consts above (which stay the authoritative literal source, so the corpus test's import type { MagnitudeSeries } from './spectrum-bands.js'; imports remain the single truth). This is the ANCHOR the whole sensitivity mapping pins to: thresholdsFor`(0, 'live')` and `(0, 'fixed')` both deep-equal this object, so an untouched channel runs the identical detector it does now. `plateauFloorDb` is omitted ⇒ the #140 plateau route is OFF at the default.
 */
/// @{
/** The `growthConfirmFrames` of DEFAULT_THRESHOLDS. */
#define OMX_DEFAULT_THRESHOLDS_GROWTH_CONFIRM_FRAMES 2
/** The `growthMaxDipDb` of DEFAULT_THRESHOLDS. */
#define OMX_DEFAULT_THRESHOLDS_GROWTH_MAX_DIP_DB 0.5f
/** OMX_DEFAULT_THRESHOLDS_GROWTH_MAX_DIP_DB as a double literal. */
#define OMX_DEFAULT_THRESHOLDS_GROWTH_MAX_DIP_DB_DOUBLE 0.5
/** The `growthMaxSingleFrameShare` of DEFAULT_THRESHOLDS. */
#define OMX_DEFAULT_THRESHOLDS_GROWTH_MAX_SINGLE_FRAME_SHARE 0.5f
/** OMX_DEFAULT_THRESHOLDS_GROWTH_MAX_SINGLE_FRAME_SHARE as a double literal. */
#define OMX_DEFAULT_THRESHOLDS_GROWTH_MAX_SINGLE_FRAME_SHARE_DOUBLE 0.5
/** The `growthMinDbPerFrame` of DEFAULT_THRESHOLDS. */
#define OMX_DEFAULT_THRESHOLDS_GROWTH_MIN_DB_PER_FRAME 0.8f
/** OMX_DEFAULT_THRESHOLDS_GROWTH_MIN_DB_PER_FRAME as a double literal. */
#define OMX_DEFAULT_THRESHOLDS_GROWTH_MIN_DB_PER_FRAME_DOUBLE 0.8
/** The `growthMinTotalRiseDb` of DEFAULT_THRESHOLDS. */
#define OMX_DEFAULT_THRESHOLDS_GROWTH_MIN_TOTAL_RISE_DB 6
/** The `paprDb` of DEFAULT_THRESHOLDS. */
#define OMX_DEFAULT_THRESHOLDS_PAPR_DB 18
/** The `persistFrames` of DEFAULT_THRESHOLDS. */
#define OMX_DEFAULT_THRESHOLDS_PERSIST_FRAMES 5
/** The `phprDb` of DEFAULT_THRESHOLDS. */
#define OMX_DEFAULT_THRESHOLDS_PHPR_DB 9
/** The `pnprDb` of DEFAULT_THRESHOLDS. */
#define OMX_DEFAULT_THRESHOLDS_PNPR_DB 12
/** The `pshrMinDb` of DEFAULT_THRESHOLDS. */
#define OMX_DEFAULT_THRESHOLDS_PSHR_MIN_DB 10
/** The `subCleanFrames` of DEFAULT_THRESHOLDS. */
#define OMX_DEFAULT_THRESHOLDS_SUB_CLEAN_FRAMES 5
/// @}
/** The corner of the analog mode's one-pole low pass in the loop, Hz: lower than tape's, a bucket-brigade's dark repeats. (Hz) */
#define OMX_DELAY_ANALOG_LP_HZ 3000
/**
 * @name DELAY_DUCK_DB_RANGE
 * The wet's ducking depth, dB: the deepest the repeats go while the dry is loud (omx_duck: max(depth, min(0, T - L)) on the dry's level). 0 ducks nothing, the come-up value, bit for bit.
 */
/// @{
/** The `default` of DELAY_DUCK_DB_RANGE. */
#define OMX_DELAY_DUCK_DB_RANGE_DEFAULT 0
/** The `max` of DELAY_DUCK_DB_RANGE. */
#define OMX_DELAY_DUCK_DB_RANGE_MAX 0
/** The `min` of DELAY_DUCK_DB_RANGE. */
#define OMX_DELAY_DUCK_DB_RANGE_MIN -24
/** The `step` of DELAY_DUCK_DB_RANGE. */
#define OMX_DELAY_DUCK_DB_RANGE_STEP 0.5f
/** OMX_DELAY_DUCK_DB_RANGE_STEP as a double literal. */
#define OMX_DELAY_DUCK_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name DELAY_DUCK_RELEASE_MS_RANGE
 * How fast the repeats come back once the dry falls, ms: the dry follower's release; its attack is FX_DUCK_ATTACK_MS.
 */
/// @{
/** The `default` of DELAY_DUCK_RELEASE_MS_RANGE. */
#define OMX_DELAY_DUCK_RELEASE_MS_RANGE_DEFAULT 250
/** The `max` of DELAY_DUCK_RELEASE_MS_RANGE. */
#define OMX_DELAY_DUCK_RELEASE_MS_RANGE_MAX 2000
/** The `min` of DELAY_DUCK_RELEASE_MS_RANGE. */
#define OMX_DELAY_DUCK_RELEASE_MS_RANGE_MIN 20
/** The `step` of DELAY_DUCK_RELEASE_MS_RANGE. */
#define OMX_DELAY_DUCK_RELEASE_MS_RANGE_STEP 10
/// @}
/**
 * @name DELAY_DUCK_THRESHOLD_DB_RANGE
 * The dry level the ducking starts at, dBFS.
 */
/// @{
/** The `default` of DELAY_DUCK_THRESHOLD_DB_RANGE. */
#define OMX_DELAY_DUCK_THRESHOLD_DB_RANGE_DEFAULT -30
/** The `max` of DELAY_DUCK_THRESHOLD_DB_RANGE. */
#define OMX_DELAY_DUCK_THRESHOLD_DB_RANGE_MAX 0
/** The `min` of DELAY_DUCK_THRESHOLD_DB_RANGE. */
#define OMX_DELAY_DUCK_THRESHOLD_DB_RANGE_MIN -60
/** The `step` of DELAY_DUCK_THRESHOLD_DB_RANGE. */
#define OMX_DELAY_DUCK_THRESHOLD_DB_RANGE_STEP 1
/// @}
/**
 * @name DELAY_MIX_RANGE
 * The FX DELAY's wet/dry — the same split as REVERB_MIX_RANGE, its own declaration.
 */
/// @{
/** The `byKind.fxReturn` of DELAY_MIX_RANGE. */
#define OMX_DELAY_MIX_RANGE_BY_KIND_FX_RETURN 1
/** The `default` of DELAY_MIX_RANGE. */
#define OMX_DELAY_MIX_RANGE_DEFAULT 0.3f
/** OMX_DELAY_MIX_RANGE_DEFAULT as a double literal. */
#define OMX_DELAY_MIX_RANGE_DEFAULT_DOUBLE 0.3
/** The `max` of DELAY_MIX_RANGE. */
#define OMX_DELAY_MIX_RANGE_MAX 1
/** The `min` of DELAY_MIX_RANGE. */
#define OMX_DELAY_MIX_RANGE_MIN 0
/** The `step` of DELAY_MIX_RANGE. */
#define OMX_DELAY_MIX_RANGE_STEP 0
/// @}
/** The corner of the tape mode's one-pole low pass in the loop, Hz: each repeat a little darker than the last. (Hz) */
#define OMX_DELAY_TAPE_LP_HZ 6000
/** The tape mode's wow rate, Hz: the read head's slow drift. (Hz) */
#define OMX_DELAY_TAPE_WOW_HZ 0.4f
/** OMX_DELAY_TAPE_WOW_HZ as a double literal. */
#define OMX_DELAY_TAPE_WOW_HZ_DOUBLE 0.4
/** The tape mode's wow depth, ms: the read head drifts between the time and the time plus this. (ms) */
#define OMX_DELAY_TAPE_WOW_MS 0.3f
/** OMX_DELAY_TAPE_WOW_MS as a double literal. */
#define OMX_DELAY_TAPE_WOW_MS_DOUBLE 0.3
/**
 * @name DELAY_TONE_RANGE
 * The FX delay's TONE — a NORMALIZED_RANGE instance with its own come-up value, matching the console's delay default tone. See REVERB_SIZE_RANGE for why this is a named constant rather than a default on the shared shape.
 */
/// @{
/** The `default` of DELAY_TONE_RANGE. */
#define OMX_DELAY_TONE_RANGE_DEFAULT 0.3f
/** OMX_DELAY_TONE_RANGE_DEFAULT as a double literal. */
#define OMX_DELAY_TONE_RANGE_DEFAULT_DOUBLE 0.3
/** The `max` of DELAY_TONE_RANGE. */
#define OMX_DELAY_TONE_RANGE_MAX 1
/** The `min` of DELAY_TONE_RANGE. */
#define OMX_DELAY_TONE_RANGE_MIN 0
/** The `step` of DELAY_TONE_RANGE. */
#define OMX_DELAY_TONE_RANGE_STEP 0
/// @}
/** The shortest attack, in ms, at which the compressor's detector does not alias. Below it the compressor engages its 4x control path, which adds 72 samples of latency; a zero-latency band has no such path, so its attack travel stops at this floor. (ms) */
#define OMX_DETECTOR_OVERSAMPLE_AUTO_MS 0.5f
/** OMX_DETECTOR_OVERSAMPLE_AUTO_MS as a double literal. */
#define OMX_DETECTOR_OVERSAMPLE_AUTO_MS_DOUBLE 0.5
/** What the 4x detector oversampling costs the strip it is engaged on, in samples: the causal linear-phase half-band's reach, by which the audio is delayed so the gain still lands on the sample it was computed from. A sample count, not a time: the same at every rate. omx-dsp's oversampler declares the same number as OMX_OVS_LATENCY_4X, and its comp and gate faces report it while 4x is engaged. (none) */
#define OMX_DETECTOR_OVERSAMPLE_LATENCY_SAMPLES 72
/**
 * @name DIVERGENCE_RANGE
 * The LCR divergence travel: 0 is a hard discrete centre, 1 full phantom, the stereo image. A property of the LCR panner law itself, the same authority class as PAN_RANGE: no device narrows it.
 */
/// @{
/** The `max` of DIVERGENCE_RANGE. */
#define OMX_DIVERGENCE_RANGE_MAX 1
/** The `min` of DIVERGENCE_RANGE. */
#define OMX_DIVERGENCE_RANGE_MIN 0
/** The `step` of DIVERGENCE_RANGE. */
#define OMX_DIVERGENCE_RANGE_STEP 0
/// @}
/**
 * @name DOUBLER_DELAY_MS_RANGE
 * The voices' mean delay, ms: voice n sits at this times DOUBLER_VOICE_DELAYS[n].
 */
/// @{
/** The `default` of DOUBLER_DELAY_MS_RANGE. */
#define OMX_DOUBLER_DELAY_MS_RANGE_DEFAULT 12
/** The `max` of DOUBLER_DELAY_MS_RANGE. */
#define OMX_DOUBLER_DELAY_MS_RANGE_MAX 40
/** The `min` of DOUBLER_DELAY_MS_RANGE. */
#define OMX_DOUBLER_DELAY_MS_RANGE_MIN 5
/** The `step` of DOUBLER_DELAY_MS_RANGE. */
#define OMX_DOUBLER_DELAY_MS_RANGE_STEP 0.1f
/** OMX_DOUBLER_DELAY_MS_RANGE_STEP as a double literal. */
#define OMX_DOUBLER_DELAY_MS_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name DOUBLER_DETUNE_CENTS_RANGE
 * The voices' detune, cents: voice n is tuned by this times DOUBLER_VOICE_DETUNES[n], up for a positive factor and down for a negative one, by the Doppler of a moving read (omx_doppler), no pitch shifter.
 */
/// @{
/** The `default` of DOUBLER_DETUNE_CENTS_RANGE. */
#define OMX_DOUBLER_DETUNE_CENTS_RANGE_DEFAULT 8
/** The `max` of DOUBLER_DETUNE_CENTS_RANGE. */
#define OMX_DOUBLER_DETUNE_CENTS_RANGE_MAX 25
/** The `min` of DOUBLER_DETUNE_CENTS_RANGE. */
#define OMX_DOUBLER_DETUNE_CENTS_RANGE_MIN 0
/** The `step` of DOUBLER_DETUNE_CENTS_RANGE. */
#define OMX_DOUBLER_DETUNE_CENTS_RANGE_STEP 0.5f
/** OMX_DOUBLER_DETUNE_CENTS_RANGE_STEP as a double literal. */
#define OMX_DOUBLER_DETUNE_CENTS_RANGE_STEP_DOUBLE 0.5
/// @}
/** The share of the head's sweep the crossfade to the next head takes. (none) */
#define OMX_DOUBLER_FADE 0.25f
/** OMX_DOUBLER_FADE as a double literal. */
#define OMX_DOUBLER_FADE_DOUBLE 0.25
/** The most voices the doubler carries; its state holds this many heads. (none) */
#define OMX_DOUBLER_MAX_VOICES 4
/**
 * @name DOUBLER_MIX_PCT_RANGE
 * The share of the voices, percent: 0 is the input bit for bit, 100 the voices alone, their sum divided by the voice count.
 */
/// @{
/** The `default` of DOUBLER_MIX_PCT_RANGE. */
#define OMX_DOUBLER_MIX_PCT_RANGE_DEFAULT 50
/** The `max` of DOUBLER_MIX_PCT_RANGE. */
#define OMX_DOUBLER_MIX_PCT_RANGE_MAX 100
/** The `min` of DOUBLER_MIX_PCT_RANGE. */
#define OMX_DOUBLER_MIX_PCT_RANGE_MIN 0
/** The `step` of DOUBLER_MIX_PCT_RANGE. */
#define OMX_DOUBLER_MIX_PCT_RANGE_STEP 1
/// @}
/**
 * @name DOUBLER_MOD_DEPTH_MS_RANGE
 * How far the voices' delays drift either side of their mean, ms. 0 holds them still.
 */
/// @{
/** The `default` of DOUBLER_MOD_DEPTH_MS_RANGE. */
#define OMX_DOUBLER_MOD_DEPTH_MS_RANGE_DEFAULT 0.5f
/** OMX_DOUBLER_MOD_DEPTH_MS_RANGE_DEFAULT as a double literal. */
#define OMX_DOUBLER_MOD_DEPTH_MS_RANGE_DEFAULT_DOUBLE 0.5
/** The `max` of DOUBLER_MOD_DEPTH_MS_RANGE. */
#define OMX_DOUBLER_MOD_DEPTH_MS_RANGE_MAX 3
/** The `min` of DOUBLER_MOD_DEPTH_MS_RANGE. */
#define OMX_DOUBLER_MOD_DEPTH_MS_RANGE_MIN 0
/** The `step` of DOUBLER_MOD_DEPTH_MS_RANGE. */
#define OMX_DOUBLER_MOD_DEPTH_MS_RANGE_STEP 0.05f
/** OMX_DOUBLER_MOD_DEPTH_MS_RANGE_STEP as a double literal. */
#define OMX_DOUBLER_MOD_DEPTH_MS_RANGE_STEP_DOUBLE 0.05
/// @}
/**
 * @name DOUBLER_MOD_RATE_HZ_RANGE
 * The rate the voices' delays drift at, Hz; each voice's drift is a further 1 / voices of a turn along.
 */
/// @{
/** The `default` of DOUBLER_MOD_RATE_HZ_RANGE. */
#define OMX_DOUBLER_MOD_RATE_HZ_RANGE_DEFAULT 0.6f
/** OMX_DOUBLER_MOD_RATE_HZ_RANGE_DEFAULT as a double literal. */
#define OMX_DOUBLER_MOD_RATE_HZ_RANGE_DEFAULT_DOUBLE 0.6
/** The `max` of DOUBLER_MOD_RATE_HZ_RANGE. */
#define OMX_DOUBLER_MOD_RATE_HZ_RANGE_MAX 2
/** The `min` of DOUBLER_MOD_RATE_HZ_RANGE. */
#define OMX_DOUBLER_MOD_RATE_HZ_RANGE_MIN 0.1f
/** OMX_DOUBLER_MOD_RATE_HZ_RANGE_MIN as a double literal. */
#define OMX_DOUBLER_MOD_RATE_HZ_RANGE_MIN_DOUBLE 0.1
/** The `step` of DOUBLER_MOD_RATE_HZ_RANGE. */
#define OMX_DOUBLER_MOD_RATE_HZ_RANGE_STEP 0.05f
/** OMX_DOUBLER_MOD_RATE_HZ_RANGE_STEP as a double literal. */
#define OMX_DOUBLER_MOD_RATE_HZ_RANGE_STEP_DOUBLE 0.05
/// @}
/**
 * @name DOUBLER_SPREAD_PCT_RANGE
 * How wide the voices sit, percent: voice n is panned to this share of DOUBLER_VOICE_PANS[n] by the console's balance law.
 */
/// @{
/** The `default` of DOUBLER_SPREAD_PCT_RANGE. */
#define OMX_DOUBLER_SPREAD_PCT_RANGE_DEFAULT 60
/** The `max` of DOUBLER_SPREAD_PCT_RANGE. */
#define OMX_DOUBLER_SPREAD_PCT_RANGE_MAX 100
/** The `min` of DOUBLER_SPREAD_PCT_RANGE. */
#define OMX_DOUBLER_SPREAD_PCT_RANGE_MIN 0
/** The `step` of DOUBLER_SPREAD_PCT_RANGE. */
#define OMX_DOUBLER_SPREAD_PCT_RANGE_STEP 1
/// @}
/**
 * @name DOUBLER_VOICES_RANGE
 * How many doubled voices sound beside the dry, 1 to DOUBLER_MAX_VOICES. Two comes up.
 */
/// @{
/** The `default` of DOUBLER_VOICES_RANGE. */
#define OMX_DOUBLER_VOICES_RANGE_DEFAULT 2
/** The `max` of DOUBLER_VOICES_RANGE. */
#define OMX_DOUBLER_VOICES_RANGE_MAX 4
/** The `min` of DOUBLER_VOICES_RANGE. */
#define OMX_DOUBLER_VOICES_RANGE_MIN 1
/** The `step` of DOUBLER_VOICES_RANGE. */
#define OMX_DOUBLER_VOICES_RANGE_STEP 1
/// @}
/** The Doppler head's sweep, ms: the window a detuned voice's delay moves through before the crossfade re-centres it. (ms) */
#define OMX_DOUBLER_WINDOW_MS 20
/**
 * @name DRIVE_AMOUNT_RANGE
 * The PRE-CURVE gain, in dB. 0 dB is unity into the curve, so a stage switched on at its defaults changes almost nothing — which is what an `on` switch should mean. The ceiling is where every curve is saturating hard on any signal a console carries.
 */
/// @{
/** The `default` of DRIVE_AMOUNT_RANGE. */
#define OMX_DRIVE_AMOUNT_RANGE_DEFAULT 0
/** The `max` of DRIVE_AMOUNT_RANGE. */
#define OMX_DRIVE_AMOUNT_RANGE_MAX 36
/** The `min` of DRIVE_AMOUNT_RANGE. */
#define OMX_DRIVE_AMOUNT_RANGE_MIN 0
/** The `step` of DRIVE_AMOUNT_RANGE. */
#define OMX_DRIVE_AMOUNT_RANGE_STEP 0.1f
/** OMX_DRIVE_AMOUNT_RANGE_STEP as a double literal. */
#define OMX_DRIVE_AMOUNT_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name DRIVE_BAND_FREQ_RANGE
 * The band split / tilt corner, in Hz. ONE travel for all three band modes, because the C imposes one constraint on all of them: it is the corner of a single biquad, and the split's other half is `x − LP(x)`. The full audio band, not the channel filters' narrower HPF/LPF pair — the same reasoning REVERB_CUT_RANGE records: a surface narrower than the row removes reach the console genuinely has.
 */
/// @{
/** The `default` of DRIVE_BAND_FREQ_RANGE. */
#define OMX_DRIVE_BAND_FREQ_RANGE_DEFAULT 2000
/** The `max` of DRIVE_BAND_FREQ_RANGE. */
#define OMX_DRIVE_BAND_FREQ_RANGE_MAX 20000
/** The `min` of DRIVE_BAND_FREQ_RANGE. */
#define OMX_DRIVE_BAND_FREQ_RANGE_MIN 20
/** The `step` of DRIVE_BAND_FREQ_RANGE. */
#define OMX_DRIVE_BAND_FREQ_RANGE_STEP 1
/// @}
/** The bias at `character = +1`, in the SHAPER's own input units — `OMX_DRIVE_BIAS_MAX`. DERIVED, not chosen. In the saturating limit every knee tends to `sgn`, the output is a rectangular wave of duty `d = ½ + asin(b)/π`, and its `n`th harmonic goes as `sin(nπd)/n` — so `|H2|` is maximal at `d = ¾`, which is `b = sin(π/4) = 1/√2` exactly. Measured too: the H2-maximising bias for a full-scale sine at unity drive is 0.712 (soft), 0.658 (tape), 0.645 (tube), and this one value costs 0.000 / 0.035 / 0.088 dB of H2 against each curve's own optimum. ONE normalised bias for all three knees, not three. (none) */
#define OMX_DRIVE_BIAS_MAX 0.7071067811865476f
/** OMX_DRIVE_BIAS_MAX as a double literal. */
#define OMX_DRIVE_BIAS_MAX_DOUBLE 0.7071067811865476
/**
 * @name DRIVE_CHARACTER_RANGE
 * The EVEN/ODD harmonic balance. −1 is pure odd (the curve itself), +1 is pure even, 0 an equal blend — the kernel splits its own distortion residue into an odd and an even half and fades between them, so the two ends are exact rather than approximate (`mix_drive.h`, measured: H2 at −317 dBc at −1, H3 at −118 dBc at +1). Unitless, and signed by construction. NOT NORMALIZED_RANGE: this is a BALANCE with a meaningful centre, and a 0..1 travel would put its centre at 0.5 and its neutral nowhere.
 */
/// @{
/** The `default` of DRIVE_CHARACTER_RANGE. */
#define OMX_DRIVE_CHARACTER_RANGE_DEFAULT 0
/** The `max` of DRIVE_CHARACTER_RANGE. */
#define OMX_DRIVE_CHARACTER_RANGE_MAX 1
/** The `min` of DRIVE_CHARACTER_RANGE. */
#define OMX_DRIVE_CHARACTER_RANGE_MIN -1
/** The `step` of DRIVE_CHARACTER_RANGE. */
#define OMX_DRIVE_CHARACTER_RANGE_STEP 0.01f
/** OMX_DRIVE_CHARACTER_RANGE_STEP as a double literal. */
#define OMX_DRIVE_CHARACTER_RANGE_STEP_DOUBLE 0.01
/// @}
/**
 * @name DRIVE_MIX_RANGE
 * The wet/dry, as a PERCENT — the unit a drive control is read in on every desk that has one. Deliberately not NORMALIZED_RANGE (which is unitless 0..1): a mix a surface must label "0.35" when the operator thinks in "35 %" is a control whose declaration is fighting its reader. The kernel divides by 100 where it resolves the atom, in one place. The come-up is 100 %, not the reverb's modest 30: a saturator's insert use is IN the path. The `exciter` curve seeds itself lower (DRIVE_EXCITER_SEED), because that is a property of the TOPOLOGY and not of the travel.
 */
/// @{
/** The `default` of DRIVE_MIX_RANGE. */
#define OMX_DRIVE_MIX_RANGE_DEFAULT 100
/** The `max` of DRIVE_MIX_RANGE. */
#define OMX_DRIVE_MIX_RANGE_MAX 100
/** The `min` of DRIVE_MIX_RANGE. */
#define OMX_DRIVE_MIX_RANGE_MIN 0
/** The `step` of DRIVE_MIX_RANGE. */
#define OMX_DRIVE_MIX_RANGE_STEP 0.1f
/** OMX_DRIVE_MIX_RANGE_STEP as a double literal. */
#define OMX_DRIVE_MIX_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name DRIVE_TRIM_RANGE
 * The POST-stage make-up, in dB. Symmetric enough either way that an operator can null the stage in and out against the bypass, which is how a colour stage is actually judged.
 */
/// @{
/** The `default` of DRIVE_TRIM_RANGE. */
#define OMX_DRIVE_TRIM_RANGE_DEFAULT 0
/** The `max` of DRIVE_TRIM_RANGE. */
#define OMX_DRIVE_TRIM_RANGE_MAX 12
/** The `min` of DRIVE_TRIM_RANGE. */
#define OMX_DRIVE_TRIM_RANGE_MIN -24
/** The `step` of DRIVE_TRIM_RANGE. */
#define OMX_DRIVE_TRIM_RANGE_STEP 0.1f
/** OMX_DRIVE_TRIM_RANGE_STEP as a double literal. */
#define OMX_DRIVE_TRIM_RANGE_STEP_DOUBLE 0.1
/// @}
/** The rate a unit-range DSP knob is quoted at (R-058): the rate this desk runs (ruling 2026-09-14). A kernel raises the knob to `REF / sr`, so at this rate the exponent is exactly 1 and the response is bit-for-bit the one cut before the law. Emitted to C as `OMX_DSP_KNOB_REFERENCE_RATE`; the reverb damping and FX-delay tone references `#define` from it. (Hz) */
#define OMX_DSP_KNOB_REFERENCE_RATE 96000
/**
 * @name DYNEQ_ATTACK_RANGE
 * The band detector's attack, in ms: band_dyn's attack travel by reference, which stops at the oversampling floor; a step reaches 56.65 % at the attack time (the 4-stage cascade).
 */
/// @{
/** The `default` of DYNEQ_ATTACK_RANGE. */
#define OMX_DYNEQ_ATTACK_RANGE_DEFAULT 5
/** The `max` of DYNEQ_ATTACK_RANGE. */
#define OMX_DYNEQ_ATTACK_RANGE_MAX 100
/** The `min` of DYNEQ_ATTACK_RANGE. */
#define OMX_DYNEQ_ATTACK_RANGE_MIN 0.5f
/** OMX_DYNEQ_ATTACK_RANGE_MIN as a double literal. */
#define OMX_DYNEQ_ATTACK_RANGE_MIN_DOUBLE 0.5
/** The `step` of DYNEQ_ATTACK_RANGE. */
#define OMX_DYNEQ_ATTACK_RANGE_STEP 0.1f
/** OMX_DYNEQ_ATTACK_RANGE_STEP as a double literal. */
#define OMX_DYNEQ_ATTACK_RANGE_STEP_DOUBLE 0.1
/// @}
/** How many bands the dynamic EQ carries, each a static EQ section and its dynamics: six, the floating bands of the reference desks' dynamic EQs. Its state holds this many band-dynamics states. (none) */
#define OMX_DYNEQ_BANDS 6
/**
 * @name DYNEQ_FREQ_RANGE
 * The band's centre or corner, the EQ band's travel by reference. The come-up centres are the EQ's default rule for six bands (omx_eq_default_centres), so the bands start spread across the audio band.
 */
/// @{
/** The `max` of DYNEQ_FREQ_RANGE. */
#define OMX_DYNEQ_FREQ_RANGE_MAX 20000
/** The `min` of DYNEQ_FREQ_RANGE. */
#define OMX_DYNEQ_FREQ_RANGE_MIN 20
/** The `step` of DYNEQ_FREQ_RANGE. */
#define OMX_DYNEQ_FREQ_RANGE_STEP 10
/// @}
/**
 * @name DYNEQ_GAIN_RANGE
 * The band's static gain, the EQ band's travel by reference: the curve the band draws with its dynamics at rest. 0 dB parks the static section, as on the EQ, and the dynamics still act.
 */
/// @{
/** The `default` of DYNEQ_GAIN_RANGE. */
#define OMX_DYNEQ_GAIN_RANGE_DEFAULT 0
/** The `max` of DYNEQ_GAIN_RANGE. */
#define OMX_DYNEQ_GAIN_RANGE_MAX 15
/** The `min` of DYNEQ_GAIN_RANGE. */
#define OMX_DYNEQ_GAIN_RANGE_MIN -15
/** The `step` of DYNEQ_GAIN_RANGE. */
#define OMX_DYNEQ_GAIN_RANGE_STEP 0.5f
/** OMX_DYNEQ_GAIN_RANGE_STEP as a double literal. */
#define OMX_DYNEQ_GAIN_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name DYNEQ_MIX_PCT_RANGE
 * The share of the processed output, in percent: 100 is the dynamic EQ alone, bit for bit, 0 the input.
 */
/// @{
/** The `default` of DYNEQ_MIX_PCT_RANGE. */
#define OMX_DYNEQ_MIX_PCT_RANGE_DEFAULT 100
/** The `max` of DYNEQ_MIX_PCT_RANGE. */
#define OMX_DYNEQ_MIX_PCT_RANGE_MAX 100
/** The `min` of DYNEQ_MIX_PCT_RANGE. */
#define OMX_DYNEQ_MIX_PCT_RANGE_MIN 0
/** The `step` of DYNEQ_MIX_PCT_RANGE. */
#define OMX_DYNEQ_MIX_PCT_RANGE_STEP 1
/// @}
/**
 * @name DYNEQ_Q_RANGE
 * The bell's Q, the EQ band's travel by reference; the dynamics' bandpass takes the same Q. A shelf reads it as the EQ's shelf does.
 */
/// @{
/** The `default` of DYNEQ_Q_RANGE. */
#define OMX_DYNEQ_Q_RANGE_DEFAULT 1
/** The `max` of DYNEQ_Q_RANGE. */
#define OMX_DYNEQ_Q_RANGE_MAX 8
/** The `min` of DYNEQ_Q_RANGE. */
#define OMX_DYNEQ_Q_RANGE_MIN 0.3f
/** OMX_DYNEQ_Q_RANGE_MIN as a double literal. */
#define OMX_DYNEQ_Q_RANGE_MIN_DOUBLE 0.3
/** The `step` of DYNEQ_Q_RANGE. */
#define OMX_DYNEQ_Q_RANGE_STEP 0.1f
/** OMX_DYNEQ_Q_RANGE_STEP as a double literal. */
#define OMX_DYNEQ_Q_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name DYNEQ_RANGE_RANGE
 * The most the dynamics add to the band's static gain, signed: below zero the deepest cut, above zero the largest boost. band_dyn's range travel by reference.
 */
/// @{
/** The `default` of DYNEQ_RANGE_RANGE. */
#define OMX_DYNEQ_RANGE_RANGE_DEFAULT -6
/** The `max` of DYNEQ_RANGE_RANGE. */
#define OMX_DYNEQ_RANGE_RANGE_MAX 12
/** The `min` of DYNEQ_RANGE_RANGE. */
#define OMX_DYNEQ_RANGE_RANGE_MIN -24
/** The `step` of DYNEQ_RANGE_RANGE. */
#define OMX_DYNEQ_RANGE_RANGE_STEP 0.5f
/** OMX_DYNEQ_RANGE_RANGE_STEP as a double literal. */
#define OMX_DYNEQ_RANGE_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name DYNEQ_RATIO_RANGE
 * How hard the band is held past its threshold: band_dyn's ratio travel by reference.
 */
/// @{
/** The `default` of DYNEQ_RATIO_RANGE. */
#define OMX_DYNEQ_RATIO_RANGE_DEFAULT 2
/** The `max` of DYNEQ_RATIO_RANGE. */
#define OMX_DYNEQ_RATIO_RANGE_MAX 20
/** The `min` of DYNEQ_RATIO_RANGE. */
#define OMX_DYNEQ_RATIO_RANGE_MIN 1
/** The `step` of DYNEQ_RATIO_RANGE. */
#define OMX_DYNEQ_RATIO_RANGE_STEP 0.1f
/** OMX_DYNEQ_RATIO_RANGE_STEP as a double literal. */
#define OMX_DYNEQ_RATIO_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name DYNEQ_RELEASE_RANGE
 * The band detector's release, in ms: band_dyn's release travel by reference; a step falls to 43.35 % at the release time (the 4-stage cascade).
 */
/// @{
/** The `default` of DYNEQ_RELEASE_RANGE. */
#define OMX_DYNEQ_RELEASE_RANGE_DEFAULT 100
/** The `max` of DYNEQ_RELEASE_RANGE. */
#define OMX_DYNEQ_RELEASE_RANGE_MAX 3000
/** The `min` of DYNEQ_RELEASE_RANGE. */
#define OMX_DYNEQ_RELEASE_RANGE_MIN 5
/** The `step` of DYNEQ_RELEASE_RANGE. */
#define OMX_DYNEQ_RELEASE_RANGE_STEP 1
/// @}
/**
 * @name DYNEQ_THRESHOLD_RANGE
 * Where the band's dynamics start, in dB of the band's own level after its static section: band_dyn's threshold travel by reference.
 */
/// @{
/** The `default` of DYNEQ_THRESHOLD_RANGE. */
#define OMX_DYNEQ_THRESHOLD_RANGE_DEFAULT -30
/** The `max` of DYNEQ_THRESHOLD_RANGE. */
#define OMX_DYNEQ_THRESHOLD_RANGE_MAX 0
/** The `min` of DYNEQ_THRESHOLD_RANGE. */
#define OMX_DYNEQ_THRESHOLD_RANGE_MIN -60
/** The `step` of DYNEQ_THRESHOLD_RANGE. */
#define OMX_DYNEQ_THRESHOLD_RANGE_STEP 0.5f
/** OMX_DYNEQ_THRESHOLD_RANGE_STEP as a double literal. */
#define OMX_DYNEQ_THRESHOLD_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name EQ_BAND_COUNTS
 * How many bands each strip type's EQ starts with (`default`) and may hold (`max`). `channel` is the console's channel strip: it starts with four and the operator adds bands up to EQ_MAX_BANDS (a band added takes the frequency it is added at, so no default is declared for it). `strip` is the omx-strip plugin's EQ and `eq8`, `eq16` and `eq32` the omx-eq plugins, whose bands are fixed. A default may never exceed its maximum, and the default centres of any count come from the one default rule, whatever the strip.
 */
/// @{
/** The `channel.default` of EQ_BAND_COUNTS. */
#define OMX_EQ_BAND_COUNTS_CHANNEL_DEFAULT 4
/** The `channel.max` of EQ_BAND_COUNTS. */
#define OMX_EQ_BAND_COUNTS_CHANNEL_MAX 24
/** The `eq16.default` of EQ_BAND_COUNTS. */
#define OMX_EQ_BAND_COUNTS_EQ16_DEFAULT 16
/** The `eq16.max` of EQ_BAND_COUNTS. */
#define OMX_EQ_BAND_COUNTS_EQ16_MAX 16
/** The `eq32.default` of EQ_BAND_COUNTS. */
#define OMX_EQ_BAND_COUNTS_EQ32_DEFAULT 32
/** The `eq32.max` of EQ_BAND_COUNTS. */
#define OMX_EQ_BAND_COUNTS_EQ32_MAX 32
/** The `eq8.default` of EQ_BAND_COUNTS. */
#define OMX_EQ_BAND_COUNTS_EQ8_DEFAULT 8
/** The `eq8.max` of EQ_BAND_COUNTS. */
#define OMX_EQ_BAND_COUNTS_EQ8_MAX 8
/** The `strip.default` of EQ_BAND_COUNTS. */
#define OMX_EQ_BAND_COUNTS_STRIP_DEFAULT 4
/** The `strip.max` of EQ_BAND_COUNTS. */
#define OMX_EQ_BAND_COUNTS_STRIP_MAX 4
/// @}
/**
 * @name EQ_BAND_DEFAULTS
 * What every band of a fresh EQ comes up at, whatever its type and its strip: flat (0 dB) and Q 1. The band frequencies and types come from the default rule.
 */
/// @{
/** The `gainDb` of EQ_BAND_DEFAULTS. */
#define OMX_EQ_BAND_DEFAULTS_GAIN_DB 0
/** The `q` of EQ_BAND_DEFAULTS. */
#define OMX_EQ_BAND_DEFAULTS_Q 1
/// @}
/**
 * @name EQ_FREQ_RANGE
 * The frequency band a parametric band can be dragged across.
 */
/// @{
/** The `max` of EQ_FREQ_RANGE. */
#define OMX_EQ_FREQ_RANGE_MAX 20000
/** The `min` of EQ_FREQ_RANGE. */
#define OMX_EQ_FREQ_RANGE_MIN 20
/** The `step` of EQ_FREQ_RANGE. */
#define OMX_EQ_FREQ_RANGE_STEP 10
/// @}
/**
 * @name EQ_GAIN_RANGE
 * The parametric-band gain travel, in dB. The ± scalar `clampEqGain` enforces is READ off this (the engine derives its gain limit from `max`), so the bound is stated once and a surface cannot draw a wider knob than the clamp will honour.
 */
/// @{
/** The `max` of EQ_GAIN_RANGE. */
#define OMX_EQ_GAIN_RANGE_MAX 15
/** The `min` of EQ_GAIN_RANGE. */
#define OMX_EQ_GAIN_RANGE_MIN -15
/** The `step` of EQ_GAIN_RANGE. */
#define OMX_EQ_GAIN_RANGE_STEP 0.5f
/** OMX_EQ_GAIN_RANGE_STEP as a double literal. */
#define OMX_EQ_GAIN_RANGE_STEP_DOUBLE 0.5
/// @}
/** The maximum number of parametric bands a channel EQ may hold — the operator can add up to this many. Rendered into `omxdsp/omxdsp_definitions.h` as `OMX_EQ_MAX_BANDS`, which `omx_biquad.h` reads (the engine's conformance test reads the header): the native lane allocates exactly this many biquad slots per channel and silently clamps above it, so the model, the wire, and the DSP agree by test, not by memory. (none) */
#define OMX_EQ_MAX_BANDS 24
/**
 * @name EQ_NOTCH_Q_RANGE
 * The Q extent of a NOTCH band. Wider than EQ_Q_RANGE on purpose (#116, operator decision): an ultra-narrow surgical cut is the notch shape's whole point — the FBS ring-out pool plants dbx-AFS-style ring killers at Q ≈ 116 (~1/80 octave), inaudible on program yet fatal to a single ringing frequency. Gain-carrying shapes stay bounded at EQ_Q_RANGE — a Q-116 bell is a resonator, not an EQ move.
 */
/// @{
/** The `max` of EQ_NOTCH_Q_RANGE. */
#define OMX_EQ_NOTCH_Q_RANGE_MAX 116
/** The `min` of EQ_NOTCH_Q_RANGE. */
#define OMX_EQ_NOTCH_Q_RANGE_MIN 0.3f
/** OMX_EQ_NOTCH_Q_RANGE_MIN as a double literal. */
#define OMX_EQ_NOTCH_Q_RANGE_MIN_DOUBLE 0.3
/** The `step` of EQ_NOTCH_Q_RANGE. */
#define OMX_EQ_NOTCH_Q_RANGE_STEP 0.1f
/** OMX_EQ_NOTCH_Q_RANGE_STEP as a double literal. */
#define OMX_EQ_NOTCH_Q_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name EQ_PASS_FILTER_DEFAULTS
 * The corners the high-pass and low-pass filters of a fresh EQ are parked at, whatever the strip. Both come up off.
 */
/// @{
/** The `hpfFreqHz` of EQ_PASS_FILTER_DEFAULTS. */
#define OMX_EQ_PASS_FILTER_DEFAULTS_HPF_FREQ_HZ 80
/** The `lpfFreqHz` of EQ_PASS_FILTER_DEFAULTS. */
#define OMX_EQ_PASS_FILTER_DEFAULTS_LPF_FREQ_HZ 18000
/// @}
/**
 * @name EQ_Q_RANGE
 * The Q (resonance) extent of a parametric band.
 */
/// @{
/** The `max` of EQ_Q_RANGE. */
#define OMX_EQ_Q_RANGE_MAX 8
/** The `min` of EQ_Q_RANGE. */
#define OMX_EQ_Q_RANGE_MIN 0.3f
/** OMX_EQ_Q_RANGE_MIN as a double literal. */
#define OMX_EQ_Q_RANGE_MIN_DOUBLE 0.3
/** The `step` of EQ_Q_RANGE. */
#define OMX_EQ_Q_RANGE_STEP 0.1f
/** OMX_EQ_Q_RANGE_STEP as a double literal. */
#define OMX_EQ_Q_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name FBS_BANDS_RANGE
 * How many bands this corrector may plant at once (its pool), from one to the EQ's band count. An integer control.
 */
/// @{
/** The `default` of FBS_BANDS_RANGE. */
#define OMX_FBS_BANDS_RANGE_DEFAULT 6
/** The `max` of FBS_BANDS_RANGE. */
#define OMX_FBS_BANDS_RANGE_MAX 24
/** The `min` of FBS_BANDS_RANGE. */
#define OMX_FBS_BANDS_RANGE_MIN 1
/** The `step` of FBS_BANDS_RANGE. */
#define OMX_FBS_BANDS_RANGE_STEP 1
/// @}
/**
 * @name FBS_COOLDOWN_RANGE
 * How long a released find's escalation is remembered, in seconds. An integer control.
 */
/// @{
/** The `default` of FBS_COOLDOWN_RANGE. */
#define OMX_FBS_COOLDOWN_RANGE_DEFAULT 600
/** The `max` of FBS_COOLDOWN_RANGE. */
#define OMX_FBS_COOLDOWN_RANGE_MAX 7200
/** The `min` of FBS_COOLDOWN_RANGE. */
#define OMX_FBS_COOLDOWN_RANGE_MIN 0
/** The `step` of FBS_COOLDOWN_RANGE. */
#define OMX_FBS_COOLDOWN_RANGE_STEP 10
/// @}
/** How long a fully released find's re-clamp count is remembered, in seconds, so a chronic ringer re-plants with its doubled holds. (s) */
#define OMX_FBS_DEFAULT_COOLDOWN_SECONDS 600
/** The clean hold, in seconds, before a live band's first release stage. (s) */
#define OMX_FBS_DEFAULT_HOLD_SECONDS 5
/** How many bands one corrector may plant when nobody set its cap; a full pool plants nothing and says pool-full. (none) */
#define OMX_FBS_DEFAULT_MAX_AUTO_BANDS 6
/** The merge radius, in cents: a confirmed ring this close to a band already planted is that band's find, and plants nothing. (cents) */
#define OMX_FBS_DEFAULT_MERGE_CENTS 100
/** Default planted-notch width (Q) — the dbx-AFS-style ultra-narrow notch the corrector plants. `addBand`'s per-shape clamp holds it at 116 for a fixed notch and narrows it to 8 for a live bell, so this is the declared starting point rather than the width every notch ends up with. (none) */
#define OMX_FBS_DEFAULT_NOTCH_Q 116
/** The magnitude floor of every detector dB reading (−240 dB): a zero bin stays finite. (none) */
#define OMX_FBS_FLOOR_LIN 1e-12f
/** OMX_FBS_FLOOR_LIN as a double literal. */
#define OMX_FBS_FLOOR_LIN_DOUBLE 1e-12
/**
 * @name FBS_HOLD_RANGE
 * The clean hold, in seconds, before a live band's first release stage.
 */
/// @{
/** The `default` of FBS_HOLD_RANGE. */
#define OMX_FBS_HOLD_RANGE_DEFAULT 5
/** The `max` of FBS_HOLD_RANGE. */
#define OMX_FBS_HOLD_RANGE_MAX 300
/** The `min` of FBS_HOLD_RANGE. */
#define OMX_FBS_HOLD_RANGE_MIN 1
/** The `step` of FBS_HOLD_RANGE. */
#define OMX_FBS_HOLD_RANGE_STEP 1
/// @}
/** How long one release stage's gain ramp takes, in ms, from the depth the band stands at to the stage's depth. (ms) */
#define OMX_FBS_LIFT_RAMP_MS 3000
/** The equal steps one release ramp takes, FBS_LIFT_RAMP_MS / this apart: the last step lands the stage's depth exactly. (none) */
#define OMX_FBS_LIFT_RAMP_STEPS 10
/** The deepest cut a band of an fbs-* origin may hold, in dB: the feedback corrector's own safety bound, never the operator EQ's ±15 (EQ_GAIN_RANGE), which stays the bound of every other origin. This library owns it beside EQ_GAIN_RANGE (data/kernels/eq.json) as its own word, never a widening of that travel, and no second copy of the number is ever written. (dB) */
#define OMX_FBS_MAX_CUT_DB -24
/** THE NOTCH DEPTH LAW: the deepest cut any machine planter may propose from one reading, in dB. A number, not a literal at the call site, because two planters now lean on it. FBS derives a candidate's depth from the peak-to-average ratio around its bin and stops here; Room Analysis derives a mode's depth from its height above the median curve and is held to the SAME ceiling (the room-analysis design — "never more than the FBS notch depth law"). Written twice as `18` those two bounds would be equal by coincidence and could drift apart in silence. It is a CEILING ON THE PROPOSAL, not the final gain: a band that lands as a rampable BELL is bounded again by `EQ_GAIN_RANGE`, which is tighter. A planter must honour both and REPORT when either bit, never quietly deliver a shallower cut than it was asked for. (dB) */
#define OMX_FBS_MAX_NOTCH_DEPTH_DB 18
/**
 * @name FBS_MERGE_RANGE
 * The merge radius, in cents. An integer control.
 */
/// @{
/** The `default` of FBS_MERGE_RANGE. */
#define OMX_FBS_MERGE_RANGE_DEFAULT 100
/** The `max` of FBS_MERGE_RANGE. */
#define OMX_FBS_MERGE_RANGE_MAX 600
/** The `min` of FBS_MERGE_RANGE. */
#define OMX_FBS_MERGE_RANGE_MIN 0
/** The `step` of FBS_MERGE_RANGE. */
#define OMX_FBS_MERGE_RANGE_STEP 10
/// @}
/** The smallest pool a corrector may be given: one band. A pool of none is no corrector; the cap is held at or above this. (none) */
#define OMX_FBS_MIN_AUTO_BANDS 1
/** PAPR's guard band: bins either side of the peak left out of the floor average (its own skirt). (none) */
#define OMX_FBS_PAPR_GUARD_BINS 2
/** How many harmonics PHPR reads above the candidate: 2f, 3f and 4f. (none) */
#define OMX_FBS_PHPR_HARMONICS 3
/** The guard inside PNPR's neighbourhood radius that belongs to the peak's own skirt, in bins. (none) */
#define OMX_FBS_PNPR_GUARD_BINS 1
/** PNPR's neighbourhood radius in bins, and the guard inside it that belongs to the peak's own skirt: a pure tone is compared with bins 2 and 3 away on each side. (none) */
#define OMX_FBS_PNPR_RADIUS_BINS 3
/** PSHR skips sub-window bins within this many of the candidate, so a low bin never measures its own skirt as a sub-harmonic. (none) */
#define OMX_FBS_PSHR_SELF_SKIP_BINS 3
/** The most times the clean hold doubles after re-clamps: the hold before the first release stage is the channel's hold times 2^min(re-clamps, this). (none) */
#define OMX_FBS_RECLAMP_BACKOFF_CAP 3
/** How much deeper a live band's full depth goes each time regrowth re-clamps a release, in dB, down to FBS_MAX_CUT_DB: regrowth at a depth is the measurement that the depth was not enough. (dB) */
#define OMX_FBS_RECLAMP_DEEPEN_DB 3
/** A release stage whose depth is shallower than this, in dB, removes the band instead of holding it: the cut is inaudible. (dB) */
#define OMX_FBS_RELEASE_DONE_DB 1
/** How long a band listens at an eased stage, clean, before the next stage begins, in ms. (ms) */
#define OMX_FBS_RELEASE_DWELL_MS 8000
/** Each release stage holds the planted depth times this ratio to the stage's power: every stage releases a third of what remains. (none) */
#define OMX_FBS_RELEASE_RATIO 0.6666666666666666f
/** OMX_FBS_RELEASE_RATIO as a double literal. */
#define OMX_FBS_RELEASE_RATIO_DOUBLE 0.6666666666666666
/** The highest frequency a confirmed ring is planted at, in Hz; one above it is refused as rejected-edge. (Hz) */
#define OMX_FBS_RING_HZ_MAX 16000
/** The lowest frequency a confirmed ring is planted at, in Hz; one below it is refused as rejected-edge, never clamped onto the EQ's edge. (Hz) */
#define OMX_FBS_RING_HZ_MIN 60
/** Released finds the ring memory holds at once. The console's list is unbounded and expires by FBS_DEFAULT_COOLDOWN_SECONDS; a full memory here forgets its oldest entry first, and counts it. (none) */
#define OMX_FBS_RING_MEMORY_SLOTS 32
/** Two readings are the same ringing frequency within this many Hz, or FBS_SAME_RING_SHARE of the higher one, whichever is wider. (Hz) */
#define OMX_FBS_SAME_RING_HZ 3
/** The share of the higher frequency within which two readings are the same ringing frequency, when it is wider than FBS_SAME_RING_HZ: FFT bin spacing grows with frequency. (none) */
#define OMX_FBS_SAME_RING_SHARE 0.015f
/** OMX_FBS_SAME_RING_SHARE as a double literal. */
#define OMX_FBS_SAME_RING_SHARE_DOUBLE 0.015
/** Tracked candidate bins per armed channel. A candidate that finds no free slot is COUNTED and reported, never dropped silently; the identity corpus peaks at 34. (none) */
#define OMX_FBS_TRACK_SLOTS 512
/** The detector's frame rate, in Hz: the cadence the detector gates were tuned at. The live shell's hop is the rate over this. (Hz) */
#define OMX_FBS_WATCH_HZ 10
/** The order the chorus and flanger construct their lines with (four taps). (none) */
#define OMX_FDELAY_MOD_READ_ORDER 3
/** `Λ` for the chorus/flanger read — ONE value; the header and every law read this. (none) */
#define OMX_FDELAY_READ_L1_NORM 1.25f
/** OMX_FDELAY_READ_L1_NORM as a double literal. */
#define OMX_FDELAY_READ_L1_NORM_DOUBLE 1.25
/** The flanger's base delay, ms: the sweep rides on it up to the depth. (ms) */
#define OMX_FLANGER_BASE_MS 0.5f
/** OMX_FLANGER_BASE_MS as a double literal. */
#define OMX_FLANGER_BASE_MS_DOUBLE 0.5
/**
 * @name FLANGER_DEPTH_RANGE
 * The flanger's peak-to-peak excursion above its declared 0.5 ms base.
 */
/// @{
/** The `default` of FLANGER_DEPTH_RANGE. */
#define OMX_FLANGER_DEPTH_RANGE_DEFAULT 2
/** The `max` of FLANGER_DEPTH_RANGE. */
#define OMX_FLANGER_DEPTH_RANGE_MAX 5
/** The `min` of FLANGER_DEPTH_RANGE. */
#define OMX_FLANGER_DEPTH_RANGE_MIN 0
/** The `step` of FLANGER_DEPTH_RANGE. */
#define OMX_FLANGER_DEPTH_RANGE_STEP 0.1f
/** OMX_FLANGER_DEPTH_RANGE_STEP as a double literal. */
#define OMX_FLANGER_DEPTH_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name FLANGER_FEEDBACK_RANGE
 * The flanger's SIGNED resonance. The ±0.95 clamp is DECLARED here — a PATCH asking for 1.0 is answered by this travel — and the kernel's own clamp is the enforcement half.
 */
/// @{
/** The `default` of FLANGER_FEEDBACK_RANGE. */
#define OMX_FLANGER_FEEDBACK_RANGE_DEFAULT 0.6f
/** OMX_FLANGER_FEEDBACK_RANGE_DEFAULT as a double literal. */
#define OMX_FLANGER_FEEDBACK_RANGE_DEFAULT_DOUBLE 0.6
/** The `max` of FLANGER_FEEDBACK_RANGE. */
#define OMX_FLANGER_FEEDBACK_RANGE_MAX 0.95f
/** OMX_FLANGER_FEEDBACK_RANGE_MAX as a double literal. */
#define OMX_FLANGER_FEEDBACK_RANGE_MAX_DOUBLE 0.95
/** The `min` of FLANGER_FEEDBACK_RANGE. */
#define OMX_FLANGER_FEEDBACK_RANGE_MIN -0.95f
/** OMX_FLANGER_FEEDBACK_RANGE_MIN as a double literal. */
#define OMX_FLANGER_FEEDBACK_RANGE_MIN_DOUBLE -0.95
/** The `step` of FLANGER_FEEDBACK_RANGE. */
#define OMX_FLANGER_FEEDBACK_RANGE_STEP 0.01f
/** OMX_FLANGER_FEEDBACK_RANGE_STEP as a double literal. */
#define OMX_FLANGER_FEEDBACK_RANGE_STEP_DOUBLE 0.01
/// @}
/** The longest delay a flanger sweep reaches, rounded up to whole ms: the base delay plus the depth maximum. The delay line is sized from it. (ms) */
#define OMX_FLANGER_MAX_MS 6
/**
 * @name FLANGER_MIX_RANGE
 * The flanger's wet/dry, as an instance of DRIVE_MIX_RANGE: 50 %, equal dry and wet, where the comb is deepest.
 */
/// @{
/** The `default` of FLANGER_MIX_RANGE. */
#define OMX_FLANGER_MIX_RANGE_DEFAULT 50
/** The `max` of FLANGER_MIX_RANGE. */
#define OMX_FLANGER_MIX_RANGE_MAX 100
/** The `min` of FLANGER_MIX_RANGE. */
#define OMX_FLANGER_MIX_RANGE_MIN 0
/** The `step` of FLANGER_MIX_RANGE. */
#define OMX_FLANGER_MIX_RANGE_STEP 0.1f
/** OMX_FLANGER_MIX_RANGE_STEP as a double literal. */
#define OMX_FLANGER_MIX_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name FLANGER_RATE_RANGE
 * The flanger's jet sweep.
 */
/// @{
/** The `default` of FLANGER_RATE_RANGE. */
#define OMX_FLANGER_RATE_RANGE_DEFAULT 0.25f
/** OMX_FLANGER_RATE_RANGE_DEFAULT as a double literal. */
#define OMX_FLANGER_RATE_RANGE_DEFAULT_DOUBLE 0.25
/** The `max` of FLANGER_RATE_RANGE. */
#define OMX_FLANGER_RATE_RANGE_MAX 5
/** The `min` of FLANGER_RATE_RANGE. */
#define OMX_FLANGER_RATE_RANGE_MIN 0.05f
/** OMX_FLANGER_RATE_RANGE_MIN as a double literal. */
#define OMX_FLANGER_RATE_RANGE_MIN_DOUBLE 0.05
/** The `step` of FLANGER_RATE_RANGE. */
#define OMX_FLANGER_RATE_RANGE_STEP 0.01f
/** OMX_FLANGER_RATE_RANGE_STEP as a double literal. */
#define OMX_FLANGER_RATE_RANGE_STEP_DOUBLE 0.01
/// @}
/**
 * @name FX_DELAY_FEEDBACK_RANGE
 * The FX delay's REGENERATION travel — how much of the wet tail feeds back in. Stops short of 1: unity feedback is a runaway the kernel hard-clamps anyway, so offering it would draw travel the DSP refuses. Not NORMALIZED_RANGE for exactly that reason — it is a 0..1 quantity whose top the DSP does not accept, and a control must not offer a value its actuator will eat.
 */
/// @{
/** The `default` of FX_DELAY_FEEDBACK_RANGE. */
#define OMX_FX_DELAY_FEEDBACK_RANGE_DEFAULT 0.3f
/** OMX_FX_DELAY_FEEDBACK_RANGE_DEFAULT as a double literal. */
#define OMX_FX_DELAY_FEEDBACK_RANGE_DEFAULT_DOUBLE 0.3
/** The `max` of FX_DELAY_FEEDBACK_RANGE. */
#define OMX_FX_DELAY_FEEDBACK_RANGE_MAX 0.99f
/** OMX_FX_DELAY_FEEDBACK_RANGE_MAX as a double literal. */
#define OMX_FX_DELAY_FEEDBACK_RANGE_MAX_DOUBLE 0.99
/** The `min` of FX_DELAY_FEEDBACK_RANGE. */
#define OMX_FX_DELAY_FEEDBACK_RANGE_MIN 0
/** The `step` of FX_DELAY_FEEDBACK_RANGE. */
#define OMX_FX_DELAY_FEEDBACK_RANGE_STEP 0
/// @}
/**
 * @name FX_DELAY_TAPS_RANGE
 * How many integer read taps of the FX delay's one ring are engaged. 1 is the one-tap delay, bit for bit; `max` is the tap-array ceiling the kernel sizes its arrays by, so the arrays and the travel are one fact. An array extent is an integer constant: the C reader takes OMX_FX_DELAY_TAPS_RANGE_MAX (4) for `OMX_FXDELAY_MAX_TAPS` in `omx_delay.h`.
 */
/// @{
/** The `default` of FX_DELAY_TAPS_RANGE. */
#define OMX_FX_DELAY_TAPS_RANGE_DEFAULT 1
/** The `max` of FX_DELAY_TAPS_RANGE. */
#define OMX_FX_DELAY_TAPS_RANGE_MAX 4
/** The `min` of FX_DELAY_TAPS_RANGE. */
#define OMX_FX_DELAY_TAPS_RANGE_MIN 1
/** The `step` of FX_DELAY_TAPS_RANGE. */
#define OMX_FX_DELAY_TAPS_RANGE_STEP 1
/// @}
/**
 * @name FX_DELAY_TIME_RANGE
 * The FX delay's time travel. `max` is the NATIVE ring's ceiling — the same number as `OMX_FXDELAY_MAX_MS` in `omx_delay.h`, pinned to it by the engine's conformance test. Distinct from the per-route ALIGNMENT delay (`OUTPUT_DELAY_RANGE`, 1 s, `OMX_DELAY_MAX_MS`): one is a musical echo with feedback and mix, the other is a pure latency. Two facts, two rings, two ceilings — sharing the word "delay" is why they keep being confused for each other.
 */
/// @{
/** The `default` of FX_DELAY_TIME_RANGE. */
#define OMX_FX_DELAY_TIME_RANGE_DEFAULT 300
/** The `max` of FX_DELAY_TIME_RANGE. */
#define OMX_FX_DELAY_TIME_RANGE_MAX 2000
/** The `min` of FX_DELAY_TIME_RANGE. */
#define OMX_FX_DELAY_TIME_RANGE_MIN 0
/** The `step` of FX_DELAY_TIME_RANGE. */
#define OMX_FX_DELAY_TIME_RANGE_STEP 1
/// @}
/** How fast a wet duck follows its dry key up, ms: the follower's attack, one for every effect that ducks its wet (the delay, the reverb). (ms) */
#define OMX_FX_DUCK_ATTACK_MS 5
/**
 * @name GATE_KEY_TILT_DB_RANGE
 * The key's tilt, in dB per octave at KEY_TILT_PIVOT_HZ, after the key filter: positive lets the gate open on a brighter key (a tom, a snare) and stay shut on the low end, negative the other way (omx_tilt, the compressor's key tilt). 0 is flat, the come-up value, bit for bit.
 */
/// @{
/** The `default` of GATE_KEY_TILT_DB_RANGE. */
#define OMX_GATE_KEY_TILT_DB_RANGE_DEFAULT 0
/** The `max` of GATE_KEY_TILT_DB_RANGE. */
#define OMX_GATE_KEY_TILT_DB_RANGE_MAX 6
/** The `min` of GATE_KEY_TILT_DB_RANGE. */
#define OMX_GATE_KEY_TILT_DB_RANGE_MIN -6
/** The `step` of GATE_KEY_TILT_DB_RANGE. */
#define OMX_GATE_KEY_TILT_DB_RANGE_STEP 0.5f
/** OMX_GATE_KEY_TILT_DB_RANGE_STEP as a double literal. */
#define OMX_GATE_KEY_TILT_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/** The graphic EQ's section count: DERIVED from ISO_THIRD_OCTAVE_CENTRES_HZ, never a second literal. Rendered into `omxdsp/omxdsp_definitions.h` as `OMX_GEQ_BANDS`, which sizes `mix_geq.h`'s section array — its OWN budget, never a claimant on EQ_MAX_BANDS (roster ruling 3). (none) */
#define OMX_GEQ_BANDS 31
/** The graphic EQ's band Q: the ISO third-octave bandwidth, `2^(1/6)/(2^(1/3) − 1) ≈ 4.318`, to one decimal as the M32 GEQ carries it. Read by the `geq31` preset and the graphic EQ's section design. (none) */
#define OMX_GEQ_BAND_Q 4.3f
/** OMX_GEQ_BAND_Q as a double literal. */
#define OMX_GEQ_BAND_Q_DOUBLE 4.3
/**
 * @name GEQ_BAND_RANGE
 * One graphic-EQ fader's travel: EQ_GAIN_RANGE by reference (a graphic fader is an EQ gain), with the come-up value it needs as a row field — flat, 0 dB.
 */
/// @{
/** The `default` of GEQ_BAND_RANGE. */
#define OMX_GEQ_BAND_RANGE_DEFAULT 0
/** The `max` of GEQ_BAND_RANGE. */
#define OMX_GEQ_BAND_RANGE_MAX 15
/** The `min` of GEQ_BAND_RANGE. */
#define OMX_GEQ_BAND_RANGE_MIN -15
/** The `step` of GEQ_BAND_RANGE. */
#define OMX_GEQ_BAND_RANGE_STEP 0.5f
/** OMX_GEQ_BAND_RANGE_STEP as a double literal. */
#define OMX_GEQ_BAND_RANGE_STEP_DOUBLE 0.5
/// @}
/** The ceiling on a section gain the `true` solve may ask for: above the travel's worst case (26.0 dB, the alternating ±15 set) with margin. A solve past it clamps and the row says so (`designClamped`), never silently. (dB) */
#define OMX_GEQ_DESIGN_GAIN_MAX_DB 30
/** The prototype gain the `true` solve's first interaction matrix is evaluated at: the middle of the travel's magnitude, where a matched-Z section's dB response per dB of gain is closest to its average. (dB) */
#define OMX_GEQ_PROTO_DB 12
/**
 * @name GLUE_KEY_HP_HZ_RANGE
 * The detector's high-pass corner, Hz: 0 is out; a corner keeps the low end from pumping the whole bus. In the feedback detectorTopology the corner is inert: the loop hears the full band, a keyed slot being feed-forward.
 */
/// @{
/** The `default` of GLUE_KEY_HP_HZ_RANGE. */
#define OMX_GLUE_KEY_HP_HZ_RANGE_DEFAULT 0
/** The `max` of GLUE_KEY_HP_HZ_RANGE. */
#define OMX_GLUE_KEY_HP_HZ_RANGE_MAX 500
/** The `min` of GLUE_KEY_HP_HZ_RANGE. */
#define OMX_GLUE_KEY_HP_HZ_RANGE_MIN 0
/** The `step` of GLUE_KEY_HP_HZ_RANGE. */
#define OMX_GLUE_KEY_HP_HZ_RANGE_STEP 1
/// @}
/** The knee, dB, across the threshold: narrow, so the detents keep the character of a VCA bus compressor. (dB) */
#define OMX_GLUE_KNEE_DB 3
/**
 * @name GLUE_MAKEUP_DB_RANGE
 * The gain after the pull, dB.
 */
/// @{
/** The `default` of GLUE_MAKEUP_DB_RANGE. */
#define OMX_GLUE_MAKEUP_DB_RANGE_DEFAULT 0
/** The `max` of GLUE_MAKEUP_DB_RANGE. */
#define OMX_GLUE_MAKEUP_DB_RANGE_MAX 24
/** The `min` of GLUE_MAKEUP_DB_RANGE. */
#define OMX_GLUE_MAKEUP_DB_RANGE_MIN 0
/** The `step` of GLUE_MAKEUP_DB_RANGE. */
#define OMX_GLUE_MAKEUP_DB_RANGE_STEP 0.5f
/** OMX_GLUE_MAKEUP_DB_RANGE_STEP as a double literal. */
#define OMX_GLUE_MAKEUP_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name GLUE_MIX_PCT_RANGE
 * The compressed bus against the dry one, percent: below 100 is parallel compression.
 */
/// @{
/** The `default` of GLUE_MIX_PCT_RANGE. */
#define OMX_GLUE_MIX_PCT_RANGE_DEFAULT 100
/** The `max` of GLUE_MIX_PCT_RANGE. */
#define OMX_GLUE_MIX_PCT_RANGE_MAX 100
/** The `min` of GLUE_MIX_PCT_RANGE. */
#define OMX_GLUE_MIX_PCT_RANGE_MIN 0
/** The `step` of GLUE_MIX_PCT_RANGE. */
#define OMX_GLUE_MIX_PCT_RANGE_STEP 1
/// @}
/**
 * @name GLUE_THRESHOLD_DB_RANGE
 * The level, dBFS, above which the bus is pulled down.
 */
/// @{
/** The `default` of GLUE_THRESHOLD_DB_RANGE. */
#define OMX_GLUE_THRESHOLD_DB_RANGE_DEFAULT -10
/** The `max` of GLUE_THRESHOLD_DB_RANGE. */
#define OMX_GLUE_THRESHOLD_DB_RANGE_MAX 0
/** The `min` of GLUE_THRESHOLD_DB_RANGE. */
#define OMX_GLUE_THRESHOLD_DB_RANGE_MIN -40
/** The `step` of GLUE_THRESHOLD_DB_RANGE. */
#define OMX_GLUE_THRESHOLD_DB_RANGE_STEP 0.5f
/** OMX_GLUE_THRESHOLD_DB_RANGE_STEP as a double literal. */
#define OMX_GLUE_THRESHOLD_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/** How many CONSECUTIVE frames the ramp gate (`omx_fbs_is_growing`) must hold before a bin confirms — the time-domain tiebreak for the one shape a single window cannot judge: a smooth musical crescendo. Within one window a +6 dB swell (live corpus: a C6 at 1 049 Hz, `[-53.7, -51.4, -50.4, -48, -47.3]`) is indistinguishable from slow feedback — the difference is only visible on the NEXT hop, where a crescendo peaks and plateaus (the window's rise collapses) while feedback keeps climbing. Requiring the gate to hold twice in a row costs a clean ramp nothing — its gate is already true while persistence is still arming — and blocks the swell that peaks the very frame it would have confirmed. (none) */
#define OMX_GROWTH_CONFIRM_FRAMES 2
/** Largest single-frame DIP (dB) a run-away window may contain. Real feedback climbs every hop; a small allowance covers measurement jitter and the −0.3 dB-ish wobble frames seen on live captures, while any real drop (a note ending, tremolo) breaks the run. (dB) */
#define OMX_GROWTH_MAX_DIP_DB 0.5f
/** OMX_GROWTH_MAX_DIP_DB as a double literal. */
#define OMX_GROWTH_MAX_DIP_DB_DOUBLE 0.5
/** Cap on the share of the window's total rise a SINGLE frame may carry. Feedback is exponential in amplitude ⇒ linear in dB ⇒ its rise is distributed roughly evenly across the window; a musical attack / re-articulation puts almost the whole rise in one hop (the exact shape behind every one of the 72 live false positives of the live-music corpus). One half is generous to real ramps (an even N-frame ramp puts 1/(N−1) per hop) and fatal to steps. A side effect worth knowing: a 2-sample window can never pass (one hop IS the whole rise), so confirmation always rests on at least three frames of actual ramp evidence. (none) */
#define OMX_GROWTH_MAX_SINGLE_FRAME_SHARE 0.5f
/** OMX_GROWTH_MAX_SINGLE_FRAME_SHARE as a double literal. */
#define OMX_GROWTH_MAX_SINGLE_FRAME_SHARE_DOUBLE 0.5
/** Minimum least-squares slope, in dB PER FRAME, over GROWTH_WINDOW_FRAMES for a bin to count as "running away" rather than merely holding level. Set with real margin on BOTH sides — comfortably above measurement jitter on a held tone or a quiet/broadband mix (well under 0.2 dB/frame in practice) and comfortably below a genuine feedback ramp, which climbs several dB across a handful of ~20 ms hops as it heads toward clipping. The acceptance bar is "arming must never burst-alarm on non-growing content", so this errs conservative (under-fire a marginal ramp) rather than tight (risk a false alarm) — the ramp gate (`omx_fbs_is_growing`) also requires the window's end to sit above its start, a cheap belt-and-braces check against a fit dragged positive by one mid-window wobble. (dB) */
#define OMX_GROWTH_MIN_DB_PER_FRAME 0.8f
/** OMX_GROWTH_MIN_DB_PER_FRAME as a double literal. */
#define OMX_GROWTH_MIN_DB_PER_FRAME_DOUBLE 0.8
/** Minimum END-TO-END rise (dB) across the window. The live-music corpus showed sustained melody notes drifting up ~4–5 dB over a window without ever being feedback; a loop past unity gains several dB per hop and clears 6 dB in a window trivially. Below this the tone is holding level, not running away. (dB) */
#define OMX_GROWTH_MIN_TOTAL_RISE_DB 6
/** How many of a candidate bin's most recent dB readings feed the growth-slope estimate (the engine's least-squares slope) — set equal to DEFAULT_THRESHOLDS's `persistFrames` so growth is judged over the SAME window persistence already demands (the full run since the bin became a candidate, not a shorter or longer slice of it). Long enough that a couple of noisy/vibrato frames can't fake a trend by themselves. (none) */
#define OMX_GROWTH_WINDOW_FRAMES 5
/**
 * @name HPF_FREQ_RANGE
 * Where the HPF cutoff can sit (low end); mirrors the drawn low-frequency handle.
 */
/// @{
/** The `max` of HPF_FREQ_RANGE. */
#define OMX_HPF_FREQ_RANGE_MAX 1000
/** The `min` of HPF_FREQ_RANGE. */
#define OMX_HPF_FREQ_RANGE_MIN 20
/** The `step` of HPF_FREQ_RANGE. */
#define OMX_HPF_FREQ_RANGE_STEP 10
/// @}
/**
 * @name HRP_AMOUNT_RANGE
 * How much of the computed correction HRP applies: 0 = analyse only, 1 = the full cut. It comes up at 0.5, half the computed cut.
 */
/// @{
/** The `default` of HRP_AMOUNT_RANGE. */
#define OMX_HRP_AMOUNT_RANGE_DEFAULT 0.5f
/** OMX_HRP_AMOUNT_RANGE_DEFAULT as a double literal. */
#define OMX_HRP_AMOUNT_RANGE_DEFAULT_DOUBLE 0.5
/** The `max` of HRP_AMOUNT_RANGE. */
#define OMX_HRP_AMOUNT_RANGE_MAX 1
/** The `min` of HRP_AMOUNT_RANGE. */
#define OMX_HRP_AMOUNT_RANGE_MIN 0
/** The `step` of HRP_AMOUNT_RANGE. */
#define OMX_HRP_AMOUNT_RANGE_STEP 0
/// @}
/** The ATTRIBUTION window: two partials closer than this are one spectral event as far as the desk can tell — the same ±1 bin `omx_hrp_bin_peak` reads over, expressed musically so it holds across the range. A different law from HRP_COLLISION_CENTS (the SELECTION window, which stays above it): this one is the analyser's resolution, that one the corrector's. Rendered into `omxdsp/omxdsp_definitions.h` as `OMX_HRP_ATTRIBUTION_CENTS`; `hrp_attribute.h` reads it. (cents) */
#define OMX_HRP_ATTRIBUTION_CENTS 60
/**
 * @name HRP_BANDS_RANGE
 * How many bands one HRP instance may plant at once, from one up to the size of its answer set (HRP_DEFAULT_MAX_AUTO_BANDS). It comes up at the full budget. An integer control: a face rounds the word inside the travel and reads a non-finite one as the default.
 */
/// @{
/** The `default` of HRP_BANDS_RANGE. */
#define OMX_HRP_BANDS_RANGE_DEFAULT 8
/** The `max` of HRP_BANDS_RANGE. */
#define OMX_HRP_BANDS_RANGE_MAX 8
/** The `min` of HRP_BANDS_RANGE. */
#define OMX_HRP_BANDS_RANGE_MIN 1
/** The `step` of HRP_BANDS_RANGE. */
#define OMX_HRP_BANDS_RANGE_STEP 1
/// @}
/** The Q of an HRP correction band — one declared number, operator-adjustable later. A musical cut, not a ring killer: FBS's default notch Q is ~116 because a howl is a single frequency, whereas a partial that is too loud is a note an instrument is playing and a surgical null on it would be heard as a hole. 5 sits mid-band in the 4…6 range a musical subtractive cut occupies, and inside EQ_Q_RANGE — the gain-carrying shapes' extent — so it survives the store's clamp unchanged. Deliberately NOT derived from the harmonic index. A derivation invented with no measurement behind it is a second declaration wearing a formula; this is revisited when the analyse-only surface shows what real corrections look like. (none) */
#define OMX_HRP_BAND_Q 5
/** How close two correction candidates must sit, in cents, to count as the SAME spectral region: within it, the stronger candidate is selected and the weaker discarded rather than two near-identical filters being planted. One semitone, and the number follows from HRP_BAND_Q: a Q-5 bell's −3 dB half-width is ~170 cents, so two cuts a semitone apart overlap over most of their travel — the second one deepens the first far more than it shapes anything of its own. The design makes the threshold configurable and tested: declared once here, a parameter everywhere below. (cents) */
#define OMX_HRP_COLLISION_CENTS 100
/** How many bands HRP may plant on ONE channel — its half of the machine budget, and the number that makes the budget FORECASTABLE rather than emergent. Without it the count is whatever the material happens to provoke: the placement law is sparse, so a clean note plants two bands and a resonant room plants nine, and nobody can answer "will this fit?" until it does not. With FBS senior that discovery would happen at showtime, to the junior planter, silently. Declared instead, the whole sum is arithmetic an operator can read before a note is played: operator bands + FBS maxAutoBands + HRP maxAutoBands \<= EQ_MAX_BANDS EIGHT — ONE CORRECTION PER VOICE at full polyphony (operator sizing, 2026-09-01). The detector's ceiling is HRP_MAX_VOICES voices, a guitar's six strings and a piano/string voicing's eight, and a budget below the voice count would decide by arithmetic which of the notes actually sounding may be corrected — a limit nobody could read off the desk. Scarcity is handled where it belongs instead: the candidates from every voice meet in ONE globally ranked list and the head of it is what gets planted. It is NOT a claim to seniority, and it is larger than FBS's six. Seniority is the ALLOCATION ORDER — `HrpPlanter` takes only `EQ_MAX_BANDS − others.length`, the remainder after the operator and every other machine — so a bigger junior default cannot reach a senior slot; it only means HRP asks for more of what is left. the engine's conformance test asserts both halves. The per-channel DEFAULT; the row's travel is HRP_AUTO_BANDS_RANGE, so a channel that needs more can have more, as an explicit decision with a visible cost. (none) */
#define OMX_HRP_DEFAULT_MAX_AUTO_BANDS 8
/** How far a partial must stand above its LEARNED BASELINE before HRP calls it a resonance (increment 8: `deviation = measured − baseline[note][harmonic]`). A partial level tells you nothing on its own; what marks a resonance is a partial louder than THIS instrument normally measures at THIS note. Three decibels is the same order as the feedback detector's peak-to-neighbour gate and for the same reason: below it, an ordinary performance's variation — a bow dug in, a verse sung harder — would be flattened, and flattening expression is not correcting a room. (Before the learned model existed this same threshold gated the placeholder neighbour rule; the number and its reasoning carried over.) (dB) */
#define OMX_HRP_EXCESS_THRESHOLD_DB 3
/**
 * @name HRP_FAMILY_RANGE
 * The instrument family HRP searches for, as an OmxHrpFamily value: -1 (OMX_HRP_FAMILY_NONE) is the console's wide default search, 0 to 9 the families of INSTRUMENT_FAMILIES in their order. A broad prior on where the fundamentals lie and how many voices sound at once; it never moves the confidence floor. An integer control: a face rounds the word inside the travel and reads a non-finite one as the default.
 */
/// @{
/** The `default` of HRP_FAMILY_RANGE. */
#define OMX_HRP_FAMILY_RANGE_DEFAULT -1
/** The `max` of HRP_FAMILY_RANGE. */
#define OMX_HRP_FAMILY_RANGE_MAX 9
/** The `min` of HRP_FAMILY_RANGE. */
#define OMX_HRP_FAMILY_RANGE_MIN -1
/** The `step` of HRP_FAMILY_RANGE. */
#define OMX_HRP_FAMILY_RANGE_STEP 1
/// @}
/**
 * @name HRP_FAMILY_SEARCH
 * What each instrument family implies about the HRP search: `maxVoices`, how many notes the family can sound at once (the number its corrections are budgeted against), and `f0MinHz` to `f0MaxHz`, where its fundamentals can lie. Each is the widest the family can be, not the typical one: a range too narrow hides a note from the detector, one slightly too wide costs a few more sweep steps. One entry per INSTRUMENT_FAMILIES id; the confidence floor does not follow the family.
 */
/// @{
/** The `bass.f0MaxHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_BASS_F0_MAX_HZ 400
/** The `bass.f0MinHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_BASS_F0_MIN_HZ 25
/** The `bass.maxVoices` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_BASS_MAX_VOICES 2
/** The `brass.f0MaxHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_BRASS_F0_MAX_HZ 1400
/** The `brass.f0MinHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_BRASS_F0_MIN_HZ 40
/** The `brass.maxVoices` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_BRASS_MAX_VOICES 2
/** The `drums.f0MaxHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_DRUMS_F0_MAX_HZ 400
/** The `drums.f0MinHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_DRUMS_F0_MIN_HZ 35
/** The `drums.maxVoices` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_DRUMS_MAX_VOICES 1
/** The `guitar.f0MaxHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_GUITAR_F0_MAX_HZ 1400
/** The `guitar.f0MinHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_GUITAR_F0_MIN_HZ 60
/** The `guitar.maxVoices` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_GUITAR_MAX_VOICES 6
/** The `keys.f0MaxHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_KEYS_F0_MAX_HZ 4200
/** The `keys.f0MinHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_KEYS_F0_MIN_HZ 27.5f
/** OMX_HRP_FAMILY_SEARCH_KEYS_F0_MIN_HZ as a double literal. */
#define OMX_HRP_FAMILY_SEARCH_KEYS_F0_MIN_HZ_DOUBLE 27.5
/** The `keys.maxVoices` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_KEYS_MAX_VOICES 8
/** The `percussion.f0MaxHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_PERCUSSION_F0_MAX_HZ 1200
/** The `percussion.f0MinHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_PERCUSSION_F0_MIN_HZ 60
/** The `percussion.maxVoices` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_PERCUSSION_MAX_VOICES 1
/** The `playback.f0MaxHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_PLAYBACK_F0_MAX_HZ 2100
/** The `playback.f0MinHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_PLAYBACK_F0_MIN_HZ 25
/** The `playback.maxVoices` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_PLAYBACK_MAX_VOICES 8
/** The `strings.f0MaxHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_STRINGS_F0_MAX_HZ 2800
/** The `strings.f0MinHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_STRINGS_F0_MIN_HZ 40
/** The `strings.maxVoices` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_STRINGS_MAX_VOICES 8
/** The `vocals.f0MaxHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_VOCALS_F0_MAX_HZ 1200
/** The `vocals.f0MinHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_VOCALS_F0_MIN_HZ 80
/** The `vocals.maxVoices` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_VOCALS_MAX_VOICES 2
/** The `wind.f0MaxHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_WIND_F0_MAX_HZ 2600
/** The `wind.f0MinHz` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_WIND_F0_MIN_HZ 55
/** The `wind.maxVoices` of HRP_FAMILY_SEARCH. */
#define OMX_HRP_FAMILY_SEARCH_WIND_MAX_VOICES 2
/// @}
/** How far ONE reconcile pass may move an HRP band's gain, in dB — the gain smoothing, done at the control level because the native cascade applies pushed coefficients verbatim and the design forbids interpolating there. An unbounded gain step is a waveform discontinuity — the click heard when a correction entered or left at full depth (found live, 2026-08-20, the evening increment 9 raised the enter/leave rate). Bounded, a correction ENTERS by deepening from silence and LEAVES by walking to exactly 0 dB — the provable parked identity (`eqBandIsIdentity`) — from which removal is bit-identical. Two decibels per pass at the corrector's 10 Hz cadence walks the full HRP_MAX_CUT_DB travel in ~300 ms: the same order as the tracker's stability hold and FBS's lift steps, while each individual move stays below audibility on a Q-5 bell. (dB) */
#define OMX_HRP_GAIN_STEP_DB 2
/** How many harmonics the HRP model tracks, H1 to H12: the partials it measures per voice. Not a band budget: the number it may cut is the channel's free EQ slots, which is smaller and belongs to the channel. omx-dsp's HRP analysis sizes its per-voice partial arrays by the same number (OMX_HRP_HARMONICS). (none) */
#define OMX_HRP_HARMONIC_COUNT 12
/** The largest automatic cut HRP may place on one harmonic, as a POSITIVE dB magnitude. Well inside the ±15 dB an operator band may hold (core's `EQ_GAIN_LIMIT_DB`): an automatic correction the operator did not ask for gets far less authority than one they dialled themselves. (dB) */
#define OMX_HRP_MAX_CUT_DB 6
/** How many simultaneous musical voices the detector may report on one channel. PINNED to `OMX_HRP_MAX_VOICES` in `pipewire-native/src/hrp_pitch.h` — the C voice set is a FIXED-SIZE array, so a TS number above the macro promises voices the analyser structurally cannot return, and one below it silently drops notes the C already found. the engine's conformance test reads the header and holds the pair equal. EIGHT (operator sizing, 2026-09-01): a guitar sounds six strings, a piano or string-section voicing sits at eight. It is a SEARCH ceiling, not a cost — the detector abandons a round as soon as no credible candidate remains, so monophonic material costs what it always did. (none) */
#define OMX_HRP_MAX_VOICES 8
/**
 * @name INPUT_ALIGN_DELAY_RANGE
 * The input alignment delay travel: how far one microphone is pushed back to meet another on the same source. Not the FX delay (FX_DELAY_TIME_RANGE, a musical echo) and not the output delay (OUTPUT_DELAY_RANGE, a loudspeaker zone). max is the alignment ring's ceiling, the same number as OMX_ALIGN_MAX_MS in omx-dsp's omx_align.h: 100 ms is 34 m of air, further apart than two microphones on one source can be. The step is fine enough to carry what the fractional read applies: 0.0001 ms is under a hundredth of a sample at 96 kHz.
 */
/// @{
/** The `max` of INPUT_ALIGN_DELAY_RANGE. */
#define OMX_INPUT_ALIGN_DELAY_RANGE_MAX 100
/** The `min` of INPUT_ALIGN_DELAY_RANGE. */
#define OMX_INPUT_ALIGN_DELAY_RANGE_MIN 0
/** The `step` of INPUT_ALIGN_DELAY_RANGE. */
#define OMX_INPUT_ALIGN_DELAY_RANGE_STEP 0.0001f
/** OMX_INPUT_ALIGN_DELAY_RANGE_STEP as a double literal. */
#define OMX_INPUT_ALIGN_DELAY_RANGE_STEP_DOUBLE 0.0001
/// @}
/** Where a key's tilt pivots, Hz: the frequency a tilt leaves at unity while it brightens or darkens what the detector hears. One pivot for every key that tilts. (Hz) */
#define OMX_KEY_TILT_PIVOT_HZ 1000
/**
 * @name LEVELER_AMOUNT_RANGE
 * How hard the leveler works, percent: the one knob. It reads the declared table (LEVELER_AMOUNT_POINTS and the rows beside it) for the threshold, ratio and make-up of the downward slot and the threshold, ratio and largest boost of the upward one, linear between the table's points. 0 is the input, bit for bit, the come-up value.
 */
/// @{
/** The `default` of LEVELER_AMOUNT_RANGE. */
#define OMX_LEVELER_AMOUNT_RANGE_DEFAULT 0
/** The `max` of LEVELER_AMOUNT_RANGE. */
#define OMX_LEVELER_AMOUNT_RANGE_MAX 100
/** The `min` of LEVELER_AMOUNT_RANGE. */
#define OMX_LEVELER_AMOUNT_RANGE_MIN 0
/** The `step` of LEVELER_AMOUNT_RANGE. */
#define OMX_LEVELER_AMOUNT_RANGE_STEP 1
/// @}
/** Both slots' knee, dB: a leveler never runs hard. (dB) */
#define OMX_LEVELER_KNEE_DB 6
/**
 * @name LIN_TO_DB_FLOOR
 * The floor of the linear-to-decibel conversions (`omx_lin_to_db`, `omx_lin_to_db_poly`): an amplitude below `lin` reads as `lin`, so silence is `db` dB, never -inf or NaN. `db` is `20·log10(lin)`. A design constant, not a control. (since 3.0.0)
 */
/// @{
/** The `db` of LIN_TO_DB_FLOOR. */
#define OMX_LIN_TO_DB_FLOOR_DB -180
/** The `lin` of LIN_TO_DB_FLOOR. */
#define OMX_LIN_TO_DB_FLOOR_LIN 1e-9f
/** OMX_LIN_TO_DB_FLOOR_LIN as a double literal. */
#define OMX_LIN_TO_DB_FLOOR_LIN_DOUBLE 1e-9
/// @}
/**
 * @name LOUDLEVEL_GATE_LUFS_RANGE
 * Under this the gain holds: a pause is not a quiet passage, and the leveler must not ride up the room between sentences.
 */
/// @{
/** The `default` of LOUDLEVEL_GATE_LUFS_RANGE. */
#define OMX_LOUDLEVEL_GATE_LUFS_RANGE_DEFAULT -50
/** The `max` of LOUDLEVEL_GATE_LUFS_RANGE. */
#define OMX_LOUDLEVEL_GATE_LUFS_RANGE_MAX -30
/** The `min` of LOUDLEVEL_GATE_LUFS_RANGE. */
#define OMX_LOUDLEVEL_GATE_LUFS_RANGE_MIN -70
/** The `step` of LOUDLEVEL_GATE_LUFS_RANGE. */
#define OMX_LOUDLEVEL_GATE_LUFS_RANGE_STEP 1
/// @}
/**
 * @name LOUDLEVEL_RANGE_DB_RANGE
 * The most the face moves the signal, dB, either way. It comes up at 0, the input bit for bit: nothing moves until it is given room.
 */
/// @{
/** The `default` of LOUDLEVEL_RANGE_DB_RANGE. */
#define OMX_LOUDLEVEL_RANGE_DB_RANGE_DEFAULT 0
/** The `max` of LOUDLEVEL_RANGE_DB_RANGE. */
#define OMX_LOUDLEVEL_RANGE_DB_RANGE_MAX 24
/** The `min` of LOUDLEVEL_RANGE_DB_RANGE. */
#define OMX_LOUDLEVEL_RANGE_DB_RANGE_MIN 0
/** The `step` of LOUDLEVEL_RANGE_DB_RANGE. */
#define OMX_LOUDLEVEL_RANGE_DB_RANGE_STEP 0.5f
/** OMX_LOUDLEVEL_RANGE_DB_RANGE_STEP as a double literal. */
#define OMX_LOUDLEVEL_RANGE_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name LOUDLEVEL_SPEED_DB_PER_S_RANGE
 * How fast the gain walks, dB per second. A slow walk is a hand on a fader; a fast one reads as compression.
 */
/// @{
/** The `default` of LOUDLEVEL_SPEED_DB_PER_S_RANGE. */
#define OMX_LOUDLEVEL_SPEED_DB_PER_S_RANGE_DEFAULT 1
/** The `max` of LOUDLEVEL_SPEED_DB_PER_S_RANGE. */
#define OMX_LOUDLEVEL_SPEED_DB_PER_S_RANGE_MAX 6
/** The `min` of LOUDLEVEL_SPEED_DB_PER_S_RANGE. */
#define OMX_LOUDLEVEL_SPEED_DB_PER_S_RANGE_MIN 0.1f
/** OMX_LOUDLEVEL_SPEED_DB_PER_S_RANGE_MIN as a double literal. */
#define OMX_LOUDLEVEL_SPEED_DB_PER_S_RANGE_MIN_DOUBLE 0.1
/** The `step` of LOUDLEVEL_SPEED_DB_PER_S_RANGE. */
#define OMX_LOUDLEVEL_SPEED_DB_PER_S_RANGE_STEP 0.1f
/** OMX_LOUDLEVEL_SPEED_DB_PER_S_RANGE_STEP as a double literal. */
#define OMX_LOUDLEVEL_SPEED_DB_PER_S_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name LOUDLEVEL_TARGET_LUFS_RANGE
 * The loudness the face holds the signal at, LUFS (BS.1770, K-weighted).
 */
/// @{
/** The `default` of LOUDLEVEL_TARGET_LUFS_RANGE. */
#define OMX_LOUDLEVEL_TARGET_LUFS_RANGE_DEFAULT -23
/** The `max` of LOUDLEVEL_TARGET_LUFS_RANGE. */
#define OMX_LOUDLEVEL_TARGET_LUFS_RANGE_MAX -6
/** The `min` of LOUDLEVEL_TARGET_LUFS_RANGE. */
#define OMX_LOUDLEVEL_TARGET_LUFS_RANGE_MIN -40
/** The `step` of LOUDLEVEL_TARGET_LUFS_RANGE. */
#define OMX_LOUDLEVEL_TARGET_LUFS_RANGE_STEP 0.5f
/** OMX_LOUDLEVEL_TARGET_LUFS_RANGE_STEP as a double literal. */
#define OMX_LOUDLEVEL_TARGET_LUFS_RANGE_STEP_DOUBLE 0.5
/// @}
/** The width of a bin of the loudness core's block histograms, from BS1770_ABSOLUTE_GATE_LUFS up: this library's resolution, not a clause of BS.1770 or EBU Tech 3342, which state no histogram. LU is written dB here until omx-contract carries the unit (omx-contract#145). (dB; since 3.0.0) */
#define OMX_LOUDNESS_HISTOGRAM_BIN_LU 0.1f
/** OMX_LOUDNESS_HISTOGRAM_BIN_LU as a double literal. */
#define OMX_LOUDNESS_HISTOGRAM_BIN_LU_DOUBLE 0.1
/**
 * @name LOWPASS_HANDOVER
 * Where the low-pass section hands its poles over from the matched pair to the prewarped bilinear pair, as fractions of Nyquist, and how that band moves with Q. At `qHi` (1/sqrt(2), the 12 dB Butterworth) the band runs from `loAtHiQ` to `hiAtHiQ`; at `qLo` (0.5412, the low-Q half of the 24 dB Butterworth) and below, from `loAtLoQ` to `hiAtLoQ`; between the two it moves linearly. Above `qHi` the prototype peaks, and the band's end moves down to `hiAtPeakQ` by `qPeak`, held above it. Below the band the low-pass is the matched design unchanged.
 */
/// @{
/** The `hiAtHiQ` of LOWPASS_HANDOVER. */
#define OMX_LOWPASS_HANDOVER_HI_AT_HI_Q 0.85f
/** OMX_LOWPASS_HANDOVER_HI_AT_HI_Q as a double literal. */
#define OMX_LOWPASS_HANDOVER_HI_AT_HI_Q_DOUBLE 0.85
/** The `hiAtLoQ` of LOWPASS_HANDOVER. */
#define OMX_LOWPASS_HANDOVER_HI_AT_LO_Q 0.9f
/** OMX_LOWPASS_HANDOVER_HI_AT_LO_Q as a double literal. */
#define OMX_LOWPASS_HANDOVER_HI_AT_LO_Q_DOUBLE 0.9
/** The `hiAtPeakQ` of LOWPASS_HANDOVER. */
#define OMX_LOWPASS_HANDOVER_HI_AT_PEAK_Q 0.8f
/** OMX_LOWPASS_HANDOVER_HI_AT_PEAK_Q as a double literal. */
#define OMX_LOWPASS_HANDOVER_HI_AT_PEAK_Q_DOUBLE 0.8
/** The `loAtHiQ` of LOWPASS_HANDOVER. */
#define OMX_LOWPASS_HANDOVER_LO_AT_HI_Q 0.6f
/** OMX_LOWPASS_HANDOVER_LO_AT_HI_Q as a double literal. */
#define OMX_LOWPASS_HANDOVER_LO_AT_HI_Q_DOUBLE 0.6
/** The `loAtLoQ` of LOWPASS_HANDOVER. */
#define OMX_LOWPASS_HANDOVER_LO_AT_LO_Q 0.8f
/** OMX_LOWPASS_HANDOVER_LO_AT_LO_Q as a double literal. */
#define OMX_LOWPASS_HANDOVER_LO_AT_LO_Q_DOUBLE 0.8
/** The `qHi` of LOWPASS_HANDOVER. */
#define OMX_LOWPASS_HANDOVER_Q_HI 0.7071067811865476f
/** OMX_LOWPASS_HANDOVER_Q_HI as a double literal. */
#define OMX_LOWPASS_HANDOVER_Q_HI_DOUBLE 0.7071067811865476
/** The `qLo` of LOWPASS_HANDOVER. */
#define OMX_LOWPASS_HANDOVER_Q_LO 0.541196100146197f
/** OMX_LOWPASS_HANDOVER_Q_LO as a double literal. */
#define OMX_LOWPASS_HANDOVER_Q_LO_DOUBLE 0.541196100146197
/** The `qPeak` of LOWPASS_HANDOVER. */
#define OMX_LOWPASS_HANDOVER_Q_PEAK 0.9f
/** OMX_LOWPASS_HANDOVER_Q_PEAK as a double literal. */
#define OMX_LOWPASS_HANDOVER_Q_PEAK_DOUBLE 0.9
/// @}
/**
 * @name LPF_FREQ_RANGE
 * Where the LPF cutoff can sit (high end).
 */
/// @{
/** The `max` of LPF_FREQ_RANGE. */
#define OMX_LPF_FREQ_RANGE_MAX 20000
/** The `min` of LPF_FREQ_RANGE. */
#define OMX_LPF_FREQ_RANGE_MIN 1000
/** The `step` of LPF_FREQ_RANGE. */
#define OMX_LPF_FREQ_RANGE_STEP 10
/// @}
/**
 * @name MBCOMP_ATTACK_MS_RANGE
 * The band detector's attack, ms. Below DETECTOR_OVERSAMPLE_AUTO_MS the whole stage takes the 4x control path, every band at one tap.
 */
/// @{
/** The `default` of MBCOMP_ATTACK_MS_RANGE. */
#define OMX_MBCOMP_ATTACK_MS_RANGE_DEFAULT 10
/** The `max` of MBCOMP_ATTACK_MS_RANGE. */
#define OMX_MBCOMP_ATTACK_MS_RANGE_MAX 200
/** The `min` of MBCOMP_ATTACK_MS_RANGE. */
#define OMX_MBCOMP_ATTACK_MS_RANGE_MIN 0.1f
/** OMX_MBCOMP_ATTACK_MS_RANGE_MIN as a double literal. */
#define OMX_MBCOMP_ATTACK_MS_RANGE_MIN_DOUBLE 0.1
/** The `step` of MBCOMP_ATTACK_MS_RANGE. */
#define OMX_MBCOMP_ATTACK_MS_RANGE_STEP 0.1f
/** OMX_MBCOMP_ATTACK_MS_RANGE_STEP as a double literal. */
#define OMX_MBCOMP_ATTACK_MS_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name MBCOMP_BANDS_RANGE
 * How many bands the stage splits into, MBC_MIN_BANDS to MBC_MAX_BANDS, one crossover fewer. Three, body, presence and air, comes up.
 */
/// @{
/** The `default` of MBCOMP_BANDS_RANGE. */
#define OMX_MBCOMP_BANDS_RANGE_DEFAULT 3
/** The `max` of MBCOMP_BANDS_RANGE. */
#define OMX_MBCOMP_BANDS_RANGE_MAX 5
/** The `min` of MBCOMP_BANDS_RANGE. */
#define OMX_MBCOMP_BANDS_RANGE_MIN 2
/** The `step` of MBCOMP_BANDS_RANGE. */
#define OMX_MBCOMP_BANDS_RANGE_STEP 1
/// @}
/**
 * @name MBCOMP_MAKEUP_DB_RANGE
 * The band's make-up, dB.
 */
/// @{
/** The `default` of MBCOMP_MAKEUP_DB_RANGE. */
#define OMX_MBCOMP_MAKEUP_DB_RANGE_DEFAULT 0
/** The `max` of MBCOMP_MAKEUP_DB_RANGE. */
#define OMX_MBCOMP_MAKEUP_DB_RANGE_MAX 24
/** The `min` of MBCOMP_MAKEUP_DB_RANGE. */
#define OMX_MBCOMP_MAKEUP_DB_RANGE_MIN 0
/** The `step` of MBCOMP_MAKEUP_DB_RANGE. */
#define OMX_MBCOMP_MAKEUP_DB_RANGE_STEP 0.5f
/** OMX_MBCOMP_MAKEUP_DB_RANGE_STEP as a double literal. */
#define OMX_MBCOMP_MAKEUP_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name MBCOMP_MIX_PCT_RANGE
 * The share of the compressed bands, percent: each band blends its own input, so the dry signal is the tree at rest and the blend is in phase. 100 is the compressed sum, 0 the tree at rest.
 */
/// @{
/** The `default` of MBCOMP_MIX_PCT_RANGE. */
#define OMX_MBCOMP_MIX_PCT_RANGE_DEFAULT 100
/** The `max` of MBCOMP_MIX_PCT_RANGE. */
#define OMX_MBCOMP_MIX_PCT_RANGE_MAX 100
/** The `min` of MBCOMP_MIX_PCT_RANGE. */
#define OMX_MBCOMP_MIX_PCT_RANGE_MIN 0
/** The `step` of MBCOMP_MIX_PCT_RANGE. */
#define OMX_MBCOMP_MIX_PCT_RANGE_STEP 1
/// @}
/**
 * @name MBCOMP_OUTPUT_DB_RANGE
 * The stage's output gain, dB, after the sum; 0 dB touches no sample.
 */
/// @{
/** The `default` of MBCOMP_OUTPUT_DB_RANGE. */
#define OMX_MBCOMP_OUTPUT_DB_RANGE_DEFAULT 0
/** The `max` of MBCOMP_OUTPUT_DB_RANGE. */
#define OMX_MBCOMP_OUTPUT_DB_RANGE_MAX 24
/** The `min` of MBCOMP_OUTPUT_DB_RANGE. */
#define OMX_MBCOMP_OUTPUT_DB_RANGE_MIN -24
/** The `step` of MBCOMP_OUTPUT_DB_RANGE. */
#define OMX_MBCOMP_OUTPUT_DB_RANGE_STEP 0.5f
/** OMX_MBCOMP_OUTPUT_DB_RANGE_STEP as a double literal. */
#define OMX_MBCOMP_OUTPUT_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name MBCOMP_RATIO_RANGE
 * The band's ratio; a hard knee.
 */
/// @{
/** The `default` of MBCOMP_RATIO_RANGE. */
#define OMX_MBCOMP_RATIO_RANGE_DEFAULT 2
/** The `max` of MBCOMP_RATIO_RANGE. */
#define OMX_MBCOMP_RATIO_RANGE_MAX 20
/** The `min` of MBCOMP_RATIO_RANGE. */
#define OMX_MBCOMP_RATIO_RANGE_MIN 1
/** The `step` of MBCOMP_RATIO_RANGE. */
#define OMX_MBCOMP_RATIO_RANGE_STEP 0.1f
/** OMX_MBCOMP_RATIO_RANGE_STEP as a double literal. */
#define OMX_MBCOMP_RATIO_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name MBCOMP_RELEASE_MS_RANGE
 * The band detector's release, ms.
 */
/// @{
/** The `default` of MBCOMP_RELEASE_MS_RANGE. */
#define OMX_MBCOMP_RELEASE_MS_RANGE_DEFAULT 150
/** The `max` of MBCOMP_RELEASE_MS_RANGE. */
#define OMX_MBCOMP_RELEASE_MS_RANGE_MAX 2000
/** The `min` of MBCOMP_RELEASE_MS_RANGE. */
#define OMX_MBCOMP_RELEASE_MS_RANGE_MIN 10
/** The `step` of MBCOMP_RELEASE_MS_RANGE. */
#define OMX_MBCOMP_RELEASE_MS_RANGE_STEP 1
/// @}
/**
 * @name MBCOMP_THRESHOLD_DB_RANGE
 * The band's threshold, dB of the band's own RMS level.
 */
/// @{
/** The `default` of MBCOMP_THRESHOLD_DB_RANGE. */
#define OMX_MBCOMP_THRESHOLD_DB_RANGE_DEFAULT -20
/** The `max` of MBCOMP_THRESHOLD_DB_RANGE. */
#define OMX_MBCOMP_THRESHOLD_DB_RANGE_MAX 0
/** The `min` of MBCOMP_THRESHOLD_DB_RANGE. */
#define OMX_MBCOMP_THRESHOLD_DB_RANGE_MIN -60
/** The `step` of MBCOMP_THRESHOLD_DB_RANGE. */
#define OMX_MBCOMP_THRESHOLD_DB_RANGE_STEP 0.5f
/** OMX_MBCOMP_THRESHOLD_DB_RANGE_STEP as a double literal. */
#define OMX_MBCOMP_THRESHOLD_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name MBCOMP_XOVER_HZ_RANGE
 * Crossover n's corner, Hz, ascending: the first bands - 1 are in use. A corner at or below the one before it is moved just above it, and a non-finite corner is the come-up corner of the band count (MBC_CROSSOVER_DEFAULTS_\<bands\>), which is also where a fresh stage starts. A corner at or above MBC_CORNER_MAX_FRACTION of the rate is designed at that line.
 */
/// @{
/** The `max` of MBCOMP_XOVER_HZ_RANGE. */
#define OMX_MBCOMP_XOVER_HZ_RANGE_MAX 16000
/** The `min` of MBCOMP_XOVER_HZ_RANGE. */
#define OMX_MBCOMP_XOVER_HZ_RANGE_MIN 40
/** The `step` of MBCOMP_XOVER_HZ_RANGE. */
#define OMX_MBCOMP_XOVER_HZ_RANGE_STEP 1
/// @}
/** The highest crossover corner the multiband design builds, as a fraction of the live rate: a corner at or above `fraction x rate` is designed AT that line, because the LR4 section degenerates toward Nyquist. omx_mbcomp_design computes the line in double, so a C reader takes OMX_MBC_CORNER_MAX_FRACTION_DOUBLE. A 20 kHz corner is under the line from 48 kHz up; at 44.1 kHz the line is 19845 Hz and a 20 kHz corner is designed there. (none) */
#define OMX_MBC_CORNER_MAX_FRACTION 0.45f
/** OMX_MBC_CORNER_MAX_FRACTION as a double literal. */
#define OMX_MBC_CORNER_MAX_FRACTION_DOUBLE 0.45
/** The most bands the multiband compressor splits into, and the capacity of its inline state: `struct omx_mbcomp_state` holds this many band compressors and one crossover fewer per leg, so the ceiling and the arrays are one fact. (none) */
#define OMX_MBC_MAX_BANDS 5
/** The crossovers of a full tree, one fewer than MBC_MAX_BANDS: the count of the corner control and the capacity of each leg's crossover state. (none) */
#define OMX_MBC_MAX_XOVERS 4
/** The fewest bands the multiband compressor splits into: one crossover. omx_mbcomp_design and omx_mbcomp_process refuse a band count below it (`band-count-declared`), and a refused count reads as this floor. (none) */
#define OMX_MBC_MIN_BANDS 2
/**
 * @name METER_VIEW
 * A meter's view law, the same on every surface: the fill is linear in dB from `floorDb` (dBFS, an empty display) to 0 dBFS (a full one); a stepped display (an LED ladder) holds its count until the level moves `stepHysteresis` of a step past the count's own rounding edge; the peak marker falls `fallDbPerSecond` dB for each second elapsed at the view's draw tick, whatever the tick's rate, silence included. A host reads a plugin's held meter at `hostReadHz`; the console streams meter values at `streamHz`. dBFS, a fraction of a step, dB per second and Hz. (since 3.0.0)
 */
/// @{
/** How fast the peak marker falls, dB per second of elapsed time. */
#define OMX_METER_VIEW_FALL_DB_PER_SECOND 18
/** dBFS at or below which a meter reads empty. */
#define OMX_METER_VIEW_FLOOR_DB -60
/** The rate a host reads a plugin's held meter at, Hz. */
#define OMX_METER_VIEW_HOST_READ_HZ 50
/** The fraction of one step a level must pass a stepped count's rounding edge by before the count changes. */
#define OMX_METER_VIEW_STEP_HYSTERESIS 0.25f
/** OMX_METER_VIEW_STEP_HYSTERESIS as a double literal. */
#define OMX_METER_VIEW_STEP_HYSTERESIS_DOUBLE 0.25
/** The rate the console streams meter values at, Hz. */
#define OMX_METER_VIEW_STREAM_HZ 20
/// @}
/** The share of one audio period a 64 x 32 mix matrix may take on one core: dense, every coefficient ramping, at the top rate, where a period is shortest. The console's whole chain warns at 60 % of `quantum / rate` (the RT admission budget), and the matrix is one stage of it, so it gets a third. It measures 6.5 % at 192 kHz on a desktop core and 1.6 % at 48 kHz, so a kernel that gets three times slower does not fit. A shared CI runner measured 27 %: the budget is the console's core, checked on the desk. (none) */
#define OMX_MIXMATRIX_PERIOD_SHARE 0.2f
/** OMX_MIXMATRIX_PERIOD_SHARE as a double literal. */
#define OMX_MIXMATRIX_PERIOD_SHARE_DOUBLE 0.2
/** The band slots kept for the operator's own hand: the reserve the feedback suppressor and the ring correction may never eat into. Nine is the working set a live EQ actually holds. The budget OPERATOR_EQ_BANDS_RESERVE + FBS_DEFAULT_MAX_AUTO_BANDS + HRP_DEFAULT_MAX_AUTO_BANDS (9 + 6 + 8 = 23) must fit EQ_MAX_BANDS, and `omx-contract validate` refuses data where it does not. (none) */
#define OMX_OPERATOR_EQ_BANDS_RESERVE 9
/**
 * @name OSCILLATOR_FREQ_RANGE
 * The oscillator's frequency travel: the audible band, 20 Hz to 20 kHz, stepped by 10 Hz on a nudge. omx-dsp's omx_gen_osc takes the frequency in Hz.
 */
/// @{
/** The `max` of OSCILLATOR_FREQ_RANGE. */
#define OMX_OSCILLATOR_FREQ_RANGE_MAX 20000
/** The `min` of OSCILLATOR_FREQ_RANGE. */
#define OMX_OSCILLATOR_FREQ_RANGE_MIN 20
/** The `step` of OSCILLATOR_FREQ_RANGE. */
#define OMX_OSCILLATOR_FREQ_RANGE_STEP 10
/// @}
/**
 * @name OUTPUT_DELAY_RANGE
 * The per-destination output delay travel, the whole-sample delay a route leg or a bus output is aligned with (omx-dsp's omx_idelay.h). max is the delay ring's ceiling: 1000 ms is the live-console norm and a distance ceiling, about 343 m of throw. A surface that offered more than the ring holds would draw travel the DSP clamps away.
 */
/// @{
/** The `max` of OUTPUT_DELAY_RANGE. */
#define OMX_OUTPUT_DELAY_RANGE_MAX 1000
/** The `min` of OUTPUT_DELAY_RANGE. */
#define OMX_OUTPUT_DELAY_RANGE_MIN 0
/** The `step` of OUTPUT_DELAY_RANGE. */
#define OMX_OUTPUT_DELAY_RANGE_STEP 1
/// @}
/**
 * @name PAN_RANGE
 * The PAN travel — −1 (hard left) … 0 (center) … +1 (hard right). A property of the pan law itself, not of any desk, so it is declared once. Continuous: a pan position has no quantum, and panGains clamps to these bounds.
 */
/// @{
/** The `max` of PAN_RANGE. */
#define OMX_PAN_RANGE_MAX 1
/** The `min` of PAN_RANGE. */
#define OMX_PAN_RANGE_MIN -1
/** The `step` of PAN_RANGE. */
#define OMX_PAN_RANGE_STEP 0
/// @}
/**
 * @name PHASER_BASE_RANGE
 * The phaser's lowest centre frequency; the sweep runs upward from it.
 */
/// @{
/** The `default` of PHASER_BASE_RANGE. */
#define OMX_PHASER_BASE_RANGE_DEFAULT 200
/** The `max` of PHASER_BASE_RANGE. */
#define OMX_PHASER_BASE_RANGE_MAX 2000
/** The `min` of PHASER_BASE_RANGE. */
#define OMX_PHASER_BASE_RANGE_MIN 50
/** The `step` of PHASER_BASE_RANGE. */
#define OMX_PHASER_BASE_RANGE_STEP 1
/// @}
/**
 * @name PHASER_DEPTH_RANGE
 * The sweep's span above `base`, in octaves; 0 is a static phaser at `base`.
 */
/// @{
/** The `default` of PHASER_DEPTH_RANGE. */
#define OMX_PHASER_DEPTH_RANGE_DEFAULT 4
/** The `max` of PHASER_DEPTH_RANGE. */
#define OMX_PHASER_DEPTH_RANGE_MAX 6
/** The `min` of PHASER_DEPTH_RANGE. */
#define OMX_PHASER_DEPTH_RANGE_MIN 0
/** The `step` of PHASER_DEPTH_RANGE. */
#define OMX_PHASER_DEPTH_RANGE_STEP 0.1f
/** OMX_PHASER_DEPTH_RANGE_STEP as a double literal. */
#define OMX_PHASER_DEPTH_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name PHASER_FEEDBACK_RANGE
 * The phaser's SIGNED resonance. ±0.9, tighter than the flanger's ±0.95: the loop runs through a time-varying chain. Declared here (the refusal half); the kernel's clamp is the enforcement half.
 */
/// @{
/** The `default` of PHASER_FEEDBACK_RANGE. */
#define OMX_PHASER_FEEDBACK_RANGE_DEFAULT 0.4f
/** OMX_PHASER_FEEDBACK_RANGE_DEFAULT as a double literal. */
#define OMX_PHASER_FEEDBACK_RANGE_DEFAULT_DOUBLE 0.4
/** The `max` of PHASER_FEEDBACK_RANGE. */
#define OMX_PHASER_FEEDBACK_RANGE_MAX 0.9f
/** OMX_PHASER_FEEDBACK_RANGE_MAX as a double literal. */
#define OMX_PHASER_FEEDBACK_RANGE_MAX_DOUBLE 0.9
/** The `min` of PHASER_FEEDBACK_RANGE. */
#define OMX_PHASER_FEEDBACK_RANGE_MIN -0.9f
/** OMX_PHASER_FEEDBACK_RANGE_MIN as a double literal. */
#define OMX_PHASER_FEEDBACK_RANGE_MIN_DOUBLE -0.9
/** The `step` of PHASER_FEEDBACK_RANGE. */
#define OMX_PHASER_FEEDBACK_RANGE_STEP 0.01f
/** OMX_PHASER_FEEDBACK_RANGE_STEP as a double literal. */
#define OMX_PHASER_FEEDBACK_RANGE_STEP_DOUBLE 0.01
/// @}
/** The highest frequency the phaser's all-pass sweep reaches, Hz. (Hz) */
#define OMX_PHASER_F_TOP_HZ 16000
/**
 * @name PHASER_MIX_RANGE
 * The phaser's wet/dry, as an instance of DRIVE_MIX_RANGE: 50 %, where the notch is complete.
 */
/// @{
/** The `default` of PHASER_MIX_RANGE. */
#define OMX_PHASER_MIX_RANGE_DEFAULT 50
/** The `max` of PHASER_MIX_RANGE. */
#define OMX_PHASER_MIX_RANGE_MAX 100
/** The `min` of PHASER_MIX_RANGE. */
#define OMX_PHASER_MIX_RANGE_MIN 0
/** The `step` of PHASER_MIX_RANGE. */
#define OMX_PHASER_MIX_RANGE_STEP 0.1f
/** OMX_PHASER_MIX_RANGE_STEP as a double literal. */
#define OMX_PHASER_MIX_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name PHASER_RATE_RANGE
 * The phaser's sweep speed. `omx_lfo_inc` freezes at Nyquist; this travel stays far below.
 */
/// @{
/** The `default` of PHASER_RATE_RANGE. */
#define OMX_PHASER_RATE_RANGE_DEFAULT 0.5f
/** OMX_PHASER_RATE_RANGE_DEFAULT as a double literal. */
#define OMX_PHASER_RATE_RANGE_DEFAULT_DOUBLE 0.5
/** The `max` of PHASER_RATE_RANGE. */
#define OMX_PHASER_RATE_RANGE_MAX 5
/** The `min` of PHASER_RATE_RANGE. */
#define OMX_PHASER_RATE_RANGE_MIN 0.05f
/** OMX_PHASER_RATE_RANGE_MIN as a double literal. */
#define OMX_PHASER_RATE_RANGE_MIN_DOUBLE 0.05
/** The `step` of PHASER_RATE_RANGE. */
#define OMX_PHASER_RATE_RANGE_STEP 0.01f
/** OMX_PHASER_RATE_RANGE_STEP as a double literal. */
#define OMX_PHASER_RATE_RANGE_STEP_DOUBLE 0.01
/// @}
/**
 * @name PHASER_STAGES_RANGE
 * The phaser's section count per leg — EVEN, 2 … 12 (two sections per notch). Its ends are the kernel's state capacity (`OMX_PHASER_MAX_STAGES` reads `OMX_PHASER_STAGES_RANGE_MAX`); the row moves it through the closed member set PHASER_STAGE_COUNTS, which is derived from it.
 */
/// @{
/** The `default` of PHASER_STAGES_RANGE. */
#define OMX_PHASER_STAGES_RANGE_DEFAULT 6
/** The `max` of PHASER_STAGES_RANGE. */
#define OMX_PHASER_STAGES_RANGE_MAX 12
/** The `min` of PHASER_STAGES_RANGE. */
#define OMX_PHASER_STAGES_RANGE_MIN 2
/** The `step` of PHASER_STAGES_RANGE. */
#define OMX_PHASER_STAGES_RANGE_STEP 2
/// @}
/**
 * @name PITCH_KERNEL
 * The pitch kernel's constants that are not a control's travel. `windowMs` is the crossfade window in ms, declared rather than a control and whole in samples at every rate; `prefilterCeilingHz` is the pre-filter's highest corner, the corner being min(ceiling, sample rate / (2 x ratio), nyquistFraction x sample rate / 2); `prefilterNyquistFraction` keeps that corner off Nyquist at a rate whose Nyquist is at or below the ceiling (32 kHz), and at 0.91 it is above the ceiling's own share of Nyquist at every declared rate (20 kHz is 0.907 of 22.05 kHz), so it moves no corner there; `prefilterQ` is its Q, a second-order Butterworth.
 */
/// @{
/** The `prefilterCeilingHz` of PITCH_KERNEL. */
#define OMX_PITCH_PREFILTER_CEILING_HZ 20000
/** The `prefilterNyquistFraction` of PITCH_KERNEL. */
#define OMX_PITCH_PREFILTER_NYQUIST_FRACTION 0.91f
/** OMX_PITCH_PREFILTER_NYQUIST_FRACTION as a double literal. */
#define OMX_PITCH_PREFILTER_NYQUIST_FRACTION_DOUBLE 0.91
/** The `prefilterQ` of PITCH_KERNEL. */
#define OMX_PITCH_PREFILTER_Q 0.7071067811865476f
/** OMX_PITCH_PREFILTER_Q as a double literal. */
#define OMX_PITCH_PREFILTER_Q_DOUBLE 0.7071067811865476
/** The `windowMs` of PITCH_KERNEL. */
#define OMX_PITCH_WINDOW_MS 40
/// @}
/** The consonant band's top edge is held under this share of the rate, the de-esser's rule: the band is redesigned from its held edges, so the clamp is never silent. (none) */
#define OMX_PLOSIVE_BAND_EDGE_FRACTION 0.45f
/** OMX_PLOSIVE_BAND_EDGE_FRACTION as a double literal. */
#define OMX_PLOSIVE_BAND_EDGE_FRACTION_DOUBLE 0.45
/**
 * @name PLOSIVE_BAND_HZ_RANGE
 * The consonant band's centre, Hz: the de-esser's cookbook bandpass section.
 */
/// @{
/** The `default` of PLOSIVE_BAND_HZ_RANGE. */
#define OMX_PLOSIVE_BAND_HZ_RANGE_DEFAULT 4000
/** The `max` of PLOSIVE_BAND_HZ_RANGE. */
#define OMX_PLOSIVE_BAND_HZ_RANGE_MAX 12000
/** The `min` of PLOSIVE_BAND_HZ_RANGE. */
#define OMX_PLOSIVE_BAND_HZ_RANGE_MIN 1000
/** The `step` of PLOSIVE_BAND_HZ_RANGE. */
#define OMX_PLOSIVE_BAND_HZ_RANGE_STEP 10
/// @}
/**
 * @name PLOSIVE_BAND_WIDTH_OCT_RANGE
 * The consonant band's width, octaves; its top edge is held under PLOSIVE_BAND_EDGE_FRACTION of the rate.
 */
/// @{
/** The `default` of PLOSIVE_BAND_WIDTH_OCT_RANGE. */
#define OMX_PLOSIVE_BAND_WIDTH_OCT_RANGE_DEFAULT 2
/** The `max` of PLOSIVE_BAND_WIDTH_OCT_RANGE. */
#define OMX_PLOSIVE_BAND_WIDTH_OCT_RANGE_MAX 4
/** The `min` of PLOSIVE_BAND_WIDTH_OCT_RANGE. */
#define OMX_PLOSIVE_BAND_WIDTH_OCT_RANGE_MIN 0.5f
/** OMX_PLOSIVE_BAND_WIDTH_OCT_RANGE_MIN as a double literal. */
#define OMX_PLOSIVE_BAND_WIDTH_OCT_RANGE_MIN_DOUBLE 0.5
/** The `step` of PLOSIVE_BAND_WIDTH_OCT_RANGE. */
#define OMX_PLOSIVE_BAND_WIDTH_OCT_RANGE_STEP 0.05f
/** OMX_PLOSIVE_BAND_WIDTH_OCT_RANGE_STEP as a double literal. */
#define OMX_PLOSIVE_BAND_WIDTH_OCT_RANGE_STEP_DOUBLE 0.05
/// @}
/**
 * @name PLOSIVE_CONSONANT_DB_RANGE
 * How far a hard consonant (t, k) is cut in its band, dB, at full contrast: y = x1 + (g - 1) B, the de-esser's split, so no gain is added at any frequency. 0 cuts nothing, bit for bit.
 */
/// @{
/** The `default` of PLOSIVE_CONSONANT_DB_RANGE. */
#define OMX_PLOSIVE_CONSONANT_DB_RANGE_DEFAULT 6
/** The `max` of PLOSIVE_CONSONANT_DB_RANGE. */
#define OMX_PLOSIVE_CONSONANT_DB_RANGE_MAX 24
/** The `min` of PLOSIVE_CONSONANT_DB_RANGE. */
#define OMX_PLOSIVE_CONSONANT_DB_RANGE_MIN 0
/** The `step` of PLOSIVE_CONSONANT_DB_RANGE. */
#define OMX_PLOSIVE_CONSONANT_DB_RANGE_STEP 0.1f
/** OMX_PLOSIVE_CONSONANT_DB_RANGE_STEP as a double literal. */
#define OMX_PLOSIVE_CONSONANT_DB_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name PLOSIVE_POP_DB_RANGE
 * How far a p pop is cut below popHz, dB, at full contrast: x1 = x - (1 - g) LP, the low half of a first-order power-complementary split, so no gain is added. 0 cuts nothing, bit for bit.
 */
/// @{
/** The `default` of PLOSIVE_POP_DB_RANGE. */
#define OMX_PLOSIVE_POP_DB_RANGE_DEFAULT 12
/** The `max` of PLOSIVE_POP_DB_RANGE. */
#define OMX_PLOSIVE_POP_DB_RANGE_MAX 24
/** The `min` of PLOSIVE_POP_DB_RANGE. */
#define OMX_PLOSIVE_POP_DB_RANGE_MIN 0
/** The `step` of PLOSIVE_POP_DB_RANGE. */
#define OMX_PLOSIVE_POP_DB_RANGE_STEP 0.1f
/** OMX_PLOSIVE_POP_DB_RANGE_STEP as a double literal. */
#define OMX_PLOSIVE_POP_DB_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name PLOSIVE_POP_HZ_RANGE
 * The pop split's corner, Hz: the first-order allpass (omx_allpass1) whose halves are the pop band and the rest.
 */
/// @{
/** The `default` of PLOSIVE_POP_HZ_RANGE. */
#define OMX_PLOSIVE_POP_HZ_RANGE_DEFAULT 150
/** The `max` of PLOSIVE_POP_HZ_RANGE. */
#define OMX_PLOSIVE_POP_HZ_RANGE_MAX 300
/** The `min` of PLOSIVE_POP_HZ_RANGE. */
#define OMX_PLOSIVE_POP_HZ_RANGE_MIN 60
/** The `step` of PLOSIVE_POP_HZ_RANGE. */
#define OMX_PLOSIVE_POP_HZ_RANGE_STEP 1
/// @}
/**
 * @name PLOSIVE_POP_RECOVERY_MS_RANGE
 * The pop detector's slow attack, ms.
 */
/// @{
/** The `default` of PLOSIVE_POP_RECOVERY_MS_RANGE. */
#define OMX_PLOSIVE_POP_RECOVERY_MS_RANGE_DEFAULT 40
/** The `max` of PLOSIVE_POP_RECOVERY_MS_RANGE. */
#define OMX_PLOSIVE_POP_RECOVERY_MS_RANGE_MAX 200
/** The `min` of PLOSIVE_POP_RECOVERY_MS_RANGE. */
#define OMX_PLOSIVE_POP_RECOVERY_MS_RANGE_MIN 10
/** The `step` of PLOSIVE_POP_RECOVERY_MS_RANGE. */
#define OMX_PLOSIVE_POP_RECOVERY_MS_RANGE_STEP 1
/// @}
/**
 * @name PLOSIVE_POP_SENSITIVITY_DB_RANGE
 * The contrast, dB, the pop band's burst must pass before the pop cut starts.
 */
/// @{
/** The `default` of PLOSIVE_POP_SENSITIVITY_DB_RANGE. */
#define OMX_PLOSIVE_POP_SENSITIVITY_DB_RANGE_DEFAULT 16
/** The `max` of PLOSIVE_POP_SENSITIVITY_DB_RANGE. */
#define OMX_PLOSIVE_POP_SENSITIVITY_DB_RANGE_MAX 30
/** The `min` of PLOSIVE_POP_SENSITIVITY_DB_RANGE. */
#define OMX_PLOSIVE_POP_SENSITIVITY_DB_RANGE_MIN 0
/** The `step` of PLOSIVE_POP_SENSITIVITY_DB_RANGE. */
#define OMX_PLOSIVE_POP_SENSITIVITY_DB_RANGE_STEP 0.5f
/** OMX_PLOSIVE_POP_SENSITIVITY_DB_RANGE_STEP as a double literal. */
#define OMX_PLOSIVE_POP_SENSITIVITY_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name PLOSIVE_RECOVERY_MS_RANGE
 * The consonant detector's slow attack, ms: how long a burst's contrast lasts, so how long the cut does.
 */
/// @{
/** The `default` of PLOSIVE_RECOVERY_MS_RANGE. */
#define OMX_PLOSIVE_RECOVERY_MS_RANGE_DEFAULT 8
/** The `max` of PLOSIVE_RECOVERY_MS_RANGE. */
#define OMX_PLOSIVE_RECOVERY_MS_RANGE_MAX 50
/** The `min` of PLOSIVE_RECOVERY_MS_RANGE. */
#define OMX_PLOSIVE_RECOVERY_MS_RANGE_MIN 2
/** The `step` of PLOSIVE_RECOVERY_MS_RANGE. */
#define OMX_PLOSIVE_RECOVERY_MS_RANGE_STEP 0.1f
/** OMX_PLOSIVE_RECOVERY_MS_RANGE_STEP as a double literal. */
#define OMX_PLOSIVE_RECOVERY_MS_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name PLOSIVE_SENSITIVITY_DB_RANGE
 * The contrast, dB between the band's fast and slow envelopes (omx_envdiff), a burst must pass before the consonant cut starts; the cut is full TRANSIENT_REF_DB past it (omx_envdiff_ramp). A contrast, not a level: the same t is treated the same at any gain.
 */
/// @{
/** The `default` of PLOSIVE_SENSITIVITY_DB_RANGE. */
#define OMX_PLOSIVE_SENSITIVITY_DB_RANGE_DEFAULT 16
/** The `max` of PLOSIVE_SENSITIVITY_DB_RANGE. */
#define OMX_PLOSIVE_SENSITIVITY_DB_RANGE_MAX 30
/** The `min` of PLOSIVE_SENSITIVITY_DB_RANGE. */
#define OMX_PLOSIVE_SENSITIVITY_DB_RANGE_MIN 0
/** The `step` of PLOSIVE_SENSITIVITY_DB_RANGE. */
#define OMX_PLOSIVE_SENSITIVITY_DB_RANGE_STEP 0.5f
/** OMX_PLOSIVE_SENSITIVITY_DB_RANGE_STEP as a double literal. */
#define OMX_PLOSIVE_SENSITIVITY_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name PROGRAM_RELEASE
 * The program-dependent release's model constants — the LA-2A's opto cell as DATA. `chargeMs`/`dischargeMs` are the word's memory constants, shared by every profile; each profile's `fastMs`, `slowMinMs`, `slowMaxMs` and `share` fit the unit's published two-stage recovery (opto); `auto` spans a bus compressor's fixed release detents, 100 ms to 1.2 s, half fast and half slow. Not operator controls: rendered into `omxdsp/omxdsp_definitions.h` as `OMX_PROGRAM_RELEASE_*`, which libomxdsp's `omx_env_program_release` reads.
 */
/// @{
/** The `chargeMs` of PROGRAM_RELEASE. */
#define OMX_PROGRAM_RELEASE_CHARGE_MS 1000
/** The `dischargeMs` of PROGRAM_RELEASE. */
#define OMX_PROGRAM_RELEASE_DISCHARGE_MS 5000
/** The `profiles.auto.fastMs` of PROGRAM_RELEASE. */
#define OMX_PROGRAM_RELEASE_PROFILES_AUTO_FAST_MS 100
/** The `profiles.auto.share` of PROGRAM_RELEASE. */
#define OMX_PROGRAM_RELEASE_PROFILES_AUTO_SHARE 0.5f
/** OMX_PROGRAM_RELEASE_PROFILES_AUTO_SHARE as a double literal. */
#define OMX_PROGRAM_RELEASE_PROFILES_AUTO_SHARE_DOUBLE 0.5
/** The `profiles.auto.slowMaxMs` of PROGRAM_RELEASE. */
#define OMX_PROGRAM_RELEASE_PROFILES_AUTO_SLOW_MAX_MS 1200
/** The `profiles.auto.slowMinMs` of PROGRAM_RELEASE. */
#define OMX_PROGRAM_RELEASE_PROFILES_AUTO_SLOW_MIN_MS 300
/** The `profiles.opto.fastMs` of PROGRAM_RELEASE. */
#define OMX_PROGRAM_RELEASE_PROFILES_OPTO_FAST_MS 70
/** The `profiles.opto.share` of PROGRAM_RELEASE. */
#define OMX_PROGRAM_RELEASE_PROFILES_OPTO_SHARE 0.5f
/** OMX_PROGRAM_RELEASE_PROFILES_OPTO_SHARE as a double literal. */
#define OMX_PROGRAM_RELEASE_PROFILES_OPTO_SHARE_DOUBLE 0.5
/** The `profiles.opto.slowMaxMs` of PROGRAM_RELEASE. */
#define OMX_PROGRAM_RELEASE_PROFILES_OPTO_SLOW_MAX_MS 1100
/** The `profiles.opto.slowMinMs` of PROGRAM_RELEASE. */
#define OMX_PROGRAM_RELEASE_PROFILES_OPTO_SLOW_MIN_MS 110
/// @}
/** Minimum PSHR (`omx_fbs_pshr`) (dB) for a candidate frame to count as sub-harmonic-CLEAN. Evidence-set on the #116 corpus, margin on both sides: the four live false positives (musical overtones at 1 869.1 / 832.0 / 263.7 / 228.5 Hz) read PSHR between −10.2 and +8.8 dB across their windows — never a 5-frame sub-clean stretch, and the single closest approach (+8.8) is isolated between frames at ≈ −1 and −10 — while the committed #106 injected ramp reads ≥ +12.2 dB from its second frame on (its frame 0 sits at +9.4, costing exactly one frame of confirm latency, pinned in the corpus test). A static program line at f/2 of REAL feedback also clears this fast: the runaway gains several dB per hop on anything static, so the deferral is transient. (dB) */
#define OMX_PSHR_MIN_DB 10
/** How often a readout's value is published: 50 Hz, one value every 20 ms, the rate a host sends a moving meter at. Every readout reads it as its `rateHz` (docs/readouts.md). (Hz; since 2.4.0) */
#define OMX_READOUT_RATE_HZ 50
/**
 * @name REVERB_DAMPING_RANGE
 * The reverb's DAMPING — a NORMALIZED_RANGE instance with its own come-up value, matching `DEFAULT_REVERB_STATE`. Declared with SIZE and WIDTH (see REVERB_SIZE_RANGE for why each is a named constant rather than a default on the shared 0..1 shape).
 */
/// @{
/** The `default` of REVERB_DAMPING_RANGE. */
#define OMX_REVERB_DAMPING_RANGE_DEFAULT 0.5f
/** OMX_REVERB_DAMPING_RANGE_DEFAULT as a double literal. */
#define OMX_REVERB_DAMPING_RANGE_DEFAULT_DOUBLE 0.5
/** The `max` of REVERB_DAMPING_RANGE. */
#define OMX_REVERB_DAMPING_RANGE_MAX 1
/** The `min` of REVERB_DAMPING_RANGE. */
#define OMX_REVERB_DAMPING_RANGE_MIN 0
/** The `step` of REVERB_DAMPING_RANGE. */
#define OMX_REVERB_DAMPING_RANGE_STEP 0
/// @}
/**
 * @name REVERB_DECAY_LOOPS
 * The one loop each reverb configuration's late decay follows, the numbers the kernel runs (since 0.32.4): a Schroeder-Moorer network decays at its LONGEST loop's gain and length, so the loop gain is `gainFloor + gainSpan x size` (room's comb feedback, hall's, the plate tank's decay) and the loop is `loopSamples` at `tuningRate` (room's and hall's longest comb, the plate's longer tank half), scaled to the live rate as the lines are. The RT60 is `loopMs x ln(0.001) / ln(gain)`. The curve twin draws the decay envelope from these; the kernel reads its gains from the same place, so the picture and the tail cannot drift apart. (since 3.0.0)
 */
/// @{
/** The hall comb feedback at size 0. */
#define OMX_REVERB_DECAY_LOOPS_HALL_GAIN_FLOOR 0.9f
/** OMX_REVERB_DECAY_LOOPS_HALL_GAIN_FLOOR as a double literal. */
#define OMX_REVERB_DECAY_LOOPS_HALL_GAIN_FLOOR_DOUBLE 0.9
/** What size 1 adds to the hall comb feedback. */
#define OMX_REVERB_DECAY_LOOPS_HALL_GAIN_SPAN 0.09f
/** OMX_REVERB_DECAY_LOOPS_HALL_GAIN_SPAN as a double literal. */
#define OMX_REVERB_DECAY_LOOPS_HALL_GAIN_SPAN_DOUBLE 0.09
/** The hall's longest comb, samples at hallTuningRate. */
#define OMX_REVERB_DECAY_LOOPS_HALL_LOOP_SAMPLES 3557
/** The rate the hall's lines are tuned at, Hz. */
#define OMX_REVERB_DECAY_LOOPS_HALL_TUNING_RATE 44100
/** The plate tank's decay at size 0. */
#define OMX_REVERB_DECAY_LOOPS_PLATE_GAIN_FLOOR 0.5f
/** OMX_REVERB_DECAY_LOOPS_PLATE_GAIN_FLOOR as a double literal. */
#define OMX_REVERB_DECAY_LOOPS_PLATE_GAIN_FLOOR_DOUBLE 0.5
/** What size 1 adds to the plate tank's decay. */
#define OMX_REVERB_DECAY_LOOPS_PLATE_GAIN_SPAN 0.49f
/** OMX_REVERB_DECAY_LOOPS_PLATE_GAIN_SPAN as a double literal. */
#define OMX_REVERB_DECAY_LOOPS_PLATE_GAIN_SPAN_DOUBLE 0.49
/** The plate's longer tank half, the two lines a pass through it crosses, samples at plateTuningRate. */
#define OMX_REVERB_DECAY_LOOPS_PLATE_LOOP_SAMPLES 3564
/** The rate the plate's lines are tuned at, Hz. */
#define OMX_REVERB_DECAY_LOOPS_PLATE_TUNING_RATE 29761
/** The room comb feedback at size 0. */
#define OMX_REVERB_DECAY_LOOPS_ROOM_GAIN_FLOOR 0.28f
/** OMX_REVERB_DECAY_LOOPS_ROOM_GAIN_FLOOR as a double literal. */
#define OMX_REVERB_DECAY_LOOPS_ROOM_GAIN_FLOOR_DOUBLE 0.28
/** What size 1 adds to the room comb feedback. */
#define OMX_REVERB_DECAY_LOOPS_ROOM_GAIN_SPAN 0.7f
/** OMX_REVERB_DECAY_LOOPS_ROOM_GAIN_SPAN as a double literal. */
#define OMX_REVERB_DECAY_LOOPS_ROOM_GAIN_SPAN_DOUBLE 0.7
/** The room's longest comb, samples at roomTuningRate. */
#define OMX_REVERB_DECAY_LOOPS_ROOM_LOOP_SAMPLES 1617
/** The rate the room's lines are tuned at, Hz. */
#define OMX_REVERB_DECAY_LOOPS_ROOM_TUNING_RATE 44100
/// @}
/**
 * @name REVERB_DEESS_FREQ_HZ_RANGE
 * The wet de-esser's band, Hz: the de-esser's frequency travel by reference.
 */
/// @{
/** The `default` of REVERB_DEESS_FREQ_HZ_RANGE. */
#define OMX_REVERB_DEESS_FREQ_HZ_RANGE_DEFAULT 7000
/** The `max` of REVERB_DEESS_FREQ_HZ_RANGE. */
#define OMX_REVERB_DEESS_FREQ_HZ_RANGE_MAX 16000
/** The `min` of REVERB_DEESS_FREQ_HZ_RANGE. */
#define OMX_REVERB_DEESS_FREQ_HZ_RANGE_MIN 2000
/** The `step` of REVERB_DEESS_FREQ_HZ_RANGE. */
#define OMX_REVERB_DEESS_FREQ_HZ_RANGE_STEP 10
/// @}
/**
 * @name REVERB_DEESS_THRESHOLD_DB_RANGE
 * The wet de-esser's threshold, dB: the de-esser's threshold travel by reference.
 */
/// @{
/** The `default` of REVERB_DEESS_THRESHOLD_DB_RANGE. */
#define OMX_REVERB_DEESS_THRESHOLD_DB_RANGE_DEFAULT -30
/** The `max` of REVERB_DEESS_THRESHOLD_DB_RANGE. */
#define OMX_REVERB_DEESS_THRESHOLD_DB_RANGE_MAX 0
/** The `min` of REVERB_DEESS_THRESHOLD_DB_RANGE. */
#define OMX_REVERB_DEESS_THRESHOLD_DB_RANGE_MIN -60
/** The `step` of REVERB_DEESS_THRESHOLD_DB_RANGE. */
#define OMX_REVERB_DEESS_THRESHOLD_DB_RANGE_STEP 0.5f
/** OMX_REVERB_DEESS_THRESHOLD_DB_RANGE_STEP as a double literal. */
#define OMX_REVERB_DEESS_THRESHOLD_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name REVERB_DUCK_DB_RANGE
 * The wet's ducking depth, dB, the delay's duck (omx_duck): the tail and its reflections pull down while the dry is loud. 0 ducks nothing, the come-up value, bit for bit.
 */
/// @{
/** The `default` of REVERB_DUCK_DB_RANGE. */
#define OMX_REVERB_DUCK_DB_RANGE_DEFAULT 0
/** The `max` of REVERB_DUCK_DB_RANGE. */
#define OMX_REVERB_DUCK_DB_RANGE_MAX 0
/** The `min` of REVERB_DUCK_DB_RANGE. */
#define OMX_REVERB_DUCK_DB_RANGE_MIN -24
/** The `step` of REVERB_DUCK_DB_RANGE. */
#define OMX_REVERB_DUCK_DB_RANGE_STEP 0.5f
/** OMX_REVERB_DUCK_DB_RANGE_STEP as a double literal. */
#define OMX_REVERB_DUCK_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name REVERB_DUCK_RELEASE_MS_RANGE
 * How fast the wet comes back once the dry falls, ms; the attack is FX_DUCK_ATTACK_MS.
 */
/// @{
/** The `default` of REVERB_DUCK_RELEASE_MS_RANGE. */
#define OMX_REVERB_DUCK_RELEASE_MS_RANGE_DEFAULT 250
/** The `max` of REVERB_DUCK_RELEASE_MS_RANGE. */
#define OMX_REVERB_DUCK_RELEASE_MS_RANGE_MAX 2000
/** The `min` of REVERB_DUCK_RELEASE_MS_RANGE. */
#define OMX_REVERB_DUCK_RELEASE_MS_RANGE_MIN 20
/** The `step` of REVERB_DUCK_RELEASE_MS_RANGE. */
#define OMX_REVERB_DUCK_RELEASE_MS_RANGE_STEP 10
/// @}
/**
 * @name REVERB_DUCK_THRESHOLD_DB_RANGE
 * The dry level the ducking starts at, dBFS.
 */
/// @{
/** The `default` of REVERB_DUCK_THRESHOLD_DB_RANGE. */
#define OMX_REVERB_DUCK_THRESHOLD_DB_RANGE_DEFAULT -30
/** The `max` of REVERB_DUCK_THRESHOLD_DB_RANGE. */
#define OMX_REVERB_DUCK_THRESHOLD_DB_RANGE_MAX 0
/** The `min` of REVERB_DUCK_THRESHOLD_DB_RANGE. */
#define OMX_REVERB_DUCK_THRESHOLD_DB_RANGE_MIN -60
/** The `step` of REVERB_DUCK_THRESHOLD_DB_RANGE. */
#define OMX_REVERB_DUCK_THRESHOLD_DB_RANGE_STEP 1
/// @}
/**
 * @name REVERB_EARLY_DB_RANGE
 * The early reflections' level, dB: the declared tap set (REVERB_EARLY_TAP_MS, REVERB_EARLY_TAP_GAINS) read from the pre-delayed feed, even taps on the left, odd on the right, beside the tail. The travel's floor is off, no early reflections at all, and is the come-up value, so a fresh reverb is the one that shipped, bit for bit.
 */
/// @{
/** The `default` of REVERB_EARLY_DB_RANGE. */
#define OMX_REVERB_EARLY_DB_RANGE_DEFAULT -60
/** The `max` of REVERB_EARLY_DB_RANGE. */
#define OMX_REVERB_EARLY_DB_RANGE_MAX 0
/** The `min` of REVERB_EARLY_DB_RANGE. */
#define OMX_REVERB_EARLY_DB_RANGE_MIN -60
/** The `step` of REVERB_EARLY_DB_RANGE. */
#define OMX_REVERB_EARLY_DB_RANGE_STEP 0.5f
/** OMX_REVERB_EARLY_DB_RANGE_STEP as a double literal. */
#define OMX_REVERB_EARLY_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/** The scale the early taps' times take at earlySizePct 100, a share of REVERB_EARLY_TAP_MS; the early ring is sized for it. (none) */
#define OMX_REVERB_EARLY_SIZE_MAX 1.5f
/** OMX_REVERB_EARLY_SIZE_MAX as a double literal. */
#define OMX_REVERB_EARLY_SIZE_MAX_DOUBLE 1.5
/** The scale the early taps' times take at earlySizePct 0, a share of REVERB_EARLY_TAP_MS. (none) */
#define OMX_REVERB_EARLY_SIZE_MIN 0.5f
/** OMX_REVERB_EARLY_SIZE_MIN as a double literal. */
#define OMX_REVERB_EARLY_SIZE_MIN_DOUBLE 0.5
/**
 * @name REVERB_EARLY_SIZE_PCT_RANGE
 * The early reflections' room, percent: every tap's time scaled from REVERB_EARLY_SIZE_MIN of the table at 0 % to REVERB_EARLY_SIZE_MAX at 100 %, linearly.
 */
/// @{
/** The `default` of REVERB_EARLY_SIZE_PCT_RANGE. */
#define OMX_REVERB_EARLY_SIZE_PCT_RANGE_DEFAULT 50
/** The `max` of REVERB_EARLY_SIZE_PCT_RANGE. */
#define OMX_REVERB_EARLY_SIZE_PCT_RANGE_MAX 100
/** The `min` of REVERB_EARLY_SIZE_PCT_RANGE. */
#define OMX_REVERB_EARLY_SIZE_PCT_RANGE_MIN 0
/** The `step` of REVERB_EARLY_SIZE_PCT_RANGE. */
#define OMX_REVERB_EARLY_SIZE_PCT_RANGE_STEP 1
/// @}
/** The gated reverb's gate attack, ms: the time the gate takes to open on a hit. (ms) */
#define OMX_REVERB_GATE_ATTACK_MS 1.5f
/** OMX_REVERB_GATE_ATTACK_MS as a double literal. */
#define OMX_REVERB_GATE_ATTACK_MS_DOUBLE 1.5
/**
 * @name REVERB_GATE_THRESHOLD_RANGE
 * GATED's ARM THRESHOLD — the level the reverb's own mono feed must reach to arm the hold, in dBFS. Declared, because the alternative is a hidden onset detector: the kernel compares `|input|` against exactly this number and nothing else.
 */
/// @{
/** The `default` of REVERB_GATE_THRESHOLD_RANGE. */
#define OMX_REVERB_GATE_THRESHOLD_RANGE_DEFAULT -40
/** The `max` of REVERB_GATE_THRESHOLD_RANGE. */
#define OMX_REVERB_GATE_THRESHOLD_RANGE_MAX 0
/** The `min` of REVERB_GATE_THRESHOLD_RANGE. */
#define OMX_REVERB_GATE_THRESHOLD_RANGE_MIN -80
/** The `step` of REVERB_GATE_THRESHOLD_RANGE. */
#define OMX_REVERB_GATE_THRESHOLD_RANGE_STEP 1
/// @}
/**
 * @name REVERB_HIGHCUT_RANGE
 * The reverb's HIGH cut, as an instance of REVERB_CUT_RANGE — same bounds, its own come-up value. The top of the travel reads as "open" (`mix_reverb.h`), matching `DEFAULT_REVERB_STATE`'s seed.
 */
/// @{
/** The `default` of REVERB_HIGHCUT_RANGE. */
#define OMX_REVERB_HIGHCUT_RANGE_DEFAULT 20000
/** The `max` of REVERB_HIGHCUT_RANGE. */
#define OMX_REVERB_HIGHCUT_RANGE_MAX 20000
/** The `min` of REVERB_HIGHCUT_RANGE. */
#define OMX_REVERB_HIGHCUT_RANGE_MIN 0
/** The `step` of REVERB_HIGHCUT_RANGE. */
#define OMX_REVERB_HIGHCUT_RANGE_STEP 10
/// @}
/**
 * @name REVERB_HOLD_RANGE
 * GATED's HOLD — how long the wet stays at full level after the key falls away, in milliseconds. The Phil-Collins tail is cut where the operator puts the cut, so this is the control that decides the effect's length; `size` still decides what is being cut off.
 */
/// @{
/** The `default` of REVERB_HOLD_RANGE. */
#define OMX_REVERB_HOLD_RANGE_DEFAULT 120
/** The `max` of REVERB_HOLD_RANGE. */
#define OMX_REVERB_HOLD_RANGE_MAX 2000
/** The `min` of REVERB_HOLD_RANGE. */
#define OMX_REVERB_HOLD_RANGE_MIN 10
/** The `step` of REVERB_HOLD_RANGE. */
#define OMX_REVERB_HOLD_RANGE_STEP 10
/// @}
/**
 * @name REVERB_LOWCUT_RANGE
 * The reverb's LOW cut, as an instance of REVERB_CUT_RANGE — same bounds, its own come-up value. `0` reads as "none" (`mix_reverb.h`), which is also `DEFAULT_REVERB_STATE`'s seed, so a fresh reverb's low cut and its declared default are the same fact stated once. Split from REVERB_CUT_RANGE rather than adding `default` there: the low and high cut share ONE travel (the C imposes one constraint on both) but do NOT share one come-up value, and a `default` on the shared range could only be honest for one of the two fields that read it.
 */
/// @{
/** The `default` of REVERB_LOWCUT_RANGE. */
#define OMX_REVERB_LOWCUT_RANGE_DEFAULT 0
/** The `max` of REVERB_LOWCUT_RANGE. */
#define OMX_REVERB_LOWCUT_RANGE_MAX 20000
/** The `min` of REVERB_LOWCUT_RANGE. */
#define OMX_REVERB_LOWCUT_RANGE_MIN 0
/** The `step` of REVERB_LOWCUT_RANGE. */
#define OMX_REVERB_LOWCUT_RANGE_STEP 10
/// @}
/**
 * @name REVERB_MIX_RANGE
 * The REVERB's wet/dry. An insert keeps the dry signal (0.3); an `fxReturn` is wet-only (1), so the return carries only the effect. Both numbers live HERE and nowhere else: the row's `mix` limit, `ReverbController`'s seed and its fresh state read them through limitForKind.
 */
/// @{
/** The `byKind.fxReturn` of REVERB_MIX_RANGE. */
#define OMX_REVERB_MIX_RANGE_BY_KIND_FX_RETURN 1
/** The `default` of REVERB_MIX_RANGE. */
#define OMX_REVERB_MIX_RANGE_DEFAULT 0.3f
/** OMX_REVERB_MIX_RANGE_DEFAULT as a double literal. */
#define OMX_REVERB_MIX_RANGE_DEFAULT_DOUBLE 0.3
/** The `max` of REVERB_MIX_RANGE. */
#define OMX_REVERB_MIX_RANGE_MAX 1
/** The `min` of REVERB_MIX_RANGE. */
#define OMX_REVERB_MIX_RANGE_MIN 0
/** The `step` of REVERB_MIX_RANGE. */
#define OMX_REVERB_MIX_RANGE_STEP 0
/// @}
/**
 * @name REVERB_PLATE_MOD_DEPTH_RANGE
 * PLATE's tank MODULATION DEPTH, in percent of Dattorro's own excursion (±8 samples at his 29 761 Hz tuning rate, scaled to the live rate the way the tank lengths are). 100 is the depth the plate has always run at, so the come-up is bit-identical to the plate before the field; 0 brings back the unmodulated tank's ring; 400 is the wide, chorused EMT-style swing. The rate stays 1 Hz. Only PLATE reads it.
 */
/// @{
/** The `default` of REVERB_PLATE_MOD_DEPTH_RANGE. */
#define OMX_REVERB_PLATE_MOD_DEPTH_RANGE_DEFAULT 100
/** The `max` of REVERB_PLATE_MOD_DEPTH_RANGE. */
#define OMX_REVERB_PLATE_MOD_DEPTH_RANGE_MAX 400
/** The `min` of REVERB_PLATE_MOD_DEPTH_RANGE. */
#define OMX_REVERB_PLATE_MOD_DEPTH_RANGE_MIN 0
/** The `step` of REVERB_PLATE_MOD_DEPTH_RANGE. */
#define OMX_REVERB_PLATE_MOD_DEPTH_RANGE_STEP 1
/// @}
/**
 * @name REVERB_PREDELAY_RANGE
 * The reverb's PRE-DELAY travel — the gap before the first reflection, in milliseconds. Declared here so the unit crosses with the bounds: a `finiteNumber(0, 100)` codec publishes `unit: ''`, and a surface binding that draws a millisecond control it cannot label. The panel knowing better than the wire is the wrong way round.
 */
/// @{
/** The `default` of REVERB_PREDELAY_RANGE. */
#define OMX_REVERB_PREDELAY_RANGE_DEFAULT 0
/** The `max` of REVERB_PREDELAY_RANGE. */
#define OMX_REVERB_PREDELAY_RANGE_MAX 100
/** The `min` of REVERB_PREDELAY_RANGE. */
#define OMX_REVERB_PREDELAY_RANGE_MIN 0
/** The `step` of REVERB_PREDELAY_RANGE. */
#define OMX_REVERB_PREDELAY_RANGE_STEP 1
/// @}
/**
 * @name REVERB_RELEASE_RANGE
 * GATED's RELEASE — how long the held wet takes to reach zero, in milliseconds. A straight line down, short by design: a long release is a reverb tail again, which is the thing a gated reverb exists to remove.
 */
/// @{
/** The `default` of REVERB_RELEASE_RANGE. */
#define OMX_REVERB_RELEASE_RANGE_DEFAULT 20
/** The `max` of REVERB_RELEASE_RANGE. */
#define OMX_REVERB_RELEASE_RANGE_MAX 500
/** The `min` of REVERB_RELEASE_RANGE. */
#define OMX_REVERB_RELEASE_RANGE_MIN 1
/** The `step` of REVERB_RELEASE_RANGE. */
#define OMX_REVERB_RELEASE_RANGE_STEP 1
/// @}
/**
 * @name REVERB_REVERSE_RANGE
 * REVERSE's MIRROR WINDOW — how long a slice of the wet is played backward, in milliseconds. A DECLARED travel and never derived from `size`'s RT60: an RT60 of seconds cannot be buffered at 192 kHz, and a window silently clamped to fit is the silent-clamp defect. The ceiling is what `mix_reverb.h`'s pool budgets a ring for (`OMX_REVERB_REVERSE_MAX_MS`), so the console offers exactly the travel the kernel accepts — nothing wider, nothing narrower.
 */
/// @{
/** The `default` of REVERB_REVERSE_RANGE. */
#define OMX_REVERB_REVERSE_RANGE_DEFAULT 300
/** The `max` of REVERB_REVERSE_RANGE. */
#define OMX_REVERB_REVERSE_RANGE_MAX 500
/** The `min` of REVERB_REVERSE_RANGE. */
#define OMX_REVERB_REVERSE_RANGE_MIN 50
/** The `step` of REVERB_REVERSE_RANGE. */
#define OMX_REVERB_REVERSE_RANGE_STEP 10
/// @}
/**
 * @name REVERB_SIZE_RANGE
 * The reverb's SIZE/DAMPING/WIDTH — each a NORMALIZED_RANGE instance with its own come-up value, matching `DEFAULT_REVERB_STATE`. Three separate constants rather than one shared default on `NORMALIZED_RANGE`: that range is the bare 0..1 REPRESENTATION, reused by fields across the whole console whose come-up values disagree (a fader position is not a reverb's damping), so a default belongs on the NAMED field, never on the shape it borrows.
 */
/// @{
/** The `default` of REVERB_SIZE_RANGE. */
#define OMX_REVERB_SIZE_RANGE_DEFAULT 0.7f
/** OMX_REVERB_SIZE_RANGE_DEFAULT as a double literal. */
#define OMX_REVERB_SIZE_RANGE_DEFAULT_DOUBLE 0.7
/** The `max` of REVERB_SIZE_RANGE. */
#define OMX_REVERB_SIZE_RANGE_MAX 1
/** The `min` of REVERB_SIZE_RANGE. */
#define OMX_REVERB_SIZE_RANGE_MIN 0
/** The `step` of REVERB_SIZE_RANGE. */
#define OMX_REVERB_SIZE_RANGE_STEP 0
/// @}
/**
 * @name REVERB_TAIL_DB_RANGE
 * The tail's level, dB: the reverb network's output. 0 dB is the tail as it shipped, the come-up value; the travel's floor is off, the early reflections alone.
 */
/// @{
/** The `default` of REVERB_TAIL_DB_RANGE. */
#define OMX_REVERB_TAIL_DB_RANGE_DEFAULT 0
/** The `max` of REVERB_TAIL_DB_RANGE. */
#define OMX_REVERB_TAIL_DB_RANGE_MAX 0
/** The `min` of REVERB_TAIL_DB_RANGE. */
#define OMX_REVERB_TAIL_DB_RANGE_MIN -60
/** The `step` of REVERB_TAIL_DB_RANGE. */
#define OMX_REVERB_TAIL_DB_RANGE_STEP 0.5f
/** OMX_REVERB_TAIL_DB_RANGE_STEP as a double literal. */
#define OMX_REVERB_TAIL_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name REVERB_WIDTH_RANGE
 * The reverb's stereo WIDTH — a NORMALIZED_RANGE instance with its own come-up value, matching `DEFAULT_REVERB_STATE`. Declared with SIZE and DAMPING (see REVERB_SIZE_RANGE for why each is a named constant rather than a default on the shared 0..1 shape).
 */
/// @{
/** The `default` of REVERB_WIDTH_RANGE. */
#define OMX_REVERB_WIDTH_RANGE_DEFAULT 1
/** The `max` of REVERB_WIDTH_RANGE. */
#define OMX_REVERB_WIDTH_RANGE_MAX 1
/** The `min` of REVERB_WIDTH_RANGE. */
#define OMX_REVERB_WIDTH_RANGE_MIN 0
/** The `step` of REVERB_WIDTH_RANGE. */
#define OMX_REVERB_WIDTH_RANGE_STEP 0
/// @}
/**
 * @name ROTARY_ACCEL_RANGE
 * The scale on every speed-change time constant (`ROTARY_*_ACCEL_MS`/`_DECEL_MS`): 2 is twice as slow.
 */
/// @{
/** The `default` of ROTARY_ACCEL_RANGE. */
#define OMX_ROTARY_ACCEL_RANGE_DEFAULT 1
/** The `max` of ROTARY_ACCEL_RANGE. */
#define OMX_ROTARY_ACCEL_RANGE_MAX 4
/** The `min` of ROTARY_ACCEL_RANGE. */
#define OMX_ROTARY_ACCEL_RANGE_MIN 0.25f
/** OMX_ROTARY_ACCEL_RANGE_MIN as a double literal. */
#define OMX_ROTARY_ACCEL_RANGE_MIN_DOUBLE 0.25
/** The `step` of ROTARY_ACCEL_RANGE. */
#define OMX_ROTARY_ACCEL_RANGE_STEP 0.01f
/** OMX_ROTARY_ACCEL_RANGE_STEP as a double literal. */
#define OMX_ROTARY_ACCEL_RANGE_STEP_DOUBLE 0.01
/// @}
/**
 * @name ROTARY_BALANCE_RANGE
 * The drum/horn balance, percent: −100 drum only, +100 horn only; the law attenuates, never boosts.
 */
/// @{
/** The `default` of ROTARY_BALANCE_RANGE. */
#define OMX_ROTARY_BALANCE_RANGE_DEFAULT 0
/** The `max` of ROTARY_BALANCE_RANGE. */
#define OMX_ROTARY_BALANCE_RANGE_MAX 100
/** The `min` of ROTARY_BALANCE_RANGE. */
#define OMX_ROTARY_BALANCE_RANGE_MIN -100
/** The `step` of ROTARY_BALANCE_RANGE. */
#define OMX_ROTARY_BALANCE_RANGE_STEP 1
/// @}
/** The shortest Doppler read, ms — above the order-3 read's one-sample reach at every rate. (ms) */
#define OMX_ROTARY_BASE_MS 0.1f
/** OMX_ROTARY_BASE_MS as a double literal. */
#define OMX_ROTARY_BASE_MS_DOUBLE 0.1
/** The complementary split's low-pass corner, Hz: the Leslie 122's passive crossover. (Hz) */
#define OMX_ROTARY_CROSSOVER_HZ 800
/** The drum rotor's speed-up time constant, ms (at `accel` 1). (ms) */
#define OMX_ROTARY_DRUM_ACCEL_MS 1700
/** The drum baffle's amplitude swing `m_drum`, a ratio. (none) */
#define OMX_ROTARY_DRUM_AM 0.3f
/** OMX_ROTARY_DRUM_AM as a double literal. */
#define OMX_ROTARY_DRUM_AM_DOUBLE 0.3
/** The drum rotor's slow-down time constant, ms (at `accel` 1). (ms) */
#define OMX_ROTARY_DRUM_DECEL_MS 1800
/** The drum's Doppler excursion `D_drum`, ms: its baffle turns over a fixed woofer. (ms) */
#define OMX_ROTARY_DRUM_DOPPLER_MS 0.29f
/** OMX_ROTARY_DRUM_DOPPLER_MS as a double literal. */
#define OMX_ROTARY_DRUM_DOPPLER_MS_DOUBLE 0.29
/**
 * @name ROTARY_DRUM_FAST_RANGE
 * The drum's tremolo (fast) rate, Hz.
 */
/// @{
/** The `default` of ROTARY_DRUM_FAST_RANGE. */
#define OMX_ROTARY_DRUM_FAST_RANGE_DEFAULT 5.9f
/** OMX_ROTARY_DRUM_FAST_RANGE_DEFAULT as a double literal. */
#define OMX_ROTARY_DRUM_FAST_RANGE_DEFAULT_DOUBLE 5.9
/** The `max` of ROTARY_DRUM_FAST_RANGE. */
#define OMX_ROTARY_DRUM_FAST_RANGE_MAX 10
/** The `min` of ROTARY_DRUM_FAST_RANGE. */
#define OMX_ROTARY_DRUM_FAST_RANGE_MIN 3
/** The `step` of ROTARY_DRUM_FAST_RANGE. */
#define OMX_ROTARY_DRUM_FAST_RANGE_STEP 0.01f
/** OMX_ROTARY_DRUM_FAST_RANGE_STEP as a double literal. */
#define OMX_ROTARY_DRUM_FAST_RANGE_STEP_DOUBLE 0.01
/// @}
/**
 * @name ROTARY_DRUM_SLOW_RANGE
 * The drum's chorale (slow) rate, Hz.
 */
/// @{
/** The `default` of ROTARY_DRUM_SLOW_RANGE. */
#define OMX_ROTARY_DRUM_SLOW_RANGE_DEFAULT 0.7f
/** OMX_ROTARY_DRUM_SLOW_RANGE_DEFAULT as a double literal. */
#define OMX_ROTARY_DRUM_SLOW_RANGE_DEFAULT_DOUBLE 0.7
/** The `max` of ROTARY_DRUM_SLOW_RANGE. */
#define OMX_ROTARY_DRUM_SLOW_RANGE_MAX 2
/** The `min` of ROTARY_DRUM_SLOW_RANGE. */
#define OMX_ROTARY_DRUM_SLOW_RANGE_MIN 0.1f
/** OMX_ROTARY_DRUM_SLOW_RANGE_MIN as a double literal. */
#define OMX_ROTARY_DRUM_SLOW_RANGE_MIN_DOUBLE 0.1
/** The `step` of ROTARY_DRUM_SLOW_RANGE. */
#define OMX_ROTARY_DRUM_SLOW_RANGE_STEP 0.01f
/** OMX_ROTARY_DRUM_SLOW_RANGE_STEP as a double literal. */
#define OMX_ROTARY_DRUM_SLOW_RANGE_STEP_DOUBLE 0.01
/// @}
/** The horn rotor's speed-up time constant, ms (at `accel` 1). (ms) */
#define OMX_ROTARY_HORN_ACCEL_MS 330
/** The horn's amplitude swing `m_horn` (a −6 dB directivity dip), a ratio. (none) */
#define OMX_ROTARY_HORN_AM 0.5f
/** OMX_ROTARY_HORN_AM as a double literal. */
#define OMX_ROTARY_HORN_AM_DOUBLE 0.5
/** The horn rotor's slow-down time constant, ms (at `accel` 1). (ms) */
#define OMX_ROTARY_HORN_DECEL_MS 500
/** The horn's Doppler excursion `D_horn`, ms: a 0.15 m radius over 343 m/s. (ms) */
#define OMX_ROTARY_HORN_DOPPLER_MS 0.44f
/** OMX_ROTARY_HORN_DOPPLER_MS as a double literal. */
#define OMX_ROTARY_HORN_DOPPLER_MS_DOUBLE 0.44
/**
 * @name ROTARY_HORN_FAST_RANGE
 * The horn's tremolo (fast) rate, Hz.
 */
/// @{
/** The `default` of ROTARY_HORN_FAST_RANGE. */
#define OMX_ROTARY_HORN_FAST_RANGE_DEFAULT 6.8f
/** OMX_ROTARY_HORN_FAST_RANGE_DEFAULT as a double literal. */
#define OMX_ROTARY_HORN_FAST_RANGE_DEFAULT_DOUBLE 6.8
/** The `max` of ROTARY_HORN_FAST_RANGE. */
#define OMX_ROTARY_HORN_FAST_RANGE_MAX 10
/** The `min` of ROTARY_HORN_FAST_RANGE. */
#define OMX_ROTARY_HORN_FAST_RANGE_MIN 3
/** The `step` of ROTARY_HORN_FAST_RANGE. */
#define OMX_ROTARY_HORN_FAST_RANGE_STEP 0.01f
/** OMX_ROTARY_HORN_FAST_RANGE_STEP as a double literal. */
#define OMX_ROTARY_HORN_FAST_RANGE_STEP_DOUBLE 0.01
/// @}
/**
 * @name ROTARY_HORN_SLOW_RANGE
 * The horn's chorale (slow) rate, Hz.
 */
/// @{
/** The `default` of ROTARY_HORN_SLOW_RANGE. */
#define OMX_ROTARY_HORN_SLOW_RANGE_DEFAULT 0.8f
/** OMX_ROTARY_HORN_SLOW_RANGE_DEFAULT as a double literal. */
#define OMX_ROTARY_HORN_SLOW_RANGE_DEFAULT_DOUBLE 0.8
/** The `max` of ROTARY_HORN_SLOW_RANGE. */
#define OMX_ROTARY_HORN_SLOW_RANGE_MAX 2
/** The `min` of ROTARY_HORN_SLOW_RANGE. */
#define OMX_ROTARY_HORN_SLOW_RANGE_MIN 0.1f
/** OMX_ROTARY_HORN_SLOW_RANGE_MIN as a double literal. */
#define OMX_ROTARY_HORN_SLOW_RANGE_MIN_DOUBLE 0.1
/** The `step` of ROTARY_HORN_SLOW_RANGE. */
#define OMX_ROTARY_HORN_SLOW_RANGE_STEP 0.01f
/** OMX_ROTARY_HORN_SLOW_RANGE_STEP as a double literal. */
#define OMX_ROTARY_HORN_SLOW_RANGE_STEP_DOUBLE 0.01
/// @}
/** The sweep's longest Doppler read, ms: ROTARY_BASE_MS plus the horn's whole travel, 2·ROTARY_HORN_DOPPLER_MS (the larger rotor's). With ROTARY_BASE_MS it is the range every lag of the wet path lies in, the far end a descriptor's latency sweep refers to. (ms) */
#define OMX_ROTARY_LONGEST_MS 0.98f
/** OMX_ROTARY_LONGEST_MS as a double literal. */
#define OMX_ROTARY_LONGEST_MS_DOUBLE 0.98
/**
 * @name ROTARY_MIX_RANGE
 * The rotary's wet/dry, as an instance of DRIVE_MIX_RANGE: 100 %, because a rotary on an insert IS the sound (it replaces the cabinet) and an `fxReturn` wants it wet too.
 */
/// @{
/** The `default` of ROTARY_MIX_RANGE. */
#define OMX_ROTARY_MIX_RANGE_DEFAULT 100
/** The `max` of ROTARY_MIX_RANGE. */
#define OMX_ROTARY_MIX_RANGE_MAX 100
/** The `min` of ROTARY_MIX_RANGE. */
#define OMX_ROTARY_MIX_RANGE_MIN 0
/** The `step` of ROTARY_MIX_RANGE. */
#define OMX_ROTARY_MIX_RANGE_STEP 0.1f
/** OMX_ROTARY_MIX_RANGE_STEP as a double literal. */
#define OMX_ROTARY_MIX_RANGE_STEP_DOUBLE 0.1
/// @}
/** One inline ring per rotor per leg, floats — at least the 192 kHz need of the horn's sweep. (none) */
#define OMX_ROTARY_RING_FLOATS 256
/** Below this rate, Hz, a rotor coasting toward `stop` is snapped to exactly 0. (none) */
#define OMX_ROTARY_STOP_EPS 0.0001f
/** OMX_ROTARY_STOP_EPS as a double literal. */
#define OMX_ROTARY_STOP_EPS_DOUBLE 0.0001
/** The highest frequency any spectrum band covers, before the Nyquist clamp (omx_rta_band_f_max). (Hz) */
#define OMX_RTA_BAND_F_MAX_CAP 20000
/** The lowest frequency any spectrum band covers; below it is infrasonic and not drawn. (Hz) */
#define OMX_RTA_BAND_F_MIN 20
/** Quiet gate of the band reduction: when no band rises above this, every band reads as silence, so a rig's idle floor never draws as a flat block. Keyed on the loudest band, not per band. (dB) */
#define OMX_RTA_BAND_GATE_DB -60
/** The smallest difference between two taps of a spectrum, POST - PRE in dB, that draws a wedge: a pair of neighbouring bands is a wedge when either band's difference reaches it. The console's overlay held it as 0.004 of its 100 dB span. (dB) */
#define OMX_RTA_DELTA_DEADBAND_DB 0.4f
/** OMX_RTA_DELTA_DEADBAND_DB as a double literal. */
#define OMX_RTA_DELTA_DEADBAND_DB_DOUBLE 0.4
/** The largest RTA FFT the native ring holds — rendered into `omxdsp/omxdsp_definitions.h` as `OMX_RTA_FFT_SIZE_MAX`. (none) */
#define OMX_RTA_FFT_SIZE_MAX 32768
/** How often a watched spectrum is read and published: an operator-cadence poll, never the audio thread. (Hz) */
#define OMX_RTA_HZ 10
/** The ceiling of a spectrum display's level window: the top of the frame, where a band's peak reaching it draws a clip tick. (dB) */
#define OMX_RTA_LEVEL_DB_MAX 0
/** The floor of a spectrum display's level window: the bottom of the frame, every surface. (dB) */
#define OMX_RTA_LEVEL_DB_MIN -72
/** The spacing of a spectrum display's one dB graticule, from the ceiling down to the floor. (dB) */
#define OMX_RTA_LEVEL_GRID_DB 12
/** How far the low zoom's window must have advanced, as a fraction of that window, before it is recomputed; between recomputes the last zoom rides the fresh main spectrum unchanged. (none) */
#define OMX_RTA_LOW_ADVANCE 0.25f
/** OMX_RTA_LOW_ADVANCE as a double literal. */
#define OMX_RTA_LOW_ADVANCE_DOUBLE 0.25
/** Quiet gate of a tap's raw spectrum, before any banding: when no bin rises above this the whole tap reads as the floor. The console's overlay frame applies it; a banded reading is gated by RTA_BAND_GATE_DB instead. (dB) */
#define OMX_RTA_TAP_GATE_DB -60
/** The RTA's target bin resolution — rendered into `omxdsp/omxdsp_definitions.h` as `OMX_RTA_TARGET_BIN_HZ`, which `mix_dsp.h` reads; the engine's conformance test reads it back from the header. (Hz) */
#define OMX_RTA_TARGET_BIN_HZ 5.9f
/** OMX_RTA_TARGET_BIN_HZ as a double literal. */
#define OMX_RTA_TARGET_BIN_HZ_DOUBLE 5.9
/** A band centred below this samples the LOW (decimated) tier when the reading carries one; at or above it, the HIGH tier. Where the high tier's ~5.86 Hz bins still give ~51 bins per octave, above the finest panel's density. (Hz) */
#define OMX_RTA_TIER_CROSSOVER_HZ 300
/**
 * @name RT_HARD_TARGET
 * THE declaration: 192 kHz at quantum 128.
 */
/// @{
/** The `quantum` of RT_HARD_TARGET. */
#define OMX_RT_HARD_TARGET_QUANTUM 128
/** The `rate` of RT_HARD_TARGET. */
#define OMX_RT_HARD_TARGET_RATE 192000
/// @}
/**
 * @name SPRING_DAMP_HZ_RANGE
 * The loop's low-pass corner, Hz: the tail darkens as it falls.
 */
/// @{
/** The `default` of SPRING_DAMP_HZ_RANGE. */
#define OMX_SPRING_DAMP_HZ_RANGE_DEFAULT 4000
/** The `max` of SPRING_DAMP_HZ_RANGE. */
#define OMX_SPRING_DAMP_HZ_RANGE_MAX 10000
/** The `min` of SPRING_DAMP_HZ_RANGE. */
#define OMX_SPRING_DAMP_HZ_RANGE_MIN 1000
/** The `step` of SPRING_DAMP_HZ_RANGE. */
#define OMX_SPRING_DAMP_HZ_RANGE_STEP 100
/// @}
/**
 * @name SPRING_DECAY_S_RANGE
 * The time the tail takes to fall 60 dB, seconds.
 */
/// @{
/** The `default` of SPRING_DECAY_S_RANGE. */
#define OMX_SPRING_DECAY_S_RANGE_DEFAULT 2
/** The `max` of SPRING_DECAY_S_RANGE. */
#define OMX_SPRING_DECAY_S_RANGE_MAX 8
/** The `min` of SPRING_DECAY_S_RANGE. */
#define OMX_SPRING_DECAY_S_RANGE_MIN 0.5f
/** OMX_SPRING_DECAY_S_RANGE_MIN as a double literal. */
#define OMX_SPRING_DECAY_S_RANGE_MIN_DOUBLE 0.5
/** The `step` of SPRING_DECAY_S_RANGE. */
#define OMX_SPRING_DECAY_S_RANGE_STEP 0.1f
/** OMX_SPRING_DECAY_S_RANGE_STEP as a double literal. */
#define OMX_SPRING_DECAY_S_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name SPRING_LENGTH_MS_RANGE
 * The spring's delay, ms: the time of one trip along it, the first return.
 */
/// @{
/** The `default` of SPRING_LENGTH_MS_RANGE. */
#define OMX_SPRING_LENGTH_MS_RANGE_DEFAULT 35
/** The `max` of SPRING_LENGTH_MS_RANGE. */
#define OMX_SPRING_LENGTH_MS_RANGE_MAX 60
/** The `min` of SPRING_LENGTH_MS_RANGE. */
#define OMX_SPRING_LENGTH_MS_RANGE_MIN 20
/** The `step` of SPRING_LENGTH_MS_RANGE. */
#define OMX_SPRING_LENGTH_MS_RANGE_STEP 1
/// @}
/** The loop gain's ceiling: the tail always decays. (none) */
#define OMX_SPRING_LOOP_MAX 0.97f
/** OMX_SPRING_LOOP_MAX as a double literal. */
#define OMX_SPRING_LOOP_MAX_DOUBLE 0.97
/**
 * @name SPRING_MIX_PCT_RANGE
 * The wet against the dry, percent. It comes up at 0, the input bit for bit.
 */
/// @{
/** The `default` of SPRING_MIX_PCT_RANGE. */
#define OMX_SPRING_MIX_PCT_RANGE_DEFAULT 0
/** The `max` of SPRING_MIX_PCT_RANGE. */
#define OMX_SPRING_MIX_PCT_RANGE_MAX 100
/** The `min` of SPRING_MIX_PCT_RANGE. */
#define OMX_SPRING_MIX_PCT_RANGE_MIN 0
/** The `step` of SPRING_MIX_PCT_RANGE. */
#define OMX_SPRING_MIX_PCT_RANGE_STEP 1
/// @}
/** The right spring's length against the left's: two springs of different lengths, as a tank carries, so the legs decorrelate. (none) */
#define OMX_SPRING_RIGHT_RATIO 1.13f
/** OMX_SPRING_RIGHT_RATIO as a double literal. */
#define OMX_SPRING_RIGHT_RATIO_DOUBLE 1.13
/** The first-order all-pass sections in the loop: enough of them and the low notes come later, the sound of a spring. (none) */
#define OMX_SPRING_SECTIONS 40
/**
 * @name SPRING_TENSION_PCT_RANGE
 * How hard the spring is drawn, percent: it sets the dispersion's corner from SPRING_CHIRP_HZ, and with it the drip of the low notes arriving after the high ones.
 */
/// @{
/** The `default` of SPRING_TENSION_PCT_RANGE. */
#define OMX_SPRING_TENSION_PCT_RANGE_DEFAULT 50
/** The `max` of SPRING_TENSION_PCT_RANGE. */
#define OMX_SPRING_TENSION_PCT_RANGE_MAX 100
/** The `min` of SPRING_TENSION_PCT_RANGE. */
#define OMX_SPRING_TENSION_PCT_RANGE_MIN 0
/** The `step` of SPRING_TENSION_PCT_RANGE. */
#define OMX_SPRING_TENSION_PCT_RANGE_STEP 1
/// @}
/** The largest interpolation factor a plan may carry. 44.1 kHz -\> 192 kHz is 640/147, the widest pair the console's standard rates produce; the cap refuses an exotic pair before it allocates a table nobody budgeted for, rather than discovering it as a bad_alloc. (none) */
#define OMX_SRC_MAX_L 1024
/** How much of the shared Nyquist the passband keeps. The remaining 8 % is the transition band the window needs to reach its stopband: at 48 kHz that puts the corner at 22.08 kHz and the stopband in place by 24 kHz. (none) */
#define OMX_SRC_ROLLOFF 0.92f
/** OMX_SRC_ROLLOFF as a double literal. */
#define OMX_SRC_ROLLOFF_DOUBLE 0.92
/** Stopband attenuation the window is designed for, in dB. 96 dB is below the noise floor of any take this console can capture (float capture of a 24-bit converter), so the conversion is not what limits the file. (dB) */
#define OMX_SRC_STOPBAND_DB 96
/** Taps per phase. 64 puts the design's transition band comfortably inside what a Kaiser of this beta can hold at every standard rate pair, and costs 64 multiply-adds per output sample — about 12 M flops per track-minute at 48 kHz, which is nothing offline. (none) */
#define OMX_SRC_TAPS 64
/** How far (bins) around bin/2 and bin/3 PSHR (`omx_fbs_pshr`) scans for a sub-harmonic line. A true half/third partial lands mid-bin in the worst case (bin/2 and bin/3 are rarely integers) and spectral leakage smears it one bin further; ±2 covers both. Measured on the #116 live-music corpus: every false positive's sub-line sat inside this window. Deliberately NOT wider — at low bins the window is already a large fraction of an octave, and widening it would start reading unrelated program lines as "sub-harmonics" of real feedback. (none) */
#define OMX_SUBHARMONIC_RADIUS_BINS 2
/**
 * @name SUBOCT_DRY_RANGE
 * The input, percent. With both octaves at 0 and the dry at 100 the face is the input bit for bit, the divider still tracking.
 */
/// @{
/** The `default` of SUBOCT_DRY_RANGE. */
#define OMX_SUBOCT_DRY_RANGE_DEFAULT 100
/** The `max` of SUBOCT_DRY_RANGE. */
#define OMX_SUBOCT_DRY_RANGE_MAX 100
/** The `min` of SUBOCT_DRY_RANGE. */
#define OMX_SUBOCT_DRY_RANGE_MIN 0
/** The `step` of SUBOCT_DRY_RANGE. */
#define OMX_SUBOCT_DRY_RANGE_STEP 0.1f
/** OMX_SUBOCT_DRY_RANGE_STEP as a double literal. */
#define OMX_SUBOCT_DRY_RANGE_STEP_DOUBLE 0.1
/// @}
/** The tracked peak's attack, ms: within the first cycle of the highest tracked note. (ms) */
#define OMX_SUBOCT_ENV_ATTACK_MS 1
/** The tracked peak's release, ms: longer than the lowest tracked period, so the threshold does not sag between peaks. (ms) */
#define OMX_SUBOCT_ENV_RELEASE_MS 100
/** How long the octaves take to fade out under the gate, or in over it, ms. (ms) */
#define OMX_SUBOCT_GATE_RAMP_MS 5
/** The arming threshold as a share of the tracked peak: a toggle needs a dip under -1/4 of the peak, which a fundamental-dominant note makes once a cycle and noise under half of it never makes. (none) */
#define OMX_SUBOCT_HYSTERESIS 0.25f
/** OMX_SUBOCT_HYSTERESIS as a double literal. */
#define OMX_SUBOCT_HYSTERESIS_DOUBLE 0.25
/**
 * @name SUBOCT_OCT1_RANGE
 * The octave below, percent of the tracked band divided once (omx_divider) and low-passed at the range's post1.
 */
/// @{
/** The `default` of SUBOCT_OCT1_RANGE. */
#define OMX_SUBOCT_OCT1_RANGE_DEFAULT 100
/** The `max` of SUBOCT_OCT1_RANGE. */
#define OMX_SUBOCT_OCT1_RANGE_MAX 100
/** The `min` of SUBOCT_OCT1_RANGE. */
#define OMX_SUBOCT_OCT1_RANGE_MIN 0
/** The `step` of SUBOCT_OCT1_RANGE. */
#define OMX_SUBOCT_OCT1_RANGE_STEP 0.1f
/** OMX_SUBOCT_OCT1_RANGE_STEP as a double literal. */
#define OMX_SUBOCT_OCT1_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name SUBOCT_OCT2_RANGE
 * Two octaves below, percent: the first octave divided again and low-passed at the range's post2.
 */
/// @{
/** The `default` of SUBOCT_OCT2_RANGE. */
#define OMX_SUBOCT_OCT2_RANGE_DEFAULT 0
/** The `max` of SUBOCT_OCT2_RANGE. */
#define OMX_SUBOCT_OCT2_RANGE_MAX 100
/** The `min` of SUBOCT_OCT2_RANGE. */
#define OMX_SUBOCT_OCT2_RANGE_MIN 0
/** The `step` of SUBOCT_OCT2_RANGE. */
#define OMX_SUBOCT_OCT2_RANGE_STEP 0.1f
/** OMX_SUBOCT_OCT2_RANGE_STEP as a double literal. */
#define OMX_SUBOCT_OCT2_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name SUBOCT_TRACK_GATE_DB_RANGE
 * The tracked band's level, dBFS of its peak envelope, under which the octaves fade out over SUBOCT_GATE_RAMP_MS and trackedHz reads 0.
 */
/// @{
/** The `default` of SUBOCT_TRACK_GATE_DB_RANGE. */
#define OMX_SUBOCT_TRACK_GATE_DB_RANGE_DEFAULT -50
/** The `max` of SUBOCT_TRACK_GATE_DB_RANGE. */
#define OMX_SUBOCT_TRACK_GATE_DB_RANGE_MAX -20
/** The `min` of SUBOCT_TRACK_GATE_DB_RANGE. */
#define OMX_SUBOCT_TRACK_GATE_DB_RANGE_MIN -80
/** The `step` of SUBOCT_TRACK_GATE_DB_RANGE. */
#define OMX_SUBOCT_TRACK_GATE_DB_RANGE_STEP 0.5f
/** OMX_SUBOCT_TRACK_GATE_DB_RANGE_STEP as a double literal. */
#define OMX_SUBOCT_TRACK_GATE_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/** The loudness range's relative gate of EBU Tech 3342 v4.0 (November 2023), "Loudness Range: A measure to supplement EBU R 128 loudness normalisation": the short-term blocks it measures are those above the absolute gate and no more than 20 LU below their mean. LU is written dB here until omx-contract carries the unit (omx-contract#145). (dB; since 3.0.0) */
#define OMX_TECH3342_RANGE_GATE_LU -20
/** The high end of the loudness range of EBU Tech 3342 v4.0 (November 2023), "Loudness Range: A measure to supplement EBU R 128 loudness normalisation": the 95th percentile of the gated short-term loudness distribution, as a fraction. (none; since 3.0.0) */
#define OMX_TECH3342_RANGE_HIGH_PERCENTILE 0.95f
/** OMX_TECH3342_RANGE_HIGH_PERCENTILE as a double literal. */
#define OMX_TECH3342_RANGE_HIGH_PERCENTILE_DOUBLE 0.95
/** The low end of the loudness range of EBU Tech 3342 v4.0 (November 2023), "Loudness Range: A measure to supplement EBU R 128 loudness normalisation": the 10th percentile of the gated short-term loudness distribution, as a fraction. (none; since 3.0.0) */
#define OMX_TECH3342_RANGE_LOW_PERCENTILE 0.1f
/** OMX_TECH3342_RANGE_LOW_PERCENTILE as a double literal. */
#define OMX_TECH3342_RANGE_LOW_PERCENTILE_DOUBLE 0.1
/** The DJ filter's high end: the high-pass's corner at +100, and the low-pass's as the word leaves 0; held at 0.999 of Nyquist where the rate is lower. (Hz) */
#define OMX_TONE_FILTER_HIGH_HZ 20000
/** The DJ filter's low end: the low-pass's corner at -100, and the high-pass's as the word leaves 0. (Hz) */
#define OMX_TONE_FILTER_LOW_HZ 20
/**
 * @name TONE_FILTER_RANGE
 * The DJ filter: below 0 a low-pass whose corner falls from TONE_FILTER_HIGH_HZ toward TONE_FILTER_LOW_HZ at -100, above 0 a high-pass whose corner rises from TONE_FILTER_LOW_HZ toward TONE_FILTER_HIGH_HZ at +100, on a log map; the EQ's own low-pass and high-pass sections. 0 is open, the come-up value, bit for bit.
 */
/// @{
/** The `default` of TONE_FILTER_RANGE. */
#define OMX_TONE_FILTER_RANGE_DEFAULT 0
/** The `max` of TONE_FILTER_RANGE. */
#define OMX_TONE_FILTER_RANGE_MAX 100
/** The `min` of TONE_FILTER_RANGE. */
#define OMX_TONE_FILTER_RANGE_MIN -100
/** The `step` of TONE_FILTER_RANGE. */
#define OMX_TONE_FILTER_RANGE_STEP 1
/// @}
/**
 * @name TONE_PIVOT_HZ_RANGE
 * The tilt's pivot, Hz: the frequency the tilt leaves at 0 dB, at every rate (the section is prewarped there).
 */
/// @{
/** The `default` of TONE_PIVOT_HZ_RANGE. */
#define OMX_TONE_PIVOT_HZ_RANGE_DEFAULT 1000
/** The `max` of TONE_PIVOT_HZ_RANGE. */
#define OMX_TONE_PIVOT_HZ_RANGE_MAX 5000
/** The `min` of TONE_PIVOT_HZ_RANGE. */
#define OMX_TONE_PIVOT_HZ_RANGE_MIN 200
/** The `step` of TONE_PIVOT_HZ_RANGE. */
#define OMX_TONE_PIVOT_HZ_RANGE_STEP 10
/// @}
/**
 * @name TONE_RESONANCE_RANGE
 * The DJ filter's resonance, its section's Q: the magnitude at the corner. It comes up at 0.71, the Butterworth's 1/sqrt(2) on the step.
 */
/// @{
/** The `default` of TONE_RESONANCE_RANGE. */
#define OMX_TONE_RESONANCE_RANGE_DEFAULT 0.71f
/** OMX_TONE_RESONANCE_RANGE_DEFAULT as a double literal. */
#define OMX_TONE_RESONANCE_RANGE_DEFAULT_DOUBLE 0.71
/** The `max` of TONE_RESONANCE_RANGE. */
#define OMX_TONE_RESONANCE_RANGE_MAX 4
/** The `min` of TONE_RESONANCE_RANGE. */
#define OMX_TONE_RESONANCE_RANGE_MIN 0.5f
/** OMX_TONE_RESONANCE_RANGE_MIN as a double literal. */
#define OMX_TONE_RESONANCE_RANGE_MIN_DOUBLE 0.5
/** The `step` of TONE_RESONANCE_RANGE. */
#define OMX_TONE_RESONANCE_RANGE_STEP 0.01f
/** OMX_TONE_RESONANCE_RANGE_STEP as a double literal. */
#define OMX_TONE_RESONANCE_RANGE_STEP_DOUBLE 0.01
/// @}
/**
 * @name TONE_TILT_DB_RANGE
 * The tilt's span, dB: one first-order complementary shelf pair about pivotHz (omx_tilt, a = 10^(tiltDb/40)), -tiltDb/2 at DC, +tiltDb/2 at Nyquist and 0 dB at the pivot. Positive brightens, negative darkens; +6 is a +-3 dB pair, about 2 dB per octave at the pivot. 0 is flat, the come-up value, bit for bit. A wider reach is a wider travel here, never a second section.
 */
/// @{
/** The `default` of TONE_TILT_DB_RANGE. */
#define OMX_TONE_TILT_DB_RANGE_DEFAULT 0
/** The `max` of TONE_TILT_DB_RANGE. */
#define OMX_TONE_TILT_DB_RANGE_MAX 6
/** The `min` of TONE_TILT_DB_RANGE. */
#define OMX_TONE_TILT_DB_RANGE_MIN -6
/** The `step` of TONE_TILT_DB_RANGE. */
#define OMX_TONE_TILT_DB_RANGE_STEP 0.5f
/** OMX_TONE_TILT_DB_RANGE_STEP as a double literal. */
#define OMX_TONE_TILT_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/** The fast follower's attack, ms: exactly the dynamics detector's no-oversampling line. (ms) */
#define OMX_TRANSIENT_FAST_ATTACK_MS 0.5f
/** OMX_TRANSIENT_FAST_ATTACK_MS as a double literal. */
#define OMX_TRANSIENT_FAST_ATTACK_MS_DOUBLE 0.5
/** The fast follower's release, ms: one 50 Hz cycle held, a 180 bpm sixteenth forgotten. (ms) */
#define OMX_TRANSIENT_FAST_RELEASE_MS 20
/** The detector floor, linear (−100 dBFS): below it both envelopes read the floor and the stage is unity. (none) */
#define OMX_TRANSIENT_FLOOR_LIN 0.00001f
/** OMX_TRANSIENT_FLOOR_LIN as a double literal. */
#define OMX_TRANSIENT_FLOOR_LIN_DOUBLE 0.00001
/** The contrast, dB, at which a knob reaches its full value. (dB) */
#define OMX_TRANSIENT_REF_DB 6
/**
 * @name TREMOLO_DEPTH_RANGE
 * The tremolo's modulation depth: the gain's trough is `1 − mix·depth`.
 */
/// @{
/** The `default` of TREMOLO_DEPTH_RANGE. */
#define OMX_TREMOLO_DEPTH_RANGE_DEFAULT 50
/** The `max` of TREMOLO_DEPTH_RANGE. */
#define OMX_TREMOLO_DEPTH_RANGE_MAX 100
/** The `min` of TREMOLO_DEPTH_RANGE. */
#define OMX_TREMOLO_DEPTH_RANGE_MIN 0
/** The `step` of TREMOLO_DEPTH_RANGE. */
#define OMX_TREMOLO_DEPTH_RANGE_STEP 0.1f
/** OMX_TREMOLO_DEPTH_RANGE_STEP as a double literal. */
#define OMX_TREMOLO_DEPTH_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name TREMOLO_MIX_RANGE
 * The tremolo's wet/dry, as an instance of DRIVE_MIX_RANGE: 100 %. The kernel folds the wet/dry into its one gain, so a strip tremolo IS the sound (as the rotary's mix 100 % is).
 */
/// @{
/** The `default` of TREMOLO_MIX_RANGE. */
#define OMX_TREMOLO_MIX_RANGE_DEFAULT 100
/** The `max` of TREMOLO_MIX_RANGE. */
#define OMX_TREMOLO_MIX_RANGE_MAX 100
/** The `min` of TREMOLO_MIX_RANGE. */
#define OMX_TREMOLO_MIX_RANGE_MIN 0
/** The `step` of TREMOLO_MIX_RANGE. */
#define OMX_TREMOLO_MIX_RANGE_STEP 0.1f
/** OMX_TREMOLO_MIX_RANGE_STEP as a double literal. */
#define OMX_TREMOLO_MIX_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name TREMOLO_RATE_RANGE
 * The tremolo's LFO rate (the catalogue row's 0.1..20 Hz); 4 Hz is the classic amp tremolo.
 */
/// @{
/** The `default` of TREMOLO_RATE_RANGE. */
#define OMX_TREMOLO_RATE_RANGE_DEFAULT 4
/** The `max` of TREMOLO_RATE_RANGE. */
#define OMX_TREMOLO_RATE_RANGE_MAX 20
/** The `min` of TREMOLO_RATE_RANGE. */
#define OMX_TREMOLO_RATE_RANGE_MIN 0.1f
/** OMX_TREMOLO_RATE_RANGE_MIN as a double literal. */
#define OMX_TREMOLO_RATE_RANGE_MIN_DOUBLE 0.1
/** The `step` of TREMOLO_RATE_RANGE. */
#define OMX_TREMOLO_RATE_RANGE_STEP 0.01f
/** OMX_TREMOLO_RATE_RANGE_STEP as a double literal. */
#define OMX_TREMOLO_RATE_RANGE_STEP_DOUBLE 0.01
/// @}
/**
 * @name TRIM_RANGE
 * The digital input trim, in dB: the console's fine head gain, applied first in a channel's lane as a click-free ramp from the last block's gain to this one (the console's trim range, -24..+24 dB in 0.1 dB steps, spelled minDb/maxDb/stepDb there). It comes up at 0 dB, the trim a channel has when none is set. omx-dsp's trim face takes it as `trim_db`.
 */
/// @{
/** The `default` of TRIM_RANGE. */
#define OMX_TRIM_RANGE_DEFAULT 0
/** The `max` of TRIM_RANGE. */
#define OMX_TRIM_RANGE_MAX 24
/** The `min` of TRIM_RANGE. */
#define OMX_TRIM_RANGE_MIN -24
/** The `step` of TRIM_RANGE. */
#define OMX_TRIM_RANGE_STEP 0.1f
/** OMX_TRIM_RANGE_STEP as a double literal. */
#define OMX_TRIM_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name UPWARD_ATTACK_MS_RANGE
 * How fast the detector follows a rise, ms; a rise takes the lift away.
 */
/// @{
/** The `default` of UPWARD_ATTACK_MS_RANGE. */
#define OMX_UPWARD_ATTACK_MS_RANGE_DEFAULT 5
/** The `max` of UPWARD_ATTACK_MS_RANGE. */
#define OMX_UPWARD_ATTACK_MS_RANGE_MAX 100
/** The `min` of UPWARD_ATTACK_MS_RANGE. */
#define OMX_UPWARD_ATTACK_MS_RANGE_MIN 0.1f
/** OMX_UPWARD_ATTACK_MS_RANGE_MIN as a double literal. */
#define OMX_UPWARD_ATTACK_MS_RANGE_MIN_DOUBLE 0.1
/** The `step` of UPWARD_ATTACK_MS_RANGE. */
#define OMX_UPWARD_ATTACK_MS_RANGE_STEP 0.1f
/** OMX_UPWARD_ATTACK_MS_RANGE_STEP as a double literal. */
#define OMX_UPWARD_ATTACK_MS_RANGE_STEP_DOUBLE 0.1
/// @}
/** The face's knee, dB, across the threshold: the lift eases in rather than starting on a corner. (dB) */
#define OMX_UPWARD_KNEE_DB 6
/**
 * @name UPWARD_RANGE_DB_RANGE
 * The largest lift, dB: a signal far under the threshold, the room's noise included, rises by this and no more.
 */
/// @{
/** The `default` of UPWARD_RANGE_DB_RANGE. */
#define OMX_UPWARD_RANGE_DB_RANGE_DEFAULT 6
/** The `max` of UPWARD_RANGE_DB_RANGE. */
#define OMX_UPWARD_RANGE_DB_RANGE_MAX 24
/** The `min` of UPWARD_RANGE_DB_RANGE. */
#define OMX_UPWARD_RANGE_DB_RANGE_MIN 0
/** The `step` of UPWARD_RANGE_DB_RANGE. */
#define OMX_UPWARD_RANGE_DB_RANGE_STEP 0.5f
/** OMX_UPWARD_RANGE_DB_RANGE_STEP as a double literal. */
#define OMX_UPWARD_RANGE_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name UPWARD_RATIO_RANGE
 * The upward ratio: the lift is (1 - 1/ratio) of the distance under the threshold. 1 lifts nothing and is the input, bit for bit, the come-up value.
 */
/// @{
/** The `default` of UPWARD_RATIO_RANGE. */
#define OMX_UPWARD_RATIO_RANGE_DEFAULT 1
/** The `max` of UPWARD_RATIO_RANGE. */
#define OMX_UPWARD_RATIO_RANGE_MAX 10
/** The `min` of UPWARD_RATIO_RANGE. */
#define OMX_UPWARD_RATIO_RANGE_MIN 1
/** The `step` of UPWARD_RATIO_RANGE. */
#define OMX_UPWARD_RATIO_RANGE_STEP 0.1f
/** OMX_UPWARD_RATIO_RANGE_STEP as a double literal. */
#define OMX_UPWARD_RATIO_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name UPWARD_RELEASE_MS_RANGE
 * How fast the detector follows a fall, ms; a fall brings the lift in.
 */
/// @{
/** The `default` of UPWARD_RELEASE_MS_RANGE. */
#define OMX_UPWARD_RELEASE_MS_RANGE_DEFAULT 200
/** The `max` of UPWARD_RELEASE_MS_RANGE. */
#define OMX_UPWARD_RELEASE_MS_RANGE_MAX 3000
/** The `min` of UPWARD_RELEASE_MS_RANGE. */
#define OMX_UPWARD_RELEASE_MS_RANGE_MIN 5
/** The `step` of UPWARD_RELEASE_MS_RANGE. */
#define OMX_UPWARD_RELEASE_MS_RANGE_STEP 5
/// @}
/**
 * @name UPWARD_THRESHOLD_DB_RANGE
 * The level, dBFS, under which the face starts to lift.
 */
/// @{
/** The `default` of UPWARD_THRESHOLD_DB_RANGE. */
#define OMX_UPWARD_THRESHOLD_DB_RANGE_DEFAULT -40
/** The `max` of UPWARD_THRESHOLD_DB_RANGE. */
#define OMX_UPWARD_THRESHOLD_DB_RANGE_MAX 0
/** The `min` of UPWARD_THRESHOLD_DB_RANGE. */
#define OMX_UPWARD_THRESHOLD_DB_RANGE_MIN -70
/** The `step` of UPWARD_THRESHOLD_DB_RANGE. */
#define OMX_UPWARD_THRESHOLD_DB_RANGE_STEP 0.5f
/** OMX_UPWARD_THRESHOLD_DB_RANGE_STEP as a double literal. */
#define OMX_UPWARD_THRESHOLD_DB_RANGE_STEP_DOUBLE 0.5
/// @}
/**
 * @name WIDTH_BASS_MONO_HZ_RANGE
 * Where the low end goes mono, Hz: the crossover's corner.
 */
/// @{
/** The `default` of WIDTH_BASS_MONO_HZ_RANGE. */
#define OMX_WIDTH_BASS_MONO_HZ_RANGE_DEFAULT 120
/** The `max` of WIDTH_BASS_MONO_HZ_RANGE. */
#define OMX_WIDTH_BASS_MONO_HZ_RANGE_MAX 300
/** The `min` of WIDTH_BASS_MONO_HZ_RANGE. */
#define OMX_WIDTH_BASS_MONO_HZ_RANGE_MIN 20
/** The `step` of WIDTH_BASS_MONO_HZ_RANGE. */
#define OMX_WIDTH_BASS_MONO_HZ_RANGE_STEP 1
/// @}
/** The complementary comb's gain g: its legs ripple between 1 - g and 1 + g, 0.5 a +3.5 / -6 dB comb, the mono sum unmoved. (none) */
#define OMX_WIDTH_COMB_GAIN 0.5f
/** OMX_WIDTH_COMB_GAIN as a double literal. */
#define OMX_WIDTH_COMB_GAIN_DOUBLE 0.5
/** The longest pseudo-stereo delay, ms: pseudoMs' top, the delay omx_width_delay holds to, the ring sized for it at 192 kHz. (ms) */
#define OMX_WIDTH_MAX_DELAY_MS 30
/**
 * @name WIDTH_PSEUDO_MS_RANGE
 * The pseudo-stereo's delay d, ms, rounded to the nearest sample.
 */
/// @{
/** The `default` of WIDTH_PSEUDO_MS_RANGE. */
#define OMX_WIDTH_PSEUDO_MS_RANGE_DEFAULT 12
/** The `max` of WIDTH_PSEUDO_MS_RANGE. */
#define OMX_WIDTH_PSEUDO_MS_RANGE_MAX 30
/** The `min` of WIDTH_PSEUDO_MS_RANGE. */
#define OMX_WIDTH_PSEUDO_MS_RANGE_MIN 0
/** The `step` of WIDTH_PSEUDO_MS_RANGE. */
#define OMX_WIDTH_PSEUDO_MS_RANGE_STEP 0.1f
/** OMX_WIDTH_PSEUDO_MS_RANGE_STEP as a double literal. */
#define OMX_WIDTH_PSEUDO_MS_RANGE_STEP_DOUBLE 0.1
/// @}
/**
 * @name WIDTH_WIDTH_PCT_RANGE
 * The stereo width, percent: the side (L - R)/2 times widthPct/100 over the mid (L + R)/2. 0 is mono, 100 the input bit for bit (the come-up value), 200 the side doubled.
 */
/// @{
/** The `default` of WIDTH_WIDTH_PCT_RANGE. */
#define OMX_WIDTH_WIDTH_PCT_RANGE_DEFAULT 100
/** The `max` of WIDTH_WIDTH_PCT_RANGE. */
#define OMX_WIDTH_WIDTH_PCT_RANGE_MAX 200
/** The `min` of WIDTH_WIDTH_PCT_RANGE. */
#define OMX_WIDTH_WIDTH_PCT_RANGE_MIN 0
/** The `step` of WIDTH_WIDTH_PCT_RANGE. */
#define OMX_WIDTH_WIDTH_PCT_RANGE_STEP 1
/// @}
/**
 * @name XOVER_LIMITS
 * The Linkwitz-Riley crossover primitives' numbers. `lr2SectionQ` is the Q of the one cookbook section an LR2 crossover runs per band (a squared first-order Butterworth, critically damped); `lr4SectionQ` is the Q of each of the two sections an LR4 crossover runs (a squared second-order Butterworth); `partitionTolerance` is the fraction of max(1, block peak) within which the low and high outputs must sum to the all-pass output. Measured worst case 9e-8. Design constants and tolerances, not controls.
 */
/// @{
/** The `lr2SectionQ` of XOVER_LIMITS. */
#define OMX_XOVER_LR2_SECTION_Q 0.5f
/** OMX_XOVER_LR2_SECTION_Q as a double literal. */
#define OMX_XOVER_LR2_SECTION_Q_DOUBLE 0.5
/** The `lr4SectionQ` of XOVER_LIMITS. */
#define OMX_XOVER_LR4_SECTION_Q 0.7071067811865476f
/** OMX_XOVER_LR4_SECTION_Q as a double literal. */
#define OMX_XOVER_LR4_SECTION_Q_DOUBLE 0.7071067811865476
/** The `partitionTolerance` of XOVER_LIMITS. */
#define OMX_XOVER_PARTITION_TOLERANCE 0.00001f
/** OMX_XOVER_PARTITION_TOLERANCE as a double literal. */
#define OMX_XOVER_PARTITION_TOLERANCE_DOUBLE 0.00001
/// @}

/* Every list rendered to C, sorted by list: OMX_<LIST>_COUNT and OMX_<LIST>_INIT { a, b, … }. */
/** The number of values of BALANCER_BAND_HZ, the length of OMX_BALANCER_BAND_HZ_INIT. */
#define OMX_BALANCER_BAND_HZ_COUNT 4u
/** The centres of the bands measured and moved, Hz, two octaves apart. The values in declaration order, an initialiser. (Hz) */
#define OMX_BALANCER_BAND_HZ_INIT { 125, 500, 2000, 8000 }
/** The number of values of DELAY_DIVISION_BEATS, the length of OMX_DELAY_DIVISION_BEATS_INIT. */
#define OMX_DELAY_DIVISION_BEATS_COUNT 18u
/** The beats each division of the delay lasts, in the order of its ids: whole, half, quarter, eighth, sixteenth, thirty-second, each straight, dotted (x 3/2) and triplet (x 2/3). A quarter note is one beat. The values in declaration order, an initialiser. (none) */
#define OMX_DELAY_DIVISION_BEATS_INIT { 4, 6, 2.6666666666666665f, 2, 3, 1.3333333333333333f, 1, 1.5f, 0.6666666666666666f, 0.5f, 0.75f, 0.3333333333333333f, 0.25f, 0.375f, 0.16666666666666666f, 0.125f, 0.1875f, 0.08333333333333333f }
/** The number of values of DELAY_TAP_FACTOR_DENS, the length of OMX_DELAY_TAP_FACTOR_DENS_INIT. */
#define OMX_DELAY_TAP_FACTOR_DENS_COUNT 11u
/** The denominators of the FX delay's tap factor set, in member order, one per numerator of DELAY_TAP_FACTOR_NUMS: the factor at position i is DELAY_TAP_FACTOR_NUMS[i] / DELAY_TAP_FACTOR_DENS[i]. The values in declaration order, an initialiser. (none) */
#define OMX_DELAY_TAP_FACTOR_DENS_INIT { 4, 3, 8, 2, 3, 4, 1, 3, 2, 1, 1 }
/** The number of values of DELAY_TAP_FACTOR_NUMS, the length of OMX_DELAY_TAP_FACTOR_NUMS_INIT. */
#define OMX_DELAY_TAP_FACTOR_NUMS_COUNT 11u
/** The numerators of the FX delay's tap factor set, in member order: 1/4, 1/3, 3/8, 1/2, 2/3, 3/4, 1, 4/3, 3/2, 2, 3. A read tap k \>= 1 sits at the base time times numerator / denominator (DELAY_TAP_FACTOR_DENS at the same position), formed in double and bounded in milliseconds to FX_DELAY_TIME_RANGE.max by `omx_fxdelay_tap_ms`, then rounded once to samples. The set is two integer lists, not a set of ids, because a factor such as 1/2 is no id the schema accepts and the ratio must stay exact (1/3 has no finite decimal). The values in declaration order, an initialiser. (none) */
#define OMX_DELAY_TAP_FACTOR_NUMS_INIT { 1, 1, 3, 1, 2, 3, 1, 4, 3, 2, 3 }
/** The number of values of DOUBLER_VOICE_DELAYS, the length of OMX_DOUBLER_VOICE_DELAYS_INIT. */
#define OMX_DOUBLER_VOICE_DELAYS_COUNT 4u
/** Each voice's share of delayMs, so the voices never land on one sample: two singers do not breathe together. The values in declaration order, an initialiser. (none) */
#define OMX_DOUBLER_VOICE_DELAYS_INIT { 1, 1.31f, 0.78f, 1.17f }
/** The number of values of DOUBLER_VOICE_DETUNES, the length of OMX_DOUBLER_VOICE_DETUNES_INIT. */
#define OMX_DOUBLER_VOICE_DETUNES_COUNT 4u
/** Each voice's share of detuneCents, signed: the first up, the second down, the third and fourth half as far, up and down. The values in declaration order, an initialiser. (none) */
#define OMX_DOUBLER_VOICE_DETUNES_INIT { 1, -1, 0.5f, -0.5f }
/** The number of values of DOUBLER_VOICE_PANS, the length of OMX_DOUBLER_VOICE_PANS_INIT. */
#define OMX_DOUBLER_VOICE_PANS_COUNT 4u
/** Each voice's place at a spread of 100 %, -1 hard left to +1 hard right. The values in declaration order, an initialiser. (none) */
#define OMX_DOUBLER_VOICE_PANS_INIT { -1, 1, -0.5f, 0.5f }
/** The number of values of EQ_CENTRE_SERIES_R20_HZ, the length of OMX_EQ_CENTRE_SERIES_R20_HZ_INIT. */
#define OMX_EQ_CENTRE_SERIES_R20_HZ_COUNT 61u
/** The ISO 266 R20 preferred frequencies from 20 Hz to 20 kHz (61 values, sixth-octave spacing): the finer series the default band rule falls back to where R10 (ISO_THIRD_OCTAVE_CENTRES_HZ, 31 values) has too few to give every band its own centre. R10 is a subset of it. The values in declaration order, an initialiser. (Hz) */
#define OMX_EQ_CENTRE_SERIES_R20_HZ_INIT { 20, 22.4f, 25, 28, 31.5f, 35.5f, 40, 45, 50, 56, 63, 71, 80, 90, 100, 112, 125, 140, 160, 180, 200, 224, 250, 280, 315, 355, 400, 450, 500, 560, 630, 710, 800, 900, 1000, 1120, 1250, 1400, 1600, 1800, 2000, 2240, 2500, 2800, 3150, 3550, 4000, 4500, 5000, 5600, 6300, 7100, 8000, 9000, 10000, 11200, 12500, 14000, 16000, 18000, 20000 }
/** The number of values of EQ_DEFAULT_CENTRES_FOUR_BAND_HZ, the length of OMX_EQ_DEFAULT_CENTRES_FOUR_BAND_HZ_INIT. */
#define OMX_EQ_DEFAULT_CENTRES_FOUR_BAND_HZ_COUNT 4u
/** The default band centres of a four-band EQ, Hz: ISO 266 preferred values, two to two and a half octaves apart. The default rule gives every other band count its centres from the preferred series; four bands start here. The values in declaration order, an initialiser. (Hz) */
#define OMX_EQ_DEFAULT_CENTRES_FOUR_BAND_HZ_INIT { 100, 400, 2000, 8000 }
/** The number of values of GLUE_ATTACK_MS, the length of OMX_GLUE_ATTACK_MS_INIT. */
#define OMX_GLUE_ATTACK_MS_COUNT 6u
/** The attacks of the `attack` detents, ms. The values in declaration order, an initialiser. (ms) */
#define OMX_GLUE_ATTACK_MS_INIT { 0.1f, 0.3f, 1, 3, 10, 30 }
/** The number of values of GLUE_RATIO_VALUES, the length of OMX_GLUE_RATIO_VALUES_INIT. */
#define OMX_GLUE_RATIO_VALUES_COUNT 3u
/** The ratios of the `ratio` detents. The values in declaration order, an initialiser. (none) */
#define OMX_GLUE_RATIO_VALUES_INIT { 2, 4, 10 }
/** The number of values of GLUE_RELEASE_MS, the length of OMX_GLUE_RELEASE_MS_INIT. */
#define OMX_GLUE_RELEASE_MS_COUNT 4u
/** The releases of the `release` detents but the last, ms. The values in declaration order, an initialiser. (ms) */
#define OMX_GLUE_RELEASE_MS_INIT { 100, 300, 600, 1200 }
/** The number of values of ISO_THIRD_OCTAVE_CENTRES_HZ, the length of OMX_ISO_THIRD_OCTAVE_CENTRES_HZ_INIT. */
#define OMX_ISO_THIRD_OCTAVE_CENTRES_HZ_COUNT 31u
/** The standard ISO 266 one-third-octave centre frequencies, 20 Hz–20 kHz (31 bands) — the classic 31-band graphic-EQ layout's centres. The values in declaration order, an initialiser. (Hz) */
#define OMX_ISO_THIRD_OCTAVE_CENTRES_HZ_INIT { 20, 25, 31.5f, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000, 20000 }
/** The number of values of LEVELER_AMOUNT_POINTS, the length of OMX_LEVELER_AMOUNT_POINTS_INIT. */
#define OMX_LEVELER_AMOUNT_POINTS_COUNT 5u
/** The amounts the table's rows are written at, percent; between two the leveler reads the rows linearly. The values in declaration order, an initialiser. (percent) */
#define OMX_LEVELER_AMOUNT_POINTS_INIT { 0, 25, 50, 75, 100 }
/** The number of values of LEVELER_DOWN_MAKEUP_DB, the length of OMX_LEVELER_DOWN_MAKEUP_DB_INIT. */
#define OMX_LEVELER_DOWN_MAKEUP_DB_COUNT 5u
/** The downward slot's make-up at each point, dB: part of the reduction a programme above its threshold takes, so the level the operator hears stays near where it was. The values in declaration order, an initialiser. (dB) */
#define OMX_LEVELER_DOWN_MAKEUP_DB_INIT { 0, 1, 2.5f, 4, 6 }
/** The number of values of LEVELER_DOWN_RATIO, the length of OMX_LEVELER_DOWN_RATIO_INIT. */
#define OMX_LEVELER_DOWN_RATIO_COUNT 5u
/** The downward slot's ratio at each point; 1 is no compression. The values in declaration order, an initialiser. (none) */
#define OMX_LEVELER_DOWN_RATIO_INIT { 1, 1.5f, 2, 3, 4 }
/** The number of values of LEVELER_DOWN_THRESHOLD_DB, the length of OMX_LEVELER_DOWN_THRESHOLD_DB_INIT. */
#define OMX_LEVELER_DOWN_THRESHOLD_DB_COUNT 5u
/** The downward slot's threshold at each point of LEVELER_AMOUNT_POINTS, dBFS of the RMS detector. The values in declaration order, an initialiser. (dB) */
#define OMX_LEVELER_DOWN_THRESHOLD_DB_INIT { 0, -10, -16, -22, -28 }
/** The number of values of LEVELER_SPEED_ATTACK_MS, the length of OMX_LEVELER_SPEED_ATTACK_MS_INIT. */
#define OMX_LEVELER_SPEED_ATTACK_MS_COUNT 3u
/** Each speed's attack, ms: slow, medium, fast. The values in declaration order, an initialiser. (ms) */
#define OMX_LEVELER_SPEED_ATTACK_MS_INIT { 30, 10, 3 }
/** The number of values of LEVELER_SPEED_RELEASE_MS, the length of OMX_LEVELER_SPEED_RELEASE_MS_INIT. */
#define OMX_LEVELER_SPEED_RELEASE_MS_COUNT 3u
/** Each speed's release, ms: slow, medium, fast. The values in declaration order, an initialiser. (ms) */
#define OMX_LEVELER_SPEED_RELEASE_MS_INIT { 800, 300, 100 }
/** The number of values of LEVELER_UP_RANGE_DB, the length of OMX_LEVELER_UP_RANGE_DB_INIT. */
#define OMX_LEVELER_UP_RANGE_DB_COUNT 5u
/** The upward slot's largest boost at each point, dB, so a pause is never lifted by more than this. The values in declaration order, an initialiser. (dB) */
#define OMX_LEVELER_UP_RANGE_DB_INIT { 0, 3, 6, 9, 12 }
/** The number of values of LEVELER_UP_RATIO, the length of OMX_LEVELER_UP_RATIO_INIT. */
#define OMX_LEVELER_UP_RATIO_COUNT 5u
/** The upward slot's ratio at each point; 1 is no lift. The values in declaration order, an initialiser. (none) */
#define OMX_LEVELER_UP_RATIO_INIT { 1, 1.5f, 2, 2.5f, 3 }
/** The number of values of LEVELER_UP_THRESHOLD_DB, the length of OMX_LEVELER_UP_THRESHOLD_DB_INIT. */
#define OMX_LEVELER_UP_THRESHOLD_DB_COUNT 5u
/** The upward slot's threshold at each point, dBFS of the RMS detector: what lies below it is raised toward it. The values in declaration order, an initialiser. (dB) */
#define OMX_LEVELER_UP_THRESHOLD_DB_INIT { -40, -42, -45, -48, -50 }
/** The number of values of MBC_CROSSOVER_DEFAULTS_2, the length of OMX_MBC_CROSSOVER_DEFAULTS_2_INIT. */
#define OMX_MBC_CROSSOVER_DEFAULTS_2_COUNT 1u
/** The come-up crossover corners of a 2-band multiband stage, Hz, ascending: one split at 1 kHz. The values in declaration order, an initialiser. (Hz) */
#define OMX_MBC_CROSSOVER_DEFAULTS_2_INIT { 1000 }
/** The number of values of MBC_CROSSOVER_DEFAULTS_3, the length of OMX_MBC_CROSSOVER_DEFAULTS_3_INIT. */
#define OMX_MBC_CROSSOVER_DEFAULTS_3_COUNT 2u
/** The come-up crossover corners of a 3-band multiband stage, Hz, ascending: body, presence, air. The values in declaration order, an initialiser. (Hz) */
#define OMX_MBC_CROSSOVER_DEFAULTS_3_INIT { 200, 2000 }
/** The number of values of MBC_CROSSOVER_DEFAULTS_4, the length of OMX_MBC_CROSSOVER_DEFAULTS_4_INIT. */
#define OMX_MBC_CROSSOVER_DEFAULTS_4_COUNT 3u
/** The come-up crossover corners of a 4-band multiband stage, Hz, ascending. The values in declaration order, an initialiser. (Hz) */
#define OMX_MBC_CROSSOVER_DEFAULTS_4_INIT { 150, 800, 5000 }
/** The number of values of MBC_CROSSOVER_DEFAULTS_5, the length of OMX_MBC_CROSSOVER_DEFAULTS_5_INIT. */
#define OMX_MBC_CROSSOVER_DEFAULTS_5_COUNT 4u
/** The come-up crossover corners of a 5-band multiband stage, Hz, ascending: two octaves apart. The values in declaration order, an initialiser. (Hz) */
#define OMX_MBC_CROSSOVER_DEFAULTS_5_INIT { 100, 400, 1600, 6400 }
/** The number of values of ORACLE_FLOOR_RATES, the length of OMX_ORACLE_FLOOR_RATES_INIT. */
#define OMX_ORACLE_FLOOR_RATES_COUNT 4u
/** The four rates every kernel oracle runs at, whatever STANDARD_SAMPLE_RATES says: a declared list missing one of them stops a suite instead of testing less (omx-dsp, omx-plugins and omx-clap-host run their oracles at these). The values in declaration order, an initialiser. (Hz; since 1.3.0) */
#define OMX_ORACLE_FLOOR_RATES_INIT { 44100, 48000, 96000, 192000 }
/** The number of values of ORACLE_FULL_RATES, the length of OMX_ORACLE_FULL_RATES_INIT. */
#define OMX_ORACLE_FULL_RATES_COUNT 9u
/** The full grid of rates a professional interface can clock at, nine of them from 32 to 192 kHz: where ORACLE_FLOOR_RATES is the four rates every kernel oracle must run at, this is the whole grid an oracle runs at when it runs at all of them. STANDARD_SAMPLE_RATES is a subset of it; an oracle that runs here also covers 32, 64 and 128 kHz, where a kernel still has to meet its closed form even though the console never asks for them. The values in declaration order, an initialiser. (Hz; since 1.3.0) */
#define OMX_ORACLE_FULL_RATES_INIT { 32000, 44100, 48000, 64000, 88200, 96000, 128000, 176400, 192000 }
/** The number of values of REVERB_EARLY_TAP_GAINS, the length of OMX_REVERB_EARLY_TAP_GAINS_INIT. */
#define OMX_REVERB_EARLY_TAP_GAINS_COUNT 8u
/** The early reflections' gains, linear, one per REVERB_EARLY_TAP_MS entry, falling with the time. The values in declaration order, an initialiser. (none) */
#define OMX_REVERB_EARLY_TAP_GAINS_INIT { 0.84f, 0.75f, 0.67f, 0.6f, 0.53f, 0.47f, 0.42f, 0.37f }
/** The number of values of REVERB_EARLY_TAP_MS, the length of OMX_REVERB_EARLY_TAP_MS_INIT. */
#define OMX_REVERB_EARLY_TAP_MS_COUNT 8u
/** The early reflections' times, ms after the pre-delay, at a size scale of 1: eight taps, the even ones on the left, the odd on the right, none landing on another's sample at any oracle rate. The values in declaration order, an initialiser. (ms) */
#define OMX_REVERB_EARLY_TAP_MS_INIT { 3.1f, 5.3f, 7.9f, 10.7f, 13.9f, 17.3f, 21.1f, 25.3f }
/** The number of values of RTA_GRID_FREQS, the length of OMX_RTA_GRID_FREQS_INIT. */
#define OMX_RTA_GRID_FREQS_COUNT 10u
/** The frequencies of a spectrum display's labelled vertical graticule lines, Hz, on the log frequency axis: the decades and their 2 and 5 steps from 20 Hz to 20 kHz, every surface. The values in declaration order, an initialiser. (Hz) */
#define OMX_RTA_GRID_FREQS_INIT { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 }
/** The number of values of SPRING_CHIRP_HZ, the length of OMX_SPRING_CHIRP_HZ_INIT. */
#define OMX_SPRING_CHIRP_HZ_COUNT 2u
/** The all-pass sections' corner at a tension of 0 and of 100 %, Hz, moved linearly between. The values in declaration order, an initialiser. (Hz) */
#define OMX_SPRING_CHIRP_HZ_INIT { 1200, 3600 }
/** The number of values of SUBOCT_POST1_HZ, the length of OMX_SUBOCT_POST1_HZ_INIT. */
#define OMX_SUBOCT_POST1_HZ_COUNT 3u
/** Each range's low-pass on the first octave, Hz, 24 dB per octave. The values in declaration order, an initialiser. (Hz) */
#define OMX_SUBOCT_POST1_HZ_INIT { 90, 180, 360 }
/** The number of values of SUBOCT_POST2_HZ, the length of OMX_SUBOCT_POST2_HZ_INIT. */
#define OMX_SUBOCT_POST2_HZ_COUNT 3u
/** Each range's low-pass on the second octave, Hz, 24 dB per octave. The values in declaration order, an initialiser. (Hz) */
#define OMX_SUBOCT_POST2_HZ_INIT { 45, 90, 180 }
/** The number of values of SUBOCT_RANGE_HI_HZ, the length of OMX_SUBOCT_RANGE_HI_HZ_INIT. */
#define OMX_SUBOCT_RANGE_HI_HZ_COUNT 3u
/** Each range's highest tracked fundamental, Hz: the tracking band's low-pass. The values in declaration order, an initialiser. (Hz) */
#define OMX_SUBOCT_RANGE_HI_HZ_INIT { 140, 280, 560 }
/** The number of values of SUBOCT_RANGE_LO_HZ, the length of OMX_SUBOCT_RANGE_LO_HZ_INIT. */
#define OMX_SUBOCT_RANGE_LO_HZ_COUNT 3u
/** Each range's lowest tracked fundamental, Hz; the tracking band's high-pass sits an octave under it. The values in declaration order, an initialiser. (Hz) */
#define OMX_SUBOCT_RANGE_LO_HZ_INIT { 35, 70, 140 }

/* Every set rendered to C, sorted by set: an enum of the ids (a named id's position, or a numeric id's number), `OMX_<SET>_COUNT` and,
 * where the set names a default, `OMX_<SET>_DEFAULT`. */
/** The biquad section kinds the EQ design builds, in the order of the integer a kernel reads: the bell, the two shelves, the pass filters, the notch, the bandpass (the de-esser's detector band, never an EQ band), and the first- and second-order allpasses (the dynamic EQ's and the phase tool's sections). The design itself is omx-dsp's `omx_eq_design`; the console draws its curves from a twin of it, and both dispatch on this list. No come-up id: a kind is chosen by the stage that designs the section. (since 2.14.0) A named id's value is its position. */
enum omx_biquad_kinds {
  OMX_BIQUAD_KINDS_PEAKING = 0, ///< The id `peaking`.
  OMX_BIQUAD_KINDS_LOW_SHELF = 1, ///< The id `lowShelf`.
  OMX_BIQUAD_KINDS_HIGH_SHELF = 2, ///< The id `highShelf`.
  OMX_BIQUAD_KINDS_HIGHPASS = 3, ///< The id `highpass`.
  OMX_BIQUAD_KINDS_LOWPASS = 4, ///< The id `lowpass`.
  OMX_BIQUAD_KINDS_NOTCH = 5, ///< The id `notch`.
  OMX_BIQUAD_KINDS_BANDPASS = 6, ///< The id `bandpass`.
  OMX_BIQUAD_KINDS_ALLPASS1 = 7, ///< The id `allpass1`.
  OMX_BIQUAD_KINDS_ALLPASS2 = 8, ///< The id `allpass2`.
};
/** The number of ids of BIQUAD_KINDS. */
#define OMX_BIQUAD_KINDS_COUNT 9u
/** How the detector folds the key's two legs into one level, in the order of the integer the kernel reads: linked, the louder leg (the compressor's own detection, the come-up value); mid, the mid (L + R)/2; side, the side (L - R)/2; max, the louder of the mid and the side. One gain serves both legs whatever the fold. A named id's value is its position. */
enum omx_comp_detect_links {
  OMX_COMP_DETECT_LINKS_LINKED = 0, ///< Linked
  OMX_COMP_DETECT_LINKS_MID = 1, ///< Mid
  OMX_COMP_DETECT_LINKS_SIDE = 2, ///< Side
  OMX_COMP_DETECT_LINKS_MAX = 3, ///< Max
};
/** The number of ids of COMP_DETECT_LINKS. */
#define OMX_COMP_DETECT_LINKS_COUNT 4u
/** The id COMP_DETECT_LINKS names as its default. */
#define OMX_COMP_DETECT_LINKS_DEFAULT OMX_COMP_DETECT_LINKS_LINKED
/** The ducker: on, the reduction the key's level asks for is held to rangeDb, so the programme gain is max(rangeDb, the compressor's law at the key's level). It comes up off. A named id's value is its position. */
enum omx_comp_ducks {
  OMX_COMP_DUCKS_OFF = 0, ///< Off
  OMX_COMP_DUCKS_ON = 1, ///< On
};
/** The number of ids of COMP_DUCKS. */
#define OMX_COMP_DUCKS_COUNT 2u
/** The id COMP_DUCKS names as its default. */
#define OMX_COMP_DUCKS_DEFAULT OMX_COMP_DUCKS_OFF
/** While on, the output is what the detector hears, the key after its filter and tilt, folded as detectLink folds it: linked the two legs, mid the mid on both, side the side on both, max the mid on the left and the side on the right. The compressor keeps running underneath. It comes up off. A named id's value is its position. */
enum omx_comp_key_listens {
  OMX_COMP_KEY_LISTENS_OFF = 0, ///< Off
  OMX_COMP_KEY_LISTENS_ON = 1, ///< On
};
/** The number of ids of COMP_KEY_LISTENS. */
#define OMX_COMP_KEY_LISTENS_COUNT 2u
/** The id COMP_KEY_LISTENS names as its default. */
#define OMX_COMP_KEY_LISTENS_DEFAULT OMX_COMP_KEY_LISTENS_OFF
/** Where the compressor's detector listens: its own input (self), or the key handed to its sidechain port (sidechain), in the order of the integer the key port carries; the gate's keySource ids. Which strip or bus feeds the sidechain is console routing. It comes up self. A named id's value is its position. */
enum omx_comp_key_sources {
  OMX_COMP_KEY_SOURCES_SELF = 0, ///< Self
  OMX_COMP_KEY_SOURCES_SIDECHAIN = 1, ///< Sidechain
};
/** The number of ids of COMP_KEY_SOURCES. */
#define OMX_COMP_KEY_SOURCES_COUNT 2u
/** The id COMP_KEY_SOURCES names as its default. */
#define OMX_COMP_KEY_SOURCES_DEFAULT OMX_COMP_KEY_SOURCES_SELF
/** The compressor's character, which picks its detector: `comp` runs the RMS detector, `limiter` the peak detector. The console's comp row carries it (`CompState.kind`, `COMP_KINDS`) and comes up as `comp`. A named id's value is its position. */
enum omx_comp_kinds {
  OMX_COMP_KINDS_COMP = 0, ///< Compressor
  OMX_COMP_KINDS_LIMITER = 1, ///< Limiter
};
/** The number of ids of COMP_KINDS. */
#define OMX_COMP_KINDS_COUNT 2u
/** The id COMP_KINDS names as its default. */
#define OMX_COMP_KINDS_DEFAULT OMX_COMP_KINDS_COMP
/** How the compressor releases, in the order of the integer the kernel reads: `fixed`, the detector's release at releaseMs, the compressor that shipped bit for bit and the come-up value; `opto`, the program-dependent release of PROGRAM_RELEASE_PROFILES.opto (an LA-2A's two-stage recovery, slower the longer the cell was driven); `auto`, the same word with PROGRAM_RELEASE_PROFILES.auto (a bus compressor's AUTO position). In opto and auto the detector releases at its attack pole and releaseMs is inert, kept for when fixed returns (omx_env_program_release on the gain, after the gain computer). A named id's value is its position. */
enum omx_comp_release_modes {
  OMX_COMP_RELEASE_MODES_FIXED = 0, ///< Fixed
  OMX_COMP_RELEASE_MODES_OPTO = 1, ///< Opto
  OMX_COMP_RELEASE_MODES_AUTO = 2, ///< Auto
};
/** The number of ids of COMP_RELEASE_MODES. */
#define OMX_COMP_RELEASE_MODES_COUNT 3u
/** The id COMP_RELEASE_MODES names as its default. */
#define OMX_COMP_RELEASE_MODES_DEFAULT OMX_COMP_RELEASE_MODES_FIXED
/** What omx_curve_points answers in its code: `ok`, a curve; `kindUnknown`, a kind, family or head filter slope with no closed form; `termMissing`, a term the closed form has no default for is not in the request (a band's frequency and shape, a bell's or shelf's gain, a head filter's corner, and its slope unless it comes from the role's declared enumeration as dB per octave; threshold and ratio for dynamics and the de-esser, threshold for a limiter (its ratio is infinity, a hard corner), threshold, range and ratio for a gate; both ends of a gate's knee range or neither); `unitRefused`, a term in a unit its role does not accept; `valueNotFinite`, a value, the rate or an extent that is not finite, a rate that is not positive; `pointsRange`, grid points outside 2 to CURVE_POINTS_MAX (for an EQ, with three anchors a drawn band), a band index past what fits, or extents that bound nothing; `room`, a caller's buffers too small. Where omx-dsp declares a default the curve takes it: a band's Q is EQ_BAND_DEFAULTS q, clamped for its shape as clampEqQFor clamps it. No curve is drawn from a guess. For the time family (since 0.32.4): `termMissing` a delay with no time (a synced one with no division or no tempo in the request), a delay with no feedback, a reverb with no mix or with neither size nor decay; `kindUnknown` a division past DELAY_DIVISION_BEATS, or an algorithm the closed form has no loop for (reverse, gated); `room` a delay's stems with no buffer for them. (since 3.0.0) A named id's value is its position. */
enum omx_curve_codes {
  OMX_CURVE_CODES_OK = 0, ///< The id `ok`.
  OMX_CURVE_CODES_KIND_UNKNOWN = 1, ///< The id `kindUnknown`.
  OMX_CURVE_CODES_TERM_MISSING = 2, ///< The id `termMissing`.
  OMX_CURVE_CODES_UNIT_REFUSED = 3, ///< The id `unitRefused`.
  OMX_CURVE_CODES_VALUE_NOT_FINITE = 4, ///< The id `valueNotFinite`.
  OMX_CURVE_CODES_POINTS_RANGE = 5, ///< The id `pointsRange`.
  OMX_CURVE_CODES_ROOM = 6, ///< The id `room`.
};
/** The number of ids of CURVE_CODES. */
#define OMX_CURVE_CODES_COUNT 7u
/** The displays a panel description draws from a closed form, the kinds omx_curve_points answers: an EQ's magnitude response, x in Hz and y in dB, and a dynamics stage's static transfer curve, input dB to output dB. The wire spells each id in snake case (`eq_response`, `transfer_curve`), the description's own display word. `time_response` (since 0.32.4) is the picture of a stage whose shape is a function of TIME, x in ms from 0 and y in dB relative to a unit impulse: a delay's tap train, answered as STEMS (one per tap: its time, its level, its brightness as a shade 0..1 and the leg it lands on), and a reverb's decay envelope, answered as points; the answer says which arrays it carries, never the family. (since 3.0.0) A named id's value is its position. */
enum omx_curve_displays {
  OMX_CURVE_DISPLAYS_EQ_RESPONSE = 0, ///< The id `eqResponse`.
  OMX_CURVE_DISPLAYS_TRANSFER_CURVE = 1, ///< The id `transferCurve`.
  OMX_CURVE_DISPLAYS_TIME_RESPONSE = 2, ///< The id `timeResponse`.
};
/** The number of ids of CURVE_DISPLAYS. */
#define OMX_CURVE_DISPLAYS_COUNT 3u
/** The panel families omx_curve_points draws, by the description's family word. `eq` draws an eq_response; the other four a transfer_curve, the family choosing the gain computer's mode: `dynamics`, `limiter` and `deesser` act ABOVE the threshold, `gate` BELOW it with the shifted knee of the gate's knee range. `drive` draws a transfer_curve too, input dB to output dB like the others: a steady level through the drive kernel's static path, its shaper biased by the character, faded by the mix and scaled by the trim (since 0.32.4). Any other family has no closed form here. `delay` and `reverb` (since 0.32.4) draw a time_response: the delay the tap train its kernel produces (the dry tap at 1 - mix, echo n at n x time at mix x feedback^(n-1), darkened by the tone one-pole each pass, L/R alternating under ping-pong, stopping below CURVE_EXTENTS timeFloorDb of the loudest tap or at CURVE_TAPS_MAX), the reverb the closed form of its longest loop (REVERB_DECAY_LOOPS: level g^((t - predelay) / loop) scaled by the mix, 0 before the predelay, ending at its RT60). (since 3.0.0) A named id's value is its position. */
enum omx_curve_families {
  OMX_CURVE_FAMILIES_EQ = 0, ///< The id `eq`.
  OMX_CURVE_FAMILIES_DYNAMICS = 1, ///< The id `dynamics`.
  OMX_CURVE_FAMILIES_LIMITER = 2, ///< The id `limiter`.
  OMX_CURVE_FAMILIES_GATE = 3, ///< The id `gate`.
  OMX_CURVE_FAMILIES_DEESSER = 4, ///< The id `deesser`.
  OMX_CURVE_FAMILIES_DRIVE = 5, ///< The id `drive`.
  OMX_CURVE_FAMILIES_DELAY = 6, ///< The id `delay`.
  OMX_CURVE_FAMILIES_REVERB = 7, ///< The id `reverb`.
};
/** The number of ids of CURVE_FAMILIES. */
#define OMX_CURVE_FAMILIES_COUNT 8u
/** The leg a delay's tap lands on (since 0.32.4): `both` (the dry tap, and every echo of a plain delay), `left` and `right` (the echoes of a ping-pong delay, alternating from the first). A surface draws it as the stem's letter. (since 3.0.0) A named id's value is its position. */
enum omx_curve_legs {
  OMX_CURVE_LEGS_BOTH = 0, ///< The id `both`.
  OMX_CURVE_LEGS_LEFT = 1, ///< The id `left`.
  OMX_CURVE_LEGS_RIGHT = 2, ///< The id `right`.
};
/** The number of ids of CURVE_LEGS. */
#define OMX_CURVE_LEGS_COUNT 3u
/** Where an EQ band of a curve request comes from, as the console's strip EQ state says it (its BAND_ORIGINS): `user`, the operator's own band, and the bands a planter put there, `fbsFixed` and `fbsLive` (the feedback corrector's ring-out and show pools), `hrp` (the harmonic resonance processor's cuts) and `roomMode` (a room mode's correction), spelt `user`, `fbs-fixed`, `fbs-live`, `hrp` and `room-mode` on the wire. The sum ignores it: every drawn band is in the curve. A band whose origin is not `user` is answered as a mark (its frequency, depth, Q and origin), and a request that asks for it also gets the curve over the `user` bands alone. A plugin's descriptor carries no origin: a request with none draws no marks. (since 3.0.0) A named id's value is its position. */
enum omx_curve_origins {
  OMX_CURVE_ORIGINS_USER = 0, ///< The id `user`.
  OMX_CURVE_ORIGINS_FBS_FIXED = 1, ///< The id `fbsFixed`.
  OMX_CURVE_ORIGINS_FBS_LIVE = 2, ///< The id `fbsLive`.
  OMX_CURVE_ORIGINS_HRP = 3, ///< The id `hrp`.
  OMX_CURVE_ORIGINS_ROOM_MODE = 4, ///< The id `roomMode`.
};
/** The number of ids of CURVE_ORIGINS. */
#define OMX_CURVE_ORIGINS_COUNT 5u
/** The roles of a panel description a curve request reads, by the description's role word (the wire spells each id in snake case: `band_freq`, `knee_start`). The EQ's indexed band roles and head filters, the gain computer's terms, and the drive's (since 0.32.4): `drive` the pre-shaper gain in dB, `tone` the character -1..+1 that biases the shaper, `model` the shaper (the drive kernel's curve, 0 soft, 1 tape, 2 tube, 3 exciter), `mix` the wet share in percent and `output` the trim in dB; a drive curve needs `drive` and `model`, and draws `tone` -1, `mix` 100 % and `output` 0 dB where a term is absent (a card's picture draws no make-up or parallel mix). A term of any other role is no term of the curve. The time family's (since 0.32.4), the words of omx-panel's role table: a delay's `time` (ms or s), `feedback` and `mix` (a share: none or coef 0..1, or pc), `tone` (0..1, the feedback low-pass), `pingpong` (a switch), `sync` (a switch) and `division` (a choice into DELAY_DIVISION_BEATS, read with the request's tempo); a reverb's `size` (0..1), `decay` (ms or s, the RT60 said outright), `algorithm` (a choice: the reverb kernel's 0 room, 1 plate, 2 hall), `predelay` (ms or s), `damping` and `width` (0..1), `low_cut` and `high_cut` (Hz). A delay curve needs `time` (or `sync` on, `division` and a tempo) and `feedback`, and draws `mix` 1 where it is absent; a reverb curve needs `mix` and `size` with `algorithm` or `decay`; the others ride along for a later picture. (since 3.0.0) A named id's value is its position. */
enum omx_curve_roles {
  OMX_CURVE_ROLES_BAND_TYPE = 0, ///< The id `bandType`.
  OMX_CURVE_ROLES_BAND_FREQ = 1, ///< The id `bandFreq`.
  OMX_CURVE_ROLES_BAND_GAIN = 2, ///< The id `bandGain`.
  OMX_CURVE_ROLES_BAND_Q = 3, ///< The id `bandQ`.
  OMX_CURVE_ROLES_BAND_ON = 4, ///< The id `bandOn`.
  OMX_CURVE_ROLES_HPF_FREQ = 5, ///< The id `hpfFreq`.
  OMX_CURVE_ROLES_HPF_SLOPE = 6, ///< The id `hpfSlope`.
  OMX_CURVE_ROLES_HPF_ON = 7, ///< The id `hpfOn`.
  OMX_CURVE_ROLES_LPF_FREQ = 8, ///< The id `lpfFreq`.
  OMX_CURVE_ROLES_LPF_SLOPE = 9, ///< The id `lpfSlope`.
  OMX_CURVE_ROLES_LPF_ON = 10, ///< The id `lpfOn`.
  OMX_CURVE_ROLES_THRESHOLD = 11, ///< The id `threshold`.
  OMX_CURVE_ROLES_RATIO = 12, ///< The id `ratio`.
  OMX_CURVE_ROLES_KNEE = 13, ///< The id `knee`.
  OMX_CURVE_ROLES_MAKEUP = 14, ///< The id `makeup`.
  OMX_CURVE_ROLES_RANGE = 15, ///< The id `range`.
  OMX_CURVE_ROLES_KNEE_START = 16, ///< The id `kneeStart`.
  OMX_CURVE_ROLES_KNEE_END = 17, ///< The id `kneeEnd`.
  OMX_CURVE_ROLES_DRIVE = 18, ///< The id `drive`.
  OMX_CURVE_ROLES_TONE = 19, ///< The id `tone`.
  OMX_CURVE_ROLES_MODEL = 20, ///< The id `model`.
  OMX_CURVE_ROLES_MIX = 21, ///< The id `mix`.
  OMX_CURVE_ROLES_OUTPUT = 22, ///< The id `output`.
  OMX_CURVE_ROLES_TIME = 23, ///< The id `time`.
  OMX_CURVE_ROLES_FEEDBACK = 24, ///< The id `feedback`.
  OMX_CURVE_ROLES_PINGPONG = 25, ///< The id `pingpong`.
  OMX_CURVE_ROLES_SYNC = 26, ///< The id `sync`.
  OMX_CURVE_ROLES_DIVISION = 27, ///< The id `division`.
  OMX_CURVE_ROLES_SIZE = 28, ///< The id `size`.
  OMX_CURVE_ROLES_DECAY = 29, ///< The id `decay`.
  OMX_CURVE_ROLES_ALGORITHM = 30, ///< The id `algorithm`.
  OMX_CURVE_ROLES_PREDELAY = 31, ///< The id `predelay`.
  OMX_CURVE_ROLES_DAMPING = 32, ///< The id `damping`.
  OMX_CURVE_ROLES_WIDTH = 33, ///< The id `width`.
  OMX_CURVE_ROLES_LOW_CUT = 34, ///< The id `lowCut`.
  OMX_CURVE_ROLES_HIGH_CUT = 35, ///< The id `highCut`.
};
/** The number of ids of CURVE_ROLES. */
#define OMX_CURVE_ROLES_COUNT 36u
/** The shape of an EQ band, as the caller resolves it from the description: a `band_type` role's `shapes` list indexed by the control's choice value, or the fixed `shape` of a band no control selects. `off` is a band that is not drawn, and so is `unshaped`, spelt `-` on the wire, a choice value with no shape. The others design the band through omx_eq_design: a bell, the two shelves, the pass filters, the notch, and the first- and second-order allpasses, which draw what they run, 0 dB. (since 3.0.0) A named id's value is its position. */
enum omx_curve_shapes {
  OMX_CURVE_SHAPES_OFF = 0, ///< The id `off`.
  OMX_CURVE_SHAPES_BELL = 1, ///< The id `bell`.
  OMX_CURVE_SHAPES_LOWSHELF = 2, ///< The id `lowshelf`.
  OMX_CURVE_SHAPES_HIGHSHELF = 3, ///< The id `highshelf`.
  OMX_CURVE_SHAPES_HIGHPASS = 4, ///< The id `highpass`.
  OMX_CURVE_SHAPES_LOWPASS = 5, ///< The id `lowpass`.
  OMX_CURVE_SHAPES_NOTCH = 6, ///< The id `notch`.
  OMX_CURVE_SHAPES_UNSHAPED = 7, ///< The id `unshaped`.
  OMX_CURVE_SHAPES_ALLPASS1 = 8, ///< The id `allpass1`.
  OMX_CURVE_SHAPES_ALLPASS2 = 9, ///< The id `allpass2`.
};
/** The number of ids of CURVE_SHAPES. */
#define OMX_CURVE_SHAPES_COUNT 10u
/** The units a curve term's value may come in: the PARAM_UNITS ids (the wire word, never its x-symbol) the served description gives a role, so a host sends the unit it was given. omx_curve_points converts once: `coef` to dB for a gain, `oct` to Q for a bandwidth, `kHz` to Hz; a level in `dB`, `dBFS`, `dBu` or `LUFS` is read as dB from the threshold's own scale; `none` is a plain number, a switch or a choice; `ratio` a ratio. A unit its role does not accept is refused, and so is any other spelling of an id (`hz`, `db`). `percent` (since 0.32.4) is the drive's `mix`. `ms` and `s` (since 0.32.4) are a time, read as ms. (since 3.0.0) A named id's value is its position. */
enum omx_curve_units {
  OMX_CURVE_UNITS_HZ = 0, ///< The id `Hz`.
  OMX_CURVE_UNITS_K_HZ = 1, ///< The id `kHz`.
  OMX_CURVE_UNITS_D_B = 2, ///< The id `dB`.
  OMX_CURVE_UNITS_D_BFS = 3, ///< The id `dBFS`.
  OMX_CURVE_UNITS_D_BU = 4, ///< The id `dBu`.
  OMX_CURVE_UNITS_LUFS = 5, ///< The id `LUFS`.
  OMX_CURVE_UNITS_COEF = 6, ///< The id `coef`.
  OMX_CURVE_UNITS_OCT = 7, ///< The id `oct`.
  OMX_CURVE_UNITS_NONE = 8, ///< The id `none`.
  OMX_CURVE_UNITS_RATIO = 9, ///< The id `ratio`.
  OMX_CURVE_UNITS_PERCENT = 10, ///< The id `percent`.
  OMX_CURVE_UNITS_MS = 11, ///< The id `ms`.
  OMX_CURVE_UNITS_S = 12, ///< The id `s`.
};
/** The number of ids of CURVE_UNITS. */
#define OMX_CURVE_UNITS_COUNT 13u
/** The de-esser's modes, in the order of the integer the kernel reads: split attenuates the detected band only, wideband the whole signal while the sibilance lasts. A named id's value is its position. */
enum omx_deess_modes {
  OMX_DEESS_MODES_SPLIT = 0, ///< The id `split`.
  OMX_DEESS_MODES_WIDEBAND = 1, ///< The id `wideband`.
};
/** The number of ids of DEESS_MODES. */
#define OMX_DEESS_MODES_COUNT 2u
/** The note value the time follows while sync is tempo: 1/1 to 1/32, each straight, dotted (x 3/2) and triplet (x 2/3), in beats of a quarter note (DELAY_DIVISION_BEATS, in this order; the 1/32 three appended last, so every earlier id keeps its number). It comes up a quarter. A named id's value is its position. */
enum omx_delay_divisions {
  OMX_DELAY_DIVISIONS_WHOLE = 0, ///< 1/1
  OMX_DELAY_DIVISIONS_WHOLE_DOTTED = 1, ///< 1/1 dotted
  OMX_DELAY_DIVISIONS_WHOLE_TRIPLET = 2, ///< 1/1 triplet
  OMX_DELAY_DIVISIONS_HALF = 3, ///< 1/2
  OMX_DELAY_DIVISIONS_HALF_DOTTED = 4, ///< 1/2 dotted
  OMX_DELAY_DIVISIONS_HALF_TRIPLET = 5, ///< 1/2 triplet
  OMX_DELAY_DIVISIONS_QUARTER = 6, ///< 1/4
  OMX_DELAY_DIVISIONS_QUARTER_DOTTED = 7, ///< 1/4 dotted
  OMX_DELAY_DIVISIONS_QUARTER_TRIPLET = 8, ///< 1/4 triplet
  OMX_DELAY_DIVISIONS_EIGHTH = 9, ///< 1/8
  OMX_DELAY_DIVISIONS_EIGHTH_DOTTED = 10, ///< 1/8 dotted
  OMX_DELAY_DIVISIONS_EIGHTH_TRIPLET = 11, ///< 1/8 triplet
  OMX_DELAY_DIVISIONS_SIXTEENTH = 12, ///< 1/16
  OMX_DELAY_DIVISIONS_SIXTEENTH_DOTTED = 13, ///< 1/16 dotted
  OMX_DELAY_DIVISIONS_SIXTEENTH_TRIPLET = 14, ///< 1/16 triplet
  OMX_DELAY_DIVISIONS_THIRTY_SECOND = 15, ///< 1/32
  OMX_DELAY_DIVISIONS_THIRTY_SECOND_DOTTED = 16, ///< 1/32 dotted
  OMX_DELAY_DIVISIONS_THIRTY_SECOND_TRIPLET = 17, ///< 1/32 triplet
};
/** The number of ids of DELAY_DIVISIONS. */
#define OMX_DELAY_DIVISIONS_COUNT 18u
/** The id DELAY_DIVISIONS names as its default. */
#define OMX_DELAY_DIVISIONS_DEFAULT OMX_DELAY_DIVISIONS_QUARTER
/** The repeats' colour, in the order of the integer the kernel reads: digital, the delay as it shipped, bit for bit; tape, a one-pole low pass at DELAY_TAPE_LP_HZ in the loop and a slow wow (DELAY_TAPE_WOW_HZ, DELAY_TAPE_WOW_MS deep) on the read head; analog, a lower loop low pass at DELAY_ANALOG_LP_HZ and the soft saturation x / (1 + |x|) in the feedback. It comes up digital. A named id's value is its position. */
enum omx_delay_modes {
  OMX_DELAY_MODES_DIGITAL = 0, ///< Digital
  OMX_DELAY_MODES_TAPE = 1, ///< Tape
  OMX_DELAY_MODES_ANALOG = 2, ///< Analog
};
/** The number of ids of DELAY_MODES. */
#define OMX_DELAY_MODES_COUNT 3u
/** The id DELAY_MODES names as its default. */
#define OMX_DELAY_MODES_DEFAULT OMX_DELAY_MODES_DIGITAL
/** The FX delay's PING-PONG switch: on cross-feeds the two legs' feedback, so each leg's repeats come from the other leg's damped tap and the echoes bounce L\<-\>R (`omx_delay.h`'s `struct omx_fx_delay.pingpong`, read where the feedback is formed). A live control on the console's delay row (`DelayState.pingpong`) and on the omx-delay plugin. It comes up off. A named id's value is its position. */
enum omx_delay_pingpongs {
  OMX_DELAY_PINGPONGS_OFF = 0, ///< Off
  OMX_DELAY_PINGPONGS_ON = 1, ///< On
};
/** The number of ids of DELAY_PINGPONGS. */
#define OMX_DELAY_PINGPONGS_COUNT 2u
/** The id DELAY_PINGPONGS names as its default. */
#define OMX_DELAY_PINGPONGS_DEFAULT OMX_DELAY_PINGPONGS_OFF
/** Where the delay time comes from: free, the time control in ms; tempo, the division at the host's tempo, 60000 / BPM x the division's beats. While the host hands no tempo (no transport, or one without a tempo), a delay at tempo keeps the time control, exactly as free, and says so (omx_delay_instance_synced reads 0). It comes up free, so a fresh delay is the one that shipped. A named id's value is its position. */
enum omx_delay_syncs {
  OMX_DELAY_SYNCS_FREE = 0, ///< Free
  OMX_DELAY_SYNCS_TEMPO = 1, ///< Tempo
};
/** The number of ids of DELAY_SYNCS. */
#define OMX_DELAY_SYNCS_COUNT 2u
/** The id DELAY_SYNCS names as its default. */
#define OMX_DELAY_SYNCS_DEFAULT OMX_DELAY_SYNCS_FREE
/** Read tap 1's time as a factor of the delay time, in the order of the integer the kernel reads: the members of DELAY_TAP_FACTOR_NUMS / DELAY_TAP_FACTOR_DENS at the same position. The tap sits at time x num / den, bounded to the time travel's top (omx_fxdelay_tap_ms). It comes up at 1/2. A named id's value is its position. */
enum omx_delay_tap1_factors {
  OMX_DELAY_TAP1_FACTORS_QUARTER = 0, ///< 1/4
  OMX_DELAY_TAP1_FACTORS_THIRD = 1, ///< 1/3
  OMX_DELAY_TAP1_FACTORS_THREE_EIGHTHS = 2, ///< 3/8
  OMX_DELAY_TAP1_FACTORS_HALF = 3, ///< 1/2
  OMX_DELAY_TAP1_FACTORS_TWO_THIRDS = 4, ///< 2/3
  OMX_DELAY_TAP1_FACTORS_THREE_QUARTERS = 5, ///< 3/4
  OMX_DELAY_TAP1_FACTORS_ONE = 6, ///< 1
  OMX_DELAY_TAP1_FACTORS_FOUR_THIRDS = 7, ///< 4/3
  OMX_DELAY_TAP1_FACTORS_THREE_HALVES = 8, ///< 3/2
  OMX_DELAY_TAP1_FACTORS_TWO = 9, ///< 2
  OMX_DELAY_TAP1_FACTORS_THREE = 10, ///< 3
};
/** The number of ids of DELAY_TAP1_FACTORS. */
#define OMX_DELAY_TAP1_FACTORS_COUNT 11u
/** The id DELAY_TAP1_FACTORS names as its default. */
#define OMX_DELAY_TAP1_FACTORS_DEFAULT OMX_DELAY_TAP1_FACTORS_HALF
/** Read tap 2's time as a factor of the delay time, in the order of the integer the kernel reads: the members of DELAY_TAP_FACTOR_NUMS / DELAY_TAP_FACTOR_DENS at the same position. The tap sits at time x num / den, bounded to the time travel's top (omx_fxdelay_tap_ms). It comes up at 3/4. A named id's value is its position. */
enum omx_delay_tap2_factors {
  OMX_DELAY_TAP2_FACTORS_QUARTER = 0, ///< 1/4
  OMX_DELAY_TAP2_FACTORS_THIRD = 1, ///< 1/3
  OMX_DELAY_TAP2_FACTORS_THREE_EIGHTHS = 2, ///< 3/8
  OMX_DELAY_TAP2_FACTORS_HALF = 3, ///< 1/2
  OMX_DELAY_TAP2_FACTORS_TWO_THIRDS = 4, ///< 2/3
  OMX_DELAY_TAP2_FACTORS_THREE_QUARTERS = 5, ///< 3/4
  OMX_DELAY_TAP2_FACTORS_ONE = 6, ///< 1
  OMX_DELAY_TAP2_FACTORS_FOUR_THIRDS = 7, ///< 4/3
  OMX_DELAY_TAP2_FACTORS_THREE_HALVES = 8, ///< 3/2
  OMX_DELAY_TAP2_FACTORS_TWO = 9, ///< 2
  OMX_DELAY_TAP2_FACTORS_THREE = 10, ///< 3
};
/** The number of ids of DELAY_TAP2_FACTORS. */
#define OMX_DELAY_TAP2_FACTORS_COUNT 11u
/** The id DELAY_TAP2_FACTORS names as its default. */
#define OMX_DELAY_TAP2_FACTORS_DEFAULT OMX_DELAY_TAP2_FACTORS_THREE_QUARTERS
/** Read tap 3's time as a factor of the delay time, in the order of the integer the kernel reads: the members of DELAY_TAP_FACTOR_NUMS / DELAY_TAP_FACTOR_DENS at the same position. The tap sits at time x num / den, bounded to the time travel's top (omx_fxdelay_tap_ms). It comes up at 3/2. A named id's value is its position. */
enum omx_delay_tap3_factors {
  OMX_DELAY_TAP3_FACTORS_QUARTER = 0, ///< 1/4
  OMX_DELAY_TAP3_FACTORS_THIRD = 1, ///< 1/3
  OMX_DELAY_TAP3_FACTORS_THREE_EIGHTHS = 2, ///< 3/8
  OMX_DELAY_TAP3_FACTORS_HALF = 3, ///< 1/2
  OMX_DELAY_TAP3_FACTORS_TWO_THIRDS = 4, ///< 2/3
  OMX_DELAY_TAP3_FACTORS_THREE_QUARTERS = 5, ///< 3/4
  OMX_DELAY_TAP3_FACTORS_ONE = 6, ///< 1
  OMX_DELAY_TAP3_FACTORS_FOUR_THIRDS = 7, ///< 4/3
  OMX_DELAY_TAP3_FACTORS_THREE_HALVES = 8, ///< 3/2
  OMX_DELAY_TAP3_FACTORS_TWO = 9, ///< 2
  OMX_DELAY_TAP3_FACTORS_THREE = 10, ///< 3
};
/** The number of ids of DELAY_TAP3_FACTORS. */
#define OMX_DELAY_TAP3_FACTORS_COUNT 11u
/** The id DELAY_TAP3_FACTORS names as its default. */
#define OMX_DELAY_TAP3_FACTORS_DEFAULT OMX_DELAY_TAP3_FACTORS_THREE_HALVES
/** The compressor's detector oversampling: `auto` runs the control path at 4x only while the attack is faster than DETECTOR_OVERSAMPLE_AUTO_MS, `off` never, `4x` always (omx-dsp's OMX_DYN_OVS_AUTO, _OFF and _X4, in that order, so the index mapping holds). The ids are the console's wire vocabulary (`CompState.detectorOversampling`, what the dynamics row's OPTIONS publishes and a session stores), so the console re-exports this set; it comes up `auto`. A named id's value is its position. */
enum omx_detector_oversamplings {
  OMX_DETECTOR_OVERSAMPLINGS_AUTO = 0, ///< Auto
  OMX_DETECTOR_OVERSAMPLINGS_OFF = 1, ///< Off
  OMX_DETECTOR_OVERSAMPLINGS_4X = 2, ///< 4x
};
/** The number of ids of DETECTOR_OVERSAMPLINGS. */
#define OMX_DETECTOR_OVERSAMPLINGS_COUNT 3u
/** The id DETECTOR_OVERSAMPLINGS names as its default. */
#define OMX_DETECTOR_OVERSAMPLINGS_DEFAULT OMX_DETECTOR_OVERSAMPLINGS_AUTO
/** Where the compressor's detector listens: `feedforward` hears the slot's input (or its key), `feedback` hears the slot's own pre-make-up output DYN_FB_LOOP_DELAY_SAMPLES late, through one envelope stage, and a keyed slot stays feed-forward (omx-dsp's OMX_DYN_TOPOLOGY_FEEDFORWARD 0 and _FEEDBACK 1, in that order; the set is named for its control, so C spells them OMX_DETECTOR_TOPOLOGY_*). The console's comp row carries it (`detectorTopology`) and comes up `feedforward`, so a zeroed atom is the compressor every slot runs. A named id's value is its position. */
enum omx_detector_topology {
  OMX_DETECTOR_TOPOLOGY_FEEDFORWARD = 0, ///< Feedforward
  OMX_DETECTOR_TOPOLOGY_FEEDBACK = 1, ///< Feedback
};
/** The number of ids of DETECTOR_TOPOLOGY. */
#define OMX_DETECTOR_TOPOLOGY_COUNT 2u
/** The id DETECTOR_TOPOLOGY names as its default. */
#define OMX_DETECTOR_TOPOLOGY_DEFAULT OMX_DETECTOR_TOPOLOGY_FEEDFORWARD
/** The drive's AUTO-GAIN switch: on, the stage measures its own level change and matches it out (`omx_drive.h`'s `struct omx_drive.auto_gain`), so turning the drive up adds colour rather than loudness. A field of the console's drive row (`DriveState.autoGain`), on at come-up there and in omx-dsp's drive face. A named id's value is its position. */
enum omx_drive_auto_gains {
  OMX_DRIVE_AUTO_GAINS_OFF = 0, ///< Off
  OMX_DRIVE_AUTO_GAINS_ON = 1, ///< On
};
/** The number of ids of DRIVE_AUTO_GAINS. */
#define OMX_DRIVE_AUTO_GAINS_COUNT 2u
/** The id DRIVE_AUTO_GAINS names as its default. */
#define OMX_DRIVE_AUTO_GAINS_DEFAULT OMX_DRIVE_AUTO_GAINS_ON
/** Which part of the spectrum the drive's curve is fed, in the order of the integer the kernel reads. The split is complementary, so mix 0 stays bit-identical to the dry signal. A named id's value is its position. */
enum omx_drive_bands {
  OMX_DRIVE_BANDS_FULL = 0, ///< The id `full`.
  OMX_DRIVE_BANDS_LOW = 1, ///< The id `low`.
  OMX_DRIVE_BANDS_HIGH = 2, ///< The id `high`.
  OMX_DRIVE_BANDS_TILT = 3, ///< The id `tilt`.
};
/** The number of ids of DRIVE_BANDS. */
#define OMX_DRIVE_BANDS_COUNT 4u
/** The id DRIVE_BANDS names as its default. */
#define OMX_DRIVE_BANDS_DEFAULT OMX_DRIVE_BANDS_FULL
/** The drive's shaper curves, in the order of the integer the kernel reads. Exciter runs the soft shape in a parallel-sum topology. A named id's value is its position. */
enum omx_drive_curves {
  OMX_DRIVE_CURVES_SOFT = 0, ///< The id `soft`.
  OMX_DRIVE_CURVES_TAPE = 1, ///< The id `tape`.
  OMX_DRIVE_CURVES_TUBE = 2, ///< The id `tube`.
  OMX_DRIVE_CURVES_EXCITER = 3, ///< The id `exciter`.
};
/** The number of ids of DRIVE_CURVES. */
#define OMX_DRIVE_CURVES_COUNT 4u
/** The id DRIVE_CURVES names as its default. */
#define OMX_DRIVE_CURVES_DEFAULT OMX_DRIVE_CURVES_SOFT
/** The drive's post-curve HIGH-FREQUENCY ROLL-OFF: `off` (the section does not run: `omx_drive.h`'s `struct omx_drive.hf_on` is 0), `12000` or `16000`, each carrying its corner in Hz as its value and `off` none (operator ruling, 2026-10-13: the choice is an enumeration and the frequency is data of each member, so no 0 Hz doubles as a sentinel). The ids are the console's drive-row wire vocabulary (`DriveState.hfRolloff`, what the drive row's OPTIONS publishes and a session stores), so the console re-exports this set; a kernel reads the corner from the id's value. It comes up off, as omx-dsp's drive face does. A named id's value is its position. */
enum omx_drive_hf_rolloffs {
  OMX_DRIVE_HF_ROLLOFFS_OFF = 0, ///< Off
  OMX_DRIVE_HF_ROLLOFFS_12000 = 1, ///< 12 kHz
  OMX_DRIVE_HF_ROLLOFFS_16000 = 2, ///< 16 kHz
};
/** The number of ids of DRIVE_HF_ROLLOFFS. */
#define OMX_DRIVE_HF_ROLLOFFS_COUNT 3u
/** The id DRIVE_HF_ROLLOFFS names as its default. */
#define OMX_DRIVE_HF_ROLLOFFS_DEFAULT OMX_DRIVE_HF_ROLLOFFS_OFF
/** The unit of the values the ids of DRIVE_HF_ROLLOFFS carry. */
#define OMX_DRIVE_HF_ROLLOFFS_UNIT "Hz"
/** The value the id `12000` of DRIVE_HF_ROLLOFFS carries (OMX_DRIVE_HF_ROLLOFFS_UNIT). */
#define OMX_DRIVE_HF_ROLLOFFS_12000_VALUE 12000.0f
/** The value the id `16000` of DRIVE_HF_ROLLOFFS carries (OMX_DRIVE_HF_ROLLOFFS_UNIT). */
#define OMX_DRIVE_HF_ROLLOFFS_16000_VALUE 16000.0f
/**
 * The value an id of DRIVE_HF_ROLLOFFS carries.
 * @param id an enumerator of enum omx_drive_hf_rolloffs
 * @param out where the value is written, when the id carries one; the caller owns it
 * @return 1 when the id carries a value, written to `out`; 0 for an id that carries none, `out` untouched
 */
static inline int omx_drive_hf_rolloffs_value(int id, float *out) {
  switch (id) {
  case OMX_DRIVE_HF_ROLLOFFS_12000: *out = OMX_DRIVE_HF_ROLLOFFS_12000_VALUE; return 1;
  case OMX_DRIVE_HF_ROLLOFFS_16000: *out = OMX_DRIVE_HF_ROLLOFFS_16000_VALUE; return 1;
  default: return 0;
  }
}
/** The drive's OVERSAMPLING factor: the shaper runs inside the console's one oversampler at 1x, 2x or 4x, and the id is the factor (`omx_drive.h`'s `omx_drive_factor_of` member set, `struct omx_drive.os_factor`). A field of the console's drive row (`DriveState.oversample`, the console's DRIVE_OVERSAMPLE_FACTORS; the set is named for its control), 4 at come-up there and in omx-dsp's drive face (OMX_DRIVE_INSTANCE_FACTOR, the oversampler's largest). A numeric id is its own value. */
enum omx_drive_oversamples {
  OMX_DRIVE_OVERSAMPLES_1 = 1, ///< 1x
  OMX_DRIVE_OVERSAMPLES_2 = 2, ///< 2x
  OMX_DRIVE_OVERSAMPLES_4 = 4, ///< 4x
};
/** The number of ids of DRIVE_OVERSAMPLES. */
#define OMX_DRIVE_OVERSAMPLES_COUNT 3u
/** The id DRIVE_OVERSAMPLES names as its default. */
#define OMX_DRIVE_OVERSAMPLES_DEFAULT OMX_DRIVE_OVERSAMPLES_4
/** The drive's STEREO LINK switch: on, ONE auto-gain path is driven by both legs' summed energy, so the image cannot shift under the level match; off, each leg matches its own (`omx_drive.h`'s `struct omx_drive.stereo_link`). A field of the console's drive row (`DriveState.stereoLink`), on at come-up there and in omx-dsp's drive face. A named id's value is its position. */
enum omx_drive_stereo_links {
  OMX_DRIVE_STEREO_LINKS_OFF = 0, ///< Off
  OMX_DRIVE_STEREO_LINKS_ON = 1, ///< On
};
/** The number of ids of DRIVE_STEREO_LINKS. */
#define OMX_DRIVE_STEREO_LINKS_COUNT 2u
/** The id DRIVE_STEREO_LINKS names as its default. */
#define OMX_DRIVE_STEREO_LINKS_DEFAULT OMX_DRIVE_STEREO_LINKS_ON
/** A band's switch. Off, the band is not in the EQ's bank and its dynamics do not run; on, its static section runs, parked at 0 dB as an EQ band is, and its dynamics run as its dynMode says. A band comes up off, as an EQ band does: the ids and the come-up value are EQ_BAND_ONS's, the EQ's own band switch, and the face holds the two equal at compile time. A named id's value is its position. */
enum omx_dyneq_band_ons {
  OMX_DYNEQ_BAND_ONS_OFF = 0, ///< Off
  OMX_DYNEQ_BAND_ONS_ON = 1, ///< On
};
/** The number of ids of DYNEQ_BAND_ONS. */
#define OMX_DYNEQ_BAND_ONS_COUNT 2u
/** The id DYNEQ_BAND_ONS names as its default. */
#define OMX_DYNEQ_BAND_ONS_DEFAULT OMX_DYNEQ_BAND_ONS_OFF
/** The band's shape, in the order of the integer the kernel reads, which is the order of the first three EQ_BAND_TYPES: a bell, a low shelf, a high shelf. A bell's dynamics act through the cookbook bandpass at its centre, a shelf's through half of the first-order allpass at its corner (omx_band_dyn_design). A named id's value is its position. */
enum omx_dyneq_band_types {
  OMX_DYNEQ_BAND_TYPES_BELL = 0, ///< Bell
  OMX_DYNEQ_BAND_TYPES_LOW_SHELF = 1, ///< Low Shelf
  OMX_DYNEQ_BAND_TYPES_HIGH_SHELF = 2, ///< High Shelf
};
/** The number of ids of DYNEQ_BAND_TYPES. */
#define OMX_DYNEQ_BAND_TYPES_COUNT 3u
/** The id DYNEQ_BAND_TYPES names as its default. */
#define OMX_DYNEQ_BAND_TYPES_DEFAULT OMX_DYNEQ_BAND_TYPES_BELL
/** Whether and when the band's dynamics act, in the order of the integer the kernel reads: off, the band is its static section alone; down, it acts above its threshold, when the band is loud (band_dyn's above); up, it acts below its threshold, when the band is quiet (band_dyn's below). The range's sign says whether it cuts or boosts. A band comes up off, so a fresh dynamic EQ is an EQ. A named id's value is its position. */
enum omx_dyneq_dyn_modes {
  OMX_DYNEQ_DYN_MODES_OFF = 0, ///< Off
  OMX_DYNEQ_DYN_MODES_DOWN = 1, ///< Down
  OMX_DYNEQ_DYN_MODES_UP = 2, ///< Up
};
/** The number of ids of DYNEQ_DYN_MODES. */
#define OMX_DYNEQ_DYN_MODES_COUNT 3u
/** The id DYNEQ_DYN_MODES names as its default. */
#define OMX_DYNEQ_DYN_MODES_DEFAULT OMX_DYNEQ_DYN_MODES_OFF
/** While on, the output is what the band's detector hears, its section B over the signal reaching the band, in place of the EQ's output; with more than one band listening, their sum. It comes up off. A named id's value is its position. */
enum omx_dyneq_listens {
  OMX_DYNEQ_LISTENS_OFF = 0, ///< Off
  OMX_DYNEQ_LISTENS_ON = 1, ///< On
};
/** The number of ids of DYNEQ_LISTENS. */
#define OMX_DYNEQ_LISTENS_COUNT 2u
/** The id DYNEQ_LISTENS names as its default. */
#define OMX_DYNEQ_LISTENS_DEFAULT OMX_DYNEQ_LISTENS_OFF
/** A band's switch. Off, the band is not in the EQ's bank (the slots close up); on, it runs, parked when it is a gain-carrying shape at 0 dB. A band comes up off (operator ruling, 2026-10-13): an omx-eq plugin brings every band up off so a racked instance is a wire, and the console's channel EQ reads its come-up value here. A stored session keeps its own band states; only a fresh band comes up off. A named id's value is its position. */
enum omx_eq_band_ons {
  OMX_EQ_BAND_ONS_OFF = 0, ///< Off
  OMX_EQ_BAND_ONS_ON = 1, ///< On
};
/** The number of ids of EQ_BAND_ONS. */
#define OMX_EQ_BAND_ONS_COUNT 2u
/** The id EQ_BAND_ONS names as its default. */
#define OMX_EQ_BAND_ONS_DEFAULT OMX_EQ_BAND_ONS_OFF
/** The generic EQ band shapes, in the order a picker offers them and in the order of the integer a plugin host sees on a band type port. The id is the name; the position is the value. A bell is the shape a band added by the operator starts as. A named id's value is its position. */
enum omx_eq_band_types {
  OMX_EQ_BAND_TYPES_BELL = 0, ///< Bell
  OMX_EQ_BAND_TYPES_LOW_SHELF = 1, ///< Low Shelf
  OMX_EQ_BAND_TYPES_HIGH_SHELF = 2, ///< High Shelf
  OMX_EQ_BAND_TYPES_NOTCH = 3, ///< Notch
  OMX_EQ_BAND_TYPES_ALLPASS1 = 4, ///< All-pass 1st
  OMX_EQ_BAND_TYPES_ALLPASS2 = 5, ///< All-pass 2nd
};
/** The number of ids of EQ_BAND_TYPES. */
#define OMX_EQ_BAND_TYPES_COUNT 6u
/** The id EQ_BAND_TYPES names as its default. */
#define OMX_EQ_BAND_TYPES_DEFAULT OMX_EQ_BAND_TYPES_BELL
/** The HPF's switch: off, the filter is not in the EQ's bank at all; on, its one or two Butterworth sections run after the bands. It comes up off, on the console's channel EQ and in omx-dsp's EQ face (see EQ_PASS_FILTER_DEFAULTS). A named id's value is its position. */
enum omx_eq_hpf_ons {
  OMX_EQ_HPF_ONS_OFF = 0, ///< Off
  OMX_EQ_HPF_ONS_ON = 1, ///< On
};
/** The number of ids of EQ_HPF_ONS. */
#define OMX_EQ_HPF_ONS_COUNT 2u
/** The id EQ_HPF_ONS names as its default. */
#define OMX_EQ_HPF_ONS_DEFAULT OMX_EQ_HPF_ONS_OFF
/** The LPF's switch: off, the filter is not in the EQ's bank at all; on, its one or two Butterworth sections run after the bands and the HPF. It comes up off, on the console's channel EQ and in omx-dsp's EQ face (see EQ_PASS_FILTER_DEFAULTS). A named id's value is its position. */
enum omx_eq_lpf_ons {
  OMX_EQ_LPF_ONS_OFF = 0, ///< Off
  OMX_EQ_LPF_ONS_ON = 1, ///< On
};
/** The number of ids of EQ_LPF_ONS. */
#define OMX_EQ_LPF_ONS_COUNT 2u
/** The id EQ_LPF_ONS names as its default. */
#define OMX_EQ_LPF_ONS_DEFAULT OMX_EQ_LPF_ONS_OFF
/** cut runs the planted bands on the audio; report keeps the audio bit-exact while the detector and the placement run and the report carries what cut would plant. It comes up at cut: a host with no controller gets the corrector. The integer the shell reads is the id's position. A named id's value is its position. */
enum omx_fbs_modes {
  OMX_FBS_MODES_CUT = 0, ///< The id `cut`.
  OMX_FBS_MODES_REPORT = 1, ///< The id `report`.
};
/** The number of ids of FBS_MODES. */
#define OMX_FBS_MODES_COUNT 2u
/** The id FBS_MODES names as its default. */
#define OMX_FBS_MODES_DEFAULT OMX_FBS_MODES_CUT
/** Which pool the finds go to: fixed (ring-out: a true notch that never lifts) or live (a rampable bell with the staged release). The integer the shell reads is the id's position, OmxFbsPool's. A named id's value is its position. */
enum omx_fbs_pools {
  OMX_FBS_POOLS_FIXED = 0, ///< The id `fixed`.
  OMX_FBS_POOLS_LIVE = 1, ///< The id `live`.
};
/** The number of ids of FBS_POOLS. */
#define OMX_FBS_POOLS_COUNT 2u
/** The id FBS_POOLS names as its default. */
#define OMX_FBS_POOLS_DEFAULT OMX_FBS_POOLS_LIVE
/** The pass-filter slopes the EQ offers, in dB per octave: the HPF's slope, and by use the LPF's. The id is the slope: 12 is one maximally flat section, 24 the two-section Butterworth. A numeric id is its own value. */
enum omx_filter_slopes {
  OMX_FILTER_SLOPES_12 = 12, ///< The id `12`.
  OMX_FILTER_SLOPES_24 = 24, ///< The id `24`.
};
/** The number of ids of FILTER_SLOPES. */
#define OMX_FILTER_SLOPES_COUNT 2u
/** The id FILTER_SLOPES names as its default. */
#define OMX_FILTER_SLOPES_DEFAULT OMX_FILTER_SLOPES_12
/** While on, the outputs carry what the gate's detector reads: the key (the sidechain, or the gate's own mono sum) after the key filter and the tilt on both outputs, or, for a self-keyed gate with no key filter and a flat tilt, each input leg on its own output, since the detector then reads the legs. The gate keeps running underneath. It comes up off. A named id's value is its position. */
enum omx_gate_key_listens {
  OMX_GATE_KEY_LISTENS_OFF = 0, ///< Off
  OMX_GATE_KEY_LISTENS_ON = 1, ///< On
};
/** The number of ids of GATE_KEY_LISTENS. */
#define OMX_GATE_KEY_LISTENS_COUNT 2u
/** The id GATE_KEY_LISTENS names as its default. */
#define OMX_GATE_KEY_LISTENS_DEFAULT OMX_GATE_KEY_LISTENS_OFF
/** Where the gate listens: its own input (self), or the key it is handed (sidechain), in the order of the integer the key port carries. A named id's value is its position. */
enum omx_gate_key_sources {
  OMX_GATE_KEY_SOURCES_SELF = 0, ///< The id `self`.
  OMX_GATE_KEY_SOURCES_SIDECHAIN = 1, ///< The id `sidechain`.
};
/** The number of ids of GATE_KEY_SOURCES. */
#define OMX_GATE_KEY_SOURCES_COUNT 2u
/** The generator kinds, in surface order: its position is the integer omx-dsp's omx_gen_fill takes as its kind (OMX_GEN_OSC, OMX_GEN_WHITE, OMX_GEN_PINK). A named id's value is its position. */
enum omx_generator_kinds {
  OMX_GENERATOR_KINDS_OSCILLATOR = 0, ///< The id `oscillator`.
  OMX_GENERATOR_KINDS_WHITE = 1, ///< The id `white`.
  OMX_GENERATOR_KINDS_PINK = 2, ///< The id `pink`.
};
/** The number of ids of GENERATOR_KINDS. */
#define OMX_GENERATOR_KINDS_COUNT 3u
/** How the graphic EQ designs its sections from the faders, in the order of the integer the kernel reads (omx-dsp's enum omx_geq_mode): `standard` gives each section its fader as its gain, the classic proportional build-up; `true` solves the band interaction so the summed curve meets the faders at the centres, each section gain clamped at GEQ_DESIGN_GAIN_MAX_DB. A fresh instance comes up `standard`. A named id's value is its position. */
enum omx_geq_modes {
  OMX_GEQ_MODES_STANDARD = 0, ///< The id `standard`.
  OMX_GEQ_MODES_TRUE = 1, ///< The id `true`.
};
/** The number of ids of GEQ_MODES. */
#define OMX_GEQ_MODES_COUNT 2u
/** The id GEQ_MODES names as its default. */
#define OMX_GEQ_MODES_DEFAULT OMX_GEQ_MODES_STANDARD
/** The attack, in the order of the integer the kernel reads: GLUE_ATTACK_MS. It comes up at 10 ms, which lets a kick through before the bus moves. A named id's value is its position. */
enum omx_glue_attacks {
  OMX_GLUE_ATTACKS_A01 = 0, ///< 0.1 ms
  OMX_GLUE_ATTACKS_A03 = 1, ///< 0.3 ms
  OMX_GLUE_ATTACKS_A1 = 2, ///< 1 ms
  OMX_GLUE_ATTACKS_A3 = 3, ///< 3 ms
  OMX_GLUE_ATTACKS_A10 = 4, ///< 10 ms
  OMX_GLUE_ATTACKS_A30 = 5, ///< 30 ms
};
/** The number of ids of GLUE_ATTACKS. */
#define OMX_GLUE_ATTACKS_COUNT 6u
/** The id GLUE_ATTACKS names as its default. */
#define OMX_GLUE_ATTACKS_DEFAULT OMX_GLUE_ATTACKS_A10
/** The ratio, the detents a bus compressor offers, in the order of the integer the kernel reads: GLUE_RATIO_VALUES. A named id's value is its position. */
enum omx_glue_ratios {
  OMX_GLUE_RATIOS_R2 = 0, ///< 2:1
  OMX_GLUE_RATIOS_R4 = 1, ///< 4:1
  OMX_GLUE_RATIOS_R10 = 2, ///< 10:1
};
/** The number of ids of GLUE_RATIOS. */
#define OMX_GLUE_RATIOS_COUNT 3u
/** The id GLUE_RATIOS names as its default. */
#define OMX_GLUE_RATIOS_DEFAULT OMX_GLUE_RATIOS_R2
/** The release, in the order of the integer the kernel reads: GLUE_RELEASE_MS, then auto, the compressor's program-dependent release (PROGRAM_RELEASE_PROFILES.auto). A named id's value is its position. */
enum omx_glue_releases {
  OMX_GLUE_RELEASES_R100 = 0, ///< 0.1 s
  OMX_GLUE_RELEASES_R300 = 1, ///< 0.3 s
  OMX_GLUE_RELEASES_R600 = 2, ///< 0.6 s
  OMX_GLUE_RELEASES_R1200 = 3, ///< 1.2 s
  OMX_GLUE_RELEASES_AUTO = 4, ///< Auto
};
/** The number of ids of GLUE_RELEASES. */
#define OMX_GLUE_RELEASES_COUNT 5u
/** The id GLUE_RELEASES names as its default. */
#define OMX_GLUE_RELEASES_DEFAULT OMX_GLUE_RELEASES_AUTO
/** cut runs the planted bands on the audio; report keeps the audio bit-exact while the analysis runs and the report carries what cut would plant. It comes up at cut: a host with no controller gets the corrector. The integer the shell reads is the id's position. A named id's value is its position. */
enum omx_hrp_modes {
  OMX_HRP_MODES_CUT = 0, ///< The id `cut`.
  OMX_HRP_MODES_REPORT = 1, ///< The id `report`.
};
/** The number of ids of HRP_MODES. */
#define OMX_HRP_MODES_COUNT 2u
/** The id HRP_MODES names as its default. */
#define OMX_HRP_MODES_DEFAULT OMX_HRP_MODES_CUT
/** The instrument families a channel may be assigned, a broad prior and never an identification: HRP narrows where its detector looks for a fundamental and how many voices it looks for by the family (HRP_FAMILY_SEARCH), and nothing else. The order is the integer that crosses a port or a process boundary (omx-dsp's OmxHrpFamily, the HRP live face's family port). A channel with no family carries -1, which no id names, and the detector then runs its own wide defaults; so the set has no come-up id. (since 2.14.0) A named id's value is its position. */
enum omx_instrument_families {
  OMX_INSTRUMENT_FAMILIES_DRUMS = 0, ///< The id `drums`.
  OMX_INSTRUMENT_FAMILIES_PERCUSSION = 1, ///< The id `percussion`.
  OMX_INSTRUMENT_FAMILIES_BASS = 2, ///< The id `bass`.
  OMX_INSTRUMENT_FAMILIES_GUITAR = 3, ///< The id `guitar`.
  OMX_INSTRUMENT_FAMILIES_KEYS = 4, ///< The id `keys`.
  OMX_INSTRUMENT_FAMILIES_VOCALS = 5, ///< The id `vocals`.
  OMX_INSTRUMENT_FAMILIES_BRASS = 6, ///< The id `brass`.
  OMX_INSTRUMENT_FAMILIES_WIND = 7, ///< The id `wind`.
  OMX_INSTRUMENT_FAMILIES_STRINGS = 8, ///< The id `strings`.
  OMX_INSTRUMENT_FAMILIES_PLAYBACK = 9, ///< The id `playback`.
};
/** The number of ids of INSTRUMENT_FAMILIES. */
#define OMX_INSTRUMENT_FAMILIES_COUNT 10u
/** Which way the leveler works, in the order of the integer the kernel reads: down, a compressor over the loud passages; up, an upward compressor under the quiet ones (omx_dyn's UPWARD law); both, the two in series, down first. Both comes up. A named id's value is its position. */
enum omx_leveler_directions {
  OMX_LEVELER_DIRECTIONS_DOWN = 0, ///< Down
  OMX_LEVELER_DIRECTIONS_UP = 1, ///< Up
  OMX_LEVELER_DIRECTIONS_BOTH = 2, ///< Both
};
/** The number of ids of LEVELER_DIRECTIONS. */
#define OMX_LEVELER_DIRECTIONS_COUNT 3u
/** The id LEVELER_DIRECTIONS names as its default. */
#define OMX_LEVELER_DIRECTIONS_DEFAULT OMX_LEVELER_DIRECTIONS_BOTH
/** How fast the leveler follows, in the order of the integer the kernel reads: slow, medium, fast; each an attack and a release from LEVELER_SPEED_ATTACK_MS and LEVELER_SPEED_RELEASE_MS. Medium comes up. A named id's value is its position. */
enum omx_leveler_speeds {
  OMX_LEVELER_SPEEDS_SLOW = 0, ///< Slow
  OMX_LEVELER_SPEEDS_MEDIUM = 1, ///< Medium
  OMX_LEVELER_SPEEDS_FAST = 2, ///< Fast
};
/** The number of ids of LEVELER_SPEEDS. */
#define OMX_LEVELER_SPEEDS_COUNT 3u
/** The id LEVELER_SPEEDS names as its default. */
#define OMX_LEVELER_SPEEDS_DEFAULT OMX_LEVELER_SPEEDS_MEDIUM
/** What the loudness is measured over, in the order of the integer the kernel reads: momentary, 400 ms; short, 3 s. A named id's value is its position. */
enum omx_loudlevel_windows {
  OMX_LOUDLEVEL_WINDOWS_MOMENTARY = 0, ///< Momentary
  OMX_LOUDLEVEL_WINDOWS_SHORT = 1, ///< Short-term
};
/** The number of ids of LOUDLEVEL_WINDOWS. */
#define OMX_LOUDLEVEL_WINDOWS_COUNT 2u
/** The id LOUDLEVEL_WINDOWS names as its default. */
#define OMX_LOUDLEVEL_WINDOWS_DEFAULT OMX_LOUDLEVEL_WINDOWS_SHORT
/** The band's bypass: on, the band's compressor is the unity one and the band passes at the gain of the tree at rest. Named bandBypass so it is not the face's own bypass. It comes up off. A named id's value is its position. */
enum omx_mbcomp_band_bypasses {
  OMX_MBCOMP_BAND_BYPASSES_OFF = 0, ///< Off
  OMX_MBCOMP_BAND_BYPASSES_ON = 1, ///< On
};
/** The number of ids of MBCOMP_BAND_BYPASSES. */
#define OMX_MBCOMP_BAND_BYPASSES_COUNT 2u
/** The id MBCOMP_BAND_BYPASSES names as its default. */
#define OMX_MBCOMP_BAND_BYPASSES_DEFAULT OMX_MBCOMP_BAND_BYPASSES_OFF
/** The band's solo: while any band is soloed, only the soloed bands reach the sum. It comes up off. A named id's value is its position. */
enum omx_mbcomp_solos {
  OMX_MBCOMP_SOLOS_OFF = 0, ///< Off
  OMX_MBCOMP_SOLOS_ON = 1, ///< On
};
/** The number of ids of MBCOMP_SOLOS. */
#define OMX_MBCOMP_SOLOS_COUNT 2u
/** The id MBCOMP_SOLOS names as its default. */
#define OMX_MBCOMP_SOLOS_DEFAULT OMX_MBCOMP_SOLOS_OFF
/** What a subscriber that decimates a meter stream keeps of the frames it drops, one per kind in METER_KINDS order, as the engine's slot keeps between two reads: a `level` keeps the `peak` (the largest value), a `gainReduction` the `deepest` (the smallest dB). Never the newest. (since 3.0.0) A named id's value is its position. */
enum omx_meter_decimation {
  OMX_METER_DECIMATION_PEAK = 0, ///< The id `peak`.
  OMX_METER_DECIMATION_DEEPEST = 1, ///< The id `deepest`.
};
/** The number of ids of METER_DECIMATION. */
#define OMX_METER_DECIMATION_COUNT 2u
/** What a meter measures, as a plugin's descriptor and the panel description spell it: `level`, a signal level (its measurement one of METER_MEASUREMENTS), and `gainReduction`, the gain a dynamics stage takes off, dB at or below 0. The wire spells them `level` and `gain-reduction`. `rms`, `vu` and `ppm` are not kinds. (since 3.0.0) A named id's value is its position. */
enum omx_meter_kinds {
  OMX_METER_KINDS_LEVEL = 0, ///< The id `level`.
  OMX_METER_KINDS_GAIN_REDUCTION = 1, ///< The id `gainReduction`.
};
/** The number of ids of METER_KINDS. */
#define OMX_METER_KINDS_COUNT 2u
/** The measurement of a `level` meter: the sample peak (`peak`, dBFS), the true peak (`truePeak`, dBTP), and the loudness measures of omx_loudness.h: momentary, short-term and integrated loudness (`lufsM`, `lufsS`, `lufsI`, LUFS) and the loudness range (`lra`, LU). The wire spells them `peak`, `true-peak`, `lufs-m`, `lufs-s`, `lufs-i` and `lra`. A foreign meter's measurement is null, never guessed. (since 3.0.0) A named id's value is its position. */
enum omx_meter_measurements {
  OMX_METER_MEASUREMENTS_PEAK = 0, ///< The id `peak`.
  OMX_METER_MEASUREMENTS_TRUE_PEAK = 1, ///< The id `truePeak`.
  OMX_METER_MEASUREMENTS_LUFS_M = 2, ///< The id `lufsM`.
  OMX_METER_MEASUREMENTS_LUFS_S = 3, ///< The id `lufsS`.
  OMX_METER_MEASUREMENTS_LUFS_I = 4, ///< The id `lufsI`.
  OMX_METER_MEASUREMENTS_LRA = 5, ///< The id `lra`.
};
/** The number of ids of METER_MEASUREMENTS. */
#define OMX_METER_MEASUREMENTS_COUNT 6u
/** The unit a meter's values are in, its PARAM_UNITS id (a surface captions the scale with the id's x-symbol): dBFS for a sample peak, dBTP for a true peak, LUFS for a loudness, LU for a loudness range, dB for a gain reduction. (since 3.0.0) A named id's value is its position. */
enum omx_meter_units {
  OMX_METER_UNITS_D_BFS = 0, ///< The id `dBFS`.
  OMX_METER_UNITS_D_BTP = 1, ///< The id `dBTP`.
  OMX_METER_UNITS_LUFS = 2, ///< The id `LUFS`.
  OMX_METER_UNITS_LU = 3, ///< The id `LU`.
  OMX_METER_UNITS_D_B = 4, ///< The id `dB`.
};
/** The number of ids of METER_UNITS. */
#define OMX_METER_UNITS_COUNT 5u
/** The reverb's algorithms, in the order of the integer the kernel reads. Reverse and gated are the room network handed a window and a gate. A named id's value is its position. */
enum omx_reverb_algorithms {
  OMX_REVERB_ALGORITHMS_ROOM = 0, ///< The id `room`.
  OMX_REVERB_ALGORITHMS_PLATE = 1, ///< The id `plate`.
  OMX_REVERB_ALGORITHMS_HALL = 2, ///< The id `hall`.
  OMX_REVERB_ALGORITHMS_REVERSE = 3, ///< The id `reverse`.
  OMX_REVERB_ALGORITHMS_GATED = 4, ///< The id `gated`.
};
/** The number of ids of REVERB_ALGORITHMS. */
#define OMX_REVERB_ALGORITHMS_COUNT 5u
/** The wet's de-esser: on, the de-esser kernel in split mode on the wet, at deessFreqHz and deessThresholdDb with the de-esser's own come-up width, ratio, range, attack and release, so an s does not splash the tail. It comes up off. A named id's value is its position. */
enum omx_reverb_wet_deesses {
  OMX_REVERB_WET_DEESSES_OFF = 0, ///< Off
  OMX_REVERB_WET_DEESSES_ON = 1, ///< On
};
/** The number of ids of REVERB_WET_DEESSES. */
#define OMX_REVERB_WET_DEESSES_COUNT 2u
/** The id REVERB_WET_DEESSES names as its default. */
#define OMX_REVERB_WET_DEESSES_DEFAULT OMX_REVERB_WET_DEESSES_OFF
/** The rotary speaker's speed settings, in the order of the integer the kernel reads: the rotors stopped, at their chorale (slow) rate, or at their tremolo (fast) rate. A named id's value is its position. */
enum omx_rotary_speeds {
  OMX_ROTARY_SPEEDS_STOP = 0, ///< The id `stop`.
  OMX_ROTARY_SPEEDS_SLOW = 1, ///< The id `slow`.
  OMX_ROTARY_SPEEDS_FAST = 2, ///< The id `fast`.
};
/** The number of ids of ROTARY_SPEEDS. */
#define OMX_ROTARY_SPEEDS_COUNT 3u
/** The tracking range, in the order of the integer the kernel reads: low (a bass, 35 to 140 Hz), mid (70 to 280 Hz), high (140 to 560 Hz); each a row of SUBOCT_RANGE_LO_HZ, SUBOCT_RANGE_HI_HZ, SUBOCT_POST1_HZ and SUBOCT_POST2_HZ. Low comes up. A named id's value is its position. */
enum omx_suboct_ranges {
  OMX_SUBOCT_RANGES_LOW = 0, ///< Low
  OMX_SUBOCT_RANGES_MID = 1, ///< Mid
  OMX_SUBOCT_RANGES_HIGH = 2, ///< High
};
/** The number of ids of SUBOCT_RANGES. */
#define OMX_SUBOCT_RANGES_COUNT 3u
/** The id SUBOCT_RANGES names as its default. */
#define OMX_SUBOCT_RANGES_DEFAULT OMX_SUBOCT_RANGES_LOW
/** What the tremolo's one oscillator modulates, in the order of the integer the kernel reads: the level of both legs in phase (tremolo), or the balance law's position with the legs in opposition (pan). A named id's value is its position. */
enum omx_tremolo_modes {
  OMX_TREMOLO_MODES_TREMOLO = 0, ///< The id `tremolo`.
  OMX_TREMOLO_MODES_PAN = 1, ///< The id `pan`.
};
/** The number of ids of TREMOLO_MODES. */
#define OMX_TREMOLO_MODES_COUNT 2u
/** The id TREMOLO_MODES names as its default. */
#define OMX_TREMOLO_MODES_DEFAULT OMX_TREMOLO_MODES_TREMOLO
/** The selectable oscillator waveforms, in surface order: its position is the integer omx-dsp's omx_gen_osc takes (OMX_WAVE_SINE, OMX_WAVE_SQUARE, OMX_WAVE_SAW, OMX_WAVE_TRIANGLE). A named id's value is its position. */
enum omx_waveforms {
  OMX_WAVEFORMS_SINE = 0, ///< The id `sine`.
  OMX_WAVEFORMS_SQUARE = 1, ///< The id `square`.
  OMX_WAVEFORMS_SAW = 2, ///< The id `saw`.
  OMX_WAVEFORMS_TRIANGLE = 3, ///< The id `triangle`.
};
/** The number of ids of WAVEFORMS. */
#define OMX_WAVEFORMS_COUNT 4u
/** Whether the low end is summed to mono: on splits the side with a Linkwitz-Riley crossover at bassMonoHz (omx_xover, LR4) and keeps its high band only, the mid through the crossover's all-pass so the bands stay in phase. Off comes up. A named id's value is its position. */
enum omx_width_bass_monos {
  OMX_WIDTH_BASS_MONOS_OFF = 0, ///< Off
  OMX_WIDTH_BASS_MONOS_ON = 1, ///< On
};
/** The number of ids of WIDTH_BASS_MONOS. */
#define OMX_WIDTH_BASS_MONOS_COUNT 2u
/** The id WIDTH_BASS_MONOS names as its default. */
#define OMX_WIDTH_BASS_MONOS_DEFAULT OMX_WIDTH_BASS_MONOS_OFF
/** A pseudo-stereo for a mono source, in the order of the integer the kernel reads: off; comb, the complementary comb L = M + g M[n - d], R = M - g M[n - d] with g WIDTH_COMB_GAIN, whose mono sum is the mid exactly; haas, the right leg delayed by pseudoMs. Off comes up. A named id's value is its position. */
enum omx_width_pseudos {
  OMX_WIDTH_PSEUDOS_OFF = 0, ///< Off
  OMX_WIDTH_PSEUDOS_COMB = 1, ///< Comb
  OMX_WIDTH_PSEUDOS_HAAS = 2, ///< Haas
};
/** The number of ids of WIDTH_PSEUDOS. */
#define OMX_WIDTH_PSEUDOS_COUNT 3u
/** The id WIDTH_PSEUDOS names as its default. */
#define OMX_WIDTH_PSEUDOS_DEFAULT OMX_WIDTH_PSEUDOS_OFF

/* Every kernel's readouts, the meters a host draws (docs/readouts.md): an enum of the readouts at their position,
 * `OMX_<KERNEL>_READOUT_COUNT`, and per readout its display range (_MIN, _MAX), _RATE_HZ, _SCALE (an OMX_PARAM_SCALES_*
 * member) and _UNIT. */
/** The readouts of the balancer kernel, the meters a host draws, each at its position. */
enum omx_balancer_readouts {
  OMX_BALANCER_READOUT_BAND1_GAIN_DB = 0, ///< Band 1's gain now, dB. (dB)
  OMX_BALANCER_READOUT_BAND2_GAIN_DB = 1, ///< Band 2's gain now, dB. (dB)
  OMX_BALANCER_READOUT_BAND3_GAIN_DB = 2, ///< Band 3's gain now, dB. (dB)
  OMX_BALANCER_READOUT_BAND4_GAIN_DB = 3, ///< Band 4's gain now, dB. (dB)
};
/** The number of readouts of the balancer kernel. */
#define OMX_BALANCER_READOUT_COUNT 4u
/** The least value the `band1GainDb` readout shows. (dB) */
#define OMX_BALANCER_READOUT_BAND1_GAIN_DB_MIN -12.0f
/** The greatest value the `band1GainDb` readout shows. (dB) */
#define OMX_BALANCER_READOUT_BAND1_GAIN_DB_MAX 12.0f
/** How often the `band1GainDb` readout is published (Hz). */
#define OMX_BALANCER_READOUT_BAND1_GAIN_DB_RATE_HZ 50
/** How a host draws the `band1GainDb` readout, an enum omx_param_scales member. */
#define OMX_BALANCER_READOUT_BAND1_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band1GainDb` readout, as a string. */
#define OMX_BALANCER_READOUT_BAND1_GAIN_DB_UNIT "dB"
/** The least value the `band2GainDb` readout shows. (dB) */
#define OMX_BALANCER_READOUT_BAND2_GAIN_DB_MIN -12.0f
/** The greatest value the `band2GainDb` readout shows. (dB) */
#define OMX_BALANCER_READOUT_BAND2_GAIN_DB_MAX 12.0f
/** How often the `band2GainDb` readout is published (Hz). */
#define OMX_BALANCER_READOUT_BAND2_GAIN_DB_RATE_HZ 50
/** How a host draws the `band2GainDb` readout, an enum omx_param_scales member. */
#define OMX_BALANCER_READOUT_BAND2_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band2GainDb` readout, as a string. */
#define OMX_BALANCER_READOUT_BAND2_GAIN_DB_UNIT "dB"
/** The least value the `band3GainDb` readout shows. (dB) */
#define OMX_BALANCER_READOUT_BAND3_GAIN_DB_MIN -12.0f
/** The greatest value the `band3GainDb` readout shows. (dB) */
#define OMX_BALANCER_READOUT_BAND3_GAIN_DB_MAX 12.0f
/** How often the `band3GainDb` readout is published (Hz). */
#define OMX_BALANCER_READOUT_BAND3_GAIN_DB_RATE_HZ 50
/** How a host draws the `band3GainDb` readout, an enum omx_param_scales member. */
#define OMX_BALANCER_READOUT_BAND3_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band3GainDb` readout, as a string. */
#define OMX_BALANCER_READOUT_BAND3_GAIN_DB_UNIT "dB"
/** The least value the `band4GainDb` readout shows. (dB) */
#define OMX_BALANCER_READOUT_BAND4_GAIN_DB_MIN -12.0f
/** The greatest value the `band4GainDb` readout shows. (dB) */
#define OMX_BALANCER_READOUT_BAND4_GAIN_DB_MAX 12.0f
/** How often the `band4GainDb` readout is published (Hz). */
#define OMX_BALANCER_READOUT_BAND4_GAIN_DB_RATE_HZ 50
/** How a host draws the `band4GainDb` readout, an enum omx_param_scales member. */
#define OMX_BALANCER_READOUT_BAND4_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band4GainDb` readout, as a string. */
#define OMX_BALANCER_READOUT_BAND4_GAIN_DB_UNIT "dB"
/** The readouts of the clipper kernel, the meters a host draws, each at its position. */
enum omx_clipper_readouts {
  OMX_CLIPPER_READOUT_GAIN_REDUCTION_DB = 0, ///< The most the clip took off the block's loudest sample, in dB with CLAP's sign: 0 while nothing reached the knee, negative the reduction, measured after the drive. (dB)
};
/** The number of readouts of the clipper kernel. */
#define OMX_CLIPPER_READOUT_COUNT 1u
/** The least value the `gainReductionDb` readout shows. (dB) */
#define OMX_CLIPPER_READOUT_GAIN_REDUCTION_DB_MIN -60.0f
/** The greatest value the `gainReductionDb` readout shows. (dB) */
#define OMX_CLIPPER_READOUT_GAIN_REDUCTION_DB_MAX 0.0f
/** How often the `gainReductionDb` readout is published (Hz). */
#define OMX_CLIPPER_READOUT_GAIN_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `gainReductionDb` readout, an enum omx_param_scales member. */
#define OMX_CLIPPER_READOUT_GAIN_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `gainReductionDb` readout, as a string. */
#define OMX_CLIPPER_READOUT_GAIN_REDUCTION_DB_UNIT "dB"
/** The readouts of the comp kernel, the meters a host draws, each at its position. */
enum omx_comp_readouts {
  OMX_COMP_READOUT_GAIN_REDUCTION_DB = 0, ///< The compressor's gain reduction, in dB with CLAP's sign: 0 while it reduces nothing, negative the reduction it applies. The display range runs down to COMP_LIMITS.thresholdDb.min: on a signal at or below 0 dBFS the reduction (x - T)(1 - 1/R) never exceeds -T, and T is at least the threshold's minimum. (dB)
  OMX_COMP_READOUT_KEY_LEVEL_DB = 1, ///< The level the compressor's detector reads now, dBFS: the key after its filter, tilt and fold while one is set, the programme's louder leg otherwise. The display range is the threshold's travel, so the meter and the threshold share one axis. (dBFS)
  OMX_COMP_READOUT_EFFECTIVE_ATTACK_MS = 2, ///< The attack the detector runs, ms: attackMs, but in the feedback topology where the loop's pole is floored at (ratio - 1)/ratio so it cannot ring (omx_dyn_fb_attack_pole), the time constant of that floor at the running rate, -1000/(rate ln((R - 1)/R)). It equals attackMs whenever the floor is not reached. (ms)
};
/** The number of readouts of the comp kernel. */
#define OMX_COMP_READOUT_COUNT 3u
/** The least value the `gainReductionDb` readout shows. (dB) */
#define OMX_COMP_READOUT_GAIN_REDUCTION_DB_MIN -60.0f
/** The greatest value the `gainReductionDb` readout shows. (dB) */
#define OMX_COMP_READOUT_GAIN_REDUCTION_DB_MAX 0.0f
/** How often the `gainReductionDb` readout is published (Hz). */
#define OMX_COMP_READOUT_GAIN_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `gainReductionDb` readout, an enum omx_param_scales member. */
#define OMX_COMP_READOUT_GAIN_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `gainReductionDb` readout, as a string. */
#define OMX_COMP_READOUT_GAIN_REDUCTION_DB_UNIT "dB"
/** The least value the `keyLevelDb` readout shows. (dBFS) */
#define OMX_COMP_READOUT_KEY_LEVEL_DB_MIN -60.0f
/** The greatest value the `keyLevelDb` readout shows. (dBFS) */
#define OMX_COMP_READOUT_KEY_LEVEL_DB_MAX 0.0f
/** How often the `keyLevelDb` readout is published (Hz). */
#define OMX_COMP_READOUT_KEY_LEVEL_DB_RATE_HZ 50
/** How a host draws the `keyLevelDb` readout, an enum omx_param_scales member. */
#define OMX_COMP_READOUT_KEY_LEVEL_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `keyLevelDb` readout, as a string. */
#define OMX_COMP_READOUT_KEY_LEVEL_DB_UNIT "dBFS"
/** The least value the `effectiveAttackMs` readout shows. (ms) */
#define OMX_COMP_READOUT_EFFECTIVE_ATTACK_MS_MIN 0.1f
/** The greatest value the `effectiveAttackMs` readout shows. (ms) */
#define OMX_COMP_READOUT_EFFECTIVE_ATTACK_MS_MAX 100.0f
/** How often the `effectiveAttackMs` readout is published (Hz). */
#define OMX_COMP_READOUT_EFFECTIVE_ATTACK_MS_RATE_HZ 50
/** How a host draws the `effectiveAttackMs` readout, an enum omx_param_scales member. */
#define OMX_COMP_READOUT_EFFECTIVE_ATTACK_MS_SCALE OMX_PARAM_SCALES_LOG
/** The unit of the `effectiveAttackMs` readout, as a string. */
#define OMX_COMP_READOUT_EFFECTIVE_ATTACK_MS_UNIT "ms"
/** The readouts of the deesser kernel, the meters a host draws, each at its position. */
enum omx_deesser_readouts {
  OMX_DEESSER_READOUT_GAIN_REDUCTION_DB = 0, ///< The de-esser's gain reduction, in dB with CLAP's sign: 0 while it reduces nothing, negative the reduction it applies. The display range runs down to DEESS_RANGE_RANGE.min: the de-esser reduces by its range at most. (dB)
};
/** The number of readouts of the deesser kernel. */
#define OMX_DEESSER_READOUT_COUNT 1u
/** The least value the `gainReductionDb` readout shows. (dB) */
#define OMX_DEESSER_READOUT_GAIN_REDUCTION_DB_MIN -24.0f
/** The greatest value the `gainReductionDb` readout shows. (dB) */
#define OMX_DEESSER_READOUT_GAIN_REDUCTION_DB_MAX 0.0f
/** How often the `gainReductionDb` readout is published (Hz). */
#define OMX_DEESSER_READOUT_GAIN_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `gainReductionDb` readout, an enum omx_param_scales member. */
#define OMX_DEESSER_READOUT_GAIN_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `gainReductionDb` readout, as a string. */
#define OMX_DEESSER_READOUT_GAIN_REDUCTION_DB_UNIT "dB"
/** The readouts of the dyneq kernel, the meters a host draws, each at its position. */
enum omx_dyneq_readouts {
  OMX_DYNEQ_READOUT_BAND1_GAIN_DB = 0, ///< The gain band 1's dynamics apply now, in dB, both ways: a cut negative, a boost positive, 0 while the band rests, is off or its dynamics are off. The band's gain computer re-run on the level its detector reached at the block's last sample (omx_band_dyn_offset_db), so the reading and the gain the sample took are one computation. (dB)
  OMX_DYNEQ_READOUT_BAND2_GAIN_DB = 1, ///< The gain band 2's dynamics apply now, in dB, both ways: a cut negative, a boost positive, 0 while the band rests, is off or its dynamics are off. The band's gain computer re-run on the level its detector reached at the block's last sample (omx_band_dyn_offset_db), so the reading and the gain the sample took are one computation. (dB)
  OMX_DYNEQ_READOUT_BAND3_GAIN_DB = 2, ///< The gain band 3's dynamics apply now, in dB, both ways: a cut negative, a boost positive, 0 while the band rests, is off or its dynamics are off. The band's gain computer re-run on the level its detector reached at the block's last sample (omx_band_dyn_offset_db), so the reading and the gain the sample took are one computation. (dB)
  OMX_DYNEQ_READOUT_BAND4_GAIN_DB = 3, ///< The gain band 4's dynamics apply now, in dB, both ways: a cut negative, a boost positive, 0 while the band rests, is off or its dynamics are off. The band's gain computer re-run on the level its detector reached at the block's last sample (omx_band_dyn_offset_db), so the reading and the gain the sample took are one computation. (dB)
  OMX_DYNEQ_READOUT_BAND5_GAIN_DB = 4, ///< The gain band 5's dynamics apply now, in dB, both ways: a cut negative, a boost positive, 0 while the band rests, is off or its dynamics are off. The band's gain computer re-run on the level its detector reached at the block's last sample (omx_band_dyn_offset_db), so the reading and the gain the sample took are one computation. (dB)
  OMX_DYNEQ_READOUT_BAND6_GAIN_DB = 5, ///< The gain band 6's dynamics apply now, in dB, both ways: a cut negative, a boost positive, 0 while the band rests, is off or its dynamics are off. The band's gain computer re-run on the level its detector reached at the block's last sample (omx_band_dyn_offset_db), so the reading and the gain the sample took are one computation. (dB)
};
/** The number of readouts of the dyneq kernel. */
#define OMX_DYNEQ_READOUT_COUNT 6u
/** The least value the `band1GainDb` readout shows. (dB) */
#define OMX_DYNEQ_READOUT_BAND1_GAIN_DB_MIN -24.0f
/** The greatest value the `band1GainDb` readout shows. (dB) */
#define OMX_DYNEQ_READOUT_BAND1_GAIN_DB_MAX 12.0f
/** How often the `band1GainDb` readout is published (Hz). */
#define OMX_DYNEQ_READOUT_BAND1_GAIN_DB_RATE_HZ 50
/** How a host draws the `band1GainDb` readout, an enum omx_param_scales member. */
#define OMX_DYNEQ_READOUT_BAND1_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band1GainDb` readout, as a string. */
#define OMX_DYNEQ_READOUT_BAND1_GAIN_DB_UNIT "dB"
/** The least value the `band2GainDb` readout shows. (dB) */
#define OMX_DYNEQ_READOUT_BAND2_GAIN_DB_MIN -24.0f
/** The greatest value the `band2GainDb` readout shows. (dB) */
#define OMX_DYNEQ_READOUT_BAND2_GAIN_DB_MAX 12.0f
/** How often the `band2GainDb` readout is published (Hz). */
#define OMX_DYNEQ_READOUT_BAND2_GAIN_DB_RATE_HZ 50
/** How a host draws the `band2GainDb` readout, an enum omx_param_scales member. */
#define OMX_DYNEQ_READOUT_BAND2_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band2GainDb` readout, as a string. */
#define OMX_DYNEQ_READOUT_BAND2_GAIN_DB_UNIT "dB"
/** The least value the `band3GainDb` readout shows. (dB) */
#define OMX_DYNEQ_READOUT_BAND3_GAIN_DB_MIN -24.0f
/** The greatest value the `band3GainDb` readout shows. (dB) */
#define OMX_DYNEQ_READOUT_BAND3_GAIN_DB_MAX 12.0f
/** How often the `band3GainDb` readout is published (Hz). */
#define OMX_DYNEQ_READOUT_BAND3_GAIN_DB_RATE_HZ 50
/** How a host draws the `band3GainDb` readout, an enum omx_param_scales member. */
#define OMX_DYNEQ_READOUT_BAND3_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band3GainDb` readout, as a string. */
#define OMX_DYNEQ_READOUT_BAND3_GAIN_DB_UNIT "dB"
/** The least value the `band4GainDb` readout shows. (dB) */
#define OMX_DYNEQ_READOUT_BAND4_GAIN_DB_MIN -24.0f
/** The greatest value the `band4GainDb` readout shows. (dB) */
#define OMX_DYNEQ_READOUT_BAND4_GAIN_DB_MAX 12.0f
/** How often the `band4GainDb` readout is published (Hz). */
#define OMX_DYNEQ_READOUT_BAND4_GAIN_DB_RATE_HZ 50
/** How a host draws the `band4GainDb` readout, an enum omx_param_scales member. */
#define OMX_DYNEQ_READOUT_BAND4_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band4GainDb` readout, as a string. */
#define OMX_DYNEQ_READOUT_BAND4_GAIN_DB_UNIT "dB"
/** The least value the `band5GainDb` readout shows. (dB) */
#define OMX_DYNEQ_READOUT_BAND5_GAIN_DB_MIN -24.0f
/** The greatest value the `band5GainDb` readout shows. (dB) */
#define OMX_DYNEQ_READOUT_BAND5_GAIN_DB_MAX 12.0f
/** How often the `band5GainDb` readout is published (Hz). */
#define OMX_DYNEQ_READOUT_BAND5_GAIN_DB_RATE_HZ 50
/** How a host draws the `band5GainDb` readout, an enum omx_param_scales member. */
#define OMX_DYNEQ_READOUT_BAND5_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band5GainDb` readout, as a string. */
#define OMX_DYNEQ_READOUT_BAND5_GAIN_DB_UNIT "dB"
/** The least value the `band6GainDb` readout shows. (dB) */
#define OMX_DYNEQ_READOUT_BAND6_GAIN_DB_MIN -24.0f
/** The greatest value the `band6GainDb` readout shows. (dB) */
#define OMX_DYNEQ_READOUT_BAND6_GAIN_DB_MAX 12.0f
/** How often the `band6GainDb` readout is published (Hz). */
#define OMX_DYNEQ_READOUT_BAND6_GAIN_DB_RATE_HZ 50
/** How a host draws the `band6GainDb` readout, an enum omx_param_scales member. */
#define OMX_DYNEQ_READOUT_BAND6_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band6GainDb` readout, as a string. */
#define OMX_DYNEQ_READOUT_BAND6_GAIN_DB_UNIT "dB"
/** The readouts of the gate kernel, the meters a host draws, each at its position. */
enum omx_gate_readouts {
  OMX_GATE_READOUT_GAIN_REDUCTION_DB = 0, ///< The gate's gain reduction, in dB with CLAP's sign: 0 while it reduces nothing, negative the reduction it applies. The display range runs down to GATE_LIMITS.rangeDb.min: the gate attenuates by its range at most. (dB)
};
/** The number of readouts of the gate kernel. */
#define OMX_GATE_READOUT_COUNT 1u
/** The least value the `gainReductionDb` readout shows. (dB) */
#define OMX_GATE_READOUT_GAIN_REDUCTION_DB_MIN -90.0f
/** The greatest value the `gainReductionDb` readout shows. (dB) */
#define OMX_GATE_READOUT_GAIN_REDUCTION_DB_MAX 0.0f
/** How often the `gainReductionDb` readout is published (Hz). */
#define OMX_GATE_READOUT_GAIN_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `gainReductionDb` readout, an enum omx_param_scales member. */
#define OMX_GATE_READOUT_GAIN_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `gainReductionDb` readout, as a string. */
#define OMX_GATE_READOUT_GAIN_REDUCTION_DB_UNIT "dB"
/** The readouts of the geq kernel, the meters a host draws, each at its position. */
enum omx_geq_readouts {
  OMX_GEQ_READOUT_DESIGN_CLAMPED = 0, ///< Whether the last design clamped: 1 when a `true` solve asked a section for more than GEQ_DESIGN_GAIN_MAX_DB and got the ceiling, 0 otherwise and always in `standard`. Derived by the design step, never written by a host (omx-dsp's OmxGeqDesign.clamped). (none)
};
/** The number of readouts of the geq kernel. */
#define OMX_GEQ_READOUT_COUNT 1u
/** The least value the `designClamped` readout shows. (none) */
#define OMX_GEQ_READOUT_DESIGN_CLAMPED_MIN 0.0f
/** The greatest value the `designClamped` readout shows. (none) */
#define OMX_GEQ_READOUT_DESIGN_CLAMPED_MAX 1.0f
/** How often the `designClamped` readout is published (Hz). */
#define OMX_GEQ_READOUT_DESIGN_CLAMPED_RATE_HZ 50
/** How a host draws the `designClamped` readout, an enum omx_param_scales member. */
#define OMX_GEQ_READOUT_DESIGN_CLAMPED_SCALE OMX_PARAM_SCALES_STEPPED
/** The unit of the `designClamped` readout, as a string. */
#define OMX_GEQ_READOUT_DESIGN_CLAMPED_UNIT "none"
/** The readouts of the glue kernel, the meters a host draws, each at its position. */
enum omx_glue_readouts {
  OMX_GLUE_READOUT_GAIN_REDUCTION_DB = 0, ///< The compressor's gain reduction, in dB with CLAP's sign: 0 while it reduces nothing, negative the deepest gain of the block, make-up included. (dB)
};
/** The number of readouts of the glue kernel. */
#define OMX_GLUE_READOUT_COUNT 1u
/** The least value the `gainReductionDb` readout shows. (dB) */
#define OMX_GLUE_READOUT_GAIN_REDUCTION_DB_MIN -40.0f
/** The greatest value the `gainReductionDb` readout shows. (dB) */
#define OMX_GLUE_READOUT_GAIN_REDUCTION_DB_MAX 0.0f
/** How often the `gainReductionDb` readout is published (Hz). */
#define OMX_GLUE_READOUT_GAIN_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `gainReductionDb` readout, an enum omx_param_scales member. */
#define OMX_GLUE_READOUT_GAIN_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `gainReductionDb` readout, as a string. */
#define OMX_GLUE_READOUT_GAIN_REDUCTION_DB_UNIT "dB"
/** The readouts of the limiter kernel, the meters a host draws, each at its position. */
enum omx_limiter_readouts {
  OMX_LIMITER_READOUT_GAIN_REDUCTION_DB = 0, ///< The limiter's gain reduction, in dB with CLAP's sign: 0 while it reduces nothing, negative the reduction it applies. The display range runs down to LIMITER_LIMITS.ceilingDb.min: on a signal at or below 0 dBFS the limiter pulls the peak down to its ceiling at most, and the ceiling is at least its minimum. (dB)
};
/** The number of readouts of the limiter kernel. */
#define OMX_LIMITER_READOUT_COUNT 1u
/** The least value the `gainReductionDb` readout shows. (dB) */
#define OMX_LIMITER_READOUT_GAIN_REDUCTION_DB_MIN -12.0f
/** The greatest value the `gainReductionDb` readout shows. (dB) */
#define OMX_LIMITER_READOUT_GAIN_REDUCTION_DB_MAX 0.0f
/** How often the `gainReductionDb` readout is published (Hz). */
#define OMX_LIMITER_READOUT_GAIN_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `gainReductionDb` readout, an enum omx_param_scales member. */
#define OMX_LIMITER_READOUT_GAIN_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `gainReductionDb` readout, as a string. */
#define OMX_LIMITER_READOUT_GAIN_REDUCTION_DB_UNIT "dB"
/** The readouts of the loudlevel kernel, the meters a host draws, each at its position. */
enum omx_loudlevel_readouts {
  OMX_LOUDLEVEL_READOUT_LOUDNESS_LUFS = 0, ///< The loudness the detector reads now, LUFS, before the gain. (dB)
  OMX_LOUDLEVEL_READOUT_GAIN_DB = 1, ///< The gain the face applies now, dB. (dB)
};
/** The number of readouts of the loudlevel kernel. */
#define OMX_LOUDLEVEL_READOUT_COUNT 2u
/** The least value the `loudnessLufs` readout shows. (dB) */
#define OMX_LOUDLEVEL_READOUT_LOUDNESS_LUFS_MIN -70.0f
/** The greatest value the `loudnessLufs` readout shows. (dB) */
#define OMX_LOUDLEVEL_READOUT_LOUDNESS_LUFS_MAX 0.0f
/** How often the `loudnessLufs` readout is published (Hz). */
#define OMX_LOUDLEVEL_READOUT_LOUDNESS_LUFS_RATE_HZ 50
/** How a host draws the `loudnessLufs` readout, an enum omx_param_scales member. */
#define OMX_LOUDLEVEL_READOUT_LOUDNESS_LUFS_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `loudnessLufs` readout, as a string. */
#define OMX_LOUDLEVEL_READOUT_LOUDNESS_LUFS_UNIT "dB"
/** The least value the `gainDb` readout shows. (dB) */
#define OMX_LOUDLEVEL_READOUT_GAIN_DB_MIN -24.0f
/** The greatest value the `gainDb` readout shows. (dB) */
#define OMX_LOUDLEVEL_READOUT_GAIN_DB_MAX 24.0f
/** How often the `gainDb` readout is published (Hz). */
#define OMX_LOUDLEVEL_READOUT_GAIN_DB_RATE_HZ 50
/** How a host draws the `gainDb` readout, an enum omx_param_scales member. */
#define OMX_LOUDLEVEL_READOUT_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `gainDb` readout, as a string. */
#define OMX_LOUDLEVEL_READOUT_GAIN_DB_UNIT "dB"
/** The readouts of the mbcomp kernel, the meters a host draws, each at its position. */
enum omx_mbcomp_readouts {
  OMX_MBCOMP_READOUT_BAND1_GAIN_REDUCTION_DB = 0, ///< Band 1's gain reduction, dB with CLAP's sign: the band's gain computer re-run on its RMS level after the last block (omx_mbcomp_band_gr_db), make-up excluded; 0 while it reduces nothing, is bypassed or is past the band count. (dB)
  OMX_MBCOMP_READOUT_BAND2_GAIN_REDUCTION_DB = 1, ///< Band 2's gain reduction, dB with CLAP's sign: the band's gain computer re-run on its RMS level after the last block (omx_mbcomp_band_gr_db), make-up excluded; 0 while it reduces nothing, is bypassed or is past the band count. (dB)
  OMX_MBCOMP_READOUT_BAND3_GAIN_REDUCTION_DB = 2, ///< Band 3's gain reduction, dB with CLAP's sign: the band's gain computer re-run on its RMS level after the last block (omx_mbcomp_band_gr_db), make-up excluded; 0 while it reduces nothing, is bypassed or is past the band count. (dB)
  OMX_MBCOMP_READOUT_BAND4_GAIN_REDUCTION_DB = 3, ///< Band 4's gain reduction, dB with CLAP's sign: the band's gain computer re-run on its RMS level after the last block (omx_mbcomp_band_gr_db), make-up excluded; 0 while it reduces nothing, is bypassed or is past the band count. (dB)
  OMX_MBCOMP_READOUT_BAND5_GAIN_REDUCTION_DB = 4, ///< Band 5's gain reduction, dB with CLAP's sign: the band's gain computer re-run on its RMS level after the last block (omx_mbcomp_band_gr_db), make-up excluded; 0 while it reduces nothing, is bypassed or is past the band count. (dB)
};
/** The number of readouts of the mbcomp kernel. */
#define OMX_MBCOMP_READOUT_COUNT 5u
/** The least value the `band1GainReductionDb` readout shows. (dB) */
#define OMX_MBCOMP_READOUT_BAND1_GAIN_REDUCTION_DB_MIN -60.0f
/** The greatest value the `band1GainReductionDb` readout shows. (dB) */
#define OMX_MBCOMP_READOUT_BAND1_GAIN_REDUCTION_DB_MAX 0.0f
/** How often the `band1GainReductionDb` readout is published (Hz). */
#define OMX_MBCOMP_READOUT_BAND1_GAIN_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `band1GainReductionDb` readout, an enum omx_param_scales member. */
#define OMX_MBCOMP_READOUT_BAND1_GAIN_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band1GainReductionDb` readout, as a string. */
#define OMX_MBCOMP_READOUT_BAND1_GAIN_REDUCTION_DB_UNIT "dB"
/** The least value the `band2GainReductionDb` readout shows. (dB) */
#define OMX_MBCOMP_READOUT_BAND2_GAIN_REDUCTION_DB_MIN -60.0f
/** The greatest value the `band2GainReductionDb` readout shows. (dB) */
#define OMX_MBCOMP_READOUT_BAND2_GAIN_REDUCTION_DB_MAX 0.0f
/** How often the `band2GainReductionDb` readout is published (Hz). */
#define OMX_MBCOMP_READOUT_BAND2_GAIN_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `band2GainReductionDb` readout, an enum omx_param_scales member. */
#define OMX_MBCOMP_READOUT_BAND2_GAIN_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band2GainReductionDb` readout, as a string. */
#define OMX_MBCOMP_READOUT_BAND2_GAIN_REDUCTION_DB_UNIT "dB"
/** The least value the `band3GainReductionDb` readout shows. (dB) */
#define OMX_MBCOMP_READOUT_BAND3_GAIN_REDUCTION_DB_MIN -60.0f
/** The greatest value the `band3GainReductionDb` readout shows. (dB) */
#define OMX_MBCOMP_READOUT_BAND3_GAIN_REDUCTION_DB_MAX 0.0f
/** How often the `band3GainReductionDb` readout is published (Hz). */
#define OMX_MBCOMP_READOUT_BAND3_GAIN_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `band3GainReductionDb` readout, an enum omx_param_scales member. */
#define OMX_MBCOMP_READOUT_BAND3_GAIN_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band3GainReductionDb` readout, as a string. */
#define OMX_MBCOMP_READOUT_BAND3_GAIN_REDUCTION_DB_UNIT "dB"
/** The least value the `band4GainReductionDb` readout shows. (dB) */
#define OMX_MBCOMP_READOUT_BAND4_GAIN_REDUCTION_DB_MIN -60.0f
/** The greatest value the `band4GainReductionDb` readout shows. (dB) */
#define OMX_MBCOMP_READOUT_BAND4_GAIN_REDUCTION_DB_MAX 0.0f
/** How often the `band4GainReductionDb` readout is published (Hz). */
#define OMX_MBCOMP_READOUT_BAND4_GAIN_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `band4GainReductionDb` readout, an enum omx_param_scales member. */
#define OMX_MBCOMP_READOUT_BAND4_GAIN_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band4GainReductionDb` readout, as a string. */
#define OMX_MBCOMP_READOUT_BAND4_GAIN_REDUCTION_DB_UNIT "dB"
/** The least value the `band5GainReductionDb` readout shows. (dB) */
#define OMX_MBCOMP_READOUT_BAND5_GAIN_REDUCTION_DB_MIN -60.0f
/** The greatest value the `band5GainReductionDb` readout shows. (dB) */
#define OMX_MBCOMP_READOUT_BAND5_GAIN_REDUCTION_DB_MAX 0.0f
/** How often the `band5GainReductionDb` readout is published (Hz). */
#define OMX_MBCOMP_READOUT_BAND5_GAIN_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `band5GainReductionDb` readout, an enum omx_param_scales member. */
#define OMX_MBCOMP_READOUT_BAND5_GAIN_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `band5GainReductionDb` readout, as a string. */
#define OMX_MBCOMP_READOUT_BAND5_GAIN_REDUCTION_DB_UNIT "dB"
/** The readouts of the plosive kernel, the meters a host draws, each at its position. */
enum omx_plosive_readouts {
  OMX_PLOSIVE_READOUT_CONSONANT_REDUCTION_DB = 0, ///< The deepest consonant cut the last block applied, dB with CLAP's sign: 0 while nothing is cut. (dB)
  OMX_PLOSIVE_READOUT_POP_REDUCTION_DB = 1, ///< The deepest pop cut the last block applied, dB with CLAP's sign: 0 while nothing is cut. (dB)
};
/** The number of readouts of the plosive kernel. */
#define OMX_PLOSIVE_READOUT_COUNT 2u
/** The least value the `consonantReductionDb` readout shows. (dB) */
#define OMX_PLOSIVE_READOUT_CONSONANT_REDUCTION_DB_MIN -24.0f
/** The greatest value the `consonantReductionDb` readout shows. (dB) */
#define OMX_PLOSIVE_READOUT_CONSONANT_REDUCTION_DB_MAX 0.0f
/** How often the `consonantReductionDb` readout is published (Hz). */
#define OMX_PLOSIVE_READOUT_CONSONANT_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `consonantReductionDb` readout, an enum omx_param_scales member. */
#define OMX_PLOSIVE_READOUT_CONSONANT_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `consonantReductionDb` readout, as a string. */
#define OMX_PLOSIVE_READOUT_CONSONANT_REDUCTION_DB_UNIT "dB"
/** The least value the `popReductionDb` readout shows. (dB) */
#define OMX_PLOSIVE_READOUT_POP_REDUCTION_DB_MIN -24.0f
/** The greatest value the `popReductionDb` readout shows. (dB) */
#define OMX_PLOSIVE_READOUT_POP_REDUCTION_DB_MAX 0.0f
/** How often the `popReductionDb` readout is published (Hz). */
#define OMX_PLOSIVE_READOUT_POP_REDUCTION_DB_RATE_HZ 50
/** How a host draws the `popReductionDb` readout, an enum omx_param_scales member. */
#define OMX_PLOSIVE_READOUT_POP_REDUCTION_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `popReductionDb` readout, as a string. */
#define OMX_PLOSIVE_READOUT_POP_REDUCTION_DB_UNIT "dB"
/** The readouts of the suboct kernel, the meters a host draws, each at its position. */
enum omx_suboct_readouts {
  OMX_SUBOCT_READOUT_TRACKED_HZ = 0, ///< The fundamental the divider tracks, Hz: the rate over the samples between its last two toggles, one toggle an input period; 0 while gated. (Hz)
};
/** The number of readouts of the suboct kernel. */
#define OMX_SUBOCT_READOUT_COUNT 1u
/** The least value the `trackedHz` readout shows. (Hz) */
#define OMX_SUBOCT_READOUT_TRACKED_HZ_MIN 0.0f
/** The greatest value the `trackedHz` readout shows. (Hz) */
#define OMX_SUBOCT_READOUT_TRACKED_HZ_MAX 600.0f
/** How often the `trackedHz` readout is published (Hz). */
#define OMX_SUBOCT_READOUT_TRACKED_HZ_RATE_HZ 50
/** How a host draws the `trackedHz` readout, an enum omx_param_scales member. */
#define OMX_SUBOCT_READOUT_TRACKED_HZ_SCALE OMX_PARAM_SCALES_LOG
/** The unit of the `trackedHz` readout, as a string. */
#define OMX_SUBOCT_READOUT_TRACKED_HZ_UNIT "Hz"
/** The readouts of the transient kernel, the meters a host draws, each at its position. */
enum omx_transient_readouts {
  OMX_TRANSIENT_READOUT_ATTACK_GAIN_DB = 0, ///< The gain the onset contrast applies now, dB: attackDb times the contrast's ramp (omx_envdiff_ramp at offset 0) on the last sample, so 0 while the contrast is nil and attackDb at full contrast. With the other readout and outputDb it sums to the gain the stage applies. (dB)
  OMX_TRANSIENT_READOUT_SUSTAIN_GAIN_DB = 1, ///< The gain the decay contrast applies now, dB: sustainDb times the contrast's ramp (omx_envdiff_ramp at offset 0) on the last sample, so 0 while the contrast is nil and sustainDb at full contrast. With the other readout and outputDb it sums to the gain the stage applies. (dB)
};
/** The number of readouts of the transient kernel. */
#define OMX_TRANSIENT_READOUT_COUNT 2u
/** The least value the `attackGainDb` readout shows. (dB) */
#define OMX_TRANSIENT_READOUT_ATTACK_GAIN_DB_MIN -24.0f
/** The greatest value the `attackGainDb` readout shows. (dB) */
#define OMX_TRANSIENT_READOUT_ATTACK_GAIN_DB_MAX 24.0f
/** How often the `attackGainDb` readout is published (Hz). */
#define OMX_TRANSIENT_READOUT_ATTACK_GAIN_DB_RATE_HZ 50
/** How a host draws the `attackGainDb` readout, an enum omx_param_scales member. */
#define OMX_TRANSIENT_READOUT_ATTACK_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `attackGainDb` readout, as a string. */
#define OMX_TRANSIENT_READOUT_ATTACK_GAIN_DB_UNIT "dB"
/** The least value the `sustainGainDb` readout shows. (dB) */
#define OMX_TRANSIENT_READOUT_SUSTAIN_GAIN_DB_MIN -24.0f
/** The greatest value the `sustainGainDb` readout shows. (dB) */
#define OMX_TRANSIENT_READOUT_SUSTAIN_GAIN_DB_MAX 24.0f
/** How often the `sustainGainDb` readout is published (Hz). */
#define OMX_TRANSIENT_READOUT_SUSTAIN_GAIN_DB_RATE_HZ 50
/** How a host draws the `sustainGainDb` readout, an enum omx_param_scales member. */
#define OMX_TRANSIENT_READOUT_SUSTAIN_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `sustainGainDb` readout, as a string. */
#define OMX_TRANSIENT_READOUT_SUSTAIN_GAIN_DB_UNIT "dB"
/** The readouts of the upward kernel, the meters a host draws, each at its position. */
enum omx_upward_readouts {
  OMX_UPWARD_READOUT_GAIN_DB = 0, ///< What the block's loudest sample rose by, dB: 0 while nothing is lifted. (dB)
};
/** The number of readouts of the upward kernel. */
#define OMX_UPWARD_READOUT_COUNT 1u
/** The least value the `gainDb` readout shows. (dB) */
#define OMX_UPWARD_READOUT_GAIN_DB_MIN 0.0f
/** The greatest value the `gainDb` readout shows. (dB) */
#define OMX_UPWARD_READOUT_GAIN_DB_MAX 24.0f
/** How often the `gainDb` readout is published (Hz). */
#define OMX_UPWARD_READOUT_GAIN_DB_RATE_HZ 50
/** How a host draws the `gainDb` readout, an enum omx_param_scales member. */
#define OMX_UPWARD_READOUT_GAIN_DB_SCALE OMX_PARAM_SCALES_LINEAR
/** The unit of the `gainDb` readout, as a string. */
#define OMX_UPWARD_READOUT_GAIN_DB_UNIT "dB"

#endif /* OMXDSP_DEFINITIONS_H */
