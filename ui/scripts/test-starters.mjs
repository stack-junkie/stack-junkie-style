// Preset to starter test: Export's starters must build without edits.
// For every preset, generate the web starter and typecheck and build it against the locally built library.
// For the three app presets, generate the native starter and build it with CMake against the library
// installed into a temporary prefix, plus once through the FetchContent fallback with a local source.
// The native showcase's --export-starter must write the same files as the shared generator.
//
// node ../scripts/test-starters.mjs [--only web|native] [--qt <Qt prefix>] [--keep]
// Prints one PASS or FAIL line per case and exits non-zero on any failure.
import { spawn, spawnSync } from 'node:child_process';
import { existsSync, lstatSync, mkdirSync, readdirSync, readFileSync, rmdirSync, rmSync, symlinkSync, unlinkSync, writeFileSync } from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { generateStarter } from '../shared/starters/starters.mjs';

const ui = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const web = path.join(ui, 'web');
const native = path.join(ui, 'native');
const skillRoot = path.resolve(ui, '..');
const args = process.argv.slice(2);
const option = name => { const i = args.indexOf(name); return i >= 0 ? args[i + 1] : undefined; };
const only = option('--only');
const keep = args.includes('--keep');
const version = readFileSync(path.join(skillRoot, 'VERSION'), 'utf8').trim();
const manifest = JSON.parse(readFileSync(path.join(ui, 'shared/starters/starters.json'), 'utf8'));
const presets = JSON.parse(readFileSync(path.join(ui, 'shared/presets.json'), 'utf8'));
const source = { manifest, presets, template: file => readFileSync(path.join(ui, 'shared/starters', file), 'utf8') };
// Short folder names keep MSBuild paths under the Windows path limit.
const work = path.join(os.tmpdir(), `sjst-${Date.now().toString(36)}`);
mkdirSync(work, { recursive: true });

const custom = { sidebar: 'tree', statusBar: false, theme: 'light', density: 'comfortable' };
const cases = [
  ...presets.presets.map(p => ({ name: p.id, preset: p.id, overrides: {} })),
  { name: 'desktop-app custom', preset: 'desktop-app', overrides: custom, expect: { web: "{ sidebar: 'tree', statusBar: false, theme: 'light', density: 'comfortable' }", native: '{{QStringLiteral("sidebar"), QStringLiteral("tree")}, {QStringLiteral("statusBar"), false}, {QStringLiteral("theme"), QStringLiteral("light")}, {QStringLiteral("density"), QStringLiteral("comfortable")}}' } },
];

let failures = 0;
const result = (ok, label, detail = '') => { if (!ok) failures++; console.log(`${ok ? 'PASS' : 'FAIL'} ${label}${detail ? `: ${detail}` : ''}`); return ok; };
function run(command, commandArgs, options = {}) {
  const r = spawnSync(command, commandArgs, { encoding: 'utf8', maxBuffer: 64 * 1024 * 1024, ...options });
  return { ok: r.status === 0, output: `${r.stdout ?? ''}${r.stderr ?? ''}${r.error ? r.error.message : ''}` };
}
const tail = text => text.trim().split(/\r?\n/).slice(-15).join('\n');
function writeFiles(dir, files) {
  for (const file of files) { const to = path.join(dir, file.path); mkdirSync(path.dirname(to), { recursive: true }); writeFileSync(to, file.text); }
}
const slug = name => name.replace(/[^a-z0-9]+/gi, '-');

// The native showcase's CLI, once it is built: writes the same starter through the C++ filler.
let showcase = null;
let qtEnv = process.env;
function compareWithShowcase(label, platform, testCase, files) {
  if (!showcase) return;
  const out = path.join(work, `c-${platform[0]}-${slug(testCase.name)}`);
  const choiceArgs = Object.entries(testCase.overrides).flatMap(([k, v]) => ['--choice', `${k}=${v}`]);
  const r = run(showcase, ['--export-starter', out, '--starter', platform, '--preset', testCase.preset, ...choiceArgs], { env: qtEnv });
  if (!r.ok) { result(false, `${label}: native showcase export`, tail(r.output)); return; }
  const written = readdirSync(out, { recursive: true, withFileTypes: true }).filter(e => e.isFile()).map(e => path.relative(out, path.join(e.parentPath, e.name)).replace(/\\/g, '/')).sort();
  const expected = files.map(f => f.path).sort();
  const differing = expected.filter(p => !written.includes(p) || readFileSync(path.join(out, p), 'utf8') !== files.find(f => f.path === p).text);
  result(JSON.stringify(written) === JSON.stringify(expected) && !differing.length, `${label}: native showcase writes the same files`, differing.length ? `differs in ${differing.join(', ')}` : '');
}

// ---------- web ----------
function webCase(testCase) {
  const label = `web ${testCase.name}`;
  const generated = generateStarter({ platform: 'web', preset: testCase.preset, overrides: testCase.overrides, version }, source);
  if (!generated.files) return result(false, label, generated.unsupported);
  compareWithShowcase(label, 'web', testCase, generated.files);
  const dir = path.join(work, `w-${slug(testCase.name)}`);
  writeFiles(dir, generated.files);
  const pkg = JSON.parse(readFileSync(path.join(dir, 'package.json'), 'utf8'));
  const pinned = `${manifest.repository}/releases/download/v${version}/stack-junkie-ui-web-${version}.tgz`;
  if (pkg.dependencies['@stack-junkie/ui-web'] !== pinned) return result(false, label, `library dependency is ${pkg.dependencies['@stack-junkie/ui-web']}, expected ${pinned}`);
  const app = readFileSync(path.join(dir, 'src/App.tsx'), 'utf8');
  const layout = testCase.preset === 'website' ? 'WebsiteLayout' : 'AppShell';
  if (!app.includes(`<${layout} `)) return result(false, label, `App.tsx does not use ${layout}`);
  if (testCase.expect && !app.includes(`choices: Partial<PresetChoices> = ${testCase.expect.web};`)) return result(false, label, 'App.tsx does not carry the custom choices');
  // Use the local library in place of the release tag: the package.json points at ui/web, and node_modules
  // links each installed package the way npm links a file: dependency.
  pkg.dependencies['@stack-junkie/ui-web'] = `file:${web}`;
  writeFileSync(path.join(dir, 'package.json'), `${JSON.stringify(pkg, null, 2)}\n`);
  const modules = path.join(dir, 'node_modules');
  mkdirSync(path.join(modules, '@stack-junkie'), { recursive: true });
  for (const entry of readdirSync(path.join(web, 'node_modules'))) if (!entry.startsWith('.') && entry !== '@stack-junkie') symlinkSync(path.join(web, 'node_modules', entry), path.join(modules, entry), 'junction');
  symlinkSync(web, path.join(modules, '@stack-junkie/ui-web'), 'junction');
  const missing = Object.keys({ ...pkg.dependencies, ...pkg.devDependencies }).filter(name => !existsSync(path.join(modules, name, 'package.json')));
  if (missing.length) return result(false, label, `not installable here: ${missing.join(', ')}`);
  const tsc = run(process.execPath, [path.join(web, 'node_modules/typescript/bin/tsc'), '-p', dir, '--noEmit'], { cwd: dir });
  if (!tsc.ok) return result(false, label, `typecheck failed\n${tail(tsc.output)}`);
  const vite = run(process.execPath, [path.join(web, 'node_modules/vite/bin/vite.js'), 'build', '--logLevel', 'error'], { cwd: dir });
  if (!vite.ok) return result(false, label, `build failed\n${tail(vite.output)}`);
  if (!existsSync(path.join(dir, 'dist/index.html'))) return result(false, label, 'build wrote no dist/index.html');
  return result(true, label, `typecheck and build passed${testCase.expect ? ', custom choices carried' : ''}`);
}

// ---------- native ----------
function findCmake() {
  if (run('cmake', ['--version']).ok) return 'cmake';
  const vswhere = path.join(process.env['ProgramFiles(x86)'] ?? 'C:\\Program Files (x86)', 'Microsoft Visual Studio/Installer/vswhere.exe');
  if (!existsSync(vswhere)) return null;
  const vs = run(vswhere, ['-latest', '-products', '*', '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath']).output.trim();
  const cmake = path.join(vs, 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe');
  return vs && existsSync(cmake) ? cmake : null;
}
const generatorArgs = process.platform === 'win32' ? ['-G', 'Visual Studio 17 2022', '-A', 'x64'] : ['-DCMAKE_BUILD_TYPE=Release'];
const buildTail = process.platform === 'win32' ? ['--', '/nodeReuse:false'] : [];

// Starts the built program on the offscreen platform and checks it is still running after three seconds.
function starts(program) {
  return new Promise(resolve => {
    const child = spawn(program, [], { env: qtEnv, stdio: ['ignore', 'pipe', 'pipe'] });
    let output = ''; child.stdout.on('data', d => { output += d; }); child.stderr.on('data', d => { output += d; });
    let exited = false; child.on('exit', code => { exited = true; resolve({ ok: false, output: `exited early with ${code}\n${output}` }); });
    child.on('error', error => { exited = true; resolve({ ok: false, output: error.message }); });
    setTimeout(() => { if (!exited) { child.removeAllListeners('exit'); child.kill(); resolve({ ok: true, output }); } }, 3000);
  });
}
const programIn = dir => [path.join(dir, 'build/Release/MyApp.exe'), path.join(dir, 'build/MyApp')].find(existsSync);

async function nativeCase(testCase, cmake, prefix, qt, { fetch = false } = {}) {
  const label = `native ${testCase.name}${fetch ? ' (FetchContent fallback)' : ''}`;
  const generated = generateStarter({ platform: 'native', preset: testCase.preset, overrides: testCase.overrides, version }, source);
  if (!generated.files) return result(false, label, generated.unsupported);
  if (!fetch) compareWithShowcase(label, 'native', testCase, generated.files);
  const dir = path.join(work, `${fetch ? 'f' : 'n'}-${slug(testCase.name)}`);
  writeFiles(dir, generated.files);
  const main = readFileSync(path.join(dir, 'main.cpp'), 'utf8');
  if (!main.includes(`shell.setPreset(QStringLiteral("${testCase.preset}"), choices);`)) return result(false, label, 'main.cpp does not call AppShell::setPreset with the preset');
  if (testCase.expect && !main.includes(`const QVariantMap choices = ${testCase.expect.native};`)) return result(false, label, 'main.cpp does not carry the custom choices');
  const cmakeLists = readFileSync(path.join(dir, 'CMakeLists.txt'), 'utf8');
  if (!cmakeLists.includes('find_package(StackJunkie') || !cmakeLists.includes(`GIT_TAG v${version}`) || !cmakeLists.includes(`GIT_REPOSITORY ${manifest.repository}.git`)) return result(false, label, 'CMakeLists.txt is not pinned to the release tag');
  // The installed case must find the package: with fetching disconnected, a miss fails configure.
  const where = fetch ? [`-DCMAKE_PREFIX_PATH=${qt}`, `-DFETCHCONTENT_SOURCE_DIR_STACKJUNKIE=${skillRoot}`] : [`-DCMAKE_PREFIX_PATH=${prefix};${qt}`, '-DFETCHCONTENT_FULLY_DISCONNECTED=ON'];
  const configure = run(cmake, ['-S', dir, '-B', path.join(dir, 'build'), ...generatorArgs, ...where]);
  if (!configure.ok) return result(false, label, `configure failed\n${tail(configure.output)}`);
  const build = run(cmake, ['--build', path.join(dir, 'build'), '--config', 'Release', '--parallel', ...buildTail]);
  if (!build.ok) return result(false, label, `build failed\n${tail(build.output)}`);
  const program = programIn(dir);
  if (!program) return result(false, label, 'build wrote no MyApp program');
  const started = await starts(program);
  if (!started.ok) return result(false, label, `the built program did not start offscreen: ${tail(started.output)}`);
  return result(true, label, `CMake configure and Release build passed, starts offscreen${testCase.expect ? ', custom choices carried' : ''}`);
}

async function nativeTests() {
  const cmake = findCmake();
  if (!cmake) return result(false, 'native setup', 'CMake 3.21+ and a C++ toolchain are required');
  const qt = option('--qt') ?? process.env.STACKJUNKIE_QT ?? path.join(ui, '.tools/Qt/6.8.3/msvc2022_64');
  if (!existsSync(path.join(qt, 'lib/cmake/Qt6/Qt6Config.cmake'))) return result(false, 'native setup', `no Qt 6 at ${qt}; pass --qt <prefix> or set STACKJUNKIE_QT`);
  qtEnv = { ...process.env, PATH: `${path.join(qt, 'bin')}${path.delimiter}${process.env.PATH}`, QT_PLUGIN_PATH: path.join(qt, 'plugins'), QT_QPA_PLATFORM: 'offscreen' };
  const lib = path.join(work, 'lib'); const prefix = path.join(work, 'pkg');
  const steps = [
    ['configure', ['-S', native, '-B', lib, ...generatorArgs, `-DCMAKE_PREFIX_PATH=${qt}`]],
    ['build', ['--build', lib, '--config', 'Release', '--parallel', ...buildTail]],
    ['install', ['--install', lib, '--config', 'Release', '--prefix', prefix]],
  ];
  for (const [step, stepArgs] of steps) {
    const r = run(cmake, stepArgs);
    if (!r.ok) return result(false, `native library ${step}`, tail(r.output));
  }
  result(true, 'native library', `built and installed into ${prefix}`);
  showcase = [path.join(prefix, 'bin/StackJunkieShowcase.exe'), path.join(prefix, 'bin/StackJunkieShowcase')].find(existsSync) ?? null;
  if (!showcase) result(false, 'native showcase', 'not installed, so its --export-starter was not compared');
  const website = generateStarter({ platform: 'native', preset: 'website', version }, source);
  result(!!website.unsupported && !website.files, 'native website', website.unsupported ? 'no native starter, as designed' : 'unexpectedly produced files');
  for (const testCase of cases.filter(c => c.preset !== 'website')) await nativeCase(testCase, cmake, prefix, qt);
  await nativeCase(cases.find(c => c.preset === 'document-editor'), cmake, prefix, qt, { fetch: true });
}

if (only !== 'web') await nativeTests();
if (only !== 'native') {
  const library = run('npm run build:library', [], { cwd: web, shell: true });
  if (result(library.ok, 'web library', library.ok ? 'npm run build:library passed' : tail(library.output))) for (const testCase of cases) webCase(testCase);
}
// Remove the node_modules links one by one first, never what they point to: rmdir drops a Windows junction,
// unlink drops a symbolic link elsewhere.
const dropLink = link => { if (lstatSync(link).isSymbolicLink() && process.platform !== 'win32') unlinkSync(link); else rmdirSync(link); };
function clean() {
  for (const dir of readdirSync(work).filter(d => d.startsWith('w-') || d.startsWith('x-'))) {
    const modules = path.join(work, dir, 'node_modules');
    if (!existsSync(modules)) continue;
    for (const entry of readdirSync(modules)) if (entry !== '@stack-junkie') dropLink(path.join(modules, entry));
    if (existsSync(path.join(modules, '@stack-junkie/ui-web'))) dropLink(path.join(modules, '@stack-junkie/ui-web'));
  }
  rmSync(work, { recursive: true, force: true });
}
if (keep || failures) console.log(`Starters kept in ${work}`); else clean();
console.log(failures ? `${failures} failed` : 'All starter cases passed');
process.exit(failures ? 1 : 0);
