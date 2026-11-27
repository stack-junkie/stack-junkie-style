import { defineConfig, type Plugin } from 'vite';
import react from '@vitejs/plugin-react';
import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { llmsText } from '../shared/llms.mjs';

// The showcase site serves llms.txt at its root, generated from the Presets data on every build and in dev.
function llms(): Plugin {
  const read = (file: string) => readFileSync(fileURLToPath(new URL(file, import.meta.url)), 'utf8');
  const text = () => llmsText({ version: read('../../VERSION').trim(), presets: JSON.parse(read('../shared/presets.json')), manifest: JSON.parse(read('../shared/starters/starters.json')) });
  return {
    name: 'stack-junkie-llms-txt',
    configureServer(server) { server.middlewares.use('/llms.txt', (_req, res) => { res.setHeader('Content-Type', 'text/plain; charset=utf-8'); res.end(text()); }); },
    generateBundle() { this.emitFile({ type: 'asset', fileName: 'llms.txt', source: text() }); },
  };
}

export default defineConfig({
  plugins: [react(), llms()],
  base: './',
  server: { strictPort: true, fs: { allow: [fileURLToPath(new URL('..', import.meta.url))] } },
  preview: { strictPort: true },
  build: { outDir: 'dist', assetsInlineLimit: 0 },
});
