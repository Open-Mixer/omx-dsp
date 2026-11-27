// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/** helpers.mjs codeOnly: comments go, a `//` or `/*` inside a string or character literal stays. */
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { codeOnly } from './helpers.mjs';

test('block and line comments go; string and character literals stay whole', () => {
  const src = '/** OMX_HRP_FAMILY_NONE */\n#define A "http://x" // OMX_HRP_FAMILY_NONE\n#define B \'/\' /* c */\n#define C "a \\" // b"\n';
  const code = codeOnly(src);
  assert.doesNotMatch(code, /OMX_HRP_FAMILY_NONE/);
  assert.match(code, /#define A "http:\/\/x" /);
  assert.match(code, /#define B '\/' /);
  assert.match(code, /#define C "a \\" \/\/ b"/);
});
