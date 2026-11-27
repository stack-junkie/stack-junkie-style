import { createContext, useContext, useEffect, useId, useState, type KeyboardEvent, type ReactNode } from 'react';
import { Button, Icon, IconButton, type IconName } from './core';

// Tab rows must never stack directly on another tab row. A Tabs inside another Tabs panel warns in development
// unless something that visually separates it (a specimen stage, a window frame, a separate panel) declares a TabBoundary.
const TabDepth = createContext(0);
export function TabBoundary({ children }: { children: ReactNode }) { return <TabDepth.Provider value={0}>{children}</TabDepth.Provider>; }
const devBuild = Boolean((import.meta as unknown as { env?: { DEV?: boolean } }).env?.DEV);

export type TabItem = { id: string; label: string; content: ReactNode; disabled?: boolean; group?: string };
export function Tabs({ label, tabs, value, onChange, variant = 'connected', className = '', contextLabel, groups = [], onCloseTab }: {
  label: string; tabs: TabItem[]; value: string; onChange: (id: string) => void; variant?: 'connected' | 'underline'; className?: string; contextLabel?: string;
  groups?: { id: string; label: string }[]; onCloseTab?: (id: string) => void;
}) {
  const uid = useId(); const depth = useContext(TabDepth);
  const [collapsedGroups, setCollapsedGroups] = useState<string[]>([]);
  useEffect(() => { if (devBuild && depth > 0) console.warn(`Stack Junkie UI: tab row "${label}" sits inside another tab panel. Stacked tab rows are not allowed; use the sidebar tree, a toolbar toggle or a TabBoundary frame.`); }, [depth, label]);
  // Keep the selected tab in view when it changes from outside the row (navigation, search), without scrolling the page.
  useEffect(() => { const tab = document.getElementById(`${uid}-tab-${value}`); const list = tab?.closest('.sj-tab-list'); if (tab && list) { const t = tab.getBoundingClientRect(), l = list.getBoundingClientRect(); if (t.left < l.left || t.right > l.right) list.scrollLeft += t.left - l.left - 24; } }, [uid, value]);
  const shown = (t: TabItem) => !t.group || !collapsedGroups.includes(t.group) || t.id === value;
  function move(event: KeyboardEvent<HTMLButtonElement>, id: string) {
    if (!['ArrowLeft', 'ArrowRight', 'Home', 'End'].includes(event.key)) return;
    event.preventDefault(); const available = tabs.filter(t => !t.disabled && shown(t)); const current = available.findIndex(t => t.id === id);
    const target = event.key === 'Home' ? available[0] : event.key === 'End' ? available.at(-1)! : available[(current + (event.key === 'ArrowRight' ? 1 : -1) + available.length) % available.length];
    onChange(target.id); document.getElementById(`${uid}-tab-${target.id}`)?.focus();
  }
  const row: ReactNode[] = [];
  tabs.forEach((t, i) => {
    if (t.group && t.group !== tabs[i - 1]?.group) {
      const group = groups.find(g => g.id === t.group); const collapsed = collapsedGroups.includes(t.group);
      row.push(<button type="button" key={`group-${t.group}`} className="sj-tab-group" aria-expanded={!collapsed} title={collapsed ? `Show ${group?.label ?? t.group}` : `Collapse ${group?.label ?? t.group}`}
        onClick={() => setCollapsedGroups(c => collapsed ? c.filter(g => g !== t.group): [...c, t.group!])}>{group?.label ?? t.group}</button>);
    }
    if (!shown(t)) return;
    const tab = <button type="button" role="tab" id={`${uid}-tab-${t.id}`} key={t.id} aria-selected={value === t.id} aria-controls={`${uid}-panel-${t.id}`} tabIndex={value === t.id ? 0 : -1} disabled={t.disabled} data-group={t.group} onClick={() => onChange(t.id)} onKeyDown={e => { if (onCloseTab && (e.key === 'Delete' || (e.ctrlKey && e.key.toLowerCase() === 'w'))) { e.preventDefault(); onCloseTab(t.id); } else move(e, t.id); }}>{t.label}</button>;
    row.push(onCloseTab ? <span className="sj-tab-closable" key={t.id} data-selected={value === t.id}>{tab}<button type="button" className="sj-tab-close" tabIndex={-1} aria-label={`Close ${t.label}`} title={`Close ${t.label}`} onClick={() => onCloseTab(t.id)}><Icon name="close" size={12} /></button></span> : tab);
  });
  return <div className={`sj-tabs sj-tabs--${variant} ${className}`}><div className="sj-tabs-heading">{contextLabel && <span className="sj-tabs-context" title={contextLabel}>{contextLabel}</span>}<div role="tablist" aria-label={label} className="sj-tab-list">{row}</div></div>
    <TabDepth.Provider value={depth + 1}>{tabs.map(t => <div role="tabpanel" id={`${uid}-panel-${t.id}`} aria-labelledby={`${uid}-tab-${t.id}`} hidden={value !== t.id} key={t.id} tabIndex={0} className="sj-tab-panel">{t.content}</div>)}</TabDepth.Provider></div>;
}

export function Breadcrumbs({ items }: { items: { label: string; onClick?: () => void }[] }) { return <nav aria-label="Breadcrumb" className="sj-breadcrumbs"><ol>{items.map((item, i) => <li key={`${item.label}-${i}`}>{i > 0 && <Icon name="chevron-right" size={14} />}{i === items.length - 1 ? <span aria-current="page">{item.label}</span> : <Button variant="quiet" onClick={item.onClick}>{item.label}</Button>}</li>)}</ol></nav>; }
export function SideNavigation({ label = 'Pages', items, value, onChange, collapsed = false }: { label?: string; items: { id: string; label: string; icon?: IconName }[]; value: string; onChange: (id: string) => void; collapsed?: boolean }) { return <nav aria-label={label} className={`sj-side-nav ${collapsed ? 'is-collapsed' : ''}`}>{items.map(item => <button type="button" key={item.id} className="sj-nav-item" aria-current={value === item.id ? 'page' : undefined} onClick={() => onChange(item.id)} title={collapsed ? item.label : undefined} aria-label={collapsed ? item.label : undefined}>{item.icon && <Icon name={item.icon} />}<span className={collapsed ? 'sj-sr-only' : ''}>{item.label}</span></button>)}</nav>; }

// Contextual sidebar tree: the place where depth goes. Groups collapse like VS Code's sidebar sections, so an app with many pages
// gets deeper groups here instead of more tab rows.
export type NavGroup = { id: string; label: string; items: { id: string; label: string; icon?: IconName }[] };
export function NavTree({ label, groups, value, onChange, storageKey }: { label: string; groups: NavGroup[]; value: string; onChange: (id: string) => void; storageKey?: string }) {
  const [closed, setClosed] = useState<string[]>(() => { try { return storageKey ? JSON.parse(localStorage.getItem(`${storageKey}:closed`) || '[]'): []; } catch { return []; } });
  useEffect(() => { if (storageKey) try { localStorage.setItem(`${storageKey}:closed`, JSON.stringify(closed)); } catch { /* Optional storage. */ } }, [storageKey, closed]);
  return <nav aria-label={label} className="sj-nav-tree">{groups.map(group => {
    const open = !closed.includes(group.id); const id = `${label}-${group.id}`.replace(/\W+/g, '-');
    return <section key={group.id} className="sj-nav-group">
      <button type="button" className="sj-nav-group-heading" aria-expanded={open} aria-controls={id} onClick={() => setClosed(c => open ? [...c, group.id] : c.filter(g => g !== group.id))}><Icon name={open ? 'chevron-down' : 'chevron-right'} size={14} /><span>{group.label}</span><span className="sj-nav-group-count">{group.items.length}</span></button>
      <div id={id} hidden={!open} className="sj-side-nav">{group.items.map(item => <button type="button" key={item.id} className="sj-nav-item" aria-current={value === item.id ? 'page' : undefined} onClick={() => onChange(item.id)}>{item.icon && <Icon name={item.icon} />}<span>{item.label}</span></button>)}</div>
    </section>;
  })}</nav>;
}

export function Pagination({ page, pages, onChange }: { page: number; pages: number; onChange: (page: number) => void }) { return <nav aria-label="Pagination" className="sj-pagination"><IconButton icon="chevron-left" label="Previous page" disabled={page <= 1} onClick={() => onChange(page - 1)} /><span aria-live="polite">Page <strong>{page}</strong> of {Math.max(1, pages)}</span><IconButton icon="chevron-right" label="Next page" disabled={page >= pages} onClick={() => onChange(page + 1)} /></nav>; }
