import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import { fileURLToPath } from 'node:url';

// Side-by-side candidate of the web showcase. Runs on 5174 so the current showcase on 5173 stays untouched.
export default defineConfig({
  plugins: [react()],
  base: './',
  server: { port: 5174, strictPort: true, fs: { allow: [fileURLToPath(new URL('..', import.meta.url))] } },
  preview: { port: 5174, strictPort: true },
  build: { outDir: 'dist', assetsInlineLimit: 0 },
});
