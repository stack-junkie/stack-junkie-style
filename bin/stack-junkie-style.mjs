#!/usr/bin/env node
// The `new` command: creates a starter project from a Preset and choices, straight from the GitHub repo.
//   npx github:stack-junkie/stack-junkie-style new <folder> [--platform web|native] [--preset <id>]
//     [--preset-file <file>] [--choice <name>=<value>]... [--logo <image>]
// It calls the same starter generator as the showcases' Builder, so the same input writes the same files.
import { existsSync, mkdirSync, readdirSync, readFileSync, writeFileSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { generateStarter } from '../ui/shared/starters/starters.mjs';
import { parsePresetFile } from '../ui/shared/preset-file.mjs';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const shared = path.join(root, 'ui/shared');
const version = readFileSync(path.join(root, 'VERSION'), 'utf8').trim();
const manifest = JSON.parse(readFileSync(path.join(shared, 'starters/starters.json'), 'utf8'));
const presets = JSON.parse(readFileSync(path.join(shared, 'presets.json'), 'utf8'));
const source = { manifest, presets, template: file => readFileSync(path.join(shared, 'starters', file), 'utf8') };

const usage = `Stack Junkie Style ${version}

Create a project:
  npx github:stack-junkie/stack-junkie-style new <folder> [options]

Options:
  --platform web|native     The starter to write. Default: web.
  --preset <id>             ${presets.presets.map(p => p.id).join(', ')}. Default: web-app.
  --preset-file <file>      A custom preset file (base preset and changed choices). Not with --preset.
  --choice <name>=<value>   Change one choice. Repeat for more. Choices:
${Object.entries(presets.choices).map(([name, info]) => `                              ${name}: ${info.options.map(String).join(', ')}`).join('\n')}
  --logo <image>            A PNG, JPEG, SVG or WebP file for the logo slot. Default: the SJ mark.
  --help, --version
`;

function fail(message) { process.stderr.write(`${message}\n`); process.exit(2); }

/** Reads the command line into a request, or stops with a plain message. */
function parseArguments(args) {
  const options = { platform: 'web', choices: [] };
  const positional = [];
  for (let i = 0; i < args.length; i++) {
    const arg = args[i];
    const [flag, inline] = arg.startsWith('--') && arg.includes('=') ? [arg.slice(0, arg.indexOf('=')), arg.slice(arg.indexOf('=') + 1)] : [arg, undefined];
    const value = () => inline ?? (i + 1 < args.length ? args[++i] : fail(`${flag} needs a value.`));
    if (flag === '--platform') options.platform = value();
    else if (flag === '--preset') options.preset = value();
    else if (flag === '--preset-file') options.presetFile = value();
    else if (flag === '--choice') options.choices.push(value());
    else if (flag === '--logo') options.logo = value();
    else if (flag.startsWith('--')) fail(`Unknown option ${flag}. Run with --help to see the options.`);
    else positional.push(arg);
  }
  return { options, positional };
}

const args = process.argv.slice(2);
if (!args.length || args.includes('--help') || args.includes('-h')) { process.stdout.write(usage); process.exit(args.length ? 0 : 2); }
if (args.includes('--version')) { process.stdout.write(`${version}\n`); process.exit(0); }
const { options, positional } = parseArguments(args);
if (positional[0] !== 'new') fail(`Unknown command ${positional[0] ?? ''}. The command is: new <folder>`);
const folder = positional[1];
if (!folder || positional.length > 2) fail('Give one folder name: new <folder>');
if (!['web', 'native'].includes(options.platform)) fail('--platform is web or native.');
if (options.preset && options.presetFile) fail('Use --preset or --preset-file, not both.');

let preset = options.preset ?? 'web-app';
let overrides = {};
if (options.presetFile) {
  let text; try { text = readFileSync(options.presetFile, 'utf8'); } catch { fail(`Cannot read the preset file ${options.presetFile}.`); }
  const parsed = parsePresetFile(text, presets);
  if (parsed.error) fail(`Preset file not used: ${parsed.error}`);
  preset = parsed.preset.base; overrides = { ...parsed.preset.choices };
}
if (!presets.presets.some(p => p.id === preset)) fail(`Unknown preset ${preset}. Use one of: ${presets.presets.map(p => p.id).join(', ')}.`);
for (const assignment of options.choices) {
  const at = assignment.indexOf('=');
  const name = at > 0 ? assignment.slice(0, at) : assignment; const text = at > 0 ? assignment.slice(at + 1) : '';
  const info = presets.choices[name];
  if (!info) fail(`Unknown choice ${name}. Choices: ${Object.keys(presets.choices).join(', ')}.`);
  const value = info.options.find(option => String(option) === text);
  if (value === undefined) fail(`The ${name} choice cannot be ${text || 'empty'}. Use one of: ${info.options.map(String).join(', ')}.`);
  overrides[name] = value;
}
let logo;
if (options.logo) {
  let data; try { data = readFileSync(options.logo); } catch { fail(`Cannot read the logo ${options.logo}.`); }
  logo = { name: path.basename(options.logo), data };
}

const target = path.resolve(folder);
if (existsSync(target) && readdirSync(target).length) fail(`${folder} is not empty. Choose a new folder name or an empty folder.`);
const result = generateStarter({ platform: options.platform, preset, overrides, version, logo }, source);
if (result.unsupported) fail(result.unsupported);
for (const file of result.files) {
  const to = path.join(target, file.path);
  mkdirSync(path.dirname(to), { recursive: true });
  writeFileSync(to, file.data ?? file.text);
}
const label = presets.presets.find(p => p.id === preset).label;
process.stdout.write(`Created ${folder}: a ${options.platform} starter with the ${label} preset${Object.keys(overrides).length ? ' and your choices' : ''}, on Stack Junkie Style ${version}.\n\nNext:\n  cd ${folder}\n${options.platform === 'web' ? '  npm install\n  npm run dev\n' : '  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug\n  cmake --build build\n'}\nThe pages are Stubs: replace them with your app's own. ${options.platform === 'web' ? 'npm run build' : 'A Release build'} lists each one that remains.\n`);
