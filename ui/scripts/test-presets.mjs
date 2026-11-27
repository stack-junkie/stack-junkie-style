// Custom preset file test: the same file must round trip between web and native, and bad files must be rejected on both.
// Web writes a preset file; the native showcase imports it and writes it again (--import-preset, --export-preset);
// the two files must be identical, and web must read the native one back. Each bad file must be rejected by web
// with a message, and by native with a non-zero exit, a message and no file written.
//
// node ../scripts/test-presets.mjs [--showcase <StackJunkieShowcase.exe>] [--qt <Qt prefix>]
// The showcase defaults to the newest Release build under ui/native/build*/. Prints PASS or FAIL per case.
import { spawnSync } from 'node:child_process';
import { existsSync, mkdtempSync, readdirSync, readFileSync, rmSync, statSync, writeFileSync } from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { parsePresetFile, presetFileText } from '../shared/preset-file.mjs';

const ui = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const args = process.argv.slice(2);
const option = name => { const i = args.indexOf(name); return i >= 0 ? args[i + 1] : undefined; };
const presets = JSON.parse(readFileSync(path.join(ui, 'shared/presets.json'), 'utf8'));
const work = mkdtempSync(path.join(os.tmpdir(), 'sjpt-'));
let failures = 0;
const result = (ok, label, detail = '') => { if (!ok) failures++; console.log(`${ok ? 'PASS' : 'FAIL'} ${label}${detail ? `: ${detail}` : ''}`); return ok; };

const preset = { name: 'Round trip "quoted" é', base: 'desktop-app', choices: { sidebar: 'tree', statusBar: false, theme: 'light', density: 'comfortable', rail: true } };
const webText = presetFileText(preset, presets);
const expectedChoices = { sidebar: 'tree', statusBar: false, theme: 'light', density: 'comfortable' };
const readBack = parsePresetFile(webText, presets);
result(readBack.preset && JSON.stringify(readBack.preset.choices) === JSON.stringify(expectedChoices) && readBack.preset.name === preset.name, 'web writes and reads a preset file', 'unchanged choices such as rail are dropped');

const bad = {
  'not JSON': '{ "format": ',
  'another format': JSON.stringify({ format: 'something-else', version: 1, name: 'X', base: 'web-app', choices: {} }),
  'version 2': JSON.stringify({ format: 'stack-junkie-preset', version: 2, name: 'X', base: 'web-app', choices: {} }),
  'unknown base': JSON.stringify({ format: 'stack-junkie-preset', version: 1, name: 'X', base: 'spreadsheet', choices: {} }),
  'unknown choice': JSON.stringify({ format: 'stack-junkie-preset', version: 1, name: 'X', base: 'web-app', choices: { toolbar: true } }),
  'bad value': JSON.stringify({ format: 'stack-junkie-preset', version: 1, name: 'X', base: 'web-app', choices: { sidebar: 'grid' } }),
  'boolean as text': JSON.stringify({ format: 'stack-junkie-preset', version: 1, name: 'X', base: 'web-app', choices: { rail: 'false' } }),
  'empty name': JSON.stringify({ format: 'stack-junkie-preset', version: 1, name: '  ', base: 'web-app', choices: {} }),
  'extra field': JSON.stringify({ format: 'stack-junkie-preset', version: 1, name: 'X', base: 'web-app', choices: {}, author: 'me' }),
};
for (const [label, text] of Object.entries(bad)) {
  const parsed = parsePresetFile(text, presets);
  result(!parsed.preset && !!parsed.error, `web rejects ${label}`, parsed.error);
}

// ---------- native ----------
function newestShowcase() {
  const native = path.join(ui, 'native');
  const found = readdirSync(native).filter(d => d.startsWith('build')).map(d => path.join(native, d, 'Release/StackJunkieShowcase.exe')).filter(existsSync);
  return found.sort((a, b) => statSync(b).mtimeMs - statSync(a).mtimeMs)[0];
}
const showcase = option('--showcase') ?? process.env.STACKJUNKIE_SHOWCASE ?? newestShowcase();
const qt = option('--qt') ?? process.env.STACKJUNKIE_QT ?? path.join(ui, '.tools/Qt/6.8.3/msvc2022_64');
if (!showcase || !existsSync(showcase)) result(false, 'native showcase', 'not found: build it (Release) under ui/native/build*/ or pass --showcase <exe>');
else {
  // Offscreen only: the showcase never opens a window in this test.
  const env = { ...process.env, PATH: `${path.join(qt, 'bin')}${path.delimiter}${process.env.PATH}`, QT_PLUGIN_PATH: path.join(qt, 'plugins'), QT_QPA_PLATFORM: 'offscreen' };
  const run = cliArgs => { const r = spawnSync(showcase, ['-platform', 'offscreen', ...cliArgs], { env, encoding: 'utf8', timeout: 60000 }); return { status: r.status, output: `${r.stdout ?? ''}${r.stderr ?? ''}${r.error ? r.error.message : ''}`.trim() }; };
  const webFile = path.join(work, 'web.preset.json');
  const nativeFile = path.join(work, 'native.preset.json');
  writeFileSync(webFile, webText);
  const imported = run(['--import-preset', webFile, '--export-preset', nativeFile]);
  if (result(imported.status === 0 && existsSync(nativeFile), 'native imports the web file and exports it', imported.output)) {
    const nativeText = readFileSync(nativeFile, 'utf8');
    result(nativeText === webText, 'native writes the same bytes as web', nativeText === webText ? '' : `\n${nativeText}`);
    const again = parsePresetFile(nativeText, presets);
    result(!!again.preset && JSON.stringify(again.preset) === JSON.stringify(readBack.preset), 'web reads the native file back unchanged');
  }
  const fromChoices = path.join(work, 'choices.preset.json');
  const choiceArgs = Object.entries(expectedChoices).flatMap(([k, v]) => ['--choice', `${k}=${v}`]);
  const made = run(['--preset', 'desktop-app', ...choiceArgs, '--preset-name', preset.name, '--export-preset', fromChoices]);
  result(made.status === 0 && existsSync(fromChoices) && readFileSync(fromChoices, 'utf8') === webText, 'native --preset and --choice export the same file', made.output);
  const starterDir = path.join(work, 'starter');
  const starter = run(['--import-preset', webFile, '--export-starter', starterDir, '--starter', 'web']);
  const app = existsSync(path.join(starterDir, 'src/App.tsx')) ? readFileSync(path.join(starterDir, 'src/App.tsx'), 'utf8') : '';
  result(starter.status === 0 && app.includes("{ sidebar: 'tree', statusBar: false, theme: 'light', density: 'comfortable' }"), 'export starter from an imported preset uses its base and choices', starter.status === 0 ? '' : starter.output);
  for (const [label, text] of Object.entries(bad)) {
    const file = path.join(work, `bad-${label.replace(/\W+/g, '-')}.json`);
    const out = `${file}.out`;
    writeFileSync(file, text);
    const r = run(['--import-preset', file, '--export-preset', out]);
    result(r.status !== 0 && r.output.includes('Preset file not imported') && !existsSync(out), `native rejects ${label}`, r.output);
  }
}

rmSync(work, { recursive: true, force: true });
console.log(failures ? `${failures} failed` : 'All preset file cases passed');
process.exit(failures ? 1 : 0);
