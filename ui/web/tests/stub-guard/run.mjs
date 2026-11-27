// Builds the sample app with a Stub (must fail and name each location) and without (must pass).
// Runs against the library source, and also against the built package when dist-library exists.
import { spawnSync } from 'node:child_process';
import { existsSync, readFileSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const web = path.resolve(here, '../..');
const vite = path.join(web, 'node_modules/vite/bin/vite.js');
// The locations the guard must name: every line in the fixture that uses Stub or stub.
const expected = readFileSync(path.join(here, 'page-with-stub.tsx'), 'utf8').split('\n').flatMap((text, i) => /<Stub|stub\(/.test(text) ? [`page-with-stub.tsx:${i + 1}:`] : []);
const sources = existsSync(path.join(web, 'dist-library/vite.js')) ? ['source', 'package'] : ['source'];
let failed = false;

for (const from of sources) {
  for (const page of ['stub', 'clean']) {
    const run = spawnSync(process.execPath, [vite, 'build', '--config', path.join(here, 'vite.config.ts')], { cwd: web, encoding: 'utf8', env: { ...process.env, STUB_GUARD_CASE: page, STUB_GUARD_FROM: from } });
    const output = `${run.stdout}${run.stderr}`;
    const ok = page === 'stub' ? run.status !== 0 && expected.every(location => output.includes(location)) : run.status === 0;
    console.log(`${ok ? 'ok  ' : 'BAD '} ${from} library, ${page === 'stub' ? 'with a Stub' : 'without a Stub'}: build ${run.status === 0 ? 'PASSED' : 'FAILED'}`);
    if (page === 'stub') for (const line of output.split('\n').filter(l => /Release build stopped|page-with-stub\.tsx:\d+/.test(l))) console.log(`       ${line.trim()}`);
    if (!ok) { failed = true; console.log(output); }
  }
}
if (sources.length === 1) console.log('note: built package not checked; run npm run build:library first');
process.exit(failed ? 1 : 0);
