<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com> -->
# omx-rta-read

The spectrum of a JACK signal, read and never fed. `omx-rta-read` watches a processor in a JACK graph and writes its
spectrum as log-spaced bands, one JSON line ten times a second, for a panel to draw under the processor's curve. It is
the OpenMixer console's analyser, with the console's window, bands, gates and level extents, for a host that runs its
processors as JACK clients (a Zynthian, for one).

It ships in its own package, `omx-rta-read` (rpm and Debian), the only part of omx-dsp that needs JACK.

## What it reads

```
omx-rta-read [--post <port>]... [--pre <port>]... --bands <n> [--low] [--name <client name>]
```

- **POST** (`--post`): what the processor gives, read from its output ports.
- **PRE** (`--pre`): what the processor is given. Name the processor's INPUT port: the reader connects from every port
  connected to it (JACK sums them into the processor, and the reader hears the same sum) and follows the graph when
  that set changes. An input with nothing connected reads as silence.
- Each tap is one leg, or two for a stereo pair, read as their mono sum. Name only the taps you show: a tap that is
  not named is not read and costs nothing.
- `--bands` is the panel's band count (1 to 1024); `--low` adds the low zoom (an eighth of the rate, eight times the
  resolution) for the bands under 300 Hz.

## What it writes

```json
{"at":96000,"rate":48000,"hz":[21.1,23.5,...],"post":{"db":[-25.58,...],"peak":[-34.31,...]},"pre":{"db":[...],"peak":[...]}}
```

`at` is the frame count at the end of the window, `hz` each band's centre, `db` the band's level (the RMS across its
bins) and `peak` its loudest bin, in dBFS to 0.01; silence is `null`. A tap's object is there only when the tap is
read. PRE and POST in one line are the same instant: both are taken at one frame position, so POST − PRE per band is
what the processor did to the signal (a steady signal through a plugin with latency reads right; a transient's
difference is smeared by that latency). Nothing is written until every leg holds a whole window of audio since the
reader connected: an empty panel, not a guess.

The numbers come from omx-dsp: the window (`omx_rta_fft_size_for`), the band reduction (`omx_rta_bands`, the same one
the console's server runs), the tier crossover, the quiet gate and the refresh rate (`OMX_RTA_HZ`). A host draws the
lines on the level window `OMX_RTA_LEVEL_DB_MIN` to `OMX_RTA_LEVEL_DB_MAX` with a line every `OMX_RTA_LEVEL_GRID_DB`,
as the console does; the reader does not restate them.

## On demand

Start it when a panel that shows a spectrum opens, end it when the panel closes: SIGTERM ends it and closes its client,
which drops its connections. Run it as a child with its standard output on a pipe: when the reading side goes, the
reader sees a closed output and ends by itself, so it never outlives the panel. Change what the panel shows (`post`,
`pre`, `both`) by starting it again with the taps that mode needs.

Exit status: 0 on SIGTERM or SIGINT, 2 a usage error, 3 its output closed, 4 a watched port is gone (the host regrouped
the processor: start it again on the new ports), 5 the JACK server is gone, 6 the server refused the client or a
connection. The reason is on standard error.

## It only reads

The client registers input ports only and connects them only FROM the ports it watches. `make rta-read-test` proves
it on a private JACK server with the dummy backend: a source plays through a pass-through into a capture, and the
capture is bit-identical, sample for sample against the signal's closed form, with the reader absent and with it
watching both sides of the pass-through; the reader has no output port; every other port's connections are unchanged;
a feeder that joins or leaves the processor's input is followed; POST − PRE through a 0.5 gain is −6.02 dB in every
band. `make rta-read-sabotage` shows the test is not blind: a reader that feeds the processor's input, and one whose
two taps are a period apart, are each caught.

Its process callback only copies each period into omx-dsp's capture ring. `make rta-read-gate` has omx-rt-gate judge
the whole callback (`--abi jack`): no allocation, lock, blocking call or I/O, with `jack_port_get_buffer`, JACK's own
buffer lookup, the one host call it may make.

## Cost

Owed: the reader's CPU per watched panel, on a Raspberry Pi 5 at 48 kHz, for each mode (`post`, `pre`, `both`, with
and without `--low`), is not measured yet. Its audio thread only copies; the cost is the control thread's transforms,
ten times a second, and only while a panel watches. The low zoom (three decimations over 65536 samples) and reading
both taps are the expected large terms. When nothing watches, no reader runs and nothing is spent.
