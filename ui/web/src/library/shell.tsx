import { createContext, useContext, useEffect, useState, type CSSProperties, type ReactNode } from 'react';
import { ThemeProvider, type Direction, type ThemeMode } from './theme';
import { SettingsRow, Workspace } from './layout';
import { NavTree, SideNavigation, Tabs } from './navigation';
import { IconButton, type IconName } from './core';
import { Select } from './inputs';
import { AboutSection, SettingsSection, SettingsWindow, type SettingsWindowSection } from './settings';
import { resolvePreset, type PresetChoices, type PresetId } from './presets';
import { BottomPanel, MenuBar, StatusBar, type MenuBarMenu, type MenuEntry, type PanelTab } from './bars';

// The standard application frame every Stack Junkie product imports. Products supply their sections, pages and own settings;
// the frame owns where navigation, Settings, theme and About go, so those details never drift between apps.
export type ShellConfig = {
  /** Clean is the only direction; kept optional for compatibility. */ direction?: Direction; mode: ThemeMode;
  /** Thin icon rail of sections on the far left. */ rail: boolean;
  /** Sidebar listing the pages of the current section (or every page, grouped, when there is no rail). */ contextualSidebar: boolean;
  resizableSidebar: boolean; collapsibleSidebar: boolean;
};
export const defaultShellConfig: ShellConfig = { mode: 'dark', rail: true, contextualSidebar: true, resizableSidebar: true, collapsibleSidebar: true };

/** The choices a frame without a preset uses, read from its config so existing apps keep their layout. */
export function configChoices(config: ShellConfig): PresetChoices {
  return { ...resolvePreset('web-app'), rail: config.rail, sidebar: config.contextualSidebar ? (config.rail ? 'list' : 'tree') : 'off', theme: config.mode };
}
const ShellChoices = createContext<PresetChoices | null>(null);
/** The frame's resolved preset choices, for app parts the frame does not own yet, such as the command palette. */
export const useShellChoices = () => useContext(ShellChoices);

/** Follows the device's light or dark setting. */
export function useSystemTheme(): ThemeMode {
  const query = '(prefers-color-scheme: dark)';
  const [dark, setDark] = useState(() => typeof matchMedia === 'function' && matchMedia(query).matches);
  useEffect(() => {
    if (typeof matchMedia !== 'function') return;
    const media = matchMedia(query); const update = () => setDark(media.matches);
    media.addEventListener('change', update); return () => media.removeEventListener('change', update);
  }, []);
  return dark ? 'dark' : 'light';
}

/** What the frame tells a page about how it is shown. */
export type ShellPageState = {
  sidebarCollapsed: boolean;
  /** The page sits inside the frame's own tab row, so it must not add another tab row of its own. */ tabbed: boolean;
};
export type ShellPage = { id: string; label: string; content: ReactNode | ((state: ShellPageState) => ReactNode);
  /** Group heading for this page when the sidebar is a tree beside the rail. Ungrouped pages sit under the section's name. */ group?: string };
export type ShellSection = {
  id: string; label: string; icon: IconName; pages: ShellPage[];
  /** Leave the section out of the icon rail; it stays in the sidebar tree and reachable by navigation. */ rail?: boolean;
  /** Leave the section out of all navigation, for pages reached by a command or link, such as Home. */ hidden?: boolean;
  /** Replace the section's page list in the sidebar. */ sidebar?: ReactNode;
};
export type ShellLocation = { section: string; page: string };
export type SettingsState = { open: boolean; section: string };
/** The frame's own menus. Items an app adds to one of them sit above the frame's items. */
export type ShellMenu = 'file' | 'edit' | 'view' | 'help';

function renderPage(page: ShellPage, state: ShellPageState) { return typeof page.content === 'function' ? page.content(state): page.content; }

/** The theme row the frame's built-in Appearance section uses. Products that write their own Appearance section reuse it. */
export function ThemeSetting({ mode, onChange }: { mode: ThemeMode; onChange: (mode: ThemeMode) => void }) {
  return <SettingsRow label="Theme" description="Choose the light or dark appearance."><Select label="Color theme" value={mode} onChange={e => onChange(e.target.value as ThemeMode)}><option value="dark">Dark</option><option value="light">Light</option></Select></SettingsRow>;
}

export function AppShell({ name, description, version, logo, sections, config = defaultShellConfig, preset, choices: overrides, settings = [], storageKey, style, className = '',
  location: controlledLocation, onNavigate, mode: controlledMode, onModeChange, settingsState, onSettingsChange,
  brand, sidebar: sidebarOverride, sidebarHeader, sidebarTitle, overlays, mainId, skipLabel = 'Skip to content', aboutChildren, shortcuts = true,
  menus = [], menuItems = {}, status, panels = [] }: {
  name: string; description: string; version: string; logo?: ReactNode; sections: ShellSection[];
  /** Sidebar resizing and collapsing. Without a preset or choices, its rail, sidebar and theme also set the layout. */ config?: ShellConfig;
  /** Starting set of App frame choices. Its rail, sidebar and theme replace the ones in config.
   * The Website preset is drawn by WebsiteLayout, not this frame; here it only turns the rail and sidebar off. */ preset?: PresetId;
  /** Individual choices that replace the preset's. Without a preset they apply to Web app. */ choices?: Partial<PresetChoices>;
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
  /** Extra dialogs and palettes, rendered inside the frame's theme. A function receives the resolved choices, such as whether to offer the command palette. */ overlays?: ReactNode | ((choices: PresetChoices) => ReactNode);
  mainId?: string; skipLabel?: string; aboutChildren?: ReactNode;
  /** Control comma opens Settings, and with the Menu bar on, Control B toggles the sidebar and Control J the bottom panel.
   * Turn off for a frame embedded inside another app, such as a preview. */ shortcuts?: boolean;
  /** The app's own menus, drawn between View and Help when the Menu bar choice is on. */ menus?: MenuBarMenu[];
  /** Items the app adds to File, Edit, View or Help. Edit holds only app items and is left out without them. */ menuItems?: Partial<Record<ShellMenu, MenuEntry[]>>;
  /** Status bar content when the Status bar choice is on: the current item's state on the left, small facts on the right. Never ads or tips.
   * Without a state the left side shows the current page's name. */
  status?: { state?: ReactNode; facts?: ReactNode[] };
  /** Bottom panel tabs, such as Output and Problems, when the Bottom panel choice is on. Without tabs no panel is drawn. */ panels?: PanelTab[];
}) {
  const choices = preset !== undefined || overrides !== undefined ? resolvePreset(preset ?? 'web-app', overrides) : configChoices(config);
  const systemMode = useSystemTheme();
  const defaultMode = choices.theme === 'system' ? systemMode : choices.theme;
  const [ownMode, setOwnMode] = useState<ThemeMode>(defaultMode);
  useEffect(() => setOwnMode(defaultMode), [defaultMode]);
  const mode = controlledMode ?? ownMode;
  const setMode = (m: ThemeMode) => { if (controlledMode === undefined) setOwnMode(m); onModeChange?.(m); };

  const visible = sections.filter(s => !s.hidden);
  const [ownLocation, setOwnLocation] = useState<ShellLocation>({ section: visible[0]?.id ?? '', page: visible[0]?.pages[0]?.id ?? '' });
  const [remembered, setRemembered] = useState<Record<string, string>>({});
  const location = controlledLocation ?? ownLocation;
  useEffect(() => { setRemembered(r => r[location.section] === location.page ? r : { ...r, [location.section]: location.page }); }, [location.section, location.page]);
  const section = sections.find(s => s.id === location.section) ?? visible[0] ?? sections[0];
  const pageOf = (s: ShellSection) => (s.id === location.section && s.pages.some(p => p.id === location.page) ? location.page : remembered[s.id]) ?? s.pages[0]?.id;
  // Document tabs: every page opened from the sidebar stays in one row of closable tabs until closed.
  const [openPages, setOpenPages] = useState<string[]>([]);
  const currentKey = section ? `${section.id}/${pageOf(section)}` : '';
  const openPage = (key: string) => setOpenPages(o => o.includes(key) ? o : [...o, key]);
  useEffect(() => { if (choices.documentTabs && currentKey) openPage(currentKey); }, [choices.documentTabs, currentKey]);
  function navigate(sid: string, pid?: string) {
    if (pid && choices.documentTabs) openPage(`${sid}/${pid}`);
    if (controlledLocation === undefined) { const s = sections.find(x => x.id === sid); setOwnLocation({ section: sid, page: pid ?? (s ? remembered[s.id] ?? s.pages[0]?.id : '') ?? '' }); }
    onNavigate?.(sid, pid);
  }

  // The View menu hides and shows the sidebar, Status bar and bottom panel for this session; the choices decide whether they exist.
  const [sidebarCollapsed, setSidebarCollapsed] = useState(() => { try { return !!storageKey && localStorage.getItem(`${storageKey}:collapsed`) === 'true'; } catch { return false; } });
  const [statusShown, setStatusShown] = useState(true);
  const [panelShown, setPanelShown] = useState(true);
  const [panelHeight, setPanelHeight] = useState(() => { try { const value = Number(storageKey ? localStorage.getItem(`${storageKey}:panel`) : NaN); return Number.isFinite(value) && value > 0 ? value : 160; } catch { return 160; } });
  useEffect(() => { if (storageKey) try { localStorage.setItem(`${storageKey}:panel`, String(panelHeight)); } catch { /* Optional storage. */ } }, [storageKey, panelHeight]);
  const [ownSettings, setOwnSettings] = useState<SettingsState>({ open: false, section: settings[0]?.id ?? 'appearance' });
  const settingsNow = settingsState ?? ownSettings;
  const setSettings = (s: SettingsState) => { if (settingsState === undefined) setOwnSettings(s); onSettingsChange?.(s); };
  useEffect(() => {
    if (!shortcuts) return;
    function key(e: KeyboardEvent) {
      if (!(e.ctrlKey || e.metaKey) || e.altKey || e.shiftKey) return;
      if (e.key === ',') { e.preventDefault(); setSettings({ ...settingsNow, open: true }); }
      else if (choices.menuBar && e.key.toLowerCase() === 'b' && canCollapse) { e.preventDefault(); setSidebarCollapsed(c => !c); }
      else if (choices.menuBar && e.key.toLowerCase() === 'j' && panelOn) { e.preventDefault(); setPanelShown(s => !s); }
    }
    document.addEventListener('keydown', key); return () => document.removeEventListener('keydown', key);
  });

  const railSections = visible.filter(s => s.rail !== false);
  const sidebarOn = choices.sidebar !== 'off';
  const canCollapse = sidebarOn && config.collapsibleSidebar;
  const panelOn = choices.bottomPanel && panels.length > 0;
  const rail = choices.rail ? <SideNavigation collapsed label="Sections" value={railSections.some(s => s.id === section.id) ? section.id : ''} onChange={id => navigate(id)} items={railSections.map(s => ({ id: s.id, label: s.label, icon: s.icon }))} /> : undefined;
  const header = typeof sidebarHeader === 'function' ? sidebarHeader(section): sidebarHeader;
  const openKey = (key: string) => { const [sid, pid] = key.split('/'); navigate(sid, pid); };
  // Beside the rail the sidebar holds the current section's pages: a flat list, or a tree grouped by each page's group.
  // Without the rail it holds every page: a tree grouped by section, or one flat list.
  const sectionList = <SideNavigation label={`${section.label} pages`} value={pageOf(section)} onChange={id => navigate(section.id, id)} items={section.pages.map(p => ({ id: p.id, label: p.label }))} />;
  const pageGroups = [...new Set(section.pages.map(p => p.group ?? section.label))];
  const sectionTree = <NavTree label={`${section.label} pages`} storageKey={storageKey && `${storageKey}:tree:${section.id}`} value={pageOf(section)} onChange={id => navigate(section.id, id)}
    groups={pageGroups.map(g => ({ id: g, label: g, items: section.pages.filter(p => (p.group ?? section.label) === g).map(p => ({ id: p.id, label: p.label })) }))} />;
  const allTree = <NavTree label="Pages" storageKey={storageKey && `${storageKey}:tree`} groups={visible.map(s => ({ id: s.id, label: s.label, items: s.pages.map(p => ({ id: `${s.id}/${p.id}`, label: p.label })) }))} value={`${section.id}/${pageOf(section)}`} onChange={openKey} />;
  const allList = <SideNavigation label="Pages" value={`${section.id}/${pageOf(section)}`} onChange={openKey} items={visible.flatMap(s => s.pages.map(p => ({ id: `${s.id}/${p.id}`, label: p.label })))} />;
  const list = sidebarOverride ?? (choices.rail || section.hidden ? section.sidebar ?? (choices.sidebar === 'tree' && !section.hidden ? sectionTree : sectionList) : choices.sidebar === 'tree' ? allTree : allList);
  const sidebar = sidebarOn ? <>{header}{list}</> : undefined;
  const title = sidebarTitle ?? (choices.rail || section.hidden ? section.label : 'Pages');

  function closePage(key: string) {
    const index = openPages.indexOf(key); const rest = openPages.filter(k => k !== key);
    setOpenPages(rest);
    if (key === currentKey && rest.length) openKey(rest[Math.min(index, rest.length - 1)]);
  }

  // Document tabs apply beside a sidebar, which opens the pages. Without a sidebar the section's pages are already the one tab row.
  // Without a sidebar, a section's pages become the page's one tab row. Without a rail either, every page shares one grouped row.
  function body(sidebarCollapsed: boolean) {
    const state = (tabbed: boolean): ShellPageState => ({ sidebarCollapsed, tabbed });
    const single = (s: ShellSection) => s.pages.map(p => <div key={`${s.id}/${p.id}`} className="sj-shell-page" hidden={s.id !== section.id || p.id !== pageOf(s)}>{renderPage(p, state(false))}</div>);
    const tabbed = (s: ShellSection) => s.pages.length === 1 ? single(s): <div key={s.id} className="sj-shell-tabs" hidden={s.id !== section.id}><Tabs label={`${s.label} pages`} value={pageOf(s)} onChange={id => navigate(s.id, id)} tabs={s.pages.map(p => ({ id: p.id, label: p.label, content: renderPage(p, state(true)) }))} /></div>;
    if (sidebarOn && choices.documentTabs) {
      const open = openPages.flatMap(key => { const [sid, pid] = key.split('/'); const s = sections.find(x => x.id === sid); const p = s?.pages.find(x => x.id === pid); return s && p ? [{ key, page: p }] : []; });
      if (!open.length) return <div className="sj-shell-empty"><p className="sj-muted">No open pages. Choose one in the sidebar.</p></div>;
      return <div className="sj-shell-tabs"><Tabs label="Open pages" value={openPages.includes(currentKey) ? currentKey : ''} onChange={openKey} onCloseTab={closePage}
        tabs={open.map(({ key, page }) => ({ id: key, label: page.label, content: renderPage(page, state(true)) }))} /></div>;
    }
    if (sidebarOn) return sections.flatMap(single);
    if (choices.rail) return sections.map(tabbed);
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

  // File, Edit, View, the app's menus, then Help. The frame owns Settings, About and the View toggles; apps add their own items above them.
  const menuBar = choices.menuBar ? <MenuBar label={`${name} menu bar`} menus={[
    { id: 'file', label: 'File', items: [...(menuItems.file ?? []), { id: 'frame-file', separator: true }, { id: 'settings', label: 'Settings…', shortcut: 'Ctrl+,', run: () => setSettings({ ...settingsNow, open: true }) }] },
    { id: 'edit', label: 'Edit', items: menuItems.edit ?? [] },
    { id: 'view', label: 'View', items: [...(menuItems.view ?? []), { id: 'frame-view', separator: true },
      ...(canCollapse ? [{ id: 'sidebar', label: 'Sidebar', shortcut: 'Ctrl+B', checked: !sidebarCollapsed, run: () => setSidebarCollapsed(!sidebarCollapsed) }] : []),
      ...(panelOn ? [{ id: 'bottom-panel', label: 'Bottom panel', shortcut: 'Ctrl+J', checked: panelShown, run: () => setPanelShown(!panelShown) }] : []),
      ...(choices.statusBar ? [{ id: 'status-bar', label: 'Status bar', checked: statusShown, run: () => setStatusShown(!statusShown) }] : []),
      { id: 'frame-theme', separator: true }, { id: 'dark-theme', label: 'Dark theme', checked: mode === 'dark', run: () => setMode(mode === 'dark' ? 'light' : 'dark') }] },
    ...menus,
    { id: 'help', label: 'Help', items: [...(menuItems.help ?? []), { id: 'frame-help', separator: true }, { id: 'about', label: `About ${name}`, run: () => setSettings({ open: true, section: 'about' }) }] },
  ]} /> : null;
  const statusBar = choices.statusBar && statusShown ? <StatusBar state={status?.state ?? section?.pages.find(p => p.id === pageOf(section))?.label} facts={status?.facts} /> : undefined;
  const work = (sidebarCollapsed: boolean) => panelOn && panelShown
    ? <div className="sj-shell-work"><div className="sj-shell-work-main">{body(sidebarCollapsed)}</div><BottomPanel tabs={panels} height={panelHeight} onHeightChange={setPanelHeight} onClose={() => setPanelShown(false)} /></div>
    : body(sidebarCollapsed);

  const gear = <IconButton icon="settings" label="Settings" variant="quiet" aria-haspopup="dialog" aria-expanded={settingsNow.open} onClick={() => setSettings({ ...settingsNow, open: true })} />;
  const home = brand ?? <button type="button" className="sj-button sj-button--quiet sj-shell-home" aria-label={`${name} home`} title="Home" onClick={() => navigate(visible[0]?.id ?? '')}>{logo}<strong>{name}</strong></button>;

  return <ShellChoices.Provider value={choices}><ThemeProvider direction={config.direction} mode={mode} density={choices.density} className={`sj-shell ${className}`} style={style}>
    {mainId && <a className="sj-skip-link" href={`#${mainId}`}>{skipLabel}</a>}
    <Workspace contentOwnsHeader mainId={mainId} storageKey={storageKey} sidebarTitle={title} rail={rail} sidebar={sidebar} resizable={config.resizableSidebar} collapsible={config.collapsibleSidebar}
      sidebarCollapsed={sidebarCollapsed} onSidebarCollapsedChange={setSidebarCollapsed}
      header={menuBar ? <div className="sj-shell-title">{home}{menuBar}</div> : home} navigationFooter={gear} footer={statusBar}>
      {({ sidebarCollapsed }) => work(sidebarCollapsed)}
    </Workspace>
    <SettingsWindow open={settingsNow.open} onClose={() => setSettings({ ...settingsNow, open: false })} section={settingsNow.section} onSectionChange={id => setSettings({ ...settingsNow, section: id })} sections={settingsSections} />
    {typeof overlays === 'function' ? overlays(choices) : overlays}
  </ThemeProvider></ShellChoices.Provider>;
}
