// Packs the built web library into the file a release attaches:
// ui/dist/stack-junkie-ui-web-<VERSION>.tgz. Run `npm run build:library` in ui/web first.
import { cp, mkdtemp, readFile, rename, rm, writeFile, mkdir } from 'node:fs/promises';
import { execSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import os from 'node:os';
import path from 'node:path';

const ui = fileURLToPath(new URL('../', import.meta.url));
const web = path.join(ui, 'web');
const out = path.join(ui, 'dist');
const stage = await mkdtemp(path.join(os.tmpdir(), 'sj-pack-'));
try {
  const pkg = JSON.parse(await readFile(path.join(web, 'package.json'), 'utf8'));
  delete pkg.scripts;
  delete pkg.devDependencies;
  delete pkg.exports['./source'];
  pkg.files = ['dist-library', 'dist-types', 'LICENSE', 'fonts', 'licenses'];
  await writeFile(path.join(stage, 'package.json'), `${JSON.stringify(pkg, null, 2)}\n`);
  for (const [from, to] of [['web/dist-library', 'dist-library'], ['web/dist-types', 'dist-types'], ['LICENSE', 'LICENSE'], ['shared/fonts', 'fonts'], ['shared/licenses', 'licenses']]) {
    await cp(path.join(ui, from), path.join(stage, to), { recursive: true });
  }
  await mkdir(out, { recursive: true });
  // Pack inside the stage with no paths on the command line, then move the file out.
  const name = execSync('npm pack', { cwd: stage, encoding: 'utf8' }).trim().split('\n').pop().trim();
  await rename(path.join(stage, name), path.join(out, name));
  console.log(`Packed: ${path.join(out, name)}`);
} finally {
  await rm(stage, { recursive: true, force: true });
}
