// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas
/**
 * @file child_run.h
 * @brief Run one test body in a forked child, so a crash reads as a failed check, not a dead oracle.
 *
 * A check whose broken form dereferences a bad pointer (a NULL, a pointer past a pool) would kill the
 * whole test binary and hide which arm caught it. The body runs in its own child instead; the parent
 * counts it as passed only when the child exits 0, and names the signal when one kills it. Used by
 * test/fx/reverb.test.c (the pool-wrap arm) and test/analysis/hrp_track.test.c (the NULL doors).
 */
#ifndef OMX_TEST_CHILD_RUN_H
#define OMX_TEST_CHILD_RUN_H

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/** The body: returns the child's exit status, 0 for "held". */
typedef int (*omx_test_child_body)(int which);

/**
 * Run `body(which)` in a forked child. Returns 1 when the child exited with status 0; 0 when it
 * exited non-zero, was killed (the signal is printed to stderr with `what`), or could not be forked
 * or reaped. waitpid is retried on EINTR, so a signal to the parent never leaves the child unreaped
 * nor turns a held arm into a failure.
 */
static inline int omx_test_child_ok(omx_test_child_body body, int which, const char *what) {
  fflush(NULL);
  const pid_t pid = fork();
  if (pid < 0) {
    fprintf(stderr, "%s: fork failed\n", what);
    return 0;
  }
  if (pid == 0) _exit(body(which));
  int status = 0;
  pid_t got;
  do {
    got = waitpid(pid, &status, 0);
  } while (got < 0 && errno == EINTR);
  if (got != pid) {
    fprintf(stderr, "%s: the child could not be reaped\n", what);
    return 0;
  }
  if (WIFSIGNALED(status)) fprintf(stderr, "%s: the child was killed by signal %d\n", what, WTERMSIG(status));
  return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

#endif
