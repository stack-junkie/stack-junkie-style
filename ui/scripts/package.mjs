import { cp, mkdir, readFile, writeFile, access, realpath, rm } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import './prepare.mjs';

const ui = fileURLToPath(new URL('../', import.meta.url));
const skill = path.resolve(ui, '..');
const version = (await readFile(path.join(skill, 'VERSION'), 'utf8')).trim();
const target = path.join(ui, 'dist', `stack-junkie-style-${version}`);
const distributionRoot = path.join(ui, 'dist');
if (path.dirname(target) !== distributionRoot) throw new Error('Version must name a direct child of the distribution directory.');
await mkdir(distributionRoot, { recursive: true });
try {
  const resolved = await realpath(target);
  if (path.dirname(resolved) !== await realpath(distributionRoot)) throw new Error('Refusing to replace a bundle outside the distribution directory.');
  // Rebuild only this generated version bundle so removed source files cannot linger.
  await rm(target, { recursive: true });
} catch (error) { if (error.code !== 'ENOENT') throw error; }
await mkdir(target, { recursive: true });

// Explicit inputs prevent private transcripts, toolchains or dependencies entering a bundle.
const sources = [
  'SKILL.md', 'VERSION', 'agents', 'references',
  'ui/README.md', 'ui/CHANGELOG.md', 'ui/LICENSE', 'ui/THIRD_PARTY_NOTICES.md', 'ui/shared', 'ui/scripts',
  'ui/generated/version.json',
  'ui/web/src', 'ui/web/README.md', 'ui/web/index.html', 'ui/web/package.json', 'ui/web/package-lock.json',
  'ui/web/tsconfig.json', 'ui/web/tsconfig.library.json', 'ui/web/vite.config.ts', 'ui/web/vite.library.config.ts',
  'ui/native/CMakeLists.txt', 'ui/native/include', 'ui/native/src', 'ui/native/showcase',
  'ui/native/cmake', 'ui/native/resources', 'ui/native/README.md'
];
for (const relative of sources) {
  const source = path.join(skill, relative);
  await access(source);
  await cp(source, path.join(target, relative), { recursive: true });
}
for (const [from, to] of [['web/dist', 'showcase-web'], ['web/dist-library', 'library-web/dist-library'], ['web/dist-types', 'library-web/dist-types']]) {
  await cp(path.join(ui, from), path.join(target, to), { recursive: true });
}
const libraryPackage = JSON.parse(await readFile(path.join(ui, 'web/package.json'), 'utf8'));
delete libraryPackage.scripts;
delete libraryPackage.devDependencies;
delete libraryPackage.exports['./source'];
await writeFile(path.join(target, 'library-web/package.json'), `${JSON.stringify(libraryPackage, null, 2)}\n`);
await cp(path.join(ui, 'LICENSE'), path.join(target, 'library-web/LICENSE'));
await cp(path.join(ui, 'shared/fonts'), path.join(target, 'library-web/fonts'), { recursive: true });
await cp(path.join(ui, 'shared/licenses'), path.join(target, 'library-web/licenses'), { recursive: true });
let windowsIncluded = false;
try {
  await access(path.join(ui, 'dist/windows/bin/StackJunkieShowcase.exe'));
  await cp(path.join(ui, 'dist/windows'), path.join(target, 'showcase-windows'), { recursive: true });
  windowsIncluded = true;
} catch (error) { if (error.code !== 'ENOENT') throw error; }
await cp(path.join(ui, 'LICENSE'), path.join(target, 'LICENSE'));
await cp(path.join(ui, 'THIRD_PARTY_NOTICES.md'), path.join(target, 'THIRD_PARTY_NOTICES.md'));
await writeFile(path.join(target, 'README.md'), `# Stack Junkie ${version}\n\nWorking comparison preview; visual selection and owner interaction review are pending.\n\n- Web: from this bundle directory run \`node ui/scripts/serve.mjs --root showcase-web\`, then open the printed localhost URL.\n- Web library: library-web contains the built reusable module and declarations.\n- Windows: ${windowsIncluded ? 'run showcase-windows/bin/StackJunkieShowcase.exe. Microsoft Visual C++ 2015-2022 x64 Runtime is required.' : 'not included in this bundle; build from ui/native.'}\n- Linux: build from ui/native with Qt 6 Widgets and Svg. Runtime verification remains environment-specific.\n- Skill and build commands: see SKILL.md and ui/README.md.\n\nPrivate design-history transcripts are excluded.\n`);
console.log(`Bundle: ${target}`);
