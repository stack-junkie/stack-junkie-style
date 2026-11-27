import { defineConfig, type Plugin } from 'vite';
import react from '@vitejs/plugin-react';
import { fileURLToPath } from 'node:url';
import { readFile, writeFile, mkdir } from 'node:fs/promises';

const ui = fileURLToPath(new URL('../..', import.meta.url));
const dataFile = fileURLToPath(new URL('./data/projects.json', import.meta.url));

// Serves the last scan and runs a new one on request. Local only: the data lists private projects and never ships in a build.
function scanData(): Plugin {
  return {
    name: 'projects-scan-data',
    configureServer(server) {
      server.middlewares.use('/api/projects', async (req, res) => {
        res.setHeader('Content-Type', 'application/json');
        try {
          if (req.method === 'POST') {
            const { scan } = await import('./scan.mjs');
            const config = JSON.parse(await readFile(new URL('./scan.config.json', import.meta.url), 'utf8'));
            const result = await scan(config);
            await mkdir(fileURLToPath(new URL('./data', import.meta.url)), { recursive: true });
            await writeFile(dataFile, `${JSON.stringify(result, null, 2)}\n`);
            res.end(JSON.stringify(result));
            return;
          }
          res.end(await readFile(dataFile, 'utf8'));
        } catch (error) {
          const missing = (error as NodeJS.ErrnoException).code === 'ENOENT';
          res.statusCode = missing ? 404 : 500;
          res.end(JSON.stringify({ error: missing ? 'No scan yet.' : `The scan failed: ${(error as Error).message}` }));
        }
      });
    },
  };
}

export default defineConfig({
  plugins: [react(), scanData()],
  base: './',
  resolve: {
    alias: { '@stack-junkie/ui-web': fileURLToPath(new URL('../../web/src/library/index.ts', import.meta.url)) },
    // The library source sits outside this app; resolve React from here so there is one copy.
    dedupe: ['react', 'react-dom'],
  },
  server: { port: 5183, strictPort: true, fs: { allow: [ui] } },
  preview: { port: 5183, strictPort: true },
  build: { outDir: 'dist', assetsInlineLimit: 0 },
});
