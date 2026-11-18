// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * wasm-loader-check.mjs <share-dir> [<ts-dir>]: the loader ships beside the module, and the installed pair works.
 *
 *   <share-dir>  the directory omx-dsp-wasm installs, /usr/share/omx-dsp (or a DESTDIR copy of it).
 *   <ts-dir>     the npm package's directory (ts/ of the tree). When given, the installed files must be EXACTLY the
 *                files `npm pack` publishes from it: the npm package's own "files" list is the one source, so the
 *                install can never carry a loader the npm twin does not publish, nor miss one it does.
 *
 * The loader is imported FROM <share-dir> (dist/index.js, the file the npm twin's "." export resolves to), it loads
 * wasm/omx-dsp.wasm through loadCurve, and one known curve request must answer its known points, with one refusal
 * beside it. Every failure is printed by name: the file, the call or the file that is extra.
 */
import { execFileSync } from 'node:child_process';
import { mkdtempSync, readFileSync, readdirSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, relative, sep } from 'node:path';
import { pathToFileURL } from 'node:url';

const [share, ts] = process.argv.slice(2);
if (!share) {
  console.error('usage: wasm-loader-check.mjs <share-dir> [<ts-dir>]');
  process.exit(2);
}
const fail = [];

/** Every file under dir, as '/'-separated paths relative to dir, sorted. */
const walk = (dir, base = dir) =>
  readdirSync(dir, { withFileTypes: true })
    .flatMap((e) => (e.isDirectory() ? walk(join(dir, e.name), base) : [relative(base, join(dir, e.name)).split(sep).join('/')]))
    .sort();

let installed = [];
try {
  installed = walk(share);
} catch (e) {
  fail.push(`${share}: not installed (${e.code ?? e.message})`);
}

if (ts) {
  // what npm would publish from the package directory, already built (make npm-stage ran its prepack): the "files"
  // entries and package.json, nothing else; the tarball is unpacked and each installed file must be the same bytes
  const pack = mkdtempSync(join(tmpdir(), 'omx-dsp-pack-'));
  try {
    const [info] = JSON.parse(
      execFileSync('npm', ['pack', '--json', '--ignore-scripts', '--pack-destination', pack], { cwd: ts, encoding: 'utf8', stdio: ['ignore', 'pipe', 'inherit'] }),
    );
    const published = info.files.map((f) => f.path).sort();
    const tgz = join(pack, info.filename);
    execFileSync('tar', ['-xzf', tgz, '-C', pack]);
    for (const p of published) {
      if (!installed.includes(p)) fail.push(`${join(share, p)}: npm publishes it and it is not installed`);
      else if (!readFileSync(join(share, p)).equals(readFileSync(join(pack, 'package', p))))
        fail.push(`${join(share, p)}: differs from the file npm publishes`);
    }
    for (const p of installed) if (!published.includes(p)) fail.push(`${join(share, p)}: installed but not published by npm`);
  } finally {
    rmSync(pack, { recursive: true, force: true });
  }
}

// the packaged loader, imported from the packaged directory, loading the packaged module
const DYN = {
  kind: 'transfer_curve',
  family: 'dynamics',
  sampleRate: 48000,
  points: 3,
  extents: { min: -40, max: 0 },
  terms: [
    { role: 'threshold', value: -20, unit: 'db' },
    { role: 'ratio', value: 4, unit: 'ratio' },
  ],
};
const WANT_POINTS = '{"points":[{"x":-40,"y":-40},{"x":-20,"y":-20},{"x":0,"y":-15}],"marks":[]}';
const WANT_REFUSAL = '{"code":"termMissing"}';
const loaderPath = join(share, 'dist', 'index.js');
const modulePath = join(share, 'wasm', 'omx-dsp.wasm');
try {
  const loader = await import(pathToFileURL(loaderPath).href);
  const curve = await loader.loadCurve(readFileSync(modulePath));
  const got = JSON.stringify(curve.curvePoints(DYN));
  if (got !== WANT_POINTS) fail.push(`${modulePath}: the curve call through ${loaderPath} answered ${got}, want ${WANT_POINTS}`);
  const refused = JSON.stringify(curve.curvePoints({ ...DYN, terms: [] }));
  if (refused !== WANT_REFUSAL) fail.push(`${modulePath}: the refusal call answered ${refused}, want ${WANT_REFUSAL}`);
} catch (e) {
  fail.push(`${loaderPath} or ${modulePath}: the packaged loader did not load and answer: ${e.message.split('\n')[0]}`);
}

for (const f of fail) console.error(`wasm-loader-check: ${f}`);
if (!fail.length) console.log(`wasm-loader-check: ${share}: ${installed.length} files, the packaged loader answered the curve call`);
process.exit(fail.length ? 1 : 0);
