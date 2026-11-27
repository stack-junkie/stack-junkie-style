import { useEffect, useState, type CSSProperties, type ReactNode } from 'react';
import { ThemeProvider, type Direction, type ThemeMode } from './theme';
import { SettingsRow, Workspace } from './layout';
import { NavTree, SideNavigation, Tabs } from './navigation';
import { IconButton, type IconName } from './core';
import { Select } from './inputs';
import { AboutSection, SettingsSection, SettingsWindow, type SettingsWindowSection } from './settings';

// The standard application frame every Stack Junkie product imports. Products supply their sections, pages and own settings;
// the frame owns where navigation, Settings, theme and About go, so those details never drift between apps.
export type ShellConfig = {
  /** Clean utility is the only direction; kept optional for compatibility. */ direction?: Direction; mode: ThemeMode;
  /** Thin icon rail of sections on the far left. */ rail: boolean;
  /** Sidebar listing the pages of the current section (or every page, grouped, when there is no rail). */ contextualSidebar: boolean;
  resizableSidebar: boolean; collapsibleSidebar: boolean;
};
export const defaultShellConfig: ShellConfig = { mode: 'dark', rail: true, contextualSidebar: true, resizableSidebar: true, collapsibleSidebar: true };

/** What the frame tells a page about how it is shown. */
export type ShellPageState = {
  sidebarCollapsed: boolean;
  /** The page sits inside the frame's own tab row, so it must not add another tab row of its own. */ tabbed: boolean;
};
export type ShellPage = { id: string; label: string; content: ReactNode | ((state: ShellPageState) => ReactNode) };
export type ShellSection = {
  id: string; label: string; icon: IconName; pages: ShellPage[];
  /** Leave the section out of the icon rail; it stays in the sidebar tree and reachable by navigation. */ rail?: boolean;
  /** Leave the section out of all navigation, for pages reached by a command or link, such as Home. */ hidden?: boolean;
  /** Replace the section's page list in the sidebar. */ sidebar?: ReactNode;
};
export type ShellLocation = { section: string; page: string };
export type SettingsState = { open: boolean; section: string };

function renderPage(page: ShellPage, state: ShellPageState) { return typeof page.content === 'function' ? page.content(state): page.content; }

/** The theme row the frame's built-in Appearance section uses. Products that write their own Appearance section reuse it. */
export function ThemeSetting({ mode, onChange }: { mode: ThemeMode; onChange: (mode: ThemeMode) => void }) {
  return <SettingsRow label="Theme" description="Choose the light or dark appearance."><Select label="Color theme" value={mode} onChange={e => onChange(e.target.value as ThemeMode)}><option value="dark">Dark</option><option value="light">Light</option></Select></SettingsRow>;
}

export function AppShell({ name, description, version, logo, sections, config = defaultShellConfig, settings = [], storageKey, style, className = '',
  location: controlledLocation, onNavigate, mode: controlledMode, onModeChange, settingsState, onSettingsChange,
  brand, sidebar: sidebarOverride, sidebarHeader, sidebarTitle, overlays, mainId, skipLabel = 'Skip to content', aboutChildren, shortcuts = true }: {
  name: string; description: string; version: string; logo?: ReactNode; sections: ShellSection[]; config?: ShellConfig;
  /** Product settings. Built-in Appearance and About are added after them; give a section the id 'appearance' or 'about' to replace one in place. */ settings?: SettingsWindowSection[];
  storageKey?: string; style?: CSSProperties; className?: string;
  /** Controlled location. Without it the frame remembers the location itself. */ location?: ShellLocation;
  /** Called on every navigation. The page is omitted when only a section was chosen; the frame then uses the remembered page. */ onNavigate?: (section: string, page?: string) => void;
  mode?: ThemeMode; onModeChange?: (mode: ThemeMode) => void;
  settingsState?: SettingsState; onSettingsChange?: (state: SettingsState) => void;
  /** Replaces the default logo-and-name home button in the title bar. */ brand?: ReactNode;
  /** Replaces the whole sidebar, for example with search results. */ sidebar?: ReactNode;
  /** Sits above the sidebar list, such as a search field. A function receives the current section. */ sidebarHeader?: ReactNode | ((section: ShellSection) => ReactNode);
  sidebarTitle?: string;
  /** Extra dialogs and palettes, rendered inside the frame's theme. */ overlays?: ReactNode;
  mainId?: string; skipLabel?: string; aboutChildren?: ReactNode;
  /** Control comma opens Settings. Turn off for a frame embedded inside another app, such as a preview. */ shortcuts?: boolean;
}) {
  const [ownMode, setOwnMode] = useState<ThemeMode>(config.mode);
  useEffect(() => setOwnMode(config.mode), [config.mode]);
  const mode = controlledMode ?? ownMode;
  const setMode = (m: ThemeMode) => { if (controlledMode === undefined) setOwnMode(m); onModeChange?.(m); };

  const visible = sections.filter(s => !s.hidden);
  const [ownLocation, setOwnLocation] = useState<ShellLocation>({ section: visible[0]?.id ?? '', page: visible[0]?.pages[0]?.id ?? '' });
  const [remembered, setRemembered] = useState<Record<string, string>>({});
  const location = controlledLocation ?? ownLocation;
  useEffect(() => { setRemembered(r => r[location.section] === location.page ? r : { ...r, [location.section]: location.page }); }, [location.section, location.page]);
  const section = sections.find(s => s.id === location.section) ?? visible[0] ?? sections[0];
  const pageOf = (s: ShellSection) => (s.id === location.section && s.pages.some(p => p.id === location.page) ? location.page : remembered[s.id]) ?? s.pages[0]?.id;
  function navigate(sid: string, pid?: string) {
    if (controlledLocation === undefined) { const s = sections.find(x => x.id === sid); setOwnLocation({ section: sid, page: pid ?? (s ? remembered[s.id] ?? s.pages[0]?.id : '') ?? '' }); }
    onNavigate?.(sid, pid);
  }

  const [ownSettings, setOwnSettings] = useState<SettingsState>({ open: false, section: settings[0]?.id ?? 'appearance' });
  const settingsNow = settingsState ?? ownSettings;
  const setSettings = (s: SettingsState) => { if (settingsState === undefined) setOwnSettings(s); onSettingsChange?.(s); };
  useEffect(() => {
    if (!shortcuts) return;
    function key(e: KeyboardEvent) { if ((e.ctrlKey || e.metaKey) && e.key === ',') { e.preventDefault(); setSettings({ ...settingsNow, open: true }); } }
    document.addEventListener('keydown', key); return () => document.removeEventListener('keydown', key);
  });

  const railSections = visible.filter(s => s.rail !== false);
  const rail = config.rail ? <SideNavigation collapsed label="Sections" value={railSections.some(s => s.id === section.id) ? section.id : ''} onChange={id => navigate(id)} items={railSections.map(s => ({ id: s.id, label: s.label, icon: s.icon }))} /> : undefined;
  const header = typeof sidebarHeader === 'function' ? sidebarHeader(section): sidebarHeader;
  const list = sidebarOverride ?? (config.rail || section.hidden
    ? section.sidebar ?? <SideNavigation label={`${section.label} pages`} value={pageOf(section)} onChange={id => navigate(section.id, id)} items={section.pages.map(p => ({ id: p.id, label: p.label }))} />
    : <NavTree label="Pages" storageKey={storageKey && `${storageKey}:tree`} groups={visible.map(s => ({ id: s.id, label: s.label, items: s.pages.map(p => ({ id: `${s.id}/${p.id}`, label: p.label })) }))} value={`${section.id}/${pageOf(section)}`} onChange={key => { const [sid, pid] = key.split('/'); navigate(sid, pid); }} />);
  const sidebar = config.contextualSidebar ? <>{header}{list}</> : undefined;
  const title = sidebarTitle ?? (config.rail || section.hidden ? section.label : 'Pages');

  // Without a sidebar, a section's pages become the page's one tab row. Without a rail either, every page shares one grouped row.
  function body(sidebarCollapsed: boolean) {
    const state = (tabbed: boolean): ShellPageState => ({ sidebarCollapsed, tabbed });
    const single = (s: ShellSection) => s.pages.map(p => <div key={`${s.id}/${p.id}`} className="sj-shell-page" hidden={s.id !== section.id || p.id !== pageOf(s)}>{renderPage(p, state(false))}</div>);
    const tabbed = (s: ShellSection) => s.pages.length === 1 ? single(s): <div key={s.id} className="sj-shell-tabs" hidden={s.id !== section.id}><Tabs label={`${s.label} pages`} value={pageOf(s)} onChange={id => navigate(s.id, id)} tabs={s.pages.map(p => ({ id: p.id, label: p.label, content: renderPage(p, state(true)) }))} /></div>;
    if (config.contextualSidebar) return sections.flatMap(single);
    if (config.rail) return sections.map(tabbed);
    const grouped = visible.filter(s => s.pages.length);
    return <>{sections.filter(s => s.hidden).map(tabbed)}
      <div className="sj-shell-tabs" hidden={section.hidden}><Tabs label="Pages" groups={grouped.map(s => ({ id: s.id, label: s.label }))} value={`${section.id}/${pageOf(section)}`} onChange={key => { const [sid, pid] = key.split('/'); navigate(sid, pid); }}
        tabs={grouped.flatMap(s => s.pages.map(p => ({ id: `${s.id}/${p.id}`, label: p.label, group: s.id, content: renderPage(p, state(true)) })))} /></div></>;
  }

  const builtIn: SettingsWindowSection[] = [
    { id: 'appearance', label: 'Appearance', keywords: 'theme dark light mode color', content: <SettingsSection label="Appearance"><ThemeSetting mode={mode} onChange={setMode} /></SettingsSection> },
    { id: 'about', label: 'About', keywords: 'version copyright', content: <AboutSection name={name} description={description} version={version} logo={logo}>{aboutChildren}</AboutSection> },
  ];
  const settingsSections = [...settings, ...builtIn.filter(b => !settings.some(s => s.id === b.id))];

  return <ThemeProvider direction={config.direction} mode={mode} className={`sj-shell ${className}`} style={style}>
    {mainId && <a className="sj-skip-link" href={`#${mainId}`}>{skipLabel}</a>}
    <Workspace contentOwnsHeader mainId={mainId} storageKey={storageKey} sidebarTitle={title} rail={rail} sidebar={sidebar} resizable={config.resizableSidebar} collapsible={config.collapsibleSidebar}
      header={brand ?? <button type="button" className="sj-button sj-button--quiet sj-shell-home" aria-label={`${name} home`} title="Home" onClick={() => navigate(visible[0]?.id ?? '')}>{logo}<strong>{name}</strong></button>}
      navigationFooter={<IconButton icon="settings" label="Settings" variant="quiet" aria-haspopup="dialog" aria-expanded={settingsNow.open} onClick={() => setSettings({ ...settingsNow, open: true })} />}>
      {({ sidebarCollapsed }) => body(sidebarCollapsed)}
    </Workspace>
    <SettingsWindow open={settingsNow.open} onClose={() => setSettings({ ...settingsNow, open: false })} section={settingsNow.section} onSectionChange={id => setSettings({ ...settingsNow, section: id })} sections={settingsSections} />
    {overlays}
  </ThemeProvider>;
}
