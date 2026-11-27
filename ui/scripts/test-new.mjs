// The new command's test: for every preset and platform, `new` writes exactly the files the shared generator
// returns for the same input, with choices, a preset file and a logo too. It refuses a folder that is not empty.
//
// node ../scripts/test-new.mjs [--npx <package spec>]
// With --npx (such as github:stack-junkie/stack-junkie-style#v3.1.0) it runs the command through npx instead of
// this checkout, the way a clean machine does. Prints one PASS or FAIL line per case.
import { spawnSync } from 'node:child_process';
import { mkdirSync, mkdtempSync, readdirSync, readFileSync, rmSync, writeFileSync } from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { copyFileSync } from 'node:fs';
import { generateStarter, starterCommand } from '../shared/starters/starters.mjs';

const ui = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const root = path.resolve(ui, '..');
const args = process.argv.slice(2);
const npx = args.includes('--npx') ? args[args.indexOf('--npx') + 1] : undefined;
const version = readFileSync(path.join(root, 'VERSION'), 'utf8').trim();
const manifest = JSON.parse(readFileSync(path.join(ui, 'shared/starters/starters.json'), 'utf8'));
const presets = JSON.parse(readFileSync(path.join(ui, 'shared/presets.json'), 'utf8'));
const source = { manifest, presets, template: file => readFileSync(path.join(ui, 'shared/starters', file), 'utf8') };
const work = mkdtempSync(path.join(os.tmpdir(), 'sj-new-'));
const logo = path.join(ui, 'shared/brand/sj-selected.png');
const presetFile = path.join(work, 'mine.json');
writeFileSync(presetFile, `${JSON.stringify({ format: 'stack-junkie-preset', version: 1, name: 'Mine', base: 'desktop-app', choices: { sidebar: 'tree', theme: 'light' } }, null, 2)}\n`);

let failures = 0;
const result = (ok, label, detail = '') => { if (!ok) failures++; console.log(`${ok ? 'PASS' : 'FAIL'} ${label}${detail ? `: ${detail}` : ''}`); };
const command = npx ? ['npx', ['--yes', npx]] : [process.execPath, [path.join(root, 'bin/stack-junkie-style.mjs')]];
const runNew = (folder, flags) => spawnSync(command[0], [...command[1], 'new', folder, ...flags], { cwd: work, encoding: 'utf8', shell: process.platform === 'win32' && !!npx });
const listed = dir => readdirSync(dir, { recursive: true, withFileTypes: true }).filter(e => e.isFile()).map(e => path.relative(dir, path.join(e.parentPath, e.name)).replace(/\\/g, '/')).sort();

const cases = [];
for (const platform of ['web', 'native']) for (const preset of presets.presets) cases.push({ name: `${platform} ${preset.id}`, platform, flags: ['--preset', preset.id], request: { preset: preset.id, overrides: {} } });
cases.push({ name: 'web choices', platform: 'web', flags: ['--preset', 'document-editor', '--choice', 'theme=light', '--choice', 'statusBar=false'], request: { preset: 'document-editor', overrides: { theme: 'light', statusBar: false } } });
cases.push({ name: 'native preset file', platform: 'native', flags: ['--preset-file', presetFile, '--choice', 'density=comfortable'], request: { preset: 'desktop-app', overrides: { sidebar: 'tree', theme: 'light', density: 'comfortable' } } });
for (const platform of ['web', 'native']) cases.push({ name: `${platform} logo`, platform, flags: ['--logo', logo], request: { preset: 'web-app', overrides: {}, logo: { name: path.basename(logo), data: readFileSync(logo) } } });

for (const [i, c] of cases.entries()) {
  const folder = `p${i}`;
  const expected = generateStarter({ platform: c.platform, version, ...c.request }, source);
  const r = runNew(folder, ['--platform', c.platform, ...c.flags]);
  if (expected.unsupported) { result(r.status !== 0 && r.stderr.includes(expected.unsupported), `new ${c.name}`, 'refused with the generator\'s reason'); continue; }
  if (r.status !== 0) { result(false, `new ${c.name}`, r.stderr.trim() || r.stdout.trim()); continue; }
  const dir = path.join(work, folder);
  const written = listed(dir); const wanted = expected.files.map(f => f.path).sort();
  const differing = expected.files.filter(f => !written.includes(f.path) || !Buffer.from(f.data ?? f.text).equals(readFileSync(path.join(dir, f.path)))).map(f => f.path);
  result(JSON.stringify(written) === JSON.stringify(wanted) && !differing.length, `new ${c.name}`, differing.length ? `differs in ${differing.join(', ')}` : JSON.stringify(written) !== JSON.stringify(wanted) ? `wrote ${written.join(', ')}` : `${written.length} files match the generator`);
}
// The Builder copies a command for what it shows (starterCommand). Run that exact command here, beside the logo file,
// and check it writes the generator's files too, so the zip and the command give the same starter.
copyFileSync(logo, path.join(work, path.basename(logo)));
const builderCases = [
  { platform: 'web', preset: 'document-editor', overrides: { theme: 'light', bottomPanel: false } },
  { platform: 'native', preset: 'desktop-app', overrides: { sidebar: 'tree' }, logo: true },
  { platform: 'web', preset: 'website', overrides: { siteNavigation: 'sidebar', siteFooter: false }, logo: true },
];
for (const [i, c] of builderCases.entries()) {
  const folder = `cmd${i}`;
  const text = starterCommand({ platform: c.platform, preset: c.preset, overrides: c.overrides, version, logoName: c.logo ? path.basename(logo) : undefined, folder }, { manifest, presets });
  const tokens = text.split(' ');
  const pinned = tokens[1] === `github:stack-junkie/stack-junkie-style#v${version}` && tokens[2] === 'new' && tokens[3] === folder;
  const r = runNew(folder, tokens.slice(4));
  const expected = generateStarter({ platform: c.platform, preset: c.preset, overrides: c.overrides, version, logo: c.logo ? { name: path.basename(logo), data: readFileSync(logo) } : undefined }, source);
  const dir = path.join(work, folder);
  const same = r.status === 0 && JSON.stringify(listed(dir)) === JSON.stringify(expected.files.map(f => f.path).sort())
    && expected.files.every(f => Buffer.from(f.data ?? f.text).equals(readFileSync(path.join(dir, f.path))));
  result(pinned && same, `the Builder's command for ${c.platform} ${c.preset} writes the zip's files`, pinned ? (r.status === 0 ? text : r.stderr.trim()) : `not pinned to v${version}: ${text}`);
}
mkdirSync(path.join(work, 'taken')); writeFileSync(path.join(work, 'taken/notes.txt'), 'keep me\n');
const refused = runNew('taken', []);
result(refused.status !== 0 && readFileSync(path.join(work, 'taken/notes.txt'), 'utf8') === 'keep me\n' && listed(path.join(work, 'taken')).length === 1, 'new refuses a folder that is not empty', refused.stderr.trim());
const bad = runNew('bad-choice', ['--choice', 'sidebar=sideways']);
result(bad.status !== 0 && /sidebar choice cannot be sideways/.test(bad.stderr), 'new rejects a bad choice', bad.stderr.trim());
rmSync(work, { recursive: true, force: true });
console.log(failures ? `${failures} failed` : 'All new command cases passed');
process.exit(failures ? 1 : 0);
