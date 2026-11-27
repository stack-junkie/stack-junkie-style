import { useEffect, useMemo, useState } from 'react';
import * as UI from '../library';
import { ActionDemo, CollectionDemo, ContainerDemo, ContentDemo, FeedbackDemo, FoundationDemo, InputDemo, NavigationDemo, OverlayDemo } from './demos';
import { LayoutPage, layoutPages, type FrameSettings } from './layouts';
import catalog from '../../../shared/catalog.json';
import './showcase.css';
import { SiteWordmark } from './SiteWordmark';
import { exportApp } from './exportApp';

const demoFamilies: Record<string, React.ComponentType<{ id: string }>> = { foundations: FoundationDemo, actions: ActionDemo, inputs: InputDemo, navigation: NavigationDemo, containers: ContainerDemo, collections: CollectionDemo, feedback: FeedbackDemo, overlays: OverlayDemo, content: ContentDemo };
const prefix = 'sj-v3';
// The shared catalog gives Product layouts the Settings icon; Settings must have the only gear, so layouts use their own icon here.
const familyIcon = (f: { icon: string }) => f.icon as UI.IconName;
function saved<T extends string>(key: string, fallback: T, allowed: readonly string[]): T { try { const value = localStorage.getItem(`${prefix}-${key}`); return value && allowed.includes(value) ? value as T : fallback; } catch { return fallback; } }
function savedJson<T>(key: string, fallback: T): T { try { return { ...fallback, ...JSON.parse(localStorage.getItem(`${prefix}-${key}`) || '{}') }; } catch { return fallback; } }
const families = catalog.families.filter(f => f.id !== 'layouts');
const layoutsFamily = catalog.families.find(f => f.id === 'layouts')!;
const entries = families.flatMap(f => f.components.map(c => ({ ...c, family: f.id, familyLabel: f.label })));
type Entry = typeof entries[number];
type Layout = Pick<UI.ShellConfig, 'resizableSidebar' | 'collapsibleSidebar'>;
const defaultLayout: Layout = { resizableSidebar: true, collapsibleSidebar: true };
// Choices drawn by later components. The switch is kept so the preset is complete, and the note says what is missing.
const notDrawnYet: Partial<Record<UI.PresetChoice, string>> = { dashboardHome: 'Drawn in a later preview.' };
const optionLabel = (value: string) => value.charAt(0).toUpperCase() + value.slice(1);
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
  const [mode, setMode] = useState<UI.ThemeMode>(() => saved('mode', 'dark', ['light', 'dark']));
  const [layout, setLayout] = useState<Layout>(() => { const { resizableSidebar, collapsibleSidebar } = savedJson('layout', defaultLayout); return { resizableSidebar, collapsibleSidebar }; });
  const [preset, setPreset] = useState<UI.PresetId>(() => saved<UI.PresetId>('preset', 'web-app', UI.presetIds));
  const [overrides, setOverrides] = useState<Partial<UI.PresetChoices>>(() => UI.presetOverrides(preset, savedJson<Partial<UI.PresetChoices>>('choices', {})));
  const choices = UI.resolvePreset(preset, overrides);
  const systemMode = UI.useSystemTheme();
  const themeFor = (theme: UI.ThemeChoice): UI.ThemeMode => theme === 'system' ? systemMode : theme;
  const [section, setSection] = useState('home');
  const [pages, setPages] = useState<Record<string, string>>(() => savedJson('pages', defaultPages));
  const [query, setQuery] = useState(''); const [commands, setCommands] = useState(false);
  // The bottom panel's Output tab lists the pages opened in this session, newest last.
  const [opened, setOpened] = useState<string[]>([]);
  const [settings, setSettings] = useState<UI.SettingsState>({ open: false, section: 'appearance' }); const [exportOpen, setExportOpen] = useState(false);
  useEffect(() => { try { Object.entries({ mode, preset, choices: JSON.stringify(overrides), layout: JSON.stringify(layout), pages: JSON.stringify(pages) }).forEach(([k, v]) => localStorage.setItem(`${prefix}-${k}`, v)); } catch { /* Optional storage. */ } }, [mode, preset, overrides, layout, pages]);
  useEffect(() => { if (!choices.commandPalette) { setCommands(false); return; } function key(e: KeyboardEvent) { if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'k') { e.preventDefault(); setCommands(o => !o); } } document.addEventListener('keydown', key); return () => document.removeEventListener('keydown', key); }, [choices.commandPalette]);
  const shellConfig: UI.ShellConfig = useMemo(() => ({ ...UI.defaultShellConfig, mode, ...layout }), [mode, layout]);
  useEffect(() => { if (choices.theme === 'system') setMode(systemMode); }, [choices.theme, systemMode]);
  const frame: FrameSettings =useMemo(() => ({ preset, choices: overrides, config: shellConfig }), [preset, overrides, shellConfig]);
  // Picking a preset starts from its choices; changing one choice keeps the rest. The theme choice also switches the showcase's own theme.
  function choosePreset(id: UI.PresetId) { setPreset(id); setOverrides({}); setMode(themeFor(UI.resolvePreset(id).theme)); }
  function setChoice<K extends UI.PresetChoice>(key: K, value: UI.PresetChoices[K]) {
    setOverrides(o => UI.presetOverrides(preset, { ...o, [key]: value }));
    if (key === 'theme') setMode(themeFor(value as UI.ThemeChoice));
  }

  function navigate(sid: string, pid?: string) { setSection(sid); if (pid) setPages(p => ({ ...p, [sid]: pid })); else setQuery(''); }
  const pageLabel = section === 'home' ? 'Home' : section === 'layouts' ? layoutPages.find(l => l.id === pages.layouts)?.label : entries.find(e => e.id === pages[section])?.label;
  useEffect(() => { if (pageLabel) setOpened(o => o[o.length - 1] === pageLabel ? o : [...o.slice(-49), pageLabel]); }, [pageLabel]);
  function findComponent() { if (section === 'home' || (choices.rail && !families.some(f => f.id === section))) navigate('foundations'); setTimeout(() => document.getElementById('catalog-search')?.focus()); }
  function select(id: string) { const entry = entries.find(c => c.id === id)!; navigate(entry.family, id); }
  const openSettings = (id?: string) => setSettings(s => ({ open: true, section: id ?? s.section }));

  const needle = query.trim().toLowerCase();
  const found = needle ? entries.filter(c => `${c.label} ${c.usage} ${c.familyLabel}`.toLowerCase().includes(needle)): [];
  const isFamily = families.some(f => f.id === section);
  const searchHere = section !== 'home' && (!choices.rail || isFamily) && choices.sidebar !== 'off';
  const search = <UI.TextInput id="catalog-search" label="Search components" type="search" placeholder="Search the library" value={query} onChange={e => setQuery(e.target.value)} />;
  const searchResults = <><UI.NavTree label="Search results" value={section === 'layouts' ? '' : pages[section] ?? ''} onChange={select} groups={families.map(f => ({ id: f.id, label: f.label, items: found.filter(e => e.family === f.id).map(e => ({ id: e.id, label: e.label })) })).filter(g => g.items.length)} /><p className="search-count" role="status">{found.length} matching components</p></>;
  const current = [...families, layoutsFamily].find(f => f.id === section);
  const sidebarTitle = section === 'home' ? 'Home' : searchHere && needle ? 'Search results' : !choices.rail ? 'Library' : current?.label;

  const sections: UI.ShellSection[] = [
    { id: 'home', label: 'Home', icon: 'grid', hidden: true,
      sidebar: <UI.SideNavigation label="Home pages" value="home" onChange={id => id === 'home' ? navigate('home'): navigate('foundations')} items={[{ id: 'home', label: 'Overview' }, { id: 'foundations', label: 'Foundations' }]} />,
      pages: [{ id: 'overview', label: 'Overview', content: <UI.SettingsPage title="Stack Junkie UI"><p>Explore the shared components, layouts and foundations.</p><UI.Stack><UI.Button onClick={() => navigate('foundations')}>Foundations</UI.Button><p className="sj-muted">Colors, typography, spacing, icons and motion.</p></UI.Stack><div className="home-sections">{catalog.families.filter(f => f.id !== 'foundations').map(f => <UI.Button key={f.id} onClick={() => navigate(f.id)}><UI.Icon name={familyIcon(f)} />{f.label}</UI.Button>)}</div></UI.SettingsPage> }] },
    ...families.map(f => ({ id: f.id, label: f.label, icon: familyIcon(f), rail: f.id !== 'foundations',
      pages: entries.filter(e => e.family === f.id).map(e => ({ id: e.id, label: e.label, content: (state: UI.ShellPageState) => <ComponentPage entry={e} state={state} /> })) })),
    { id: 'layouts', label: layoutsFamily.label, icon: 'layout', pages: layoutPages.map(l => ({ id: l.id, label: l.label, content: <div className="layout-host"><LayoutPage id={l.id} frame={frame} /></div> })) },
  ];

  return <UI.AppShell name="Stack Junkie UI" description="A living collection of reusable interface components, layouts and interactions." version={UI.systemVersion} logo={<UI.BrandMark size={44} />}
    className="showcase" preset={preset} choices={overrides} config={shellConfig} sections={sections} storageKey={`${prefix}-shell`} mainId="catalog-main" skipLabel="Skip to component examples"
    location={{ section, page: section === 'home' ? 'overview' : pages[section] }} onNavigate={navigate}
    mode={mode} onModeChange={setMode} settingsState={settings} onSettingsChange={setSettings}
    brand={<UI.Button variant="quiet" className="app-brand home-link" aria-label="Home" title="Home" onClick={() => navigate('home')}><SiteWordmark /></UI.Button>}
    sidebarHeader={searchHere ? search : null} sidebar={searchHere && needle ? searchResults : undefined} sidebarTitle={sidebarTitle}
    menuItems={{
      file: [{ id: 'export', label: 'Export app frame…', run: () => setExportOpen(true) }],
      edit: [{ id: 'find', label: 'Find component', run: findComponent }, { id: 'copy-name', label: 'Copy component name', disabled: !pageLabel || section === 'home', run: () => { void navigator.clipboard?.writeText(pageLabel ?? ''); } }],
      view: choices.commandPalette ? [{ id: 'commands', label: 'Command palette', shortcut: 'Ctrl+K', run: () => setCommands(true) }] : [],
      help: [{ id: 'keyboard', label: 'Keyboard shortcuts', run: () => openSettings('keyboard') }],
    }}
    menus={[{ id: 'library', label: 'Library', items: [...catalog.families.map(f => ({ id: f.id, label: f.label, checked: section === f.id, run: () => navigate(f.id) }))] }]}
    status={{ state: pageLabel, facts: [current && section !== 'home' ? current.label : 'Stack Junkie UI', UI.systemVersion] }}
    panels={[
      { id: 'output', label: 'Output', content: <UI.CodeBlock language="Opened" code={opened.map(label => `Opened ${label}`).join('\n') || 'Nothing opened yet.'} /> },
      { id: 'problems', label: 'Problems', content: <p className="sj-muted">No problems in this view.</p> },
    ]}
    aboutChildren={<UI.Disclosure label="About this showcase"><p>Web and native examples use the shared libraries. Your choices and example state remain while moving between views.</p></UI.Disclosure>}
    settings={[
      { id: 'appearance', label: 'Appearance', keywords: 'theme dark light mode color', content: <UI.SettingsSection label="Appearance">
        <UI.ThemeSetting mode={mode} onChange={setMode} />
      </UI.SettingsSection> },
      { id: 'layout', label: 'Layout', keywords: 'preset rail sidebar menu status panel tabs command palette theme density dashboard resize collapse export template', content: <UI.SettingsSection label="Layout">
        <div className="showcase-only-note">Showcase only. A preset sets the App frame for a new project; change any choice, then Export hands it over.</div>
        <UI.SettingsRow label="Preset" description={UI.presets.find(p => p.id === preset)?.description}><UI.Select label="Preset" value={preset} onChange={e => choosePreset(e.target.value as UI.PresetId)}>{UI.presets.map(p => <option key={p.id} value={p.id}>{p.label}</option>)}</UI.Select></UI.SettingsRow>
        {(Object.entries(UI.presetChoices) as [UI.PresetChoice, UI.PresetChoiceInfo][]).map(([key, info]) => {
          const value = choices[key]; const description = [info.description, notDrawnYet[key]].filter(Boolean).join(' ');
          return <UI.SettingsRow key={key} label={info.label} description={description}>{typeof value === 'boolean'
            ? <UI.Switch label={info.label} checked={value} onChange={e => setChoice(key, e.target.checked as never)} />
            : <UI.Select label={info.label} value={value} onChange={e => setChoice(key, e.target.value as never)}>{info.options.map(o => <option key={String(o)} value={String(o)}>{optionLabel(String(o))}</option>)}</UI.Select>}</UI.SettingsRow>;
        })}
        {Object.keys(overrides).length > 0 && <div><UI.Button onClick={() => choosePreset(preset)}>Reset to {UI.presets.find(p => p.id === preset)?.label}</UI.Button></div>}
        {([['resizableSidebar', 'Resizable sidebar', 'Drag the sidebar edge to any width.'], ['collapsibleSidebar', 'Collapsible sidebar', 'The menu button hides and shows the sidebar.']] as const).map(([key, label, description]) =>
          <UI.SettingsRow key={key} label={label} description={description}><UI.Switch label={label} checked={layout[key]} disabled={choices.sidebar === 'off'} onChange={e => setLayout(l => ({ ...l, [key]: e.target.checked }))} /></UI.SettingsRow>)}
        <UI.SettingsRow label="Export" description="Download an App.tsx that imports the library at this version and uses this preset and these choices."><UI.Button variant="primary" onClick={() => setExportOpen(true)}>Export app frame</UI.Button></UI.SettingsRow>
        <div><UI.Button onClick={() => { navigate('layouts', 'starter'); setSettings(s => ({ ...s, open: false })); }}>Preview the starter app</UI.Button></div>
      </UI.SettingsSection> },
      { id: 'keyboard', label: 'Keyboard', keywords: 'shortcuts keys resize command', content: <UI.SettingsSection label="Keyboard"><UI.Prose><p>Drag a sidebar divider to resize it. Arrow keys adjust a focused divider; Home and End use its available range.</p><p>Control K opens command search. Control comma opens Settings. In a row of open documents, Delete or Control W closes the focused tab.</p><p>With the Menu bar on, Control B shows or hides the sidebar and Control J the bottom panel. In the Menu bar, arrow keys move between menus and items and Escape closes the open menu.</p></UI.Prose></UI.SettingsSection> },
    ]}
    overlays={<>
      <UI.Dialog open={exportOpen} onClose={() => setExportOpen(false)} title="Export app frame" size="large" actions={<><UI.Button onClick={() => setExportOpen(false)}>Close</UI.Button><UI.Button variant="primary" onClick={() => { const url = URL.createObjectURL(new Blob([exportApp(frame)], { type: 'text/plain' })); const a = document.createElement('a'); a.href = url; a.download = 'App.tsx'; a.click(); URL.revokeObjectURL(url); }}>Download App.tsx</UI.Button></>}>
        <div className="export-body"><p className="showcase-only-note">Showcase only. The file imports the library pinned to {UI.systemVersion}; it copies no components or styles.</p><UI.CodeBlock language="App.tsx" code={exportApp(frame)} /></div>
      </UI.Dialog>
      {choices.commandPalette && <UI.CommandPalette open={commands} onClose={() => setCommands(false)} commands={[...catalog.families.map(f => ({ id: f.id, label: `Open ${f.label.toLowerCase()}`, run: () => navigate(f.id) })), { id: 'settings', label: 'Open Settings', run: () => openSettings() }, { id: 'export', label: 'Export app frame', run: () => setExportOpen(true) }, { id: 'theme', label: `Use ${mode === 'dark' ? 'light' : 'dark'} theme`, run: () => setMode(mode === 'dark' ? 'light' : 'dark') }]} />}
    </>} />;
}
