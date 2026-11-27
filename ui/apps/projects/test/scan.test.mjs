import { test } from 'node:test';
import assert from 'node:assert/strict';
import { fileURLToPath } from 'node:url';
import { scan, compareVersions } from '../scan.mjs';

const fixtures = fileURLToPath(new URL('./fixtures', import.meta.url));
const latest = '3.0.0-preview.10';

test('the scan finds the projects that use the style and their versions', async () => {
  const result = await scan({ roots: [fixtures], latest, remote: false });
  const byName = Object.fromEntries(result.projects.map(p => [p.name, p]));
  assert.deepEqual(Object.keys(byName).sort(), ['not-using', 'old-version', 'uses-current']);

  assert.equal(byName['uses-current'].usesStyle, true);
  assert.equal(byName['uses-current'].version, '3.0.0-preview.10');
  assert.equal(byName['uses-current'].status, 'current');
  assert.match(byName['uses-current'].link, /^vscode:\/\/file\//);

  assert.equal(byName['old-version'].usesStyle, true);
  assert.equal(byName['old-version'].version, '3.0.0-preview.7');
  assert.equal(byName['old-version'].status, 'behind');

  assert.equal(byName['not-using'].usesStyle, false);
  assert.equal(byName['not-using'].version, null);
  assert.equal(byName['not-using'].status, 'not-using');
  assert.equal(byName['not-using'].frontEnd, true);

  for (const p of result.projects) assert.equal(p.latest, latest);
});

test('a missing GitLab CLI is reported as not connected instead of failing', async () => {
  const run = async cmd => (cmd === 'glab' ? { ok: false, stdout: '', missing: true } : { ok: false, stdout: '' });
  const result = await scan({ roots: [fixtures], latest, run });
  assert.equal(result.sources.gitlab.status, 'not connected');
  assert.equal(result.sources.github.status, 'not connected');
  assert.equal(result.projects.length, 3);
});

test('prerelease versions compare by number', () => {
  assert.ok(compareVersions('3.0.0-preview.9', '3.0.0-preview.10') < 0);
  assert.ok(compareVersions('3.0.0-preview.10', '3.0.0') < 0);
  assert.equal(compareVersions('3.0.0', '3.0.0'), 0);
});
