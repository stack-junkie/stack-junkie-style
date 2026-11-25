import { useState } from 'react';
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
function MultiTool() { const [tab, setTab] = useState('filter'); return <UI.Tabs label="Utility tools" value={tab} onChange={setTab} tabs={[{ id: 'filter', label: 'Filter samples', content: <FocusedUtility /> }, { id: 'notes', label: 'Notes', content: <UI.TextArea label="Working notes" placeholder="Notes remain when switching tools." /> }, { id: 'settings', label: 'Settings', content: <UI.Stack><UI.Select label="Output format" defaultValue="csv"><option value="csv">CSV</option><option value="text">Text</option></UI.Select><UI.Checkbox label="Include units" defaultChecked /></UI.Stack> }]} />; }
function Workbench() {
  const [section, setSection] = useState('editor');
  const [document, setDocument] = useState('layout');
  const [views, setViews] = useState<Record<string, string>>({ layout: 'preview', notes: 'preview' });
  const [collapsed, setCollapsed] = useState(false);
  return <div className="demo-workbench">
    <div className="demo-workbench-rail"><UI.SideNavigation collapsed label="Workspace section" value={section} onChange={setSection} items={[{ id: 'editor', label: 'Editor', icon: 'code' }, { id: 'files', label: 'Files', icon: 'folder' }]} /></div>
    <div className="demo-workbench-main">
      <div hidden={section !== 'editor'}>
        <UI.Toolbar label="Workbench navigation"><UI.Button variant="quiet" aria-expanded={!collapsed} onClick={() => setCollapsed(!collapsed)}>{collapsed ? 'Show documents' : 'Hide documents'}</UI.Button></UI.Toolbar>
        <div className={`demo-document-layout ${collapsed ? 'is-collapsed' : ''}`}>
          <aside hidden={collapsed}><UI.List label="Open documents" selected={document} onSelect={setDocument} items={[{ id: 'layout', title: 'Layout study' }, { id: 'notes', title: 'Design notes' }]} /></aside>
          <div className="demo-document-content">
            {['layout', 'notes'].map(id => <section key={id} hidden={document !== id} aria-label={id === 'layout' ? 'Layout study' : 'Design notes'}>
              <UI.Tabs label="Document view" value={views[id]} onChange={value => setViews(current => ({ ...current, [id]: value }))} tabs={[
                { id: 'preview', label: 'Preview', content: id === 'layout' ? <ContentDemo id="canvas" /> : <UI.TextArea label="Design notes" defaultValue="The selected object, zoom and inspector edits survive a change of document or view." /> },
                { id: 'source', label: 'Source', content: <UI.CodeBlock language="Document structure" code={id === 'layout' ? 'Layout study\n  Title block\n  Content block' : 'Design notes\n  Preserve selection\n  Keep errors visible\n  Restore panel sizes'} /> },
              ]} />
            </section>)}
          </div>
        </div>
      </div>
      <div hidden={section !== 'files'}><CollectionDemo id="table" /></div>
    </div>
  </div>;
}
function Publication() { const [section, setSection] = useState('principles'); return <div className="demo-publication"><UI.SideNavigation value={section} onChange={setSection} label="Reference sections" items={[{ id: 'principles', label: 'Principles' }, { id: 'keyboard', label: 'Keyboard' }]} /><div><div hidden={section !== 'principles'}><ContentDemo id="prose" /></div><div hidden={section !== 'keyboard'}><UI.Prose><h2>Keyboard reference</h2><p>Use Tab to move between controls, arrow keys within tabs, and Escape to dismiss a dialog or help preview.</p><p>Split-pane dividers accept left and right arrow keys. Native fields retain their familiar keyboard behavior.</p></UI.Prose></div></div></div>; }
function GuidedWorkflow() { const [step, setStep] = useState(1); const [name, setName] = useState(''); const [format, setFormat] = useState('csv'); const [error, setError] = useState(''); const [done, setDone] = useState(false); return <UI.Stack><UI.Toolbar label="Workflow steps"><UI.Badge tone={step === 1 ? 'success' : 'neutral'}>1. Name</UI.Badge><UI.Icon name="chevron-right" /><UI.Badge tone={step === 2 ? 'success' : 'neutral'}>2. Format</UI.Badge><UI.Icon name="chevron-right" /><UI.Badge tone={step === 3 ? 'success' : 'neutral'}>3. Review</UI.Badge></UI.Toolbar><div hidden={step !== 1}><UI.TextInput label="Document name" value={name} onChange={e => setName(e.target.value)} error={error || undefined} /></div><div hidden={step !== 2}><UI.RadioGroup label="Export format" value={format} onChange={setFormat} options={[{ value: 'csv', label: 'CSV table' }, { value: 'txt', label: 'Plain text' }]} /></div><div hidden={step !== 3}><UI.Panel title="Review"><UI.Stack><p><strong>{name || 'Unnamed'}</strong></p><p>Format: {format.toUpperCase()}</p></UI.Stack></UI.Panel></div><UI.Toolbar label="Workflow actions"><UI.Button disabled={step === 1} onClick={() => { setStep(step - 1); setDone(false); }}>Back</UI.Button><UI.Button variant="primary" onClick={() => { if (step === 1 && !name.trim()) { setError('Enter a document name.'); return; } setError(''); if (step < 3) setStep(step + 1); else setDone(true); }}>{step === 3 ? 'Confirm example' : 'Continue'}</UI.Button></UI.Toolbar>{done && <UI.Alert tone="success" title="Example confirmed">{name}.{format} is ready in this local workflow specimen.</UI.Alert>}</UI.Stack>; }
function Monitoring() { return <UI.Stack><FeedbackDemo id="progress" /><UI.Chart label="Fixed timing fixture" unit="ms" data={chartData} /><UI.Disclosure label="Sample log"><UI.CodeBlock language="Local fixture" code={'sample A   12 ms\nsample B   19 ms\nsample C   14 ms\nsample D   25 ms\nsample E   18 ms'} /></UI.Disclosure></UI.Stack>; }
export function LayoutDemo() { const [selected, setSelected] = useState('focused-utility'); const components: Record<string, React.ReactNode> = { 'focused-utility': <FocusedUtility />, 'multi-tool': <MultiTool />, embedded: <FocusedUtility embedded />, data: <CollectionDemo id="table" />, workbench: <Workbench />, publication: <Publication />, guided: <GuidedWorkflow />, monitoring: <Monitoring /> }; return <UI.Stack gap={24}><UI.Select label="Product layout" value={selected} onChange={e => setSelected(e.target.value)}>{catalog.layouts.map(l => <option value={l.id} key={l.id}>{l.label}</option>)}</UI.Select>{catalog.layouts.map(layout => <section hidden={selected !== layout.id} key={layout.id} aria-label={layout.label}>{components[layout.id]}</section>)}</UI.Stack>; }
