// Project scan: finds the projects that use Stack Junkie Style and the version each one uses.
// Local folders are walked on disk; GitHub and GitLab repos are listed through the gh and glab CLIs.
// Run directly (`node scan.mjs`) to write data/projects.json, or import `scan` for tests.
import { readFile, readdir, stat, writeFile, mkdir } from 'node:fs/promises';
import { execFile } from 'node:child_process';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const styleRoot = path.resolve(here, '../../..');
export const PACKAGE = '@stack-junkie/ui-web';

// Folders that never hold a project's own source.
const SKIP_DIRS = new Set(['node_modules', '.git', 'dist', 'dist-library', 'dist-types', 'out', 'coverage', 'target', 'bin', 'obj',
  '.tools', '.next', '.nuxt', '.svelte-kit', '.turbo', '.cache', '.venv', 'venv', '__pycache__', 'worktrees', 'site-packages']);
// Hidden folders are skipped except the ones agents keep skills in.
const HIDDEN_KEPT = new Set(['.claude', '.codex', '.agents', '.cursor']);
const SOURCE_EXT = new Set(['.ts', '.tsx', '.js', '.jsx', '.mjs', '.cjs', '.vue', '.svelte']);
const FRONTEND_DEPS = ['react', 'react-dom', 'vue', 'svelte', 'next', 'nuxt', 'vite', 'astro', 'solid-js', 'preact', '@angular/core', 'electron', '@tauri-apps/api', 'lit'];
const FRONTEND_LANGUAGES = new Set(['HTML', 'CSS', 'Vue', 'Svelte', 'Astro']);
const MAX_SOURCE_BYTES = 256 * 1024;

// ---------- versions ----------

/** The first version number in a dependency spec, tag or file, without a leading v. */
export function parseVersion(text) {
  const match = /v?(\d+\.\d+\.\d+(?:-[0-9A-Za-z.]+)?)/.exec(String(text ?? ''));
  return match ? match[1] : null;
}

/** Negative when a is older than b. A prerelease is older than its release; numeric prerelease parts compare as numbers. */
export function compareVersions(a, b) {
  const split = v => { const [core, pre] = v.split('-', 2); return { core: core.split('.').map(Number), pre: pre ? pre.split('.') : [] }; };
  const x = split(a), y = split(b);
  for (let i = 0; i < 3; i++) if (x.core[i] !== y.core[i]) return x.core[i] - y.core[i];
  if (!x.pre.length || !y.pre.length) return y.pre.length - x.pre.length;
  for (let i = 0; i < Math.max(x.pre.length, y.pre.length); i++) {
    const p = x.pre[i], q = y.pre[i];
    if (p === undefined) return -1;
    if (q === undefined) return 1;
    const pn = /^\d+$/.test(p), qn = /^\d+$/.test(q);
    if (pn && qn && Number(p) !== Number(q)) return Number(p) - Number(q);
    if (pn !== qn) return pn ? -1 : 1;
    if (p !== q) return p < q ? -1 : 1;
  }
  return 0;
}

export async function readLatestVersion(root = styleRoot) {
  return (await readFile(path.join(root, 'VERSION'), 'utf8')).trim();
}

// ---------- markers (shared by local and remote) ----------

/** Markers in a package.json: a dependency on the library and whether the project has a front end. */
export function packageMarkers(text, file) {
  let pkg;
  try { pkg = JSON.parse(text); } catch { return { markers: [], frontEnd: false, name: null, isLibrary: false }; }
  const deps = { ...pkg.peerDependencies, ...pkg.devDependencies, ...pkg.dependencies };
  const markers = [];
  if (typeof deps[PACKAGE] === 'string') markers.push({ kind: 'package', file, spec: deps[PACKAGE], version: parseVersion(deps[PACKAGE]) });
  return { markers, frontEnd: FRONTEND_DEPS.some(d => d in deps), name: typeof pkg.name === 'string' ? pkg.name : null, isLibrary: pkg.name === PACKAGE };
}

/** Markers in a CMake file: find_package(StackJunkie...), or FetchContent or add_subdirectory of the style repo. */
export function cmakeMarkers(text, file) {
  const markers = [];
  for (const m of text.matchAll(/find_package\s*\(\s*StackJunkie\w*([^)]*)\)/gi)) markers.push({ kind: 'cmake', file, version: parseVersion(m[1]) });
  for (const m of text.matchAll(/FetchContent_Declare\s*\(([^)]*)\)/gi)) {
    if (!/stack-junkie-style|stack-junkie-agent-skills/i.test(m[1])) continue;
    const tag = /GIT_TAG\s+(\S+)/i.exec(m[1]);
    markers.push({ kind: 'cmake', file, version: parseVersion(tag?.[1] ?? m[1]) });
  }
  for (const m of text.matchAll(/add_subdirectory\s*\(([^)]*stack-junkie-style[^)]*)\)/gi)) markers.push({ kind: 'cmake', file, version: null, note: m[1].trim() });
  const frontEnd = /add_executable\s*\([^)]*\bWIN32\b/i.test(text) || /WIN32_EXECUTABLE/i.test(text) || /find_package\s*\(\s*Qt\d/i.test(text) || /imgui/i.test(text);
  return { markers, frontEnd };
}

const importsLibrary = text => text.includes(`'${PACKAGE}`) || text.includes(`"${PACKAGE}`);
const isSkillFile = text => /^---[\s\S]*?^name:\s*stack-junkie-style\s*$/m.test(text.slice(0, 600));

// ---------- local ----------

function vscodeLink(dir) { return `vscode://file/${encodeURI(dir.replace(/\\/g, '/'))}`; }

/** Git remote URLs as host/owner/name, lower case, for matching local clones to listed repos. */
export function normalizeRemote(url) {
  const m = /(?:@|\/\/)(?:[^@/]+@)?([^/:]+)[:/](.+?)(?:\.git)?\/?$/.exec(String(url).trim());
  return m ? `${m[1]}/${m[2]}`.toLowerCase() : null;
}

async function readSmall(file, limit = MAX_SOURCE_BYTES) {
  const info = await stat(file);
  if (info.size > limit) return null;
  return readFile(file, 'utf8');
}

/** Walks each root. Every git repo is a project; folders outside any repo belong to the root's top-level folder they sit in. */
export async function scanLocal(roots, { skip = [], skipPaths = [], maxDepth = 10 } = {}) {
  const skipNames = new Set([...SKIP_DIRS, ...skip.map(s => s.toLowerCase())]);
  const skipAbs = new Set(skipPaths.map(p => path.resolve(p).toLowerCase()));
  const projects = [];
  let errors = 0;

  function newProject(dir, root) {
    const project = { name: path.basename(dir), where: 'local', path: dir, root, link: vscodeLink(dir), remotes: [], packageNames: [], markers: [], frontEnd: false, source: false, importFiles: 0 };
    projects.push(project); return project;
  }

  async function visit(dir, project, depth) {
    let entries;
    try { entries = await readdir(dir, { withFileTypes: true }); } catch { errors++; return; }
    const names = new Set(entries.map(e => e.name));
    if (names.has('.git')) {
      const gitEntry = entries.find(e => e.name === '.git');
      if (gitEntry.isFile()) {
        const pointer = await readFile(path.join(dir, '.git'), 'utf8').catch(() => '');
        if (/[\\/]worktrees[\\/]/.test(pointer)) return; // a worktree checkout of a repo scanned elsewhere
      }
      if (dir !== project.path) project = newProject(dir, project.root);
      project.git = true;
      if (gitEntry.isDirectory()) {
        const config = await readFile(path.join(dir, '.git', 'config'), 'utf8').catch(() => '');
        for (const m of config.matchAll(/^\s*url\s*=\s*(.+)$/gm)) { const r = normalizeRemote(m[1]); if (r) project.remotes.push(r); }
      }
    }
    // The style's own source tree: report it as the source and do not count it as a user.
    if (path.basename(dir) === 'stack-junkie-style' && names.has('SKILL.md')) {
      const lib = await readFile(path.join(dir, 'ui', 'web', 'package.json'), 'utf8').catch(() => null);
      const version = await readLatestVersion(dir).catch(() => null);
      if (lib && packageMarkers(lib, '').isLibrary) { project.source = true; project.sourceVersion = version; return; }
      const skill = await readSmall(path.join(dir, 'SKILL.md')).catch(() => null);
      if (skill && isSkillFile(skill)) project.markers.push({ kind: 'skill', file: path.relative(project.path, path.join(dir, 'SKILL.md')), version: version ? parseVersion(version) : null });
      return;
    }
    for (const entry of entries) {
      const full = path.join(dir, entry.name);
      if (entry.isDirectory()) {
        const lower = entry.name.toLowerCase();
        if (skipNames.has(lower) || lower.startsWith('build') || lower.startsWith('cmake-build') || skipAbs.has(full.toLowerCase())) continue;
        if (entry.name.startsWith('.') && !HIDDEN_KEPT.has(lower)) continue;
        if (depth < maxDepth) await visit(full, project, depth + 1);
        continue;
      }
      if (!entry.isFile()) continue;
      const rel = path.relative(project.path, full);
      try {
        if (entry.name === 'package.json') {
          const text = await readSmall(full); if (text === null) continue;
          const found = packageMarkers(text, rel);
          if (found.isLibrary) continue;
          if (found.name) project.packageNames.push(found.name);
          project.frontEnd ||= found.frontEnd;
          for (const marker of found.markers) {
            // The installed copy, when present, knows the exact version a range resolved to.
            const installed = await readFile(path.join(dir, 'node_modules', ...PACKAGE.split('/'), 'package.json'), 'utf8').then(t => JSON.parse(t).version).catch(() => null);
            if (installed) marker.version = installed;
            else if (/^(file|link):/.test(marker.spec)) marker.version = await readFile(path.join(dir, marker.spec.replace(/^(file|link):/, ''), 'package.json'), 'utf8').then(t => JSON.parse(t).version).catch(() => null);
            project.markers.push(marker);
          }
        } else if (entry.name === 'CMakeLists.txt' || entry.name.endsWith('.cmake')) {
          const text = await readSmall(full); if (text === null) continue;
          const found = cmakeMarkers(text, rel);
          project.frontEnd ||= found.frontEnd;
          project.markers.push(...found.markers);
        } else if (entry.name === 'index.html' && dir === project.path) {
          project.frontEnd = true;
        } else if (SOURCE_EXT.has(path.extname(entry.name))) {
          const text = await readSmall(full); if (text === null) continue;
          if (importsLibrary(text)) {
            project.importFiles++;
            if (project.importFiles === 1) project.markers.push({ kind: 'import', file: rel, version: null });
          }
        }
      } catch { errors++; }
    }
  }

  for (const root of roots) {
    let entries;
    try { entries = await readdir(root, { withFileTypes: true }); } catch { errors++; continue; }
    for (const entry of entries) {
      if (!entry.isDirectory()) continue;
      const lower = entry.name.toLowerCase();
      const full = path.join(root, entry.name);
      if (skipNames.has(lower) || skipAbs.has(full.toLowerCase()) || (entry.name.startsWith('.') && !HIDDEN_KEPT.has(lower))) continue;
      await visit(full, newProject(full, root), 1);
    }
  }
  // A top-level folder that only groups repos (like Games) is not a project itself unless it holds markers or a front end.
  const kept = projects.filter(p => p.git || p.markers.length || p.frontEnd || p.source || !projects.some(q => q !== p && q.path.startsWith(p.path + path.sep)));
  return { projects: kept, errors };
}

// ---------- remote ----------

/** Runs a CLI without a shell. Resolves { ok, stdout } and never rejects. */
export function runCommand(cmd, args, { timeout = 30000 } = {}) {
  return new Promise(resolve => {
    execFile(cmd, args, { timeout, maxBuffer: 32 * 1024 * 1024, windowsHide: true }, (error, stdout) => resolve({ ok: !error, stdout: String(stdout ?? ''), missing: error?.code === 'ENOENT' }));
  });
}

async function pool(items, size, work) {
  const queue = [...items];
  await Promise.all(Array.from({ length: Math.min(size, queue.length) }, async () => { while (queue.length) await work(queue.shift()); }));
}

function remoteProject(where, { name, fullName, url, archived, language }) {
  return { name, fullName, where, url, link: url, remotes: [normalizeRemote(url)].filter(Boolean), packageNames: [], markers: [], frontEnd: FRONTEND_LANGUAGES.has(language ?? ''), source: false, archived: !!archived, language: language ?? null };
}

/** Reads markers from a repo's root files. `list` returns root file names; `fetch` returns a file's text or null. */
async function inspectRemote(project, list, fetch, budget) {
  if (budget.left <= 0) { project.unchecked = true; return; }
  budget.left--;
  const files = await list();
  if (!files) { project.unchecked = true; return; }
  for (const file of ['package.json', 'CMakeLists.txt']) {
    if (!files.includes(file)) continue;
    if (budget.left <= 0) { project.unchecked = true; return; }
    budget.left--;
    const text = await fetch(file);
    if (text === null) continue;
    if (file === 'package.json') {
      const found = packageMarkers(text, file);
      if (found.isLibrary) { project.source = true; continue; }
      if (found.name) project.packageNames.push(found.name);
      project.frontEnd ||= found.frontEnd; project.markers.push(...found.markers);
    } else {
      const found = cmakeMarkers(text, file);
      project.frontEnd ||= found.frontEnd; project.markers.push(...found.markers);
    }
  }
  if (files.includes('index.html')) project.frontEnd = true;
}

export async function scanGitHub({ run = runCommand, limit = 200, budget = { left: 120 }, skipRemotes = new Set(), includeForks = false, includeArchived = false, sourceRepos = [] } = {}) {
  const version = await run('gh', ['--version']);
  if (!version.ok) return { status: 'not connected', reason: version.missing ? 'The gh CLI is not installed.' : 'The gh CLI did not run.', projects: [] };
  const login = await run('gh', ['api', 'user', '--jq', '.login']);
  if (!login.ok) return { status: 'not connected', reason: 'The gh CLI is not signed in. Run gh auth login.', projects: [] };
  const account = login.stdout.trim();
  const listed = await run('gh', ['repo', 'list', '--limit', String(limit), '--json', 'name,nameWithOwner,url,isArchived,isFork,isEmpty,primaryLanguage']);
  if (!listed.ok) return { status: 'not connected', account, reason: 'gh repo list failed.', projects: [] };
  let repos;
  try { repos = JSON.parse(listed.stdout); } catch { return { status: 'not connected', account, reason: 'gh repo list returned unreadable output.', projects: [] }; }
  repos = repos.filter(r => (includeForks || !r.isFork) && (includeArchived || !r.isArchived));
  const projects = repos.map(r => remoteProject('github', { name: r.name, fullName: r.nameWithOwner, url: r.url, archived: r.isArchived, language: r.primaryLanguage?.name }));
  for (const p of projects) if (sourceRepos.includes(p.fullName.toLowerCase())) p.source = true;
  const empty = new Set(repos.filter(r => r.isEmpty).map(r => r.nameWithOwner));
  const toCheck = projects.filter(p => !p.source && !empty.has(p.fullName) && !p.remotes.some(r => skipRemotes.has(r)));
  await pool(toCheck, 4, p => inspectRemote(p,
    async () => { const r = await run('gh', ['api', `repos/${p.fullName}/contents`, '--jq', '.[].name']); return r.ok ? r.stdout.split(/\r?\n/).filter(Boolean) : null; },
    async file => { const r = await run('gh', ['api', `repos/${p.fullName}/contents/${file}`, '-H', 'Accept: application/vnd.github.raw']); return r.ok ? r.stdout : null; },
    budget));
  // One code search each catches the library imported below the root and copied skills, without a call per repo.
  for (const [query, kind] of [[`"${PACKAGE}" user:${account}`, 'import'], [`"name: stack-junkie-style" filename:SKILL.md user:${account}`, 'skill']]) {
    const r = await run('gh', ['api', '-X', 'GET', 'search/code', '-f', `q=${query}`, '--jq', '.items[] | [.repository.full_name, .path] | @tsv']);
    if (!r.ok) continue;
    for (const line of r.stdout.split(/\r?\n/).filter(Boolean)) {
      const [fullName, file] = line.split('\t');
      const p = projects.find(x => x.fullName.toLowerCase() === fullName.toLowerCase());
      if (!p || p.source || p.markers.some(m => m.file === file)) continue;
      if (kind === 'import' && (file.endsWith('package.json') || !SOURCE_EXT.has(path.extname(file)))) continue;
      if (kind === 'import' && p.markers.some(m => m.kind === 'import')) continue;
      p.markers.push({ kind, file, version: null });
    }
  }
  return { status: 'connected', account, repos: projects.length, unchecked: projects.filter(p => p.unchecked).length, projects };
}

export async function scanGitLab({ run = runCommand, budget = { left: 120 }, skipRemotes = new Set(), includeForks = false, includeArchived = false } = {}) {
  const version = await run('glab', ['--version']);
  if (!version.ok) return { status: 'not connected', reason: version.missing ? 'The glab CLI is not installed.' : 'The glab CLI did not run.', projects: [] };
  const listed = await run('glab', ['repo', 'list', '--mine', '--per-page', '100', '-F', 'json']);
  if (!listed.ok) return { status: 'not connected', reason: 'glab is not signed in. Run glab auth login.', projects: [] };
  let raw;
  try { raw = JSON.parse(listed.stdout); } catch { return { status: 'not connected', reason: 'glab repo list returned unreadable output.', projects: [] }; }
  // Keep only the fields the dashboard needs; the listing also carries settings that must not be stored.
  const repos = raw.map(r => ({ id: r.id, name: r.path ?? r.name, fullName: r.path_with_namespace, url: r.web_url, branch: r.default_branch, archived: !!r.archived, fork: !!r.forked_from_project, empty: !!r.empty_repo }))
    .filter(r => (includeForks || !r.fork) && (includeArchived || !r.archived));
  const account = repos[0]?.fullName?.split('/')[0];
  const projects = repos.map(r => Object.assign(remoteProject('gitlab', r), { id: r.id, branch: r.branch, empty: r.empty }));
  const toCheck = projects.filter(p => !p.empty && p.branch && !p.remotes.some(r => skipRemotes.has(r)));
  await pool(toCheck, 4, p => inspectRemote(p,
    async () => { const r = await run('glab', ['api', `projects/${p.id}/repository/tree?per_page=100&ref=${encodeURIComponent(p.branch)}`]); try { return r.ok ? JSON.parse(r.stdout).map(f => f.name) : null; } catch { return null; } },
    async file => { const r = await run('glab', ['api', `projects/${p.id}/repository/files/${encodeURIComponent(file)}/raw?ref=${encodeURIComponent(p.branch)}`]); return r.ok ? r.stdout : null; },
    budget));
  for (const p of projects) { delete p.id; delete p.branch; delete p.empty; }
  return { status: 'connected', account, repos: projects.length, unchecked: projects.filter(p => p.unchecked).length, projects };
}

// ---------- combine ----------

function lowestVersion(markers) {
  const known = markers.map(m => m.version).filter(Boolean);
  return known.length ? known.sort(compareVersions)[0] : null;
}

/** The project's standing against the latest version. */
export function projectStatus(p, latest) {
  if (p.source) return 'source';
  if (!p.markers.length) return 'not-using';
  if (!p.version) return 'unknown';
  const order = compareVersions(p.version, latest);
  return order < 0 ? 'behind' : order > 0 ? 'ahead' : 'current';
}

function finish(p, latest, exempt) {
  const names = [p.name, p.fullName, ...p.packageNames].filter(Boolean).map(n => n.toLowerCase());
  const version = p.source ? p.sourceVersion ?? latest : lowestVersion(p.markers);
  const out = {
    id: `${p.where}:${p.path ?? p.fullName}`,
    name: p.name,
    where: p.where,
    ...(p.path ? { path: p.path } : {}),
    ...(p.url ? { url: p.url } : {}),
    link: p.link,
    links: p.links ?? [{ label: p.where === 'local' ? 'Open in VS Code' : p.where === 'github' ? 'Open on GitHub' : 'Open on GitLab', href: p.link }],
    usesStyle: p.source || p.markers.length > 0,
    source: p.source,
    version,
    latest,
    frontEnd: p.frontEnd,
    exempt: names.some(n => exempt.includes(n) || exempt.includes(n.split('/').pop())),
    // The source tree holds copies and fixtures of its own markers; they say nothing about use.
    markers: (p.source ? [] : p.markers).map(({ kind, file, version: v }) => ({ kind, file, version: v ?? null })),
    ...(p.unchecked ? { unchecked: true } : {}),
  };
  out.status = projectStatus({ ...p, version }, latest);
  return out;
}

/** Scans local roots and, unless turned off, GitHub and GitLab. Returns the data the dashboard reads. */
export async function scan({ roots = [], latest, skip = [], skipPaths = [], exempt = [], remote = true, run = runCommand, remoteBudget = 120, includeForks = false, includeArchived = false, sourceRepos = [] } = {}) {
  latest ??= await readLatestVersion();
  const exemptNames = exempt.map(e => e.toLowerCase());
  const local = await scanLocal(roots, { skip, skipPaths });
  const localRemotes = new Set(local.projects.flatMap(p => p.remotes));
  const budget = { left: remoteBudget };
  const options = { run, budget, skipRemotes: localRemotes, includeForks, includeArchived, sourceRepos: sourceRepos.map(s => s.toLowerCase()) };
  const github = remote ? await scanGitHub(options) : { status: 'off', projects: [] };
  const gitlab = remote ? await scanGitLab(options) : { status: 'off', projects: [] };

  // A local clone of a listed repo is one project: local findings, plus a link to the repo.
  const merged = local.projects.map(p => ({ ...p, links: [{ label: 'Open in VS Code', href: p.link }] }));
  const remoteOnly = [];
  for (const r of [...github.projects, ...gitlab.projects]) {
    const clone = merged.find(p => p.remotes.some(x => r.remotes.includes(x)));
    const label = r.where === 'github' ? 'Open on GitHub' : 'Open on GitLab';
    if (clone) { clone.url ??= r.url; clone.links.push({ label, href: r.url }); clone.source ||= r.source; continue; }
    // A mirror on both hosts is one project.
    const mirror = remoteOnly.find(p => p.name.toLowerCase() === r.name.toLowerCase());
    if (mirror) { mirror.links.push({ label, href: r.url }); mirror.markers.push(...r.markers.filter(m => !mirror.markers.some(x => x.file === m.file))); mirror.frontEnd ||= r.frontEnd; mirror.source ||= r.source; continue; }
    remoteOnly.push({ ...r, links: [{ label, href: r.url }] });
  }
  const projects = [...merged, ...remoteOnly].map(p => finish(p, latest, exemptNames)).sort((a, b) => a.name.localeCompare(b.name));
  const strip = ({ projects: _, ...rest }) => rest;
  return {
    scannedAt: new Date().toISOString(),
    latest,
    roots,
    skipped: skip,
    sources: { local: { status: 'connected', roots: roots.length, projects: local.projects.length, errors: local.errors }, github: strip(github), gitlab: strip(gitlab) },
    projects,
  };
}

// ---------- command line ----------

async function main() {
  const configPath = path.join(here, 'scan.config.json');
  const config = JSON.parse(await readFile(configPath, 'utf8'));
  const args = process.argv.slice(2);
  const roots = args.filter(a => !a.startsWith('--'));
  const result = await scan({ ...config, roots: (roots.length ? roots : config.roots).map(r => path.resolve(here, r)), remote: !args.includes('--local') });
  await mkdir(path.join(here, 'data'), { recursive: true });
  await writeFile(path.join(here, 'data', 'projects.json'), `${JSON.stringify(result, null, 2)}\n`);
  const counted = result.projects.filter(p => !p.source);
  const using = counted.filter(p => p.usesStyle);
  console.log(`Scanned ${result.projects.length} projects: ${using.length} use Stack Junkie Style, ${using.filter(p => p.status === 'behind').length} behind ${result.latest}.`);
  for (const host of ['github', 'gitlab']) { const s = result.sources[host]; console.log(`${host === 'github' ? 'GitHub' : 'GitLab'}: ${s.status}${s.repos !== undefined ? `, ${s.repos} repos` : ''}${s.reason ? ` (${s.reason})` : ''}`); }
}

if (process.argv[1] && import.meta.url === pathToFileURL(path.resolve(process.argv[1])).href) {
  main().catch(error => { console.error(`Scan failed: ${error.message}`); process.exit(1); });
}
