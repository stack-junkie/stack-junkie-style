import { useEffect, useMemo, useState } from 'react';
import * as UI from '../library';
import { ActionDemo, CollectionDemo, ContainerDemo, ContentDemo, FeedbackDemo, FoundationDemo, InputDemo, NavigationDemo, OverlayDemo } from './demos';
import { initialUtility, LayoutPage, layoutPages, UtilityFixture } from './layouts';
import catalog from '../../../shared/catalog.json';
import './showcase.css';
import { SiteWordmark } from './SiteWordmark';
import { exportApp } from './exportApp';

const demoFamilies: Record<string, React.ComponentType<{ id: string }>> = { foundations: FoundationDemo, actions: ActionDemo, inputs: InputDemo, navigation: NavigationDemo, containers: ContainerDemo, collections: CollectionDemo, feedback: FeedbackDemo, overlays: OverlayDemo, content: ContentDemo };
const prefix = 'sj-next';
// The shared catalog gives Product layouts the Settings icon; Settings must have the only gear, so layouts use their own icon here.
const familyIcon = (f: { id: string; icon: string }) => (f.id === 'layouts' ? 'layout' : f.icon) as UI.IconName;
function saved<T extends string>(key: string, fallback: T, allowed: readonly string[]): T { try { const value = localStorage.getItem(`${prefix}-${key}`); return value && allowed.includes(value) ? value as T : fallback; } catch { return fallback; } }
function savedJson<T>(key: string, fallback: T): T { try { return { ...fallback, ...JSON.parse(localStorage.getItem(`${prefix}-${key}`) || '{}') }; } catch { return fallback; } }
const families = catalog.families.filter(f => f.id !== 'layouts');
const layoutsFamily = catalog.families.find(f => f.id === 'layouts')!;
const entries = families.flatMap(f => f.components.map(c => ({ ...c, family: f.id, familyLabel: f.label })));
type Entry = typeof entries[number];
type Layout = Pick<UI.ShellConfig, 'rail' | 'contextualSidebar' | 'resizableSidebar' | 'collapsibleSidebar'>;
const defaultLayout: Layout = { rail: true, contextualSidebar: true, resizableSidebar: true, collapsibleSidebar: true };
const defaultPages: Record<string, string> = { ...Object.fromEntries(families.map(f => [f.id, f.components[0].id])), layouts: 'workbench' };

function Specimen({ entry }: { entry: Entry }) { const Demo = demoFamilies[entry.family]; return <UI.TabBoundary><div className="specimen-stage"><Demo id={entry.id} /></div></UI.TabBoundary>; }
function Usage({ entry }: { entry: Entry }) { return <UI.Stack className="component-reference"><p>{entry.usage}</p><dl><dt>Variants</dt><dd>{entry.variants.join(', ')}</dd><dt>States</dt><dd>{entry.states.join(', ')}</dd></dl></UI.Stack>; }
// A component page has its own Example / Usage row only when it is not already inside the frame's tab row.
function ComponentPage({ entry, state }: { entry: Entry; state: UI.ShellPageState }) {
  const [tab, setTab] = useState('example');
  if (state.tabbed) return <UI.Stack gap={20}><Specimen entry={entry} /><UI.Disclosure label="Usage and states"><Usage entry={entry} /></UI.Disclosure></UI.Stack>;
  return <UI.Tabs contextLabel={state.sidebarCollapsed ? entry.label : undefined} className="catalog-tabs" label={`${entry.label} views`} value={tab} onChange={setTab} tabs={[
    { id: 'example', label: 'Example', content: <Specimen entry={entry} /> },
    { id: 'reference', label: 'Usage and states', content: <Usage entry={entry} /> },
  ]} />;
}

export default function App() {
  const [direction, setDirection] = useState<UI.Direction>(() => saved('direction', 'clean', ['clean', 'warm', 'technical']));
  const [mode, setMode] = useState<UI.ThemeMode>(() => saved('mode', 'dark', ['light', 'dark']));
  const [layout, setLayout] = useState<Layout>(() => savedJson('layout', defaultLayout));
  const [section, setSection] = useState('home');
  const [pages, setPages] = useState<Record<string, string>>(() => savedJson('pages', defaultPages));
  const [query, setQuery] = useState(''); const [commands, setCommands] = useState(false);
  const [settings, setSettings] = useState<UI.SettingsState>({ open: false, section: 'appearance' }); const [exportOpen, setExportOpen] = useState(false);
  const [comparison, setComparison] = useState(initialUtility);
  useEffect(() => { try { Object.entries({ direction, mode, layout: JSON.stringify(layout), pages: JSON.stringify(pages) }).forEach(([k, v]) => localStorage.setItem(`${prefix}-${k}`, v)); } catch { /* Optional storage. */ } }, [direction, mode, layout, pages]);
  useEffect(() => { function key(e: KeyboardEvent) { if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'k') { e.preventDefault(); setCommands(o => !o); } } document.addEventListener('keydown', key); return () => document.removeEventListener('keydown', key); }, []);
  const shellConfig: UI.ShellConfig = useMemo(() => ({ direction, mode, ...layout }), [direction, mode, layout]);

  function navigate(sid: string, pid?: string) { setSection(sid); if (pid) setPages(p => ({ ...p, [sid]: pid })); else setQuery(''); }
  function select(id: string) { const entry = entries.find(c => c.id === id)!; navigate(entry.family, id); }
  const openSettings = (id?: string) => setSettings(s => ({ open: true, section: id ?? s.section }));

  const needle = query.trim().toLowerCase();
  const found = needle ? entries.filter(c => `${c.label} ${c.usage} ${c.familyLabel}`.toLowerCase().includes(needle)): [];
  const isFamily = families.some(f => f.id === section);
  const searchHere = !['home', 'compare'].includes(section) && (!layout.rail || isFamily);
  const search = <UI.TextInput label="Search components" type="search" placeholder="Search the library" value={query} onChange={e => setQuery(e.target.value)} />;
  const searchResults = <><UI.NavTree label="Search results" value={section === 'layouts' ? '' : pages[section] ?? ''} onChange={select} groups={families.map(f => ({ id: f.id, label: f.label, items: found.filter(e => e.family === f.id).map(e => ({ id: e.id, label: e.label })) })).filter(g => g.items.length)} /><p className="search-count" role="status">{found.length} matching components</p></>;
  const current = [...families, layoutsFamily].find(f => f.id === section);
  const sidebarTitle = section === 'home' ? 'Home' : section === 'compare' ? 'Appearance' : searchHere && needle ? 'Search results' : !layout.rail ? 'Library' : current?.label;

  const sections: UI.ShellSection[] = [
    { id: 'home', label: 'Home', icon: 'grid', hidden: true,
      sidebar: <UI.SideNavigation label="Home pages" value="home" onChange={id => id === 'home' ? navigate('home'): navigate('foundations')} items={[{ id: 'home', label: 'Overview' }, { id: 'foundations', label: 'Foundations' }]} />,
      pages: [{ id: 'overview', label: 'Overview', content: <UI.SettingsPage title="Stack Junkie UI"><p>Explore the shared components, layouts and foundations.</p><UI.Stack><UI.Button onClick={() => navigate('foundations')}>Foundations</UI.Button><p className="sj-muted">Colors, typography, spacing, icons and motion.</p></UI.Stack><div className="home-sections">{catalog.families.filter(f => f.id !== 'foundations').map(f => <UI.Button key={f.id} onClick={() => navigate(f.id)}><UI.Icon name={familyIcon(f)} />{f.label}</UI.Button>)}</div></UI.SettingsPage> }] },
    { id: 'compare', label: 'Appearance', icon: 'grid', hidden: true,
      pages: [{ id: 'compare', label: 'Compare directions', content: <section className="comparison-page" aria-label="Direction comparison"><p className="comparison-note">Shared inputs across all three previews.</p><div className="comparison-grid">{(['clean', 'warm', 'technical'] as UI.Direction[]).map(d => <UI.ThemeProvider key={d} direction={d} mode={mode} className="comparison-column"><header><h2>{UI.directions[d].label}</h2></header><UtilityFixture state={comparison} onChange={setComparison} compact /></UI.ThemeProvider>)}</div></section> }] },
    ...families.map(f => ({ id: f.id, label: f.label, icon: familyIcon(f), rail: f.id !== 'foundations',
      pages: entries.filter(e => e.family === f.id).map(e => ({ id: e.id, label: e.label, content: (state: UI.ShellPageState) => <ComponentPage entry={e} state={state} /> })) })),
    { id: 'layouts', label: layoutsFamily.label, icon: 'layout', pages: layoutPages.map(l => ({ id: l.id, label: l.label, content: <div className="layout-host"><LayoutPage id={l.id} config={shellConfig} /></div> })) },
  ];

  return <UI.AppShell name="Stack Junkie UI" description="A living collection of reusable interface components, layouts and interactions." version={UI.systemVersion} logo={<UI.BrandMark size={44} />}
    className="showcase" config={shellConfig} sections={sections} storageKey={`${prefix}-shell`} mainId="catalog-main" skipLabel="Skip to component examples"
    location={{ section, page: section === 'home' ? 'overview' : section === 'compare' ? 'compare' : pages[section] }} onNavigate={navigate}
    mode={mode} onModeChange={setMode} settingsState={settings} onSettingsChange={setSettings}
    brand={<UI.Button variant="quiet" className="app-brand home-link" aria-label="Home" title="Home" onClick={() => navigate('home')}><SiteWordmark /></UI.Button>}
    sidebarHeader={searchHere ? search : null} sidebar={searchHere && needle ? searchResults : undefined} sidebarTitle={sidebarTitle}
    aboutChildren={<UI.Disclosure label="About this showcase"><p>Web and native examples use the shared libraries. Your choices and example state remain while moving between views.</p></UI.Disclosure>}
    settings={[
      { id: 'appearance', label: 'Appearance', keywords: 'theme dark light mode color direction', content: <UI.SettingsSection label="Appearance">
        <UI.ThemeSetting mode={mode} onChange={setMode} />
        <div className="showcase-only-note">Showcase only. The setting below compares candidates and is not part of the imported UI library.</div>
        <UI.SettingsRow label="Design direction" description="Compare the existing style candidates."><UI.Select label="Design direction" value={direction} onChange={e => setDirection(e.target.value as UI.Direction)}>{Object.entries(UI.directions).map(([id, d]) => <option value={id} key={id}>{d.label}</option>)}</UI.Select></UI.SettingsRow>
        <div><UI.Button onClick={() => { navigate('compare'); setSettings(s => ({ ...s, open: false })); }}>Compare directions</UI.Button></div>
      </UI.SettingsSection> },
      { id: 'layout', label: 'Layout', keywords: 'rail sidebar resize collapse export template', content: <UI.SettingsSection label="Layout">
        <div className="showcase-only-note">Showcase only. These switches configure the app frame for a new project, then Export hands it over.</div>
        {([['rail', 'Icon rail', 'The thin column of section icons on the far left.'], ['contextualSidebar', 'Contextual sidebar', 'The list of pages for the current section. Without the rail it lists every page in groups.'], ['resizableSidebar', 'Resizable sidebar', 'Drag the sidebar edge to any width.'], ['collapsibleSidebar', 'Collapsible sidebar', 'The menu button hides and shows the sidebar.']] as const).map(([key, label, description]) =>
          <UI.SettingsRow key={key} label={label} description={description}><UI.Switch label={label} checked={layout[key]} disabled={key !== 'rail' && key !== 'contextualSidebar' && !layout.contextualSidebar} onChange={e => setLayout(l => ({ ...l, [key]: e.target.checked }))} /></UI.SettingsRow>)}
        <UI.SettingsRow label="Export" description="Download an App.tsx that imports the library at this version and uses these layout settings."><UI.Button variant="primary" onClick={() => setExportOpen(true)}>Export app frame</UI.Button></UI.SettingsRow>
        <div><UI.Button onClick={() => { navigate('layouts', 'starter'); setSettings(s => ({ ...s, open: false })); }}>Preview the starter app</UI.Button></div>
      </UI.SettingsSection> },
      { id: 'keyboard', label: 'Keyboard', keywords: 'shortcuts keys resize command', content: <UI.SettingsSection label="Keyboard"><UI.Prose><p>Drag a sidebar divider to resize it. Arrow keys adjust a focused divider; Home and End use its available range.</p><p>Control K opens command search. Control comma opens Settings. In a row of open documents, Delete or Control W closes the focused tab.</p></UI.Prose></UI.SettingsSection> },
    ]}
    overlays={<>
      <UI.Dialog open={exportOpen} onClose={() => setExportOpen(false)} title="Export app frame" size="large" actions={<><UI.Button onClick={() => setExportOpen(false)}>Close</UI.Button><UI.Button variant="primary" onClick={() => { const url = URL.createObjectURL(new Blob([exportApp(shellConfig)], { type: 'text/plain' })); const a = document.createElement('a'); a.href = url; a.download = 'App.tsx'; a.click(); URL.revokeObjectURL(url); }}>Download App.tsx</UI.Button></>}>
        <div className="export-body"><p className="showcase-only-note">Showcase only. The file imports the library pinned to {UI.systemVersion}; it copies no components or styles.</p><UI.CodeBlock language="App.tsx" code={exportApp(shellConfig)} /></div>
      </UI.Dialog>
      <UI.CommandPalette open={commands} onClose={() => setCommands(false)} commands={[...catalog.families.map(f => ({ id: f.id, label: `Open ${f.label.toLowerCase()}`, run: () => navigate(f.id) })), { id: 'compare', label: 'Compare directions', run: () => navigate('compare') }, { id: 'settings', label: 'Open Settings', run: () => openSettings() }, { id: 'export', label: 'Export app frame', run: () => setExportOpen(true) }, { id: 'theme', label: `Use ${mode === 'dark' ? 'light' : 'dark'} theme`, run: () => setMode(mode === 'dark' ? 'light' : 'dark') }]} />
    </>} />;
}
