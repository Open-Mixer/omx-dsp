// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * rta-read-fixture — the JACK clients omx-rta-read's null test (test/rta-read/null-test.sh) runs it among.
 *
 *   rta-read-fixture source <name>             one output `out` playing fx(frame): a deterministic function of the
 *                                              absolute frame (three sines and a hashed noise), never a clock
 *   rta-read-fixture pass <name> <gain>        `in` -> `out`, each sample times <gain> (a float, as written)
 *   rta-read-fixture capture <name> <from> <gain> <n>
 *                                              `in`, connected from <from>; once connected it checks n samples against
 *                                              fx(frame) * gain BIT FOR BIT; prints `capture checked=<n> mismatches=<m>`
 *   rta-read-fixture connect <from> <to>       one connection
 *   rta-read-fixture disconnect <from> <to>    one disconnection
 *   rta-read-fixture graph                     every port, `port <name> in|out`, then each of its connections,
 *                                              `conn <port> <other>`: what jack_lsp -p -c reads, by the API
 *   rta-read-fixture watch <seconds>           the server's DSP load (jack_cpu_load, sampled ten times a second) and
 *                                              its xruns (its xrun callback) over <seconds>; prints
 *                                              `watch seconds=<s> load_mean=<%> load_max=<%> xruns=<n>`
 * source and pass run until SIGTERM. Exit 0, or 1 on a mismatch or a refusal.
 */
#define _GNU_SOURCE
#include <jack/jack.h>
#include <math.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static jack_client_t *client;
static jack_port_t *in, *out;
static float gain = 1.0f;
static volatile sig_atomic_t done;
static long want, checked, mismatches;
static _Atomic int finished, armed;

/* The fixture's signal at absolute frame f: never a clock, so every run plays the same samples at the same frames. */
static float fx(uint64_t f) {
  uint32_t h = (uint32_t)(f * 2654435761u);
  h ^= h >> 15, h *= 2246822519u, h ^= h >> 13;
  const double t = (double)(f % 48000u) / 48000.0;
  return (float)(0.2 * sin(2.0 * M_PI * 110.0 * t) + 0.1 * sin(2.0 * M_PI * 1000.0 * t) +
                 0.05 * sin(2.0 * M_PI * 9000.0 * t) + 0.05 * ((double)(h & 0xffff) / 32768.0 - 1.0));
}

static int run_source(jack_nframes_t n, void *arg) {
  (void)arg;
  float *o = jack_port_get_buffer(out, n);
  const uint64_t f0 = jack_last_frame_time(client);
  for (jack_nframes_t i = 0; i < n; i++) o[i] = fx(f0 + i);
  return 0;
}

static int run_pass(jack_nframes_t n, void *arg) {
  (void)arg;
  const float *x = jack_port_get_buffer(in, n);
  float *y = jack_port_get_buffer(out, n);
  for (jack_nframes_t i = 0; i < n; i++) y[i] = x[i] * gain;
  return 0;
}

static int run_capture(jack_nframes_t n, void *arg) {
  (void)arg;
  const float *x = jack_port_get_buffer(in, n);
  const uint64_t f0 = jack_last_frame_time(client);
  if (!atomic_load(&armed)) return 0;
  for (jack_nframes_t i = 0; i < n; i++) {
    if (checked >= want) continue;
    const float e = fx(f0 + i) * gain;
    if (memcmp(&x[i], &e, sizeof e) != 0) mismatches++;
    checked++;
  }
  if (checked >= want) atomic_store(&finished, 1);
  return 0;
}

static void on_term(int s) { (void)s; done = 1; }

static atomic_long xruns;
static int on_xrun(void *arg) { (void)arg; atomic_fetch_add(&xruns, 1); return 0; }

static int open_client(const char *name) {
  client = jack_client_open(name, JackNoStartServer, NULL);
  if (!client) fprintf(stderr, "rta-read-fixture: no server for %s\n", name);
  return client != NULL;
}

int main(int argc, char **argv) {
  if (argc < 2) return 1;
  const char *mode = argv[1];
  signal(SIGTERM, on_term);
  signal(SIGINT, on_term);
  if (!strcmp(mode, "connect") || !strcmp(mode, "disconnect")) {
    if (argc != 4 || !open_client("rta-read-fixture-ctl")) return 1;
    const int r = mode[0] == 'c' ? jack_connect(client, argv[2], argv[3]) : jack_disconnect(client, argv[2], argv[3]);
    jack_client_close(client);
    return r == 0 ? 0 : 1;
  }
  if (!strcmp(mode, "graph")) {
    if (!open_client("rta-read-fixture-graph")) return 1;
    const char **ports = jack_get_ports(client, NULL, NULL, 0);
    for (const char **p = ports; p && *p; p++) {
      if (!strncmp(*p, "rta-read-fixture-graph:", 23)) continue;
      jack_port_t *pt = jack_port_by_name(client, *p);
      printf("port %s %s\n", *p, (jack_port_flags(pt) & JackPortIsOutput) ? "out" : "in");
      const char **cs = jack_port_get_all_connections(client, pt);
      for (const char **c = cs; c && *c; c++) printf("conn %s %s\n", *p, *c);
      jack_free(cs);
    }
    jack_free(ports);
    jack_client_close(client);
    return 0;
  }
  if (!strcmp(mode, "watch")) {
    const double seconds = argc == 3 ? strtod(argv[2], NULL) : 0.0;
    if (!(seconds > 0.0) || !open_client("rta-read-fixture-watch")) return 1;
    if (jack_set_xrun_callback(client, on_xrun, NULL) || jack_activate(client)) return 1;
    double sum = 0.0, top = 0.0;
    long samples = 0;
    for (long i = 0; i < (long)(seconds * 10.0) && !done; i++) {
      usleep(100000);
      const double l = jack_cpu_load(client);
      sum += l, samples++;
      if (l > top) top = l;
    }
    jack_deactivate(client);
    printf("watch seconds=%.0f load_mean=%.2f load_max=%.2f xruns=%ld\n", samples / 10.0, samples ? sum / samples : 0.0,
           top, atomic_load(&xruns));
    jack_client_close(client);
    return 0;
  }
  if (argc < 3 || !open_client(argv[2])) return 1;
  JackProcessCallback cb = NULL;
  if (!strcmp(mode, "source")) {
    out = jack_port_register(client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    cb = run_source;
  } else if (!strcmp(mode, "pass") && argc == 4) {
    gain = strtof(argv[3], NULL);
    in = jack_port_register(client, "in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    out = jack_port_register(client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    cb = run_pass;
  } else if (!strcmp(mode, "capture") && argc == 6) {
    gain = strtof(argv[4], NULL);
    want = strtol(argv[5], NULL, 10);
    in = jack_port_register(client, "in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    cb = run_capture;
  } else {
    fprintf(stderr, "rta-read-fixture: unknown mode %s\n", mode);
    return 1;
  }
  if (jack_set_process_callback(client, cb, NULL) || jack_activate(client)) return 1;
  if (cb == run_capture) {
    /* connected from <from>, settled, then checked; it waits for its samples, at most 30 s */
    char mine[300];
    snprintf(mine, sizeof mine, "%s:in", argv[2]);
    if (jack_connect(client, argv[3], mine)) { fprintf(stderr, "rta-read-fixture: cannot connect %s\n", argv[3]); return 1; }
    usleep(200000);
    atomic_store(&armed, 1);
    for (int i = 0; i < 3000 && !atomic_load(&finished) && !done; i++) usleep(10000);
    jack_deactivate(client);
    printf("capture checked=%ld mismatches=%ld\n", checked, mismatches);
    jack_client_close(client);
    return checked == want && mismatches == 0 ? 0 : 1;
  }
  while (!done) usleep(50000);
  jack_deactivate(client);
  jack_client_close(client);
  return 0;
}
