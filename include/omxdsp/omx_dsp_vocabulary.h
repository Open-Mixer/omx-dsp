// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#ifndef OMX_DSP_VOCABULARY_H
#define OMX_DSP_VOCABULARY_H
/*
 * GENERATED: DO NOT EDIT BY HAND.
 * Produced by `omx-contract render --target c` from Open-Mixer/omx-dsp's data/: the vocabulary
 * (docs/data-layout.md). A code is OMX_ and the code in upper snake case, and a reply code that carries a resp has it as that name and _RESP; a word is OMX_<ITEM>_<NAME>, each words
 * item with its _COUNT and an _INIT of its words in their order. A consumer writes and compares these, never a
 * literal of its own.
 */

/** @file
 * @brief The vocabulary of omx-dsp: every code and every word as a string define, with its sentence
 * where it carries one. */

/* CURVE_ROLE_PREFIXES (data/primitives/curve.json) */
#define OMX_CURVE_ROLE_PREFIXES_BAND "band_" ///< The prefix every indexed band role of CURVE_ROLES carries on the wire (`band_type`, `band_freq`, `band_gain`, `band_q`, `band_on`): a role word that starts with it is a band's and carries an index. A consumer reads this word, or calls omx_curve_role_is_band(), and never writes the prefix itself.
/** The source sentence (English) of OMX_CURVE_ROLE_PREFIXES_BAND. */
#define OMX_CURVE_ROLE_PREFIXES_BAND_TEXT_EN "The prefix every indexed band role of CURVE_ROLES carries on the wire (`band_type`, `band_freq`, `band_gain`, `band_q`, `band_on`): a role word that starts with it is a band's and carries an index. A consumer reads this word, or calls omx_curve_role_is_band(), and never writes the prefix itself."
/** The number of words of CURVE_ROLE_PREFIXES. */
#define OMX_CURVE_ROLE_PREFIXES_COUNT 1u
/** The words of CURVE_ROLE_PREFIXES in their declared order, as a C initializer list. */
#define OMX_CURVE_ROLE_PREFIXES_INIT { "band_" }
/** Expands `X(code, "word", NAME)` once per word of CURVE_ROLE_PREFIXES, in order, the code being its place in the order. The prefixes a role word of CURVE_ROLES is read by: an indexed band role is one whose word starts with the `band` prefix. */
#define OMX_CURVE_ROLE_PREFIXES(X) \
    X(0u, "band_", BAND)

/* CURVE_AXES (data/primitives/curve.json) */
#define OMX_CURVE_AXES_FREQUENCY "frequency" ///< Frequency, on the points of EQ_CURVE_GRID where the curve is a response.
/** The source sentence (English) of OMX_CURVE_AXES_FREQUENCY. */
#define OMX_CURVE_AXES_FREQUENCY_TEXT_EN "Frequency, on the points of EQ_CURVE_GRID where the curve is a response."
#define OMX_CURVE_AXES_MAGNITUDE "magnitude" ///< The magnitude of a response at a frequency: 0 dB is unity, a boost positive, a cut negative.
/** The source sentence (English) of OMX_CURVE_AXES_MAGNITUDE. */
#define OMX_CURVE_AXES_MAGNITUDE_TEXT_EN "The magnitude of a response at a frequency: 0 dB is unity, a boost positive, a cut negative."
#define OMX_CURVE_AXES_INPUT_LEVEL "input-level" ///< The level a gain computer reads, the static law's input.
/** The source sentence (English) of OMX_CURVE_AXES_INPUT_LEVEL. */
#define OMX_CURVE_AXES_INPUT_LEVEL_TEXT_EN "The level a gain computer reads, the static law's input."
#define OMX_CURVE_AXES_OUTPUT_LEVEL "output-level" ///< The level the static law gives for an input level; the diagonal is no gain change.
/** The source sentence (English) of OMX_CURVE_AXES_OUTPUT_LEVEL. */
#define OMX_CURVE_AXES_OUTPUT_LEVEL_TEXT_EN "The level the static law gives for an input level; the diagonal is no gain change."
#define OMX_CURVE_AXES_INPUT_AMPLITUDE "input-amplitude" ///< A memoryless shaper's input sample, linear, -1 to +1 full scale: bipolar, so an even/odd asymmetry shows.
/** The source sentence (English) of OMX_CURVE_AXES_INPUT_AMPLITUDE. */
#define OMX_CURVE_AXES_INPUT_AMPLITUDE_TEXT_EN "A memoryless shaper's input sample, linear, -1 to +1 full scale: bipolar, so an even/odd asymmetry shows."
#define OMX_CURVE_AXES_OUTPUT_AMPLITUDE "output-amplitude" ///< The shaper's output sample for an input amplitude, linear, on the same full scale.
/** The source sentence (English) of OMX_CURVE_AXES_OUTPUT_AMPLITUDE. */
#define OMX_CURVE_AXES_OUTPUT_AMPLITUDE_TEXT_EN "The shaper's output sample for an input amplitude, linear, on the same full scale."
#define OMX_CURVE_AXES_DELAY_TIME "delay-time" ///< The time after the dry sound at which a delay's tap or repeat sounds.
/** The source sentence (English) of OMX_CURVE_AXES_DELAY_TIME. */
#define OMX_CURVE_AXES_DELAY_TIME_TEXT_EN "The time after the dry sound at which a delay's tap or repeat sounds."
#define OMX_CURVE_AXES_DECAY_TIME "decay-time" ///< The time after an impulse over which a reverb's energy decays.
/** The source sentence (English) of OMX_CURVE_AXES_DECAY_TIME. */
#define OMX_CURVE_AXES_DECAY_TIME_TEXT_EN "The time after an impulse over which a reverb's energy decays."
#define OMX_CURVE_AXES_LEVEL "level" ///< A level relative to the dry input: 0 dB is the input's own, a tap or a decay below it negative.
/** The source sentence (English) of OMX_CURVE_AXES_LEVEL. */
#define OMX_CURVE_AXES_LEVEL_TEXT_EN "A level relative to the dry input: 0 dB is the input's own, a tap or a decay below it negative."
#define OMX_CURVE_AXES_PHASE "phase" ///< The position within one period of a modulation source, 0 to 1.
/** The source sentence (English) of OMX_CURVE_AXES_PHASE. */
#define OMX_CURVE_AXES_PHASE_TEXT_EN "The position within one period of a modulation source, 0 to 1."
#define OMX_CURVE_AXES_MODULATION "modulation" ///< A modulation source's output at a phase, -1 to +1, before the depth scales it.
/** The source sentence (English) of OMX_CURVE_AXES_MODULATION. */
#define OMX_CURVE_AXES_MODULATION_TEXT_EN "A modulation source's output at a phase, -1 to +1, before the depth scales it."
#define OMX_CURVE_AXES_INPUT_PITCH "input-pitch" ///< The pitch played into a pitch effect, semitones from an origin the host picks.
/** The source sentence (English) of OMX_CURVE_AXES_INPUT_PITCH. */
#define OMX_CURVE_AXES_INPUT_PITCH_TEXT_EN "The pitch played into a pitch effect, semitones from an origin the host picks."
#define OMX_CURVE_AXES_OUTPUT_PITCH "output-pitch" ///< The pitch the effect sounds for an input pitch, semitones on the same origin: an unshifted map is the diagonal.
/** The source sentence (English) of OMX_CURVE_AXES_OUTPUT_PITCH. */
#define OMX_CURVE_AXES_OUTPUT_PITCH_TEXT_EN "The pitch the effect sounds for an input pitch, semitones on the same origin: an unshifted map is the diagonal."
/** The number of words of CURVE_AXES. */
#define OMX_CURVE_AXES_COUNT 13u
/** The words of CURVE_AXES in their declared order, as a C initializer list. */
#define OMX_CURVE_AXES_INIT { "frequency", "magnitude", "input-level", "output-level", "input-amplitude", "output-amplitude", "delay-time", "decay-time", "level", "phase", "modulation", "input-pitch", "output-pitch" }
/** Expands `X(code, "word", NAME)` once per word of CURVE_AXES, in order, the code being its place in the order. The axes a curve kind is drawn on, each with its unit and how a host draws it (docs/curves.md). A curve's values are in these units, so the drawn values can be held equal to the closed form a kernel's controls give. */
#define OMX_CURVE_AXES(X) \
    X(0u, "frequency", FREQUENCY) X(1u, "magnitude", MAGNITUDE) X(2u, "input-level", INPUT_LEVEL) X(3u, "output-level", OUTPUT_LEVEL) \
    X(4u, "input-amplitude", INPUT_AMPLITUDE) X(5u, "output-amplitude", OUTPUT_AMPLITUDE) X(6u, "delay-time", DELAY_TIME) X(7u, "decay-time", DECAY_TIME) \
    X(8u, "level", LEVEL) X(9u, "phase", PHASE) X(10u, "modulation", MODULATION) X(11u, "input-pitch", INPUT_PITCH) \
    X(12u, "output-pitch", OUTPUT_PITCH)

/* CURVE_KINDS (data/primitives/curve.json) */
#define OMX_CURVE_KINDS_EQ_CURVE "eq-curve" ///< An EQ's summed magnitude response over log frequency.
/** The source sentence (English) of OMX_CURVE_KINDS_EQ_CURVE. */
#define OMX_CURVE_KINDS_EQ_CURVE_TEXT_EN "An EQ's summed magnitude response over log frequency."
#define OMX_CURVE_KINDS_DYNAMICS_CURVE "dynamics-curve" ///< A gate's or a compressor's static law, input level to output level, with the live needle.
/** The source sentence (English) of OMX_CURVE_KINDS_DYNAMICS_CURVE. */
#define OMX_CURVE_KINDS_DYNAMICS_CURVE_TEXT_EN "A gate's or a compressor's static law, input level to output level, with the live needle."
#define OMX_CURVE_KINDS_TRANSFER_CURVE "transfer-curve" ///< A memoryless shaper's transfer, input sample to output sample over full scale.
/** The source sentence (English) of OMX_CURVE_KINDS_TRANSFER_CURVE. */
#define OMX_CURVE_KINDS_TRANSFER_CURVE_TEXT_EN "A memoryless shaper's transfer, input sample to output sample over full scale."
#define OMX_CURVE_KINDS_DELAY_TAPS "delay-taps" ///< A delay's taps and repeats, each a line at its time and level.
/** The source sentence (English) of OMX_CURVE_KINDS_DELAY_TAPS. */
#define OMX_CURVE_KINDS_DELAY_TAPS_TEXT_EN "A delay's taps and repeats, each a line at its time and level."
#define OMX_CURVE_KINDS_DECAY_ENVELOPE "decay-envelope" ///< A reverb's decay envelope after an impulse.
/** The source sentence (English) of OMX_CURVE_KINDS_DECAY_ENVELOPE. */
#define OMX_CURVE_KINDS_DECAY_ENVELOPE_TEXT_EN "A reverb's decay envelope after an impulse."
#define OMX_CURVE_KINDS_COMB_RESPONSE "comb-response" ///< The wet path's comb or notch response at the current sweep position (chorus, flanger, phaser, doubler).
/** The source sentence (English) of OMX_CURVE_KINDS_COMB_RESPONSE. */
#define OMX_CURVE_KINDS_COMB_RESPONSE_TEXT_EN "The wet path's comb or notch response at the current sweep position (chorus, flanger, phaser, doubler)."
#define OMX_CURVE_KINDS_MODULATION_SHAPE "modulation-shape" ///< A modulation source over one period (tremolo, rotary).
/** The source sentence (English) of OMX_CURVE_KINDS_MODULATION_SHAPE. */
#define OMX_CURVE_KINDS_MODULATION_SHAPE_TEXT_EN "A modulation source over one period (tremolo, rotary)."
#define OMX_CURVE_KINDS_INTERVAL_MAP "interval-map" ///< A pitch effect's map from the input pitch to the output pitch.
/** The source sentence (English) of OMX_CURVE_KINDS_INTERVAL_MAP. */
#define OMX_CURVE_KINDS_INTERVAL_MAP_TEXT_EN "A pitch effect's map from the input pitch to the output pitch."
/** The number of words of CURVE_KINDS. */
#define OMX_CURVE_KINDS_COUNT 8u
/** The words of CURVE_KINDS in their declared order, as a C initializer list. */
#define OMX_CURVE_KINDS_INIT { "eq-curve", "dynamics-curve", "transfer-curve", "delay-taps", "decay-envelope", "comb-response", "modulation-shape", "interval-map" }
/** Expands `X(code, "word", NAME)` once per word of CURVE_KINDS, in order, the code being its place in the order. The kinds of picture a plugin's panel section draws and a console's card shows, each with the two axes it is drawn on (docs/curves.md). A kernel file and a plugin declaration name a kind by its word; a host keeps only its kind-to-drawing map. Append-only: a kind is never renamed, moved or removed while a reader draws it. */
#define OMX_CURVE_KINDS(X) \
    X(0u, "eq-curve", EQ_CURVE) X(1u, "dynamics-curve", DYNAMICS_CURVE) X(2u, "transfer-curve", TRANSFER_CURVE) X(3u, "delay-taps", DELAY_TAPS) \
    X(4u, "decay-envelope", DECAY_ENVELOPE) X(5u, "comb-response", COMB_RESPONSE) X(6u, "modulation-shape", MODULATION_SHAPE) X(7u, "interval-map", INTERVAL_MAP)

#endif /* OMX_DSP_VOCABULARY_H */
