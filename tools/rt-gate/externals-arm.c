// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/*
 * externals-arm.c — the arm that measured every row of omx-rt-gate's class table (classes.tbl).
 *
 * Each case calls its symbol over a spread of arguments, through the real symbol (built -O0
 * -fno-builtin, so no call is folded or inlined away), with the program's allocator interposed: the
 * process's malloc, calloc, realloc, free and the aligned allocators count every call made while a
 * case runs, from this program or from inside the library. A case passes when the symbol ran and
 * nothing was allocated or freed. It prints `measured <case> calls=<n> allocations=0`.
 *
 *   externals-arm            every case
 *   externals-arm <case>...  the cases named
 *   externals-arm --sabotage the arm's own check: a case that allocates (strdup) must be read red
 *   externals-arm --sabotage-jack  the same for the jack case: an allocation beside the call is read red
 *
 * jack_port_get_buffer (class host-buffer) is measured where JACK documents it, on the process thread: built with
 * -DOMX_RT_GATE_ARM_JACK and -ljack, the arm starts a private server on the dummy backend, registers a port, and
 * counts the allocations made on the process thread around each call (a thread-local window, so the client's other
 * threads are not charged to it). It measures jack2 (ruled 2026-11-15): run it with jack2's libdir first on
 * LD_LIBRARY_PATH (tools/rt-gate/jack2-env.sh, as make rt-gate-arm and sabotage.sh do); it prints jackd's version,
 * the libjackserver jackd loaded and the libjack it loaded, and a PipeWire library in either is FAILED by name.
 * Built without JACK, or with no jackd to start, the case prints `SKIPPED jack_port_get_buffer <why>`: named, never
 * measured, never a pass. JACK present but failing (the server dies, never comes up, refuses the client or stops
 * cycling: every wait is bounded) is FAILED with its reason.
 *
 * What it cannot see: a lock taken inside the library (glibc's internal calls do not go through the
 * interposable symbols), and time. The class table says so beside the classes this matters for.
 */
#define _GNU_SOURCE
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern void *__libc_malloc(size_t);
extern void *__libc_calloc(size_t, size_t);
extern void *__libc_realloc(void *, size_t);
extern void __libc_free(void *);
extern void *__libc_memalign(size_t, size_t);

static volatile int armed;
static _Thread_local int armed_here; /* the jack case's window: this thread only */
static volatile long allocations;
#define ARMED (armed || armed_here)
void *malloc(size_t n) { if (ARMED) allocations++; return __libc_malloc(n); }
void *calloc(size_t a, size_t b) { if (ARMED) allocations++; return __libc_calloc(a, b); }
void *realloc(void *p, size_t n) { if (ARMED) allocations++; return __libc_realloc(p, n); }
void free(void *p) { if (ARMED && p) allocations++; __libc_free(p); }
void *memalign(size_t a, size_t n) { if (ARMED) allocations++; return __libc_memalign(a, n); }
void *aligned_alloc(size_t a, size_t n) { if (ARMED) allocations++; return __libc_memalign(a, n); }
int posix_memalign(void **p, size_t a, size_t n) {
  if (ARMED) allocations++;
  *p = __libc_memalign(a, n);
  return *p ? 0 : 12;
}

#define N 4096
static volatile float sinkf;
static volatile double sinkd;
static volatile long sinkl;
static float xf(int i) { return -20.0f + 40.0f * (float)i / N; }
static double xd(int i) { return -20.0 + 40.0 * (double)i / N; }
static float pf(int i) { return 1e-6f + 100.0f * (float)i / N; }
static double pd(int i) { return 1e-9 + 100.0 * (double)i / N; }

#define F1(f, arg) static long case_##f(void) { for (int i = 0; i < N; i++) sinkf = f(arg(i)); return N; }
#define D1(f, arg) static long case_##f(void) { for (int i = 0; i < N; i++) sinkd = f(arg(i)); return N; }
#define F2(f, a, b) static long case_##f(void) { for (int i = 0; i < N; i++) sinkf = f(a(i), b(N - 1 - i)); return N; }
#define D2(f, a, b) static long case_##f(void) { for (int i = 0; i < N; i++) sinkd = f(a(i), b(N - 1 - i)); return N; }
F1(sinf, xf) F1(cosf, xf) F1(tanf, xf) F1(tanhf, xf) F1(expf, xf) F1(exp2f, xf) F1(logf, pf) F1(log2f, pf)
F1(log10f, pf) F1(sqrtf, pf) F1(floorf, xf) F1(ceilf, xf) F2(powf, pf, xf) F2(fminf, xf, xf) F2(fmaxf, xf, xf)
F2(nextafterf, xf, xf)
D1(sin, xd) D1(cos, xd) D1(tan, xd) D1(sinh, xd) D1(exp, xd) D1(log, pd) D1(log2, pd) D1(log10, pd) D1(sqrt, pd)
D1(floor, xd) D1(ceil, xd) D2(pow, pd, xd) D2(hypot, xd, xd) D2(atan2, xd, xd) D2(fmin, xd, xd) D2(fmax, xd, xd)
D2(copysign, xd, xd)
static long case_lrintf(void) { for (int i = 0; i < N; i++) sinkl = lrintf(xf(i)); return N; }
static long case_lroundf(void) { for (int i = 0; i < N; i++) sinkl = lroundf(xf(i)); return N; }
static long case_lrint(void) { for (int i = 0; i < N; i++) sinkl = lrint(xd(i)); return N; }
static long case_llround(void) { for (int i = 0; i < N; i++) sinkl = llround(xd(i)); return N; }
static long case_lround(void) { for (int i = 0; i < N; i++) sinkl = lround(xd(i)); return N; }
static long case_ldexp(void) { for (int i = 0; i < N; i++) sinkd = ldexp(xd(i), i % 64 - 32); return N; }

static char a[4096], b[4096];
static long case_memcpy(void) { for (int i = 0; i < N; i++) memcpy(a, b, (size_t)(i % 4096)); return N; }
static long case_memmove(void) { for (int i = 0; i < N; i++) memmove(a + 1, a, (size_t)(i % 4095)); return N; }
static long case_memset(void) { for (int i = 0; i < N; i++) memset(a, i, (size_t)(i % 4096)); return N; }
static long case_memcmp(void) { for (int i = 0; i < N; i++) sinkl = memcmp(a, b, (size_t)(i % 4096)); return N; }

static sem_t sem;
static long case_sem_post(void) {
  /* no waiter: an atomic increment and no system call */
  for (int i = 0; i < N; i++) sem_post(&sem);
  return N;
}
static long case_clock_gettime(void) {
  struct timespec ts;
  for (int i = 0; i < N; i++) {
    clock_gettime(CLOCK_MONOTONIC, &ts);
    sinkl = ts.tv_nsec;
  }
  return N;
}
/* thread-identity: the calling thread's own identity, read from its descriptor. pthread_gettid_np exists from glibc
 * 2.42; on an older glibc the case reads pthread_self instead, as no caller there can name the symbol. */
static pthread_t self0;
static volatile pid_t sinkt;
static long case_pthread_self(void) { for (int i = 0; i < N; i++) sinkl = (long)pthread_self(); return N; }
static long case_pthread_equal(void) { for (int i = 0; i < N; i++) sinkl = pthread_equal(self0, pthread_self()); return N; }
static long case_pthread_gettid_np(void) {
#if defined(__GLIBC__) && __GLIBC_PREREQ(2, 42)
  for (int i = 0; i < N; i++) sinkt = pthread_gettid_np(pthread_self());
#else
  for (int i = 0; i < N; i++) sinkl = (long)pthread_self();
#endif
  return N;
}
/* libgcc's outline atomics: the calls gcc emits for an atomic add on aarch64 (-moutline-atomics). The arm calls
 * each helper by its name there. The helpers exist only on aarch64; on another target the case runs the same
 * add, which that compiler emits inline, so the row's case is always measured and is the real symbol where the
 * symbol exists. */
static int word4;
static long word8;
#if defined(__aarch64__)
extern int __aarch64_ldadd4_relax(int, int *);
extern int __aarch64_ldadd4_rel(int, int *);
extern long __aarch64_ldadd8_rel(long, long *);
#else
static int __aarch64_ldadd4_relax(int v, int *p) { return __atomic_fetch_add(p, v, __ATOMIC_RELAXED); }
static int __aarch64_ldadd4_rel(int v, int *p) { return __atomic_fetch_add(p, v, __ATOMIC_RELEASE); }
static long __aarch64_ldadd8_rel(long v, long *p) { return __atomic_fetch_add(p, v, __ATOMIC_RELEASE); }
#endif
static long case___aarch64_ldadd4_relax(void) { for (int i = 0; i < N; i++) sinkl = __aarch64_ldadd4_relax(i, &word4); return N; }
static long case___aarch64_ldadd4_rel(void) { for (int i = 0; i < N; i++) sinkl = __aarch64_ldadd4_rel(i, &word4); return N; }
static long case___aarch64_ldadd8_rel(void) { for (int i = 0; i < N; i++) sinkl = __aarch64_ldadd8_rel(i, &word8); return N; }

static long case_strdup_sabotage(void) {
  for (int i = 0; i < 4; i++) {
    char *s = strdup("x");
    sinkl = s[0];
    free(s);
  }
  return 4;
}

#ifdef OMX_RT_GATE_ARM_JACK
#include <jack/jack.h>
#include <signal.h>
#include <spawn.h>
#include <stdatomic.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;
static jack_port_t *arm_port;
static _Atomic long jack_calls, jack_cycles;
static volatile int jack_window, jack_sabotage;

/* The process callback: the window opens around the calls only, on this thread only. */
static int arm_process(jack_nframes_t nframes, void *arg) {
  (void)arg;
  if (!jack_window) return 0;
  armed_here = 1;
  for (int i = 0; i < 64; i++) {
    float *buf = jack_port_get_buffer(arm_port, nframes);
    sinkl = buf ? (long)buf[0] : 0;
    if (jack_sabotage) free(strdup("x"));
  }
  armed_here = 0;
  atomic_fetch_add(&jack_calls, 64);
  atomic_fetch_add(&jack_cycles, 1);
  return 0;
}

/* The path of the first mapping of LIB (e.g. "/libjack.so") in /proc/PID/maps, or "" when none. */
static void jack_mapped(const char *pid, const char *lib, char *out, size_t n) {
  char maps[64], line[1024];
  out[0] = 0;
  snprintf(maps, sizeof maps, "/proc/%s/maps", pid);
  FILE *f = fopen(maps, "r");
  if (!f) return;
  while (fgets(line, sizeof line, f)) {
    char *path = strchr(line, '/');
    if (path && strstr(path, lib)) {
      path[strcspn(path, "\n")] = 0;
      snprintf(out, n, "%s", path);
      break;
    }
  }
  fclose(f);
}

/* Names the JACK this run measures (ruled 2026-11-15: jack2, as tools/rt-gate/jack2-env.sh puts it first): jackd's
 * version, the libjackserver jackd loaded and the libjack this arm loaded. 0, with FAILED by name, when either is
 * PipeWire's or is not mapped. */
static int jack_named(const char *name, pid_t jackd) {
  char pid[32], server[512], client[512], version[256] = "";
  snprintf(pid, sizeof pid, "%ld", (long)jackd);
  jack_mapped(pid, "/libjackserver.so", server, sizeof server);
  jack_mapped("self", "/libjack.so", client, sizeof client);
  FILE *v = popen("jackd --version 2>&1", "r");
  if (v) {
    char line[256];
    while (fgets(line, sizeof line, v))
      if (!version[0] && strstr(line, "version")) {
        line[strcspn(line, "\n")] = 0;
        snprintf(version, sizeof version, "%s", line);
      }
    pclose(v);
  }
  printf("jack: %s; jackd's libjackserver %s; this arm's libjack %s\n", version[0] ? version : "jackd gave no version",
         server[0] ? server : "(none mapped)", client[0] ? client : "(none mapped)");
  if (!server[0] || !client[0] || strstr(server, "pipewire") || strstr(client, "pipewire")) {
    printf("FAILED %s the loaded JACK is not jack2's (PipeWire's, or none mapped): put jack2's libdir first "
           "(tools/rt-gate/jack2-env.sh)\n", name);
    return 0;
  }
  return 1;
}

/* 1 when nothing allocated (or the case is SKIPPED, JACK being absent), 0 when something did (ALLOCATES), -1 when
 * JACK is here and the case could not be measured (FAILED, with its reason). */
static int measure_jack(void) {
  const char *name = "jack_port_get_buffer";
  char server[64];
  snprintf(server, sizeof server, "omx-rt-gate-arm-%ld", (long)getpid());
  setenv("JACK_NO_AUDIO_RESERVATION", "1", 1);
  setenv("JACK_DEFAULT_SERVER", server, 1);
  /* -p 64 before -d is the server's port maximum: jack2 sizes its shared memory by it, and the default (thousands of
   * ports, ~100 MB) does not fit a container's /dev/shm (the server dies of SIGBUS). The -p after -d is the period. */
  char *argv[] = {"jackd", "--no-realtime", "-p", "64", "-n", server, "-d", "dummy", "-r", "48000", "-p", "256", NULL};
  char own[512];
  jack_mapped("self", "/libjack.so", own, sizeof own);
  if (strstr(own, "pipewire")) {
    printf("jack: this arm's libjack %s\n", own);
    printf("FAILED %s the loaded libjack is PipeWire's, not jack2's: put jack2's libdir first "
           "(tools/rt-gate/jack2-env.sh)\n", name);
    return -1;
  }
  pid_t jackd;
  if (posix_spawnp(&jackd, "jackd", NULL, NULL, argv, environ) != 0) {
    printf("SKIPPED %s no jackd to start a dummy server\n", name);
    return 1;
  }
  /* JACK is here, so from now on a failure is red with its reason, never SKIPPED, and every wait is
   * bounded: a server that dies or stops cycling ends the case instead of hanging the gate. */
  jack_client_t *client = NULL;
  int status = 0, gone = 0;
  for (int t = 0; t < 100 && !client && !gone; t++) {
    if (waitpid(jackd, &status, WNOHANG) == jackd) { gone = 1; break; }
    client = jack_client_open("externals-arm", JackNoStartServer, NULL);
    if (!client) usleep(100000);
  }
  int ok = -1;
  if (gone) {
    if (WIFSIGNALED(status))
      printf("FAILED %s the dummy jackd died of signal %d before serving\n", name, WTERMSIG(status));
    else
      printf("FAILED %s the dummy jackd exited %d before serving\n", name, WEXITSTATUS(status));
    return -1;
  }
  if (!client) {
    printf("FAILED %s the dummy server did not come up in 10 s\n", name);
  } else if (jack_named(name, jackd)) {
    arm_port = jack_port_register(client, "in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    jack_set_process_callback(client, arm_process, NULL);
    if (!arm_port || jack_activate(client) != 0) {
      printf("FAILED %s the dummy server refused the client\n", name);
    } else {
      usleep(200000); /* warm: the first cycles run outside the window */
      allocations = 0;
      jack_window = 1;
      /* 200 periods of 256 frames at 48 kHz are about 1.1 s; 20 s is a server that stopped */
      for (int t = 0; t < 2000 && atomic_load(&jack_cycles) < 200; t++) usleep(10000);
      jack_window = 0;
      usleep(50000);
      long got = allocations, cycles = atomic_load(&jack_cycles);
      if (cycles < 200) {
        printf("FAILED %s the dummy server ran %ld of 200 cycles in 20 s\n", name, cycles);
      } else {
        printf("%s %s calls=%ld allocations=%ld\n", got ? "ALLOCATES" : "measured", name, atomic_load(&jack_calls), got);
        ok = got == 0 ? 1 : 0;
      }
      jack_deactivate(client);
    }
  }
  if (client) jack_client_close(client);
  kill(jackd, SIGTERM);
  waitpid(jackd, NULL, 0);
  return ok;
}
#else
static int measure_jack(void) {
  printf("SKIPPED jack_port_get_buffer built without JACK (no jack.pc)\n");
  return 1;
}
#endif

struct arm_case { const char *name; long (*fn)(void); };
#define C(f) {#f, case_##f}
static const struct arm_case CASES[] = {
    C(sinf), C(cosf), C(tanf), C(tanhf), C(expf), C(exp2f), C(logf), C(log2f), C(log10f), C(sqrtf), C(floorf),
    C(ceilf), C(powf), C(fminf), C(fmaxf), C(nextafterf), C(lrintf), C(lroundf),
    C(sin), C(cos), C(tan), C(sinh), C(exp), C(log), C(log2), C(log10), C(sqrt), C(floor), C(ceil), C(pow),
    C(hypot), C(atan2), C(fmin), C(fmax), C(copysign), C(lrint), C(lround), C(llround), C(ldexp),
    C(memcpy), C(memmove), C(memset), C(memcmp), C(sem_post), C(clock_gettime),
    C(pthread_self), C(pthread_equal), C(pthread_gettid_np),
    C(__aarch64_ldadd4_relax), C(__aarch64_ldadd4_rel), C(__aarch64_ldadd8_rel),
};

static int measure(const struct arm_case *c) {
  c->fn(); /* warm: lazy binding and first-touch happen outside the window */
  allocations = 0;
  armed = 1;
  long calls = c->fn();
  armed = 0;
  long got = allocations;
  printf("%s %s calls=%ld allocations=%ld\n", got ? "ALLOCATES" : "measured", c->name, calls, got);
  return got == 0;
}

int main(int argc, char **argv) {
  sem_init(&sem, 0, 0);
  self0 = pthread_self();
  memset(b, 1, sizeof b);
#ifdef OMX_RT_GATE_ARM_JACK
  if (argc == 2 && !strcmp(argv[1], "--sabotage-jack")) {
    jack_sabotage = 1;
    int r = measure_jack();
    if (r != 0) {
      printf("externals-arm --sabotage-jack: FAIL, %s\n", r < 0 ? "the case could not be measured (FAILED above)"
                                                              : "an allocation beside jack_port_get_buffer was not read red");
      return 1;
    }
    printf("externals-arm --sabotage-jack: ok, an allocation on the process thread is read red\n");
    return 0;
  }
#endif
  if (argc == 2 && !strcmp(argv[1], "--sabotage")) {
    struct arm_case s = {"strdup", case_strdup_sabotage};
    if (measure(&s)) {
      printf("externals-arm --sabotage: FAIL, a case that allocates was read clean\n");
      return 1;
    }
    printf("externals-arm --sabotage: ok, an allocating case is read red\n");
    return 0;
  }
  int ok = 1, n = (int)(sizeof CASES / sizeof CASES[0]);
  for (int i = 0; i < n; i++) {
    int want = argc == 1;
    for (int k = 1; k < argc; k++)
      if (!strcmp(argv[k], CASES[i].name)) want = 1;
    if (want) ok &= measure(&CASES[i]);
  }
  int want_jack = argc == 1;
  for (int k = 1; k < argc; k++)
    if (!strcmp(argv[k], "jack_port_get_buffer")) want_jack = 1;
  if (want_jack) ok &= measure_jack() == 1;
  for (int k = 1; k < argc; k++) {
    int found = !strcmp(argv[k], "jack_port_get_buffer");
    for (int i = 0; i < n; i++)
      if (!strcmp(argv[k], CASES[i].name)) found = 1;
    if (!found) {
      printf("NO-CASE %s\n", argv[k]);
      ok = 0;
    }
  }
  return ok ? 0 : 1;
}
