import { useState, type ReactNode } from 'react';
import * as UI from '../library';
import { chartData, CollectionDemo, ContentDemo, FeedbackDemo } from './demos';
import catalog from '../../../shared/catalog.json';

export type UtilityState = { threshold: string; includeLabels: boolean; results: null | number[]; error: string };
export const initialUtility: UtilityState = { threshold: '12', includeLabels: true, results: null, error: '' };
export function UtilityFixture({ state, onChange, compact = false }: { state: UtilityState; onChange: (state: UtilityState) => void; compact?: boolean }) {
  function run() { const value = Number(state.threshold); if (!state.threshold.trim() || !Number.isFinite(value) || value < 0) { onChange({ ...state, error: 'Enter a threshold of zero or greater.', results: null }); return; } onChange({ ...state, error: '', results: chartData.filter(d => d.value >= value).map(d => d.value) }); }
  return <UI.Stack gap={compact ? 12 : 16}><UI.Toolbar label="Sample filter context"><UI.Icon name="file" /><strong>Filter sample measurements</strong><UI.HelpPopover label="About these measurements">A fixed local fixture with five values: 12, 19, 14, 25 and 18 ms. It exercises the same controls in each direction.</UI.HelpPopover></UI.Toolbar><UI.NumberInput label="Minimum duration (ms)" min={0} step={1} value={state.threshold} onChange={e => onChange({ ...state, threshold: e.target.value })} error={state.error || undefined} /><UI.Disclosure label="Output options"><UI.Checkbox label="Include value labels" checked={state.includeLabels} onChange={e => onChange({ ...state, includeLabels: e.target.checked })} /></UI.Disclosure><UI.Button variant="primary" onClick={run}>Apply filter</UI.Button>{state.results !== null && <div role="status"><strong>{state.results.length} of 5 samples match</strong>{state.results.length ? <UI.List label="Matching samples" items={state.results.map((value, i) => ({ id: `${i}`, title: state.includeLabels ? `Duration: ${value} ms` : `${value} ms` }))} /> : <p className="sj-muted">Try a lower duration to include more samples.</p>}</div>}</UI.Stack>;
}

function FocusedUtility({ embedded = false }: { embedded?: boolean }) { const [state, setState] = useState(initialUtility); return <div className={embedded ? 'demo-embedded' : 'demo-utility'}><UtilityFixture state={state} onChange={setState} compact={embedded} /></div>; }
function MultiTool() { const [tab, setTab] = useState('filter'); return <UI.Tabs label="Utility tools" value={tab} onChange={setTab} tabs={[{ id: 'filter', label: 'Filter samples', content: <FocusedUtility /> }, { id: 'notes', label: 'Notes', content: <UI.TextArea label="Working notes" defaultValue="Notes remain when switching tools." /> }, { id: 'settings', label: 'Output', content: <UI.Stack><UI.Select label="Output format" defaultValue="csv"><option value="csv">CSV</option><option value="text">Text</option></UI.Select><UI.Checkbox label="Include units" defaultChecked /></UI.Stack> }]} />; }

// Reference organization for apps with many pages, modeled on VS Code:
// rail = areas, sidebar tree = every page (depth lives here), one tab row = open documents, bottom panel = its own separate tab row.
const documents: Record<string, { label: string; group: string; source: string }> = {
  spacing: { label: 'Spacing study', group: 'documents', source: 'Spacing study\n  4 8 12 16 24 32 48\n  Content padding 20' },
  states: { label: 'Control states', group: 'documents', source: 'Control states\n  default hover focus\n  disabled invalid read-only' },
  notes: { label: 'Layout notes', group: 'documents', source: 'Layout notes\n  One tab row per view\n  Depth goes in the sidebar tree' },
  keyboard: { label: 'Keyboard map', group: 'references', source: 'Keyboard map\n  Ctrl+K  commands\n  Ctrl+W  close tab' },
  colors: { label: 'Color roles', group: 'references', source: 'Color roles\n  canvas surface raised\n  accent focus danger' },
};
const documentGroups = [{ id: 'documents', label: 'Documents' }, { id: 'references', label: 'References' }];
function Workbench() {
  const [area, setArea] = useState('explorer'); const [query, setQuery] = useState('');
  const [open, setOpen] = useState(['spacing', 'notes']); const [active, setActive] = useState('spacing');
  const [views, setViews] = useState<Record<string, 'preview' | 'source'>>({});
  const [panel, setPanel] = useState(true);
  function openDocument(id: string) { setOpen(o => o.includes(id) ? o : [...o, id]); setActive(id); }
  function close(id: string) { const index = open.indexOf(id); const next = open.filter(d => d !== id); setOpen(next); if (active === id && next.length) setActive(next[Math.min(index, next.length - 1)]); }
  const matches = Object.entries(documents).filter(([, d]) => `${d.label} ${d.source}`.toLowerCase().includes(query.trim().toLowerCase()));
  return <div className="wb">
    <div className="wb-rail"><UI.SideNavigation collapsed label="Workbench areas" value={area} onChange={setArea} items={[{ id: 'explorer', label: 'Explorer', icon: 'folder' }, { id: 'search', label: 'Search', icon: 'search' }]} /></div>
    <aside className="wb-sidebar" aria-label={area === 'explorer' ? 'Explorer' : 'Search'}>
      <header className="wb-bar"><strong>{area === 'explorer' ? 'Explorer' : 'Search'}</strong></header>
      <div className="wb-sidebar-body" hidden={area !== 'explorer'}><UI.NavTree label="Workbench pages" value={active} onChange={openDocument} groups={documentGroups.map(g => ({ id: g.id, label: g.label, items: Object.entries(documents).filter(([, d]) => d.group === g.id).map(([id, d]) => ({ id, label: d.label, icon: 'file' as const })) }))} /></div>
      <div className="wb-sidebar-body" hidden={area !== 'search'}><UI.TextInput label="Search documents" type="search" value={query} onChange={e => setQuery(e.target.value)} /><UI.SideNavigation label="Search results" value={active} onChange={openDocument} items={matches.map(([id, d]) => ({ id, label: d.label }))} />{matches.length === 0 && <p className="sj-muted">No documents match.</p>}</div>
    </aside>
    <div className="wb-main">
      <div className="wb-editor">{open.length ? <UI.Tabs label="Open documents" value={active} onChange={setActive} onCloseTab={close} tabs={open.map(id => {
        const view = views[id] ?? 'preview';
        return { id, label: documents[id].label, content: <div className="wb-document"><UI.Toolbar label={`${documents[id].label} view`}><UI.ToggleButton pressed={view === 'preview'} onPressedChange={() => setViews(v => ({ ...v, [id]: 'preview' }))}>Preview</UI.ToggleButton><UI.ToggleButton pressed={view === 'source'} onPressedChange={() => setViews(v => ({ ...v, [id]: 'source' }))}>Source</UI.ToggleButton><span className="wb-spacer" /><UI.Button variant="quiet" aria-expanded={panel} onClick={() => setPanel(!panel)}>{panel ? 'Hide panel' : 'Show panel'}</UI.Button></UI.Toolbar>{view === 'preview' ? <UI.Prose><h2>{documents[id].label}</h2>{documents[id].source.split('\n').slice(1).map((line, i) => <p key={i}>{line.trim()}</p>)}</UI.Prose> : <UI.CodeBlock language="Outline" code={documents[id].source} />}</div> };
      })} /> : <UI.EmptyState title="No open documents" description="Open a page from the Explorer tree." />}</div>
      {panel && <UI.BottomPanel initialHeight={140} onClose={() => setPanel(false)} tabs={[
        { id: 'output', label: 'Output', content: <UI.CodeBlock language="Local fixture" code={'Opened ' + open.map(id => documents[id].label).join(', ')} /> },
        { id: 'problems', label: 'Problems', content: <p className="sj-muted">No problems in the open documents.</p> },
      ]} />}
    </div>
  </div>;
}

function Publication() { const [section, setSection] = useState('principles'); return <div className="demo-publication"><UI.SideNavigation value={section} onChange={setSection} label="Reference sections" items={[{ id: 'principles', label: 'Principles' }, { id: 'keyboard', label: 'Keyboard' }]} /><div><div hidden={section !== 'principles'}><ContentDemo id="prose" /></div><div hidden={section !== 'keyboard'}><UI.Prose><h2>Keyboard reference</h2><p>Use Tab to move between controls, arrow keys within tabs, and Escape to dismiss a dialog or help preview.</p><p>Split-pane dividers accept left and right arrow keys. Native fields retain their familiar keyboard behavior.</p></UI.Prose></div></div></div>; }
function GuidedWorkflow() { const [step, setStep] = useState(1); const [name, setName] = useState(''); const [format, setFormat] = useState('csv'); const [error, setError] = useState(''); const [done, setDone] = useState(false); return <UI.Stack><UI.Toolbar label="Workflow steps"><UI.Badge tone={step === 1 ? 'success' : 'neutral'}>1. Name</UI.Badge><UI.Icon name="chevron-right" /><UI.Badge tone={step === 2 ? 'success' : 'neutral'}>2. Format</UI.Badge><UI.Icon name="chevron-right" /><UI.Badge tone={step === 3 ? 'success' : 'neutral'}>3. Review</UI.Badge></UI.Toolbar><div hidden={step !== 1}><UI.TextInput label="Document name" value={name} onChange={e => setName(e.target.value)} error={error || undefined} /></div><div hidden={step !== 2}><UI.RadioGroup label="Export format" value={format} onChange={setFormat} options={[{ value: 'csv', label: 'CSV table' }, { value: 'txt', label: 'Plain text' }]} /></div><div hidden={step !== 3}><UI.Panel title="Review"><UI.Stack><p><strong>{name || 'Unnamed'}</strong></p><p>Format: {format.toUpperCase()}</p></UI.Stack></UI.Panel></div><UI.Toolbar label="Workflow actions"><UI.Button disabled={step === 1} onClick={() => { setStep(step - 1); setDone(false); }}>Back</UI.Button><UI.Button variant="primary" onClick={() => { if (step === 1 && !name.trim()) { setError('Enter a document name.'); return; } setError(''); if (step < 3) setStep(step + 1); else setDone(true); }}>{step === 3 ? 'Confirm example' : 'Continue'}</UI.Button></UI.Toolbar>{done && <UI.Alert tone="success" title="Example confirmed">{name}.{format} is ready in this local workflow specimen.</UI.Alert>}</UI.Stack>; }
function Monitoring() { return <UI.Stack><FeedbackDemo id="progress" /><UI.Chart label="Fixed timing fixture" unit="ms" data={chartData} /><UI.Disclosure label="Sample log"><UI.CodeBlock language="Local fixture" code={'sample A   12 ms\nsample B   19 ms\nsample C   14 ms\nsample D   25 ms\nsample E   18 ms'} /></UI.Disclosure></UI.Stack>; }

/** The App frame chosen in Settings: a preset, the choices changed from it, and sidebar resizing and collapsing. */
export type FrameSettings = { preset: UI.PresetId; choices: Partial<UI.PresetChoices>; config: UI.ShellConfig };
// The exact app frame a new project gets from Export: the imported AppShell with the preset and choices chosen in Settings.
export function StarterApp({ frame }: { frame: FrameSettings }) {
  // The Website preset uses its own layout instead of the App frame; only its theme and density choices apply.
  if (frame.preset === 'website') { const { theme, density } = UI.resolvePreset('website', frame.choices); return <WebsiteSpecimen key={JSON.stringify(frame)} theme={theme} density={density} />; }
  return <UI.AppShell key={JSON.stringify(frame)} preset={frame.preset} choices={frame.choices} config={frame.config} shortcuts={false} name="Sample app" description="A new project started from the exported configuration." version="1.0.0" logo={<UI.BrandMark size={20} />}
    sections={[
      { id: 'library', label: 'Library', icon: 'folder', pages: [{ id: 'documents', label: 'Documents', content: <CollectionDemo id="list" /> }, { id: 'table', label: 'Table', content: <CollectionDemo id="table" /> }] },
      { id: 'tools', label: 'Tools', icon: 'code', pages: [{ id: 'filter', label: 'Filter samples', content: <FocusedUtility /> }, { id: 'notes', label: 'Notes', content: <UI.TextArea label="Working notes" defaultValue="Your notes stay while you move between pages." /> }] },
    ]}
    settings={[{ id: 'general', label: 'General', keywords: 'startup', content: <UI.SettingsSection label="General"><UI.SettingsRow label="Open last page on start" description="Return to where you left off."><UI.Switch label="Open last page on start" defaultChecked /></UI.SettingsRow></UI.SettingsSection> }]} />;
}

// The Website layout with a short sample site. The text describes the layout itself.
const websitePages: UI.WebsitePage[] = [
  { id: 'home', label: 'Home', content: <UI.Prose><h1>Website layout</h1><p>A header with the logo and site navigation, one reading column and a footer. The navigation marks the current page, and the toggle beside it switches dark and light.</p><p>The column stops at a comfortable line length, so long text stays easy to follow at any window width.</p><h2>At phone width</h2><p>The navigation folds into a menu button, the gutters shrink to 16 pixels and nothing scrolls sideways.</p></UI.Prose> },
  { id: 'reading', label: 'Reading', content: <UI.Prose><h1>Reading column</h1><p>Body text uses the reading size and line height. Headings, lists and quotes come from Prose.</p><ul><li>One centered column</li><li>Comfortable density</li><li>The theme follows the device until a visitor picks one</li></ul><blockquote>A quote sits inside the same column.</blockquote></UI.Prose> },
  { id: 'about', label: 'About', content: <UI.Prose><h1>About this specimen</h1><p>A sample site inside a frame. Its links switch pages in place; a real site gives each page an address.</p></UI.Prose> },
];
function WebsiteSpecimen({ theme, density }: { theme: UI.ThemeChoice; density?: UI.Density }) {
  return <UI.WebsiteLayout name="Sample site" logo={<UI.BrandMark size={20} />} pages={websitePages} theme={theme} density={density} storageKey="" mainId="website-specimen-main" />;
}

export const layoutPages = [
  ...catalog.layouts.map(l => ({ id: l.id, label: l.label, description: l.description })),
  { id: 'website', label: 'Website', description: 'The Website preset: a header with logo and navigation, a reading column, a footer and a theme toggle, instead of the App frame.' },
  { id: 'starter', label: 'Starter app', description: 'What Export gives a new project: the imported app frame with your chosen layout settings.' },
];
export function LayoutPage({ id, frame }: { id: string; frame: FrameSettings }) {
  const page = layoutPages.find(l => l.id === id)!;
  const content: Record<string, ReactNode> = { 'focused-utility': <FocusedUtility />, 'multi-tool': <MultiTool />, embedded: <FocusedUtility embedded />, data: <CollectionDemo id="table" />, workbench: <Workbench />, publication: <Publication />, guided: <GuidedWorkflow />, monitoring: <Monitoring />, website: <WebsiteSpecimen theme={frame.config.mode} />, starter: <StarterApp frame={frame} /> };
  const full = id === 'workbench' || id === 'starter' || id === 'website';
  return <div className="layout-page"><UI.TabBoundary><section className={`layout-frame ${full ? 'layout-frame--app' : ''}`} aria-label={page.label}>
    <header className="layout-frame-bar"><span className="layout-frame-name">{page.label}</span><UI.HelpPopover label={`About ${page.label.toLowerCase()}`}>{page.description}</UI.HelpPopover></header>
    <div className="layout-frame-body">{content[id]}</div>
  </section></UI.TabBoundary></div>;
}
