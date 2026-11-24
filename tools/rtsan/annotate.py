#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""annotate.py: marks the per-block entry points of a COPY of include/ nonblocking, for RealtimeSanitizer.

Usage: python3 -I tools/rtsan/annotate.py <copy of include/> [--control]

Run by tools/rtsan/rtsan.sh on its scratch copy; never on the tree's own include/ (the script refuses a directory
that sits beside a .git). Every `static inline` definition of omx_*_{process*,run,run_keyed,block,render,
render_window,render_network} under <dir>/omxdsp gets `__attribute__((nonblocking))` (the C spelling: clang rejects
`[[clang::nonblocking]]` after `static inline` in C), so clang's -fsanitize=realtime checks every call made while one
runs. Names holding init, reset, a size or validity query (bytes, floats, _ok, same, finite) or a non-block helper
(absmax, peak, _db, confidence, config, duty, excess, f0, report, rewind, voice, feed) are skipped: they are set-up or
reading paths, not the block.

--control plants the positive control the tool's self-test reads: a usleep(1) as the first statement of omx_gate_run
in <dir>/omxdsp/omx_gate.h. It is planted whether or not the function was marked, so an annotation that marks nothing
leaves the control unreported and the self-test red.

Prints `annotate: <n> entry points marked` and the sorted names; exit 1 when the directory is not a copy of include/
or the control cannot be planted.
"""
import pathlib
import re
import sys

NAME = re.compile(r'^(static inline )((?:[A-Za-z_0-9]+\s+)*?\**\s*)'
                  r'(omx_[a-z0-9_]*?(?:_process(?:_[a-z]+)?|_run(?:_keyed)?|_block|_render|_render_window'
                  r'|_render_network))\(')
SKIP = re.compile(r'init|reset|bytes|floats|_ok|same|finite|absmax|peak|_db|confidence|config|duty|excess|f0|report'
                  r'|rewind|voice|feed')
MARK = '__attribute__((nonblocking)) '
CONTROL_HEADER = 'omx_gate.h'
CONTROL_FUNCTION = 'omx_gate_run'


def die(msg):
    print('annotate: ' + msg, file=sys.stderr)
    sys.exit(1)


def annotate(root):
    names = []
    for path in sorted(root.rglob('*.h')):
        lines = path.read_text().split('\n')
        changed = False
        for i, line in enumerate(lines):
            m = NAME.match(line)
            if m and not SKIP.search(m.group(3)) and 'nonblocking' not in line:
                lines[i] = m.group(1) + MARK + line[len(m.group(1)):]
                names.append(m.group(3))
                changed = True
        if changed:
            path.write_text('\n'.join(lines))
    return names


def plant_control(root):
    path = root / CONTROL_HEADER
    if not path.is_file():
        die('no %s to plant the control in' % path)
    lines = path.read_text().split('\n')
    start = next((i for i, line in enumerate(lines)
                  if line.startswith('static inline ') and (' %s(' % CONTROL_FUNCTION) in line), None)
    if start is None:
        die('no definition of %s in %s' % (CONTROL_FUNCTION, path))
    body = next((i for i in range(start, len(lines)) if lines[i].rstrip().endswith('{')), None)
    if body is None:
        die('no body of %s in %s' % (CONTROL_FUNCTION, path))
    lines.insert(body + 1, '  usleep(1); /* rtsan positive control: a blocking call RTSan must report */')
    lines.insert(start, '#include <unistd.h>')
    path.write_text('\n'.join(lines))


def main(argv):
    args = [a for a in argv if a != '--control']
    if len(args) != 1 or len(argv) - len(args) > 1:
        die('usage: annotate.py <copy of include/> [--control]')
    top = pathlib.Path(args[0]).resolve()
    root = top / 'omxdsp'
    if not root.is_dir():
        die('%s has no omxdsp/: not a copy of include/' % top)
    if (top.parent / '.git').exists():
        die('%s sits beside a .git: the tool annotates a scratch copy, never the tree' % top)
    names = annotate(root)
    if '--control' in argv:
        plant_control(root)
    print('annotate: %d entry points marked' % len(names))
    print(' '.join(sorted(set(names))))


if __name__ == '__main__':
    main(sys.argv[1:])
