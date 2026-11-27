import { useEffect, useState, type ReactNode } from 'react';
import * as UI from '@stack-junkie/ui-web';
import type { Project, ScanData, Source, Status } from './types';

type Load = { state: 'loading' } | { state: 'empty' } | { state: 'error'; message: string } | { state: 'ready'; data: ScanData };
type ListId = 'using' | 'behind' | 'unknown' | 'not-using' | 'all';

const counted = (d: ScanData) => d.projects.filter(p => !p.source);
const lists: Record<ListId, { label: string; pick: (d: ScanData) => Project[]; empty: (d: ScanData) => string }> = {
  using: { label: 'Using the style', pick: d => counted(d).filter(p => p.usesStyle), empty: () => 'No project uses Stack Junkie Style yet.' },
  behind: { label: 'Behind the latest', pick: d => counted(d).filter(p => p.status === 'behind'), empty: d => `Every project with a readable version is on ${d.latest} or newer.` },
  unknown: { label: 'No version found', pick: d => counted(d).filter(p => p.status === 'unknown'), empty: () => 'Every project using the style names its version.' },
  'not-using': { label: 'Not using it yet', pick: d => counted(d).filter(p => !p.usesStyle && p.frontEnd && !p.exempt), empty: () => 'Every front-end project uses the style.' },
  all: { label: 'All projects', pick: d => d.projects, empty: () => 'The scan found no projects.' },
};

const statusBadge: Record<Status, { label: string; tone: 'neutral' | 'success' | 'warning' | 'danger' }> = {
  current: { label: 'Current', tone: 'success' }, behind: { label: 'Behind', tone: 'warning' }, ahead: { label: 'Newer', tone: 'success' },
  unknown: { label: 'No version', tone: 'neutral' }, 'not-using': { label: 'Not using', tone: 'neutral' }, source: { label: 'Source', tone: 'neutral' },
};
const whereLabel = (p: Project) => p.links.map(l => l.label.replace(/^Open (in|on) /, '').replace('VS Code', 'Local')).join(', ');
const markerLabel: Record<string, string> = { package: 'package.json', import: 'import', cmake: 'CMake', skill: 'copied skill' };

const plural = (n: number, one: string, many = `${one}s`) => `${n} ${n === 1 ? one : many}`;

function since(iso: string) {
  const minutes = Math.round((Date.now() - new Date(iso).getTime()) / 60000);
  if (minutes < 1) return 'just now';
  if (minutes < 60) return `${minutes} min ago`;
  const hours = Math.round(minutes / 60);
  return hours < 48 ? `${hours} h ago` : new Date(iso).toLocaleDateString();
}

function ProjectTable({ caption, rows, empty, versions = true }: { caption: string; rows: Project[]; empty: string; versions?: boolean }) {
  if (!rows.length) return <UI.EmptyState title="No projects" description={empty} />;
  const data = rows.map(p => ({ ...p, versionText: p.version ?? '', statusText: p.exempt ? 'Look-alike' : statusBadge[p.status].label, whereText: whereLabel(p), found: p.markers.map(m => markerLabel[m.kind]).join(', '), open: '' }));
  type Row = typeof data[number];
  const columns: UI.Column<Row>[] = [
    { key: 'name', label: 'Project', render: p => <strong>{p.name}</strong> },
    ...(versions ? [
      { key: 'statusText', label: 'Status', render: (p: Row) => p.exempt ? <UI.Badge>Look-alike</UI.Badge> : <UI.Badge tone={statusBadge[p.status].tone}>{statusBadge[p.status].label}</UI.Badge> },
      { key: 'versionText', label: 'Version', render: (p: Row) => p.version ? <span className="sj-mono">{p.version}</span> : <span className="sj-muted">None</span> },
      { key: 'found', label: 'Found in', render: (p: Row) => p.found || <span className="sj-muted">None</span> },
    ] satisfies UI.Column<Row>[] : []),
    { key: 'whereText', label: 'Where' },
    { key: 'open', label: 'Open', render: p => <span className="pd-links">{p.links.map(l => <a key={l.href} href={l.href} target={l.href.startsWith('http') ? '_blank' : undefined} rel="noreferrer">{l.label}</a>)}</span> },
  ];
  return <UI.DataTable caption={caption} rows={data} columns={columns} />;
}

function Tile({ label, value, detail, onOpen }: { label: string; value: number; detail?: string; onOpen: () => void }) {
  return <button type="button" className="pd-tile" onClick={onOpen}>
    <span className="pd-tile-label">{label}</span>
    <span className="pd-tile-value sj-mono">{value}</span>
    {detail && <span className="pd-tile-detail sj-muted">{detail}</span>}
  </button>;
}

function sourceLine(name: string, s: Source) {
  if (s.status === 'off') return `${name} was not scanned.`;
  if (s.status === 'not connected') return `${name} is not connected. ${s.reason ?? ''}`.trim();
  return `${name}: ${s.repos} repos${s.account ? ` for ${s.account}` : ''}.`;
}

function Dashboard({ data, open }: { data: ScanData; open: (id: ListId) => void }) {
  const count = (id: ListId) => lists[id].pick(data).length;
  const frontEnd = counted(data).filter(p => p.frontEnd && !p.exempt).length;
  const remoteIssues = (['github', 'gitlab'] as const).filter(h => data.sources[h].status !== 'connected');
  const unchecked = data.projects.filter(p => p.unchecked).map(p => p.name);
  return <UI.Stack gap={20}>
    <header className="pd-heading">
      <h1>Projects using Stack Junkie Style</h1>
      <p className="sj-muted">Latest is <span className="sj-mono">{data.latest}</span>. Scanned {since(data.scannedAt)}.</p>
    </header>
    <div className="pd-tiles">
      <Tile label="Using the style" value={count('using')} detail={`Of ${counted(data).length} projects scanned`} onOpen={() => open('using')} />
      <Tile label="Behind the latest version" value={count('behind')} detail={`Older than ${data.latest}`} onOpen={() => open('behind')} />
      <Tile label="Using it, no version found" value={count('unknown')} detail="No release tag or version in its files" onOpen={() => open('unknown')} />
      <Tile label="Front ends not using it yet" value={count('not-using')} detail={`Of ${frontEnd} front-end projects`} onOpen={() => open('not-using')} />
    </div>
    {remoteIssues.map(h => <UI.Alert key={h} tone="warning" title={sourceLine(h === 'github' ? 'GitHub' : 'GitLab', data.sources[h])}>Its repos are missing from these counts.</UI.Alert>)}
    {unchecked.length > 0 && <UI.Alert tone="info" title={`${plural(unchecked.length, 'repo')} not checked`}>{unchecked.join(', ')}. Their files could not be read, or the scan reached remoteBudget in scan.config.json.</UI.Alert>}
    {data.sources.local.errors ? <UI.Alert tone="info" title={`${plural(data.sources.local.errors, 'folder or file', 'folders or files')} could not be read`}>They are left out of the counts.</UI.Alert> : null}
  </UI.Stack>;
}

function ScanPage({ data, busy, rescan }: { data: ScanData; busy: boolean; rescan: () => void }) {
  const rows = [
    { id: 'local', source: 'Local folders', status: 'Scanned', detail: `${data.sources.local.projects} projects in ${data.roots.length} ${data.roots.length === 1 ? 'folder' : 'folders'}` },
    ...(['github', 'gitlab'] as const).map(h => { const s = data.sources[h]; return { id: h, source: h === 'github' ? 'GitHub' : 'GitLab', status: s.status === 'connected' ? 'Connected' : s.status === 'off' ? 'Off' : 'Not connected', detail: s.status === 'connected' ? `${s.repos} repos${s.account ? ` for ${s.account}` : ''}` : s.reason ?? '' }; }),
  ];
  return <UI.Stack gap={20}>
    <header className="pd-heading">
      <h1>Scan</h1>
      <p className="sj-muted">Last scan {new Date(data.scannedAt).toLocaleString()}.</p>
    </header>
    <div><UI.Button variant="primary" busy={busy} disabled={busy} onClick={rescan}>{busy ? 'Scanning' : 'Scan again'}</UI.Button></div>
    <UI.DataTable caption="Sources" rows={rows} columns={[{ key: 'source', label: 'Source' }, { key: 'status', label: 'Status' }, { key: 'detail', label: 'Detail' }]} />
    <UI.Panel title="Folders">
      <UI.Stack gap={8}>
        {data.roots.map(r => <span key={r} className="sj-mono">{r}</span>)}
        {data.skipped?.length ? <p className="sj-muted">Skipped: {data.skipped.join(', ')}. Change them in scan.config.json.</p> : null}
      </UI.Stack>
    </UI.Panel>
  </UI.Stack>;
}

function loadMode(): UI.ThemeMode { try { return localStorage.getItem('sj-projects-mode') === 'light' ? 'light' : 'dark'; } catch { return 'dark'; } }

export default function App() {
  const [load, setLoad] = useState<Load>({ state: 'loading' });
  const [busy, setBusy] = useState(false);
  const [location, setLocation] = useState<UI.ShellLocation>({ section: 'projects', page: 'dashboard' });
  const [mode, setMode] = useState<UI.ThemeMode>(loadMode);
  useEffect(() => { try { localStorage.setItem('sj-projects-mode', mode); } catch { /* Optional storage. */ } }, [mode]);

  async function read(method: 'GET' | 'POST') {
    try {
      const response = await fetch('api/projects', { method });
      const body = await response.json().catch(() => null);
      if (response.status === 404) { setLoad({ state: 'empty' }); return; }
      if (!response.ok || !body || body.error) { setLoad({ state: 'error', message: body?.error ?? `The dev server answered ${response.status}.` }); return; }
      setLoad({ state: 'ready', data: body as ScanData });
    } catch {
      setLoad({ state: 'error', message: 'The scan data could not be loaded. Start the app with npm run dev.' });
    }
  }
  useEffect(() => { void read('GET'); }, []);
  async function rescan() { setBusy(true); await read('POST'); setBusy(false); }

  const open = (id: ListId) => setLocation({ section: 'projects', page: id });
  const scanButton = <UI.Button variant="primary" busy={busy} disabled={busy} onClick={rescan}>{busy ? 'Scanning' : 'Run the scan'}</UI.Button>;
  function page(render: (data: ScanData) => ReactNode): ReactNode {
    if (load.state === 'loading') return <UI.Skeleton lines={4} label="Loading the last scan" />;
    if (load.state === 'empty') return <UI.EmptyState title="No scan yet" description="The scan reads the folders in scan.config.json and your GitHub and GitLab repos." action={scanButton} />;
    if (load.state === 'error') return <UI.Alert tone="danger" title="The scan data is not available">{load.message}</UI.Alert>;
    return render(load.data);
  }
  const listPage = (id: ListId): UI.ShellPage => ({ id, label: lists[id].label, group: 'Lists', content: page(d => <UI.Stack gap={16}>
    <h1>{lists[id].label}</h1>
    <ProjectTable caption={id === 'not-using' ? plural(lists[id].pick(d).length, 'front-end project') : `${plural(lists[id].pick(d).length, 'project')}, latest is ${d.latest}`}
      rows={lists[id].pick(d)} empty={lists[id].empty(d)} versions={id !== 'not-using'} />
  </UI.Stack>) });

  const sections: UI.ShellSection[] = [
    { id: 'projects', label: 'Projects', icon: 'grid', pages: [
      { id: 'dashboard', label: 'Dashboard', content: page(d => <Dashboard data={d} open={open} />) },
      listPage('using'), listPage('behind'), listPage('unknown'), listPage('not-using'), listPage('all'),
    ] },
    { id: 'scan', label: 'Scan', icon: 'search', pages: [{ id: 'scan', label: 'Scan', content: page(d => <ScanPage data={d} busy={busy} rescan={rescan} />) }] },
  ];

  return <UI.AppShell name="Projects" description="Which projects use Stack Junkie Style, and which version." version={UI.systemVersion}
    logo={<UI.BrandMark size={20} />} preset="web-app" sections={sections} storageKey="sj-projects" mainId="main"
    location={location} onNavigate={(section, p) => setLocation({ section, page: p ?? sections.find(s => s.id === section)?.pages[0].id ?? '' })}
    mode={mode} onModeChange={setMode} />;
}
