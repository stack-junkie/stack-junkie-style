import { useEffect, useState } from 'react';
import * as UI from '../library';
import { ActionDemo, CollectionDemo, ContainerDemo, ContentDemo, FeedbackDemo, FoundationDemo, InputDemo, NavigationDemo, OverlayDemo } from './demos';
import { initialUtility, LayoutDemo, UtilityFixture } from './layouts';
import catalog from '../../../shared/catalog.json';
import './showcase.css';
import { SiteWordmark } from './SiteWordmark';

const demoFamilies: Record<string, React.ComponentType<{ id: string }>> = { foundations: FoundationDemo, actions: ActionDemo, inputs: InputDemo, navigation: NavigationDemo, containers: ContainerDemo, collections: CollectionDemo, feedback: FeedbackDemo, overlays: OverlayDemo, content: ContentDemo, layouts: LayoutDemo };
function saved<T extends string>(key: string, fallback: T, allowed: readonly string[]): T { try { const value = localStorage.getItem(key); return value && allowed.includes(value) ? value as T : fallback; } catch { return fallback; } }
const entries = catalog.families.flatMap(f => f.components.map(c => ({ ...c, family: f.id, familyLabel: f.label })));
function initialSelections() { try { const value = JSON.parse(localStorage.getItem('sj-ui-elements') || '{}'); return Object.fromEntries(catalog.families.map(f => [f.id, f.components.some(c => c.id === value[f.id]) ? value[f.id] : f.components[0].id])); } catch { return Object.fromEntries(catalog.families.map(f => [f.id, f.components[0].id])); } }
function ComponentPage({ entry, sidebarCollapsed }: { entry: typeof entries[number]; sidebarCollapsed: boolean }) {
  const [tab, setTab] = useState('example'); const Demo = demoFamilies[entry.family];
  return <UI.Tabs contextLabel={sidebarCollapsed ? entry.label : undefined} className="catalog-tabs" label={`${entry.label} views`} value={tab} onChange={setTab} tabs={[
    { id: 'example', label: 'Example', content: <div className="specimen-stage"><Demo id={entry.id} /></div> },
    { id: 'reference', label: 'Usage and states', content: <UI.Stack className="component-reference"><p>{entry.usage}</p><dl><dt>Variants</dt><dd>{entry.variants.join(', ')}</dd><dt>States</dt><dd>{entry.states.join(', ')}</dd></dl></UI.Stack> }
  ]} />;
}
export default function App() {
  const [direction, setDirection] = useState<UI.Direction>(() => saved('sj-ui-direction', 'clean', ['clean', 'warm', 'technical']));
  const [mode, setMode] = useState<UI.ThemeMode>(() => saved('sj-ui-mode', 'dark', ['light', 'dark']));
  const [family, setFamily] = useState<string>(() => saved<string>('sj-ui-family', 'foundations', catalog.families.map(f => f.id)));
  const [selections, setSelections] = useState<Record<string, string>>(initialSelections);
  const [view, setView] = useState<'home' | 'catalog' | 'compare' | 'settings'>('home'); const [query, setQuery] = useState(''); const [commands, setCommands] = useState(false); const [settingsPage, setSettingsPage] = useState('appearance'); const [comparison, setComparison] = useState(initialUtility);
  useEffect(() => { try { localStorage.setItem('sj-ui-direction', direction); localStorage.setItem('sj-ui-mode', mode); localStorage.setItem('sj-ui-family', family); localStorage.setItem('sj-ui-elements', JSON.stringify(selections)); } catch { /* Optional storage. */ } }, [direction, mode, family, selections]);
  useEffect(() => { function key(e: KeyboardEvent) { if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'k') { e.preventDefault(); setCommands(o => !o); } } document.addEventListener('keydown', key); return () => document.removeEventListener('keydown', key); }, []);
  const selectedFamily = catalog.families.find(f => f.id === family)!;
  const selected = entries.find(c => c.family === family && c.id === selections[family])!;
  const needle = query.trim().toLowerCase();
  const visibleEntries = needle ? entries.filter(c => `${c.label} ${c.usage} ${c.familyLabel}`.toLowerCase().includes(needle)) : entries.filter(c => c.family === family);
  function navigate(id: string) { setFamily(id); setView('catalog'); setQuery(''); }
  function select(id: string) { const entry = entries.find(c => c.id === id)!; setFamily(entry.family); setSelections(s => ({ ...s, [entry.family]: id })); setView('catalog'); }
  return <UI.ThemeProvider direction={direction} mode={mode} className="showcase">
    <a className="skip-link" href="#catalog-main">Skip to component examples</a>
    <UI.Workspace contentOwnsHeader mainId="catalog-main" storageKey="sj-catalog-shell" sidebarTitle={view === 'home' ? 'Home' : view === 'settings' ? 'Settings' : view === 'compare' ? 'Appearance' : needle ? 'Search results' : selectedFamily.label}
      header={<UI.Button variant="quiet" className="app-brand home-link" aria-label="Home" title="Home" onClick={() => { setView('home'); setQuery(''); }}><SiteWordmark /></UI.Button>}
      rail={<UI.SideNavigation collapsed label="Component families" value={view === 'catalog' ? family : ''} onChange={navigate} items={catalog.families.filter(f => f.id !== 'foundations').map(f => ({ id: f.id, label: f.label, icon: f.icon as UI.IconName }))} />}
      navigationFooter={<UI.IconButton icon="settings" label="Settings" aria-pressed={view === 'settings'} onClick={() => setView('settings')} />}
      sidebar={view === 'home' ? <UI.SideNavigation label="Home pages" value="home" onChange={id => id === 'home' ? setView('home') : navigate(id)} items={[{ id: 'home', label: 'Overview' }, { id: 'foundations', label: 'Foundations' }]} /> : view === 'settings' ? <UI.SideNavigation label="Settings categories" value={settingsPage} onChange={setSettingsPage} items={[{ id: 'appearance', label: 'Appearance' }, { id: 'about', label: 'About' }]} /> : view === 'compare' ? <UI.SideNavigation label="Appearance pages" value="compare" onChange={() => setView('compare')} items={[{ id: 'compare', label: 'Compare directions' }]} /> : <><UI.TextInput label="Search components" type="search" value={query} onChange={e => setQuery(e.target.value)} placeholder="Search the library" /><UI.SideNavigation label="Components" value={selected.id} onChange={select} items={visibleEntries.map(c => ({ id: c.id, label: needle ? `${c.familyLabel}: ${c.label}` : c.label }))} />{needle && <p className="search-count" role="status">{visibleEntries.length} matching components</p>}</>}
      toolbar={<strong className="current-element">{view === 'settings' ? 'Settings' : view === 'compare' ? 'Compare directions' : selected.label}</strong>}>
      {({ sidebarCollapsed }) => <>
      <section className="component-page" hidden={view !== 'home'} aria-label="Home"><UI.SettingsPage title="Stack Junkie UI"><p>Explore the shared components, layouts and foundations.</p><UI.Stack><UI.Button onClick={() => navigate('foundations')}>Foundations</UI.Button><p className="sj-muted">Colors, typography, spacing, icons and motion.</p></UI.Stack><div className="home-sections">{catalog.families.filter(f => f.id !== 'foundations').map(f => <UI.Button key={f.id} onClick={() => navigate(f.id)}><UI.Icon name={f.icon as UI.IconName} />{f.label}</UI.Button>)}</div></UI.SettingsPage></section>
      {entries.map(entry => <section className="component-page" key={entry.id} hidden={view !== 'catalog' || selected.id !== entry.id} aria-label={entry.label}><ComponentPage entry={entry} sidebarCollapsed={sidebarCollapsed} /></section>)}
      <section className="comparison-page" hidden={view !== 'compare'} aria-label="Direction comparison"><p className="comparison-note">Shared inputs across all three previews.</p><div className="comparison-grid">{(['clean', 'warm', 'technical'] as UI.Direction[]).map(d => <UI.ThemeProvider key={d} direction={d} mode={mode} className="comparison-column"><header><h2>{UI.directions[d].label}</h2></header><UtilityFixture state={comparison} onChange={setComparison} compact /></UI.ThemeProvider>)}</div></section>
      <section className="component-page" hidden={view !== 'settings' || settingsPage !== 'appearance'}>
        <UI.SettingsPage title="Appearance">
          <UI.SettingsRow label="Theme" description="Choose the light or dark appearance."><UI.Select label="Color theme" value={mode} onChange={e => setMode(e.target.value as UI.ThemeMode)}><option value="dark">Dark</option><option value="light">Light</option></UI.Select></UI.SettingsRow>
          <div className="showcase-only-note">Showcase only · these comparison controls are excluded from the imported UI library.</div>
          <UI.SettingsRow label="Design direction" description="Compare the existing style candidates."><UI.Select label="Design direction" value={direction} onChange={e => setDirection(e.target.value as UI.Direction)}>{Object.entries(UI.directions).map(([id, d]) => <option value={id} key={id}>{d.label}</option>)}</UI.Select></UI.SettingsRow>
          <UI.Button onClick={() => setView('compare')}>Compare directions</UI.Button>
          <UI.Disclosure label="Navigation and keyboard"><p>Drag a sidebar divider to resize it. Arrow keys adjust a focused divider; Home and End use its available range. Control K opens command search.</p></UI.Disclosure>
        </UI.SettingsPage>
      </section>
      <section className="component-page" hidden={view !== 'settings' || settingsPage !== 'about'}>
        <UI.AboutPage name="Stack Junkie UI" description="A living collection of reusable interface components, layouts and interactions." version={UI.systemVersion} logo={<UI.BrandMark size={44} />}>
          <UI.Disclosure label="About this showcase"><p>Web and native examples use the shared libraries. Your choices and example state remain while moving between views.</p></UI.Disclosure>
        </UI.AboutPage>
      </section>
      </>}
    </UI.Workspace>
    <UI.CommandPalette open={commands} onClose={() => setCommands(false)} commands={[...catalog.families.map(f => ({ id: f.id, label: `Open ${f.label.toLowerCase()}`, run: () => navigate(f.id) })), { id: 'compare', label: 'Compare directions', run: () => setView('compare') }, { id: 'settings', label: 'Open Settings', run: () => setView('settings') }, { id: 'theme', label: `Use ${mode === 'dark' ? 'light' : 'dark'} theme`, run: () => setMode(mode === 'dark' ? 'light' : 'dark') }]} />

  </UI.ThemeProvider>;
}
