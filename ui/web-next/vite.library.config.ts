import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import { fileURLToPath } from 'node:url';

export default defineConfig({ plugins: [react()], build: {
  outDir: 'dist-library',
  lib: { entry: fileURLToPath(new URL('src/library/index.ts', import.meta.url)), formats: ['es'], fileName: 'stack-junkie-ui', cssFileName: 'styles' },
  rollupOptions: { external: ['react', 'react-dom', 'react/jsx-runtime'] },
} });
