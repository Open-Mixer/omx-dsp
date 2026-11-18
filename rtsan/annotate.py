#!/usr/bin/env python3
"""Scratch instrumentation (never merged): marks per-block entry points __attribute__((nonblocking))
so -fsanitize=realtime checks them.  Idempotent.  Usage: python3 -I rtsan/annotate.py"""
import re, pathlib
NAME = re.compile(r'^(static inline )((?:[A-Za-z_0-9]+\s+)*?\**\s*)(omx_[a-z0-9_]*?(?:_process(?:_[a-z]+)?|_run(?:_keyed)?|_block|_render|_render_window|_render_network))\(')
SKIP = re.compile(r'init|reset|bytes|floats|_ok|same|finite|absmax|peak|_db|confidence|config|duty|excess|f0|report|rewind|voice|feed')
n = 0; names = []
for p in pathlib.Path('include/omxdsp').rglob('*.h'):
    out = []; ch = False
    for line in p.read_text().split('\n'):
        m = NAME.match(line)
        if m and not SKIP.search(m.group(3)) and 'nonblocking' not in line:
            line = 'static inline __attribute__((nonblocking)) ' + line[len('static inline '):]
            ch = True; n += 1; names.append(m.group(3))
        out.append(line)
    if ch: p.write_text('\n'.join(out))
print(n, 'entry points marked'); print(' '.join(sorted(set(names))))
