import { readFile, writeFile, mkdir } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const ui = fileURLToPath(new URL('../', import.meta.url));
const version = (await readFile(path.join(ui, '../VERSION'), 'utf8')).trim();
await mkdir(path.join(ui, 'generated'), { recursive: true });
await writeFile(path.join(ui, 'generated/version.json'), `${JSON.stringify({ version }, null, 2)}\n`);
// npm metadata (the web library and the repo's own package, which carries the new command) is derived from the
// skill version, never a second release counter.
for (const packagePath of [path.join(ui, 'web/package.json'), path.join(ui, '../package.json')]) {
  try {
    const pkg = JSON.parse(await readFile(packagePath, 'utf8'));
    if (pkg.version !== version) {
      pkg.version = version;
      await writeFile(packagePath, `${JSON.stringify(pkg, null, 2)}\n`);
    }
  } catch (error) {
    if (error.code !== 'ENOENT') throw error;
  }
}
const lockPath = path.join(ui, 'web/package-lock.json');
try {
  const lock = JSON.parse(await readFile(lockPath, 'utf8'));
  if (lock.version !== version || lock.packages?.['']?.version !== version) {
    lock.version = version;
    if (lock.packages?.['']) lock.packages[''].version = version;
    await writeFile(lockPath, `${JSON.stringify(lock, null, 2)}\n`);
  }
} catch (error) {
  if (error.code !== 'ENOENT') throw error;
}
console.log(`Stack Junkie ${version}`);
