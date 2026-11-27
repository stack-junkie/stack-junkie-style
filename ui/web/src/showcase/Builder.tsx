import { useEffect, useLayoutEffect, useRef, useState, type ReactNode } from 'react';
import * as UI from '../library';
import { command, logoFileName, starter, starterKinds, starterManifest, type StarterLogo } from './starter';
import { zip } from './zip';
import { CustomPresetSettings } from './CustomPresets';

// Showcase only: the Builder. Pick a Preset, switch its choices from one toolbar row, and watch a blank App frame (or Website
// layout) change. The frame is drawn from the starter's own structure (starters.json), so what Export writes is what it shows.

type Platform = typeof starterKinds[number]['platform'];
export type BuilderLogo = StarterLogo & { url: string };
const optionLabel = (value: string | boolean) => typeof value === 'boolean' ? (value ? 'On' : 'Off') : value.charAt(0).toUpperCase() + value.slice(1);

// The element each choice changes, in a frame. The "?" help outlines it in the big frame and in its mini preview.
const targets: Partial<Record<UI.PresetChoice | 'logo', string>> = {
  rail: '.sj-workspace-rail', sidebar: '.sj-workspace-sidebar', menuBar: '.sj-menubar', statusBar: '.sj-status-bar', bottomPanel: '.sj-bottom-panel',
  documentTabs: '.sj-shell-tabs > .sj-tabs > .sj-tabs-heading', commandPalette: '.builder-palette', theme: '.sj-theme', density: '.sj-workspace-body, .sj-website-body',
  dashboardHome: '.sj-workspace-content', siteNavigation: '.sj-website-nav', siteFooter: '.sj-website-footer', headerSearch: '.sj-website-actions [aria-label="Search"]',
  headerButton: '.sj-website-main-action', logo: '.sj-shell-home, .sj-website-brand',
};
// What each choice looks like when it is on, for its mini preview.
function shownChoices(choices: UI.PresetChoices, key: UI.PresetChoice): UI.PresetChoices {
  const on: Partial<UI.PresetChoices> = { rail: true, menuBar: true, statusBar: true, bottomPanel: true, documentTabs: true, commandPalette: true, siteFooter: true, headerSearch: true, headerButton: true, dashboardHome: true };
  const next = { ...choices };
  if (key === 'sidebar' && choices.sidebar === 'off') next.sidebar = 'tree';
  else if (key in on) (next as Record<string, unknown>)[key] = on[key];
  if (key === 'documentTabs' && next.sidebar === 'off') next.sidebar = 'tree';
  return next;
}

// The starter's own App frame, or its Website, drawn from starters.json with the library's components and Stubs.
export function StarterFrame({ preset, choices, logo }: { preset: UI.PresetId; choices: UI.PresetChoices; logo?: BuilderLogo }) {
  const logoNode = logo ? <img src={logo.url} alt="" width={20} height={20} /> : <UI.BrandMark size={20} />;
  if (preset === 'website') {
    const articles: UI.ArticleSummary[] = starterManifest.website.articles.map(a => ({ id: a.id, title: <UI.Stub>{a.title}</UI.Stub>, date: <UI.Stub>{a.date}</UI.Stub>, summary: <UI.Stub>{a.summary}</UI.Stub> }));
    const pages: UI.WebsitePage[] = [
      { id: 'home', label: 'Home', content: <UI.WebsiteHome featured={articles[0]} recent={articles.slice(1)} /> },
      { id: 'articles', label: 'Articles', content: <UI.Prose><h1><UI.Stub>All articles</UI.Stub></h1><UI.WebsiteArticleList articles={articles} /></UI.Prose> },
      { id: 'about', label: 'About', content: <UI.Prose><h1><UI.Stub>About</UI.Stub></h1><UI.Stub block>Who runs this site and what it covers.</UI.Stub></UI.Prose> },
      ...starterManifest.website.articles.map((a, i) => ({ id: a.id, label: a.title, group: 'Articles', topBar: false, content: <UI.WebsiteArticle title={articles[i].title} date={articles[i].date}><UI.Stub block>The article.</UI.Stub></UI.WebsiteArticle> })),
    ];
    return <UI.WebsiteLayout name="My site" logo={logoNode} pages={pages} theme={choices.theme} density={choices.density} storageKey="" mainId="builder-site-main"
      navigation={choices.siteNavigation} footer={choices.siteFooter} onSearch={choices.headerSearch ? () => {} : undefined} mainAction={choices.headerButton ? { label: 'Subscribe' } : undefined} style={{ minHeight: '100%' }} />;
  }
  const sections: UI.ShellSection[] = starterManifest.structure.areas.map(area => ({ id: area.id, label: area.label, icon: area.icon as UI.IconName,
    pages: area.pages.map(page => ({ id: page.id, label: page.label, content: <UI.Stub block>{page.content}</UI.Stub> })) }));
  return <UI.AppShell name="My app" description="What this app does." version="0.1.0" logo={logoNode} preset={preset} choices={choices} sections={sections} shortcuts={false} style={{ height: '100%' }}
    panels={starterManifest.structure.panels.map(p => ({ id: p.id, label: p.label, content: <UI.Stub block>{p.content}</UI.Stub> }))} />;
}

// Outlines the element a choice changes, inside the frame it is placed in. It finds that frame through its own anchor,
// because a parent's ref is attached only after its children's layout effects run.
function Outline({ selector, inset = false }: { selector?: string; inset?: boolean }) {
  const anchor = useRef<HTMLSpanElement>(null);
  const [box, setBox] = useState<{ left: number; top: number; width: number; height: number } | null>(null);
  useLayoutEffect(() => {
    const root = anchor.current?.parentElement; if (!root || !selector) { setBox(null); return; }
    const update = () => {
      const target = root.querySelector(selector); if (!target) { setBox(null); return; }
      const r = target.getBoundingClientRect(), f = root.getBoundingClientRect();
      const next = { left: r.left - f.left, top: r.top - f.top, width: r.width, height: r.height };
      setBox(b => b && b.left === next.left && b.top === next.top && b.width === next.width && b.height === next.height ? b : next);
    };
    // The frame settles after it mounts (open tabs, theme), so measure again on every change inside it and on the next frame.
    update(); const frameId = requestAnimationFrame(update);
    const resized = new ResizeObserver(update); resized.observe(root);
    const changed = new MutationObserver(update); changed.observe(root, { subtree: true, childList: true, attributes: true, attributeFilter: ['class', 'hidden'] });
    return () => { cancelAnimationFrame(frameId); resized.disconnect(); changed.disconnect(); };
  }, [selector]);
  return <><span ref={anchor} hidden />{box && <div className={`builder-outline${inset ? ' builder-outline--inset' : ''}`} style={box} aria-hidden="true" />}</>;
}

// A choice's "?": hovering or focusing it shows what the choice does with a live mini preview, and outlines the element in the big frame.
function ChoiceHelp({ id, label, description, preview, onHighlight }: { id: string; label: string; description: string; preview: ReactNode; onHighlight: (id: string | null) => void }) {
  const [open, setOpen] = useState(false); const [place, setPlace] = useState({ left: 0, top: 0 });
  const button = useRef<HTMLButtonElement>(null); const timer = useRef<ReturnType<typeof setTimeout> | undefined>(undefined);
  function show() {
    clearTimeout(timer.current); onHighlight(id);
    const r = button.current?.getBoundingClientRect(); if (r) setPlace({ left: Math.max(8, Math.min(r.left - 160, window.innerWidth - 360)), top: r.bottom + 8 });
    setOpen(true);
  }
  function hide() { timer.current = setTimeout(() => { setOpen(false); onHighlight(null); }, 120); }
  useEffect(() => () => clearTimeout(timer.current), []);
  return <span className="builder-help" onPointerEnter={show} onPointerLeave={hide}>
    <button ref={button} type="button" className="builder-help-trigger" aria-label={`What ${label} does`} aria-expanded={open} onFocus={show} onBlur={hide}
      onKeyDown={e => { if (e.key === 'Escape') { setOpen(false); onHighlight(null); } }}>?</button>
    {open && <span className="builder-help-card" role="region" aria-label={label} style={place} onPointerEnter={() => clearTimeout(timer.current)}>
      <strong>{label}</strong><span>{description}</span><span className="builder-mini">{preview}</span>
    </span>}
  </span>;
}

function MiniPreview({ preset, choices, logo, target }: { preset: UI.PresetId; choices: UI.PresetChoices; logo?: BuilderLogo; target?: string }) {
  return <span className="builder-mini-frame">
    <span className="builder-mini-scale"><StarterFrame preset={preset} choices={choices} logo={logo} />{target === targets.commandPalette && <PaletteMock />}</span>
    <Outline selector={target} inset />
  </span>;
}
// The command palette opens over the app with Control K; its preview shows it open.
function PaletteMock() {
  return <span className="builder-palette"><span className="builder-palette-search">Find a command</span>{['First command', 'Second command', 'Third command'].map(c => <span key={c}>{c}</span>)}</span>;
}

export type BuilderProps = {
  preset: UI.PresetId; overrides: Partial<UI.PresetChoices>; mode: UI.ThemeMode;
  onPreset: (id: UI.PresetId) => void; onChoice: <K extends UI.PresetChoice>(key: K, value: UI.PresetChoices[K]) => void;
  customs: UI.CustomPreset[]; onCustomsChange: (list: UI.CustomPreset[]) => void; selectedCustom?: UI.CustomPreset;
  onSelectCustom: (preset: UI.CustomPreset | undefined) => void; onMarkCustom: (name: string | undefined) => void;
  logo?: BuilderLogo; onLogo: (logo: BuilderLogo | undefined) => void; onBack: () => void;
};

export function Builder({ preset, overrides, mode, onPreset, onChoice, customs, onCustomsChange, selectedCustom, onSelectCustom, onMarkCustom, logo, onLogo, onBack }: BuilderProps) {
  const choices = UI.resolvePreset(preset, overrides);
  const info = UI.presets.find(p => p.id === preset)!;
  const [highlight, setHighlight] = useState<string | null>(null);
  const [exportOpen, setExportOpen] = useState(false); const [presetsOpen, setPresetsOpen] = useState(false);
  const [logoError, setLogoError] = useState('');
  const logoInput = useRef<HTMLInputElement>(null);

  async function chooseLogo(file: File | undefined) {
    if (!file) return;
    const checked = logoFileName(file.name);
    if (checked.error) { setLogoError(checked.error); return; }
    setLogoError('');
    if (logo) URL.revokeObjectURL(logo.url);
    onLogo({ name: file.name, data: new Uint8Array(await file.arrayBuffer()), url: URL.createObjectURL(file) });
  }
  const help = (id: UI.PresetChoice | 'logo', label: string, description: string) => <ChoiceHelp id={id} label={label} description={description} onHighlight={setHighlight}
    preview={<MiniPreview preset={preset} choices={id === 'logo' ? choices : shownChoices(choices, id)} logo={logo} target={targets[id]} />} />;

  return <UI.ThemeProvider mode={mode} className="builder">
    <header className="builder-bar" aria-label="Builder">
      <UI.Button variant="quiet" onClick={onBack}><UI.Icon name="chevron-left" />Back</UI.Button>
      <div className="builder-preset">
        <label className="builder-choice"><span>Preset</span><select aria-label="Preset" value={selectedCustom ? `custom:${selectedCustom.name}` : preset} onChange={e => { const v = e.target.value; if (v.startsWith('custom:')) onSelectCustom(customs.find(c => `custom:${c.name}` === v)); else onPreset(v as UI.PresetId); }}>
          {UI.presets.map(p => <option key={p.id} value={p.id}>{p.label}</option>)}
          {customs.length > 0 && <optgroup label="Custom presets">{customs.map(c => <option key={c.name} value={`custom:${c.name}`}>{c.name}</option>)}</optgroup>}
        </select></label>
        <span className="builder-made-for">{info.madeFor}</span>
      </div>
      <div className="builder-choices" role="group" aria-label="Choices">
        {(Object.entries(UI.presetChoices) as [UI.PresetChoice, UI.PresetChoiceInfo][]).map(([key, choice]) => {
          const applies = UI.choiceApplies(preset, key); const value = choices[key];
          const reason = applies ? choice.description : `${choice.description} Not used by the ${info.label} preset.`;
          return <span key={key} className="builder-choice-wrap" data-applies={applies}>
            {typeof value === 'boolean'
              ? <UI.ToggleButton className="builder-toggle" pressed={value} disabled={!applies} aria-describedby={undefined} onPressedChange={on => onChoice(key, on as never)}>{choice.label}</UI.ToggleButton>
              : <label className="builder-choice"><span>{choice.label}</span><select value={String(value)} disabled={!applies} onChange={e => onChoice(key, e.target.value as never)}>{choice.options.map(o => <option key={String(o)} value={String(o)}>{optionLabel(o)}</option>)}</select></label>}
            {help(key, choice.label, reason)}
          </span>;
        })}
        <span className="builder-choice-wrap" data-applies>
          <input ref={logoInput} type="file" hidden accept=".png,.jpg,.jpeg,.svg,.webp,image/png,image/jpeg,image/svg+xml,image/webp" aria-label="Logo file" onChange={e => { void chooseLogo(e.target.files?.[0]); e.target.value = ''; }} />
          <UI.Button className="builder-toggle" onClick={() => logoInput.current?.click()}>{logo ? <><img src={logo.url} alt="" width={16} height={16} />{logo.name}</> : <><UI.BrandMark size={16} />Logo: SJ mark</>}</UI.Button>
          {logo && <UI.IconButton icon="close" label="Use the SJ mark" variant="quiet" onClick={() => { URL.revokeObjectURL(logo.url); onLogo(undefined); }} />}
          {help('logo', 'Logo', 'The top-left logo slot. The SJ mark until you choose a PNG, JPEG, SVG or WebP file; Export copies the file into the starter. Clicking the logo goes Home.')}
        </span>
      </div>
      <div className="builder-actions">
        <UI.Button onClick={() => setPresetsOpen(true)}>Custom presets</UI.Button>
        <UI.Button variant="primary" onClick={() => setExportOpen(true)}>Export</UI.Button>
      </div>
      {logoError && <UI.Alert tone="danger" title="Logo not used" action={<UI.IconButton icon="close" label="Dismiss" variant="quiet" onClick={() => setLogoError('')} />}>{logoError}</UI.Alert>}
    </header>
    <div className="builder-stage">
      <StarterFrame key={`${preset}-${JSON.stringify(choices)}`} preset={preset} choices={choices} logo={logo} />
      <Outline selector={highlight ? targets[highlight as UI.PresetChoice] : undefined} />
    </div>
    <ExportDialog open={exportOpen} onClose={() => setExportOpen(false)} preset={preset} overrides={overrides} logo={logo} customName={selectedCustom?.name} />
    <UI.Dialog open={presetsOpen} onClose={() => setPresetsOpen(false)} title="Custom presets" size="large">
      <div className="builder-presets"><UI.SettingsSection label="Custom presets">
        <CustomPresetSettings customs={customs} onCustomsChange={onCustomsChange} selected={selectedCustom} onSelect={p => onSelectCustom(p)} onMark={onMarkCustom} base={preset} overrides={UI.presetOverrides(preset, overrides)} />
      </UI.SettingsSection></div>
    </UI.Dialog>
  </UI.ThemeProvider>;
}

// Export gives exactly what the Builder shows: a zip, or one command that writes the same files.
function ExportDialog({ open, onClose, preset, overrides, logo, customName }: { open: boolean; onClose: () => void; preset: UI.PresetId; overrides: Partial<UI.PresetChoices>; logo?: BuilderLogo; customName?: string }) {
  const [platform, setPlatform] = useState<Platform>('web');
  const result = starter(platform, preset, overrides, logo);
  const files = result.files ?? [];
  const label = UI.presets.find(p => p.id === preset)?.label ?? preset;
  const changed = Object.keys(UI.presetOverrides(preset, overrides)).length;
  const cmd = command(platform, preset, overrides, logo?.name);
  function download() {
    if (!files.length) return;
    const url = URL.createObjectURL(zip(files.map(f => ({ path: `my-app/${f.path}`, text: f.text, data: f.data }))));
    const a = document.createElement('a'); a.href = url; a.download = `my-app-${platform}-starter.zip`; a.click();
    setTimeout(() => URL.revokeObjectURL(url), 1000);
  }
  return <UI.Dialog open={open} onClose={onClose} title="Export" size="large" actions={<><UI.Button onClick={onClose}>Close</UI.Button><UI.Button variant="primary" disabled={!files.length} onClick={download}>Download zip</UI.Button></>}>
    <div className="export-body">
      <p className="showcase-only-note">The starter uses {customName ? <>the custom preset {customName}: </> : null}the {label} preset{changed ? ` with ${changed} changed ${changed === 1 ? 'choice' : 'choices'}` : ''}{logo ? ` and the logo ${logo.name}` : ''}, and installs the library from the v{UI.systemVersion} release tag. The zip and the command write the same files.</p>
      <UI.RadioGroup label="Starter" value={platform} onChange={v => setPlatform(v as Platform)} options={starterKinds.map(k => ({ value: k.platform, label: `${k.label}: ${k.description}` }))} />
      {result.unsupported ? <UI.Alert tone="info" title="No native starter for this preset">{result.unsupported}</UI.Alert> : <>
        <UI.CodeBlock language={logo ? `Command (run it beside ${logo.name})` : 'Command'} code={cmd} />
        <p className="sj-muted">Paste it into a terminal, or give it to your coding agent. It needs Node.js 20 or later and no npm account.</p>
        <UI.Disclosure label={`Files (${files.length})`}><ul className="export-files">{files.map(f => <li key={f.path}><code>{f.path}</code></li>)}</ul></UI.Disclosure>
      </>}
    </div>
  </UI.Dialog>;
}
