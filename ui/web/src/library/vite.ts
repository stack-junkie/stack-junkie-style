import path from 'node:path';
import { fileURLToPath } from 'node:url';
import type { Plugin } from 'vite';

// The release guard for Stubs. In `vite build` it reads each application module before other
// transforms, finds every use of `Stub` or `stub` imported from this library, and fails the build
// with the file:line:column of each one. The library's own files are never scanned.

const packageName = '@stack-junkie/ui-web';
const stubNames = new Set(['Stub', 'stub']);
// This file sits beside the library entry both in source (src/library) and when built (dist-library).
const libraryDir = path.dirname(fileURLToPath(import.meta.url));
const importPattern = /import\s+(?!type\s)([^'";]*?)\s+from\s*['"]([^'"]+)['"]/g;
const scriptPattern = /\.[cm]?[jt]sx?$/;

export type StubLocation = { file: string; line: number; column: number; name: string };

function isInside(dir: string, file: string) { const relative = path.relative(dir, file); return !!relative && !relative.startsWith('..') && !path.isAbsolute(relative); }
function bindings(clause: string) {
  const named: string[] = []; const namespaces: string[] = [];
  const namespace = /\*\s+as\s+([\w$]+)/.exec(clause); if (namespace) namespaces.push(namespace[1]);
  const braces = /\{([^}]*)\}/.exec(clause);
  for (const part of braces ? braces[1].split(',') : []) {
    const [imported, local = imported] = part.trim().replace(/^type\s+/, '').split(/\s+as\s+/).map(x => x.trim());
    if (stubNames.has(imported) && !part.trim().startsWith('type ')) named.push(local);
  }
  return { named, namespaces };
}

export function stackJunkieStubGuard(): Plugin {
  let root = process.cwd(); const found = new Map<string, StubLocation[]>();
  return {
    name: 'stack-junkie-stub-guard', apply: 'build', enforce: 'pre',
    configResolved(config) { root = config.root; },
    buildStart() { found.clear(); },
    async transform(code, id) {
      const file = id.split('?')[0];
      if (id.startsWith('\0') || !scriptPattern.test(file) || file.includes('/node_modules/') || isInside(libraryDir, file)) return null;
      const names: string[] = []; const imports: { start: number; end: number }[] = [];
      for (const match of code.matchAll(importPattern)) {
        const { named, namespaces } = bindings(match[1]); if (!named.length && !namespaces.length) continue;
        const source = match[2]; let fromLibrary = source === packageName || source.startsWith(`${packageName}/`);
        if (!fromLibrary && source.startsWith('.')) { const resolved = await this.resolve(source, id, { skipSelf: true }); fromLibrary = !!resolved && isInside(libraryDir, resolved.id.split('?')[0]); }
        if (!fromLibrary) continue;
        names.push(...named.map(n => n.replace(/\$/g, '\\$')), ...namespaces.flatMap(ns => [...stubNames].map(s => `${ns.replace(/\$/g, '\\$')}\\.${s}`)));
        imports.push({ start: match.index, end: match.index + match[0].length });
      }
      if (!names.length) return null;
      const locations: StubLocation[] = [];
      for (const match of code.matchAll(new RegExp(`(?<![\\w$.]|</)(?:${names.join('|')})(?![\\w$])`, 'g'))) {
        if (imports.some(range => match.index >= range.start && match.index < range.end)) continue;
        const before = code.slice(0, match.index).split('\n');
        locations.push({ file: path.relative(root, file).replace(/\\/g, '/'), line: before.length, column: before[before.length - 1].length + 1, name: match[0] });
      }
      // An imported but unused Stub still names its import, so the build never passes with one in the graph.
      if (!locations.length) { const before = code.slice(0, imports[0].start).split('\n'); locations.push({ file: path.relative(root, file).replace(/\\/g, '/'), line: before.length, column: before[before.length - 1].length + 1, name: 'import' }); }
      found.set(file, locations);
      return null;
    },
    buildEnd(error) {
      if (error || !found.size) return;
      const all = [...found.values()].flat().sort((a, b) => a.file.localeCompare(b.file) || a.line - b.line || a.column - b.column);
      this.error(`Release build stopped: ${all.length} Stub${all.length === 1 ? ' remains' : 's remain'}. Replace each with real content:\n${all.map(s => `  ${s.file}:${s.line}:${s.column} (${s.name})`).join('\n')}`);
    },
  };
}
