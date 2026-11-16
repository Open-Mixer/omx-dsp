<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> -->

# Fuzzing libomxdsp

libFuzzer harnesses over the entry points of libomxdsp that take data from outside: a control
port's word (any float, int or choice a host can put on it), a block's samples (finite, or any
float word at all), a rate, a table or a stream of MIDI messages. Each harness is a plain C
`LLVMFuzzerTestOneInput` and builds with `clang -fsanitize=fuzzer,address,undefined` and
`-DOMX_CONTRACTS`, so the library's own PRE and POST laws are read after every call as well:
a broken law, like a sanitizer report, is a crash.

None of this is part of `make test`.

| harness | what it feeds | the properties it holds the library to beyond "no crash" |
|---|---|---|
| `fuzz_fx` | every instance face under `include/omxdsp/fx/` (31 of them): init at a rate, resolve with raw ports, run a block | no contract broken; an init takes exactly the rates its face owes (the grid, or above the face's floor); finite in gives finite out; an in-place run equals an out-of-place one; a refused rate leaves the face the identity |
| `fuzz_laws` | the scalar laws every port goes through: clamps, dB and linear, pan, balance and fader laws, ramp, slew, wet/dry, the one-pole constructors | a clamp lands in its travel; constant-power pan; the balance law never boosts; a ramp follows its rule, sample by sample, and stays between its ends; a mix stays between its inputs; a pole is in [0, 1) |
| `fuzz_eq` | a bank of up to 24 bands and the head filters, designed at a rate, then a block through the cascade and the bank's magnitude | every designed section finite and stable; at most as many sections as bands; section i has the gain at DC of the i-th band that is on, a bell its gain at its centre (and no more than 5 % past it with the centre at most an eighth of the rate) and a high shelf at least a quarter of it at Nyquist, and is parked exactly when that band is a peaking or shelf band at 0 dB; a bank with nothing live is the identity on each leg it runs, and 0 dB; finite out |
| `fuzz_src` | a rate pair, the polyphase table, a take cut into blocks and flushed | the plan is taken exactly when it can be sized, and is the reduced ratio; each phase of the table sums to 1; a flushed take makes at least n·l/m outputs; a take is the same cut into blocks as whole, bit for bit; output bounded by the table's own gain; the identity plan returns the take |
| `fuzz_delay` | the fractional delay line (orders 3 and 5, any ring size, any delay word) and the integer pair with its cross-fade | a line delivers the delay the rule in its comment owes; a refused order or ring leaves it unarmed, and an unarmed line leaves the block as it was; a whole delay is the take delayed, bit for bit; a fractional one is the Lagrange sum over the take, to float rounding, and bounded by the weights; a steady integer delay is exact, and a fade stays between its two taps; `omx_idelay_samples()` is the rounded delay, saturated at the ring's last slot, for any finite ms and rate |
| `fuzz_note_map` | a stream of MIDI 1.0 channel messages through a leg's policy, with the policy changed in the middle | no note sticks and none is released that never sounded (a model of the downstream synth); `held_count` is the table's population; the table holds, key by key, what the harness's account of the stream holds; a note on is delivered exactly when the leg takes it and its transposed key is a key, and sounds that key; `out_of_range` counts the others the leg takes |

There is no parser of presets, files or tables in libomxdsp: it holds no serialised format, and
its tables are compiled in. When one is added it gets a harness here.

## Running

    apt-get install -y clang llvm # clang with libFuzzer's runtime, llvm-cov; no other dependency beyond omx-contract
    make fuzz                     # build every harness into build/fuzz/
    make fuzz-run FUZZ_TIME=60    # run each for 60 s from its seed corpus; red on any crash
    make fuzz-sabotage            # every patch of fuzz/sabotage/ found, each at the check it names
    make fuzz-records             # every record met, each at its own check
    make fuzz-coverage            # the coverage witness and its floors
    make fuzz-corpus              # regenerate fuzz/corpus/ (fuzz/gen-corpus.c)

`make fuzz` needs omx-contract like every other target (see `BUILDING.md`).

A crash leaves its input in `fuzz/findings/` (ignored by git) and the log beside it. To
reproduce one:

    build/fuzz/fuzz_fx fuzz/findings/fuzz_fx-crash-<sha1>

## The checks and their records

Each property is its own `if (...) abort();` in a harness, never `assert()` and never inside a
macro, and the comment above it says what it checks and carries a record, as OSS Verify's
`tools/check --checks` reads them (its `docs/build-contract.md`, "The checks of a fuzz target"):

```
/* A clamp lands inside its travel, whatever the word.
 * mutant: include/omxdsp/omx_param.h:37 `(v <= hi ? v : hi)` -> `v`
 * input: clamp */
```

A record is a one-line change of the library and the seed it shows on: with the change, that seed
makes the check abort; without it, the seed passes. A stop after a failed allocation of the
harness's own carries `not a property: <reason>` in place of a record. A check of `fuzz/fuzz.h` is
every harness's; its record's seed names the harness whose corpus holds it.

`make fuzz-records` (`fuzz/records.sh`) proves every record at this commit. First the ratchet: a
check with neither a record nor a `not a property:` line, or with lines not of that form, is red
before anything is built (`fuzz/checks.awk` reads them as `tools/check` does). Then each record
alone: the harness, built plain at -O0 with the contracts and no sanitizer and driven by
`fuzz/standalone.c`, passes the seed as the tree is, and with the change aborts on it at that
record's own `abort()`, which each `abort()` names on stderr in that build. The count is the proof:

```
make fuzz-records: all 115 records and 6 patches met, each by its own check on its own seed (106 checks)
```

## The sabotage patches

`fuzz/sabotage/<harness>-<property>.patch` is a bug in the code a harness drives, at least one for
each harness, as `git diff` writes it. Above its `diff --git` line it names the check it breaks and
the seed that shows it, the two lines OSS Verify reads:

```
Breaks the check of fuzz/fuzz_laws.c:186 (the right leg boosts when the balance is to the right).
Caught by the seed input "pan-fader" of fuzz_laws.
```

`make fuzz-sabotage` applies each, rebuilds the sanitizer harness and runs every seed of its corpus
once, without fuzzing: the named seed must abort with the message of the check the header names (its
`FUZZ_SAY()`; the optimizer merges a harness's `abort()` calls, so the stack cannot tell one check
from another), and every seed that fails must fail at some check, not by a sanitizer report.
`make fuzz-records` proves the line the header names.

## The coverage witness

`make fuzz-coverage` builds each harness for `llvm-cov` (no sanitizers), runs it once over its
corpus and prints the line coverage of every file of `src/` and `include/` the harness holds
(`fuzz/coverage.sh`). `fuzz/coverage-floor.txt` holds the covered lines each harness reaches; one
below its floor is red, and so is a check of `fuzz/fuzz_<harness>.c` no seed reaches. Raise a floor
when coverage rises; lower one only with the change that removed the lines.

`docs/fuzz-checks.md` reads every check against its comment, with its expected value, its record and a verdict.

Where a check needs an expected value the harness works it out itself: the gcd of a plan and the output counts of a take, the
stability of a section, the gain at DC of a band and a section's gain at a bell's centre or at Nyquist, which bands are parked, the rates each instance face takes, the validity of a leg and
the leg in force, the delay a line owes and the Lagrange weights at its fraction, the last delays of
the integer pair and the delay in samples, the keys held, whether a sample is finite, none of them through the library's own function.

## The harnesses read their input as a grammar

The header comment of each `fuzz_*.c` is the grammar of its input. The seed corpus in
`fuzz/corpus/<harness>/` is shaped the way the harness reads it, written by `fuzz/gen-corpus.c`
from what the repository already holds (the instance faces' own port lists, the EQ's bands, the
converter's rate pairs, the note map's policies): nothing is fetched and no capture is read.
`fuzz/run.sh` never writes into the corpus; what the fuzzer finds goes to a scratch directory under
the build directory.

## What the harnesses leave alone

- The instance faces take the nine rates of the oracle grid. 32, 64 and 128 kHz are not declared
  rates, and at those every design records the PRE `rate-is-declared` by design
  (`test/fx/*_instance.test.c` say so); the harness skips that one token at those rates.
- A multiband stage whose corner moves between two blocks carries the old design's state into the
  new one, and the crossover's post `bands-partition-unity` is a law of one design: the harness
  skips that token, and the suite's own `xover` arm holds it where the design is fixed.
- `rate-is-declared` at a rate of the grid that is not a declared one, and `bands-partition-unity`,
  never reach the ledger of `fuzz_fx` and `fuzz_eq`: they include `fuzz.h` before the library, whose
  `fuzz_contract_note()` drops them. An instance face can record the first once per frame, and the
  ledger keeps 64 records, so they would push out every law after them.
- A block of non-finite samples (the `fuzz_fx` hostile mode) breaks the PRE `finite-in` by the
  caller's own act; the ledger is not read then and the sanitizers are the oracle.
