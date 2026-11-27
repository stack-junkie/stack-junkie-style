import os from 'node:os';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import { stackJunkieStubGuard as sourceGuard } from '../../src/library/vite';

// A tiny sample app for the Stub release guard. STUB_GUARD_CASE picks the page (stub or clean);
// STUB_GUARD_FROM=package uses the built library and plugin instead of the source.
const here = path.dirname(fileURLToPath(import.meta.url));
const web = path.resolve(here, '../..');
const fromPackage = process.env.STUB_GUARD_FROM === 'package';
const stackJunkieStubGuard: typeof sourceGuard = fromPackage ? (await import(pathToFileURL(path.join(web, 'dist-library/vite.js')).href)).stackJunkieStubGuard : sourceGuard;

export default defineConfig({
  root: here,
  logLevel: 'error',
  plugins: [react(), stackJunkieStubGuard()],
  resolve: { alias: [
    { find: /^@stack-junkie\/ui-web$/, replacement: path.join(web, fromPackage ? 'dist-library/stack-junkie-ui.js' : 'src/library/index.ts') },
    { find: '#page', replacement: path.join(here, process.env.STUB_GUARD_CASE === 'clean' ? 'page-clean.tsx' : 'page-with-stub.tsx') },
  ] },
  build: { outDir: path.join(os.tmpdir(), 'stack-junkie-stub-guard'), emptyOutDir: true },
});
