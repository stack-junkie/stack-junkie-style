import { useEffect, useRef, useState, type KeyboardEvent as ReactKeyboardEvent, type ReactNode } from 'react';
import { Icon, IconButton } from './core';
import { Tabs } from './navigation';

// Menu bar, Status bar and bottom panel: the App frame parts a Document editor adds. AppShell draws them from the preset's
// menuBar, statusBar and bottomPanel choices; apps can also use them on their own.

/** One menu entry. `shortcut` is the text shown on the right, such as "Ctrl+B"; the app or frame owns the key itself.
 * Give `checked` for an on or off item, which shows a check mark. */
export type MenuItem = { id: string; label: string; run: () => void; shortcut?: string; disabled?: boolean; checked?: boolean };
export type MenuSeparator = { id: string; separator: true };
/** A submenu, such as View > Appearance. It opens to the right on hover, Enter or the right arrow key. One level deep. */
export type MenuSubmenu = { id: string; label: string; items: (MenuItem | MenuSeparator)[]; disabled?: boolean };
export type MenuEntry = MenuItem | MenuSeparator | MenuSubmenu;
export type MenuBarMenu = { id: string; label: string; items: MenuEntry[] };
const isSeparator = (entry: MenuEntry): entry is MenuSeparator => 'separator' in entry;
const isSubmenu = (entry: MenuEntry): entry is MenuSubmenu => 'items' in entry;

/** Drops separators at either end and next to each other, so menus built from optional groups stay tidy. */
export function tidyMenu(items: MenuEntry[]): MenuEntry[] {
  const result: MenuEntry[] = [];
  for (const entry of items) {
    const item = isSubmenu(entry) ? { ...entry, items: tidyMenu(entry.items) as MenuSubmenu['items'] } : entry;
    if (isSubmenu(item) && !item.items.length) continue;
    if (!isSeparator(item) || (result.length && !isSeparator(result[result.length - 1]))) result.push(item);
  }
  while (result.length && isSeparator(result[result.length - 1])) result.pop();
  return result;
}

/** A row of menus. Arrow keys move between menus and items, Enter or Space runs an item, Escape closes the open menu. */
export function MenuBar({ menus, label = 'Menu bar' }: { menus: MenuBarMenu[]; label?: string }) {
  const shown = menus.map(m => ({ ...m, items: tidyMenu(m.items) })).filter(m => m.items.length);
  const [open, setOpen] = useState<number | null>(null);
  const [current, setCurrent] = useState(0);
  // Which item to focus once a menu opens: the first, the last, or none when it was opened by pointer.
  const [focusAt, setFocusAt] = useState<'first' | 'last' | null>(null);
  // The open submenu inside the open menu, and whether to focus its first item once it opens.
  const [sub, setSub] = useState<string | null>(null);
  const [subFocus, setSubFocus] = useState(false);
  const root = useRef<HTMLDivElement>(null);
  const buttons = useRef<(HTMLButtonElement | null)[]>([]);
  const popup = useRef<HTMLDivElement>(null);
  // The enabled items of one menu, leaving out the items of a submenu inside it.
  const itemsIn = (menu: Element | null | undefined) => [...(menu?.querySelectorAll<HTMLButtonElement>(':scope > [role^=menuitem]:not(:disabled), :scope > .sj-menubar-sub > [role^=menuitem]:not(:disabled)') ?? [])];
  const items = () => itemsIn(popup.current);

  useEffect(() => {
    if (open === null || !focusAt) return;
    const list = items(); (focusAt === 'first' ? list[0] : list[list.length - 1])?.focus(); setFocusAt(null);
  }, [open, focusAt]);
  useEffect(() => { setSub(null); }, [open]);
  useEffect(() => {
    if (!sub || !subFocus) return;
    itemsIn(popup.current?.querySelector(`[data-submenu="${sub}"]`))[0]?.focus(); setSubFocus(false);
  }, [sub, subFocus]);
  useEffect(() => {
    if (open === null) return;
    const outside = (e: PointerEvent) => { if (!root.current?.contains(e.target as Node)) setOpen(null); };
    document.addEventListener('pointerdown', outside); return () => document.removeEventListener('pointerdown', outside);
  }, [open]);
  if (!shown.length) return null;

  const at = (index: number) => (index + shown.length) % shown.length;
  function openMenu(index: number, focus: 'first' | 'last' | null) { setCurrent(index); setOpen(index); setFocusAt(focus); if (!focus) buttons.current[index]?.focus(); }
  function close(refocus: boolean) { const index = open ?? current; setOpen(null); if (refocus) buttons.current[index]?.focus(); }
  function barKey(e: ReactKeyboardEvent, index: number) {
    const move = (to: number) => { e.preventDefault(); if (open !== null) openMenu(to, 'first'); else { setCurrent(to); buttons.current[to]?.focus(); } };
    if (e.key === 'ArrowRight') move(at(index + 1));
    else if (e.key === 'ArrowLeft') move(at(index - 1));
    else if (e.key === 'Home') move(0);
    else if (e.key === 'End') move(shown.length - 1);
    else if (e.key === 'ArrowDown' || e.key === 'Enter' || e.key === ' ') { e.preventDefault(); openMenu(index, 'first'); }
    else if (e.key === 'ArrowUp') { e.preventDefault(); openMenu(index, 'last'); }
    else if (e.key === 'Escape' && open !== null) { e.preventDefault(); close(true); }
  }
  function menuKey(e: ReactKeyboardEvent) {
    const active = document.activeElement as HTMLButtonElement | null;
    const container = active?.closest('[role=menu]');
    // Inside a submenu: the arrow keys move within it, and the left arrow or Escape returns to its parent item.
    if (container && container !== popup.current) {
      const parent = container.parentElement?.querySelector<HTMLButtonElement>(':scope > [aria-haspopup=menu]');
      if (e.key === 'ArrowLeft' || e.key === 'Escape') { e.preventDefault(); e.stopPropagation(); setSub(null); parent?.focus(); return; }
      if (e.key === 'ArrowRight' && open !== null) { e.preventDefault(); openMenu(at(open + 1), 'first'); return; }
    }
    if (active?.getAttribute('aria-haspopup') === 'menu' && container === popup.current && (e.key === 'ArrowRight' || e.key === 'Enter' || e.key === ' ')) {
      e.preventDefault(); setSub(active.dataset.opens ?? null); setSubFocus(true); return;
    }
    const list = container && container !== popup.current ? itemsIn(container) : items(); const index = list.indexOf(active as HTMLButtonElement);
    const focus = (to: number) => { e.preventDefault(); list[(to + list.length) % list.length]?.focus(); };
    if (e.key === 'ArrowDown') focus(index + 1);
    else if (e.key === 'ArrowUp') focus(index < 0 ? list.length - 1 : index - 1);
    else if (e.key === 'Home') focus(0);
    else if (e.key === 'End') focus(list.length - 1);
    else if (e.key === 'ArrowRight' && open !== null) { e.preventDefault(); openMenu(at(open + 1), 'first'); }
    else if (e.key === 'ArrowLeft' && open !== null) { e.preventDefault(); openMenu(at(open - 1), 'first'); }
    else if (e.key === 'Escape') { e.preventDefault(); e.stopPropagation(); close(true); }
    else if (e.key === 'Tab') close(false);
  }

  function renderItem(item: MenuItem | MenuSeparator) {
    return isSeparator(item) ? <div key={item.id} role="separator" className="sj-menubar-separator" />
      : <button key={item.id} type="button" tabIndex={-1} role={item.checked === undefined ? 'menuitem' : 'menuitemcheckbox'} aria-checked={item.checked} disabled={item.disabled}
        aria-keyshortcuts={item.shortcut?.replace(/Ctrl/g, 'Control')} className="sj-menubar-item" onClick={() => { setOpen(null); item.run(); }}>
        <span className="sj-menubar-check">{item.checked && <Icon name="check" size={14} />}</span><span>{item.label}</span>{item.shortcut && <kbd className="sj-menubar-shortcut">{item.shortcut}</kbd>}
      </button>;
  }

  return <div ref={root} className="sj-menubar" role="menubar" aria-label={label}>
    {shown.map((menu, index) => <div key={menu.id} className="sj-menubar-menu">
      <button ref={node => { buttons.current[index] = node; }} type="button" role="menuitem" aria-haspopup="menu" aria-expanded={open === index} tabIndex={index === current ? 0 : -1}
        className="sj-menubar-button" onClick={() => open === index ? close(false) : openMenu(index, null)} onKeyDown={e => barKey(e, index)}
        onPointerEnter={() => { if (open !== null && open !== index) openMenu(index, null); }}>{menu.label}</button>
      {open === index && <div ref={popup} role="menu" aria-label={menu.label} className="sj-menubar-popup" onKeyDown={menuKey}>
        {menu.items.map(item => isSubmenu(item)
          ? <div key={item.id} className="sj-menubar-sub" onPointerEnter={() => setSub(item.id)} onPointerLeave={() => setSub(s => s === item.id ? null : s)}>
            <button type="button" tabIndex={-1} role="menuitem" aria-haspopup="menu" aria-expanded={sub === item.id} data-opens={item.id} disabled={item.disabled} className="sj-menubar-item"
              onClick={() => { setSub(item.id); setSubFocus(true); }}><span className="sj-menubar-check" /><span>{item.label}</span><Icon name="chevron-right" size={14} /></button>
            {sub === item.id && <div role="menu" aria-label={item.label} data-submenu={item.id} className="sj-menubar-popup sj-menubar-subpopup">{item.items.map(renderItem)}</div>}
          </div>
          : renderItem(item))}
      </div>}
    </div>)}
  </div>;
}

/** The thin bottom row: the current item's state on the left, small facts on the right. Never ads or tips. */
export function StatusBar({ state, facts = [], label = 'Status bar' }: { state?: ReactNode; facts?: ReactNode[]; label?: string }) {
  return <footer className="sj-status-bar" aria-label={label}>
    <div className="sj-status-state" role="status">{state}</div>
    {facts.length > 0 && <ul className="sj-status-facts">{facts.map((fact, i) => <li key={i}>{fact}</li>)}</ul>}
  </footer>;
}

export type PanelTab = { id: string; label: string; content: ReactNode };
/** A resizable panel under the work area with its own tab row, such as Output and Problems. Place it as the last child of a
 * column whose other child takes the remaining height. Drag the top edge, or focus it and use the up and down arrow keys. */
export function BottomPanel({ tabs, value: controlledValue, onChange, height: controlledHeight, onHeightChange, onClose, label = 'Bottom panel', initialHeight = 160 }: {
  tabs: PanelTab[]; value?: string; onChange?: (id: string) => void;
  height?: number; onHeightChange?: (height: number) => void; initialHeight?: number;
  /** Shows a close control in the tab row. */ onClose?: () => void; label?: string;
}) {
  const [ownValue, setOwnValue] = useState(tabs[0]?.id ?? '');
  const [ownHeight, setOwnHeight] = useState(initialHeight);
  const panel = useRef<HTMLElement>(null);
  const value = controlledValue ?? (tabs.some(t => t.id === ownValue) ? ownValue : tabs[0]?.id ?? '');
  const height = controlledHeight ?? ownHeight;
  const choose = (id: string) => { if (controlledValue === undefined) setOwnValue(id); onChange?.(id); };
  // The panel leaves at least 64 pixels of work area above it.
  const maximum = () => Math.max(0, (panel.current?.parentElement?.clientHeight ?? 400) - 64 - 5);
  const resize = (h: number) => { const next = Math.round(Math.max(0, Math.min(maximum(), h))); if (controlledHeight === undefined) setOwnHeight(next); onHeightChange?.(next); };
  if (!tabs.length) return null;
  return <>
    <div className="sj-bottom-panel-resize" role="separator" tabIndex={0} aria-orientation="horizontal" aria-label={`Resize ${label.toLowerCase()}`} aria-valuenow={height} aria-valuemin={0}
      onKeyDown={e => { if (e.key === 'ArrowUp' || e.key === 'ArrowDown') { e.preventDefault(); resize(height + (e.key === 'ArrowUp' ? 1 : -1) * (e.shiftKey ? 64 : 16)); } }}
      onPointerDown={e => { e.preventDefault(); e.currentTarget.setPointerCapture(e.pointerId); }}
      onPointerMove={e => { if (e.currentTarget.hasPointerCapture(e.pointerId) && panel.current) resize(panel.current.getBoundingClientRect().bottom - e.clientY); }}
      onPointerUp={e => { if (e.currentTarget.hasPointerCapture(e.pointerId)) e.currentTarget.releasePointerCapture(e.pointerId); }} />
    <section ref={panel} className="sj-bottom-panel" aria-label={label} style={{ height }}>
      <Tabs label={`${label} views`} value={value} onChange={choose} tabs={tabs} />
      {onClose && <IconButton className="sj-bottom-panel-close" icon="close" label={`Hide ${label.toLowerCase()}`} variant="quiet" onClick={onClose} />}
    </section>
  </>;
}
