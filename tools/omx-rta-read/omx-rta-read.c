// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * omx-rta-read — the spectrum of a JACK signal, read and never fed: a thin JACK shell over omx-dsp's capture ring
 * (omx_rta.h), its tap reading (mix/omx_rta_tap.h) and the band reduction (analysis/omx_rta_bands.h).
 *
 *   omx-rta-read [--post <port>]... [--pre <port>]... --bands <n> [--low] [--name <client name>]
 *
 * Two taps, each one or two legs (a pair is mono-summed): POST reads the named output ports; PRE reads what feeds the
 * named input ports, by connecting from every port connected to them, and follows the graph when it changes. The
 * client registers input ports only and connects them only FROM those sources: it never feeds anything.
 *
 * The process callback writes each leg's period into its ring and nothing else. It is stored in the `process` field
 * of an `omx_jack_process` (omx-rt-gate's jack.abi), so the gate judges it whole as an audio root.
 *
 * A control thread (this program's main) wakes at OMX_RTA_HZ; once every leg's ring holds a whole window of audio
 * since the connect it reads each tap at ONE frame position, so PRE and POST are the same instant, reduces each to
 * --bands bands and writes one JSON line:
 *   {"at":<frames>,"rate":<Hz>,"hz":[...],"post":{"db":[...],"peak":[...]},"pre":{"db":[...],"peak":[...]}}
 * a tap's object only when it is read, dB to 0.01, silence as null. Before the first whole window it writes nothing.
 *
 * It ends, closing its client, on SIGTERM or SIGINT (exit 0), a closed stdout (3), a watched port gone (4), the
 * server gone (5); a usage error is 2 and a refusal by the server 6. The reason is on stderr.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <getopt.h>
#include <jack/jack.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <omxdsp/analysis/omx_rta.h>
#include <omxdsp/analysis/omx_rta_bands.h>
#include <omxdsp/mix/omx_rta_tap.h>

enum { EXIT_TERM = 0, EXIT_USAGE = 2, EXIT_STDOUT = 3, EXIT_PORT_GONE = 4, EXIT_SERVER_GONE = 5, EXIT_REFUSED = 6 };
enum { TAP_POST, TAP_PRE, TAPS };
static const char *const TAP_NAMES[TAPS] = {"post", "pre"};
#define LEGS 2
#define BANDS_MAX 1024u

/* omx-rt-gate's jack.abi: the function stored in `process` is the audio root. */
struct omx_jack_process {
  JackProcessCallback process;
  void *arg;
};

struct leg {
  jack_port_t *port;     /* ours, an input */
  const char *watch;     /* POST: the output read; PRE: the input whose feeders are read */
  struct omx_ring ring;
};

struct tap {
  struct leg leg[LEGS];
  uint32_t legs;
};

static struct tap taps[TAPS];
/* Frames written into every ring since activation: stored by the process callback after the rings, the one writer. */
static _Atomic uint64_t frames_total;
static volatile sig_atomic_t stop_signal;
static atomic_int server_gone, graph_changed;

static int process(jack_nframes_t n, void *arg) {
  (void)arg;
  for (uint32_t t = 0; t < TAPS; t++)
    for (uint32_t l = 0; l < taps[t].legs; l++) {
      const float *in = jack_port_get_buffer(taps[t].leg[l].port, n);
      omx_ring_write(&taps[t].leg[l].ring, in, n);
    }
  atomic_store_explicit(&frames_total, atomic_load_explicit(&frames_total, memory_order_relaxed) + n,
                        memory_order_release);
  return 0;
}

static const struct omx_jack_process RT = {.process = process, .arg = NULL};

static void on_signal(int sig) { stop_signal = sig; }
static void on_shutdown(void *arg) { (void)arg; atomic_store(&server_gone, 1); }
static int on_graph(void *arg) { (void)arg; atomic_store(&graph_changed, 1); return 0; }

static int usage(const char *why) {
  fprintf(stderr, "omx-rta-read: %s\nusage: omx-rta-read [--post <port>]... [--pre <port>]... --bands <n> [--low] "
                  "[--name <client name>]\n", why);
  return EXIT_USAGE;
}

/* PRE: make leg `lg`'s connections the set connected to its watched input, never its own ports. */
static int follow_feeders(jack_client_t *c, struct leg *lg) {
  jack_port_t *in = jack_port_by_name(c, lg->watch);
  if (!in) return -1;
  const char **want = jack_port_get_all_connections(c, in);
  const char **have = jack_port_get_all_connections(c, lg->port);
  const char *mine = jack_port_name(lg->port);
  for (const char **h = have; h && *h; h++) {
    int keep = 0;
    for (const char **w = want; w && *w; w++) keep |= !strcmp(*h, *w);
    if (!keep) jack_disconnect(c, *h, mine);
  }
  for (const char **w = want; w && *w; w++) {
    int got = 0;
    for (const char **h = have; h && *h; h++) got |= !strcmp(*h, *w);
    if (!got && jack_port_by_name(c, *w)) jack_connect(c, *w, mine);
  }
  jack_free(want);
  jack_free(have);
  return 0;
}

/* Whether the low zoom is due: never read yet, or its window has advanced OMX_RTA_LOW_ADVANCE of itself since. */
static int low_due(uint64_t at, uint64_t last, int have) {
  return !have || (double)(at - last) >= OMX_RTA_LOW_ADVANCE_DOUBLE * (double)OMX_RTA_LOW_WINDOW;
}

static void print_db(FILE *o, double v) {
  if (isinf(v) && v < 0) fputs("null", o);
  else fprintf(o, "%.2f", v);
}

static struct omx_rta_tap_window win;
static struct omx_rta_tap_work work;
static struct omx_rta_tap_reading reading[TAPS];
static float leg_buf[LEGS][OMX_RTA_LOW_WINDOW];
static struct omx_rta_band bands[TAPS][BANDS_MAX];

/* One tap's window at frame position `pos` (every ring's write index), mono-summed into `w`. */
static void tap_window(const struct tap *t, uint32_t pos, float *w, uint32_t n) {
  for (uint32_t l = 0; l < t->legs; l++) omx_ring_read_from(&t->leg[l].ring, leg_buf[l], n, pos, 0);
  if (t->legs == 2) omx_rta_tap_mono_sum(w, leg_buf[0], leg_buf[1], n);
  else memcpy(w, leg_buf[0], n * sizeof(float));
}

int main(int argc, char **argv) {
  static const struct option opts[] = {{"post", required_argument, 0, 'o'}, {"pre", required_argument, 0, 'i'},
                                       {"bands", required_argument, 0, 'b'}, {"low", no_argument, 0, 'l'},
                                       {"name", required_argument, 0, 'n'}, {0, 0, 0, 0}};
  const char *name = "omx-rta-read";
  long nb = 0;
  int low = 0, k;
  while ((k = getopt_long(argc, argv, "", opts, NULL)) != -1) {
    struct tap *t = k == 'o' ? &taps[TAP_POST] : k == 'i' ? &taps[TAP_PRE] : NULL;
    if (t) {
      if (t->legs == LEGS) return usage("a tap has two legs at most");
      t->leg[t->legs++].watch = optarg;
    } else if (k == 'b') {
      char *end;
      nb = strtol(optarg, &end, 10);
      if (*end || nb < 1 || nb > (long)BANDS_MAX) return usage("--bands is 1 to 1024");
    } else if (k == 'l') low = 1;
    else if (k == 'n') name = optarg;
    else return usage("unknown option");
  }
  if (optind != argc) return usage("unexpected argument");
  if (!taps[TAP_POST].legs && !taps[TAP_PRE].legs) return usage("name a tap: --post or --pre");
  if (!nb) return usage("--bands is required");

  struct sigaction sa = {0};
  sa.sa_handler = on_signal;
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  signal(SIGPIPE, SIG_IGN);

  jack_client_t *c = jack_client_open(name, JackNoStartServer, NULL);
  if (!c) { fprintf(stderr, "omx-rta-read: ended: no JACK server\n"); return EXIT_REFUSED; }
  jack_on_shutdown(c, on_shutdown, NULL);
  jack_set_graph_order_callback(c, on_graph, NULL);
  for (uint32_t t = 0; t < TAPS; t++)
    for (uint32_t l = 0; l < taps[t].legs; l++) {
      char pn[32];
      snprintf(pn, sizeof pn, "%s_%u", TAP_NAMES[t], l + 1);
      taps[t].leg[l].port = jack_port_register(c, pn, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
      if (!taps[t].leg[l].port || !jack_port_by_name(c, taps[t].leg[l].watch)) {
        fprintf(stderr, "omx-rta-read: ended: cannot watch %s\n", taps[t].leg[l].watch);
        jack_client_close(c);
        return EXIT_REFUSED;
      }
    }
  if (jack_set_process_callback(c, RT.process, RT.arg) || jack_activate(c)) {
    fprintf(stderr, "omx-rta-read: ended: the server refused the client\n");
    jack_client_close(c);
    return EXIT_REFUSED;
  }
  for (uint32_t l = 0; l < taps[TAP_POST].legs; l++) {
    struct leg *lg = &taps[TAP_POST].leg[l];
    if (jack_connect(c, lg->watch, jack_port_name(lg->port)) && errno != EEXIST) {
      fprintf(stderr, "omx-rta-read: ended: cannot connect from %s\n", lg->watch);
      jack_client_close(c);
      return EXIT_REFUSED;
    }
  }
  for (uint32_t l = 0; l < taps[TAP_PRE].legs; l++) follow_feeders(c, &taps[TAP_PRE].leg[l]);

  const double rate = (double)jack_get_sample_rate(c);
  const uint32_t n = omx_rta_fft_size_for((uint32_t)rate);
  const long period_ns = 1000000000L / OMX_RTA_HZ;
  uint64_t low_at = 0;
  int low_have = 0, code = EXIT_TERM;
  const char *why = "signal";
  struct timespec next;
  clock_gettime(CLOCK_MONOTONIC, &next);
  while (!stop_signal) {
    next.tv_nsec += period_ns;
    if (next.tv_nsec >= 1000000000L) next.tv_nsec -= 1000000000L, next.tv_sec++;
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    if (stop_signal) break;
    if (atomic_load(&server_gone)) { code = EXIT_SERVER_GONE, why = "the JACK server is gone"; break; }
    int gone = 0;
    for (uint32_t t = 0; t < TAPS; t++)
      for (uint32_t l = 0; l < taps[t].legs; l++) gone |= !jack_port_by_name(c, taps[t].leg[l].watch);
    if (gone) { code = EXIT_PORT_GONE, why = "a watched port is gone"; break; }
    if (atomic_exchange(&graph_changed, 0))
      for (uint32_t l = 0; l < taps[TAP_PRE].legs; l++) follow_feeders(c, &taps[TAP_PRE].leg[l]);

    /* the first-frame rule: every leg's window is audio since the connect */
    int ready = 1;
    for (uint32_t t = 0; t < TAPS; t++)
      for (uint32_t l = 0; l < taps[t].legs; l++) ready &= omx_ring_has(&taps[t].leg[l].ring, n);
    if (!ready) continue;
    /* one position for every ring: all were written the same frames by the same calls */
    const uint64_t at = atomic_load_explicit(&frames_total, memory_order_acquire);
    const uint32_t pos = (uint32_t)(at % OMX_RTA_RING_SIZE);
    int do_low = low && low_due(at, low_at, low_have);
    for (uint32_t t = 0; t < TAPS; t++)
      do_low &= taps[t].legs == 0 || omx_ring_has(&taps[t].leg[0].ring, OMX_RTA_LOW_WINDOW);
    for (uint32_t t = 0; t < TAPS; t++) {
      if (!taps[t].legs) continue;
      tap_window(&taps[t], pos, win.block, n);
      if (do_low) tap_window(&taps[t], pos, win.block_low, OMX_RTA_LOW_WINDOW);
      omx_rta_tap_read(&win, &work, &reading[t], n, do_low);
    }
    if (do_low) low_at = at, low_have = 1;
    for (uint32_t t = 0; t < TAPS; t++) {
      if (!taps[t].legs) continue;
      const struct omx_rta_tier hi = {reading[t].mags, n / 2u, rate, n};
      const struct omx_rta_tier lo = {reading[t].low, OMX_RTA_LOW_FFT_SIZE / 2u, rate / OMX_RTA_LOW_DECIM_FACTOR,
                                      OMX_RTA_LOW_FFT_SIZE};
      omx_rta_bands(&hi, low_have ? &lo : NULL, (uint32_t)nb, bands[t]);
    }
    const uint32_t first = taps[TAP_POST].legs ? TAP_POST : TAP_PRE;
    printf("{\"at\":%llu,\"rate\":%.0f,\"hz\":[", (unsigned long long)at, rate);
    for (long b = 0; b < nb; b++) printf("%s%.6g", b ? "," : "", bands[first][b].hz);
    printf("]");
    for (uint32_t t = 0; t < TAPS; t++) {
      if (!taps[t].legs) continue;
      printf(",\"%s\":{\"db\":[", TAP_NAMES[t]);
      for (long b = 0; b < nb; b++) { if (b) putchar(','); print_db(stdout, bands[t][b].db); }
      printf("],\"peak\":[");
      for (long b = 0; b < nb; b++) { if (b) putchar(','); print_db(stdout, bands[t][b].peak_db); }
      printf("]}");
    }
    printf("}\n");
    if (fflush(stdout) != 0 || ferror(stdout)) { code = EXIT_STDOUT, why = "stdout is closed"; break; }
  }
  jack_deactivate(c);
  jack_client_close(c);
  fprintf(stderr, "omx-rta-read: ended: %s\n", why);
  return code;
}
