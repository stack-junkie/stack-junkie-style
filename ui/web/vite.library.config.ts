import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import { fileURLToPath } from 'node:url';

// Two entries: the browser library, and the Node-side Vite plugin (`./vite`) that guards release builds against Stubs.
export default defineConfig({ plugins: [react()], build: {
  outDir: 'dist-library',
  lib: { entry: { 'stack-junkie-ui': fileURLToPath(new URL('src/library/index.ts', import.meta.url)), vite: fileURLToPath(new URL('src/library/vite.ts', import.meta.url)) }, formats: ['es'], fileName: (_format, name) => `${name}.js`, cssFileName: 'styles' },
  rollupOptions: { external: ['react', 'react-dom', 'react/jsx-runtime', /^node:/, 'vite'] },
} });
