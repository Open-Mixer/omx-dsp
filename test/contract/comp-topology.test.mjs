// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
// 2.5.0 (#40): the comp's detector topology is a control, detectorTopology (DETECTOR_TOPOLOGY: feedforward, feedback).
// omx_dyn.h reads OMX_DETECTOR_TOPOLOGY_* from the render and takes a zeroed atom as feedforward, so feedforward is 0.
// The loop delay DYN_FB_LOOP_DELAY_SAMPLES was retired in dfbc289 (no reader), and the arms over it with it; the frozen
// copy of the salvage lane's local defines it was compared with (omx-dsp 91beb6d) is no longer reachable from any ref.
import assert from 'node:assert/strict';
import { test } from 'node:test';
import { loadData, resolveData } from './oc.mjs';
import { renderC } from './oc.mjs';
import { renderJson } from './oc.mjs';
import { renderTs } from './oc.mjs';
import { UNION, withoutMemberDocs } from './helpers.mjs';

const resolved = (dir = UNION) => resolveData(loadData(dir));
const header = (dir) => withoutMemberDocs(renderC(resolved(dir))[0].text);
const json = (dir) => JSON.parse(renderJson(resolved(dir), '0.0.0')[0].text);
const ts = (dir) => renderTs(resolved(dir)).map((f) => f.text).join('\n');
// detectorTopology was the last control in 2.5.0; the key path, hold and ducker (c208f5f, FX-2) and releaseMode
// (b5bfafe, FX-12) were appended after it.
test('comp: detectorTopology is a choice of DETECTOR_TOPOLOGY that comes up feedforward, followed by the FX-2 and FX-12 controls', () => {
  const c = json().kernels.comp.controls;
  const at = c.findIndex((x) => x.name === 'detectorTopology');
  assert.deepEqual(c[at], { name: 'detectorTopology', kind: 'choice', global: 'DETECTOR_TOPOLOGY' });
  assert.deepEqual(c.slice(at + 1).map((x) => x.name), ['keySource', 'keyHpHz', 'keyLpHz', 'keyTiltDb', 'keyListen', 'holdMs', 'detectLink', 'duck', 'rangeDb', 'releaseMode']);
  assert.deepEqual(json().items.DETECTOR_TOPOLOGY.value, ['feedforward', 'feedback']);
  assert.equal(json().items.DETECTOR_TOPOLOGY.default, 'feedforward');
});

test('the c render gives feedforward 0, as omx_dyn.h reads a zeroed atom, and the ts render the set', () => {
  const h = header();
  assert.match(h, /^enum omx_detector_topology \{\n {2}OMX_DETECTOR_TOPOLOGY_FEEDFORWARD = 0,\n {2}OMX_DETECTOR_TOPOLOGY_FEEDBACK = 1,\n\};$/m);
  assert.match(h, /^#define OMX_DETECTOR_TOPOLOGY_COUNT 2u$/m);
  assert.match(h, /^#define OMX_DETECTOR_TOPOLOGY_DEFAULT OMX_DETECTOR_TOPOLOGY_FEEDFORWARD$/m);
  assert.match(ts(), /^export const DETECTOR_TOPOLOGY = \[\n {2}"feedforward",\n {2}"feedback"\n\];$/m);
});
