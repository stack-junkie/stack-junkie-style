import { useEffect, useId, useRef, useState, type CSSProperties, type HTMLAttributes, type ReactNode } from 'react';
import { Button, Icon } from './core';

export function Workspace({ header, rail, sidebar, sidebarTitle, toolbar, children, navigationFooter, footer, storageKey, mainId, initialSidebarWidth = 228, contentOwnsHeader = false, resizable = true, collapsible = true, sidebarCollapsed, onSidebarCollapsedChange }: {
  header: ReactNode; rail?: ReactNode; sidebar?: ReactNode; sidebarTitle: string; toolbar?: ReactNode; children: ReactNode | ((state: { sidebarCollapsed: boolean }) => ReactNode); navigationFooter?: ReactNode; storageKey?: string; mainId?: string; initialSidebarWidth?: number; contentOwnsHeader?: boolean;
  /** Let people drag the contextual sidebar to any width. */ resizable?: boolean;
  /** Let people hide the contextual sidebar with the menu control. */ collapsible?: boolean;
  /** A full-width row under the body, such as a StatusBar. */ footer?: ReactNode;
  /** Controlled sidebar collapse, for a frame that also toggles it from a menu. Without it the workspace remembers it itself. */ sidebarCollapsed?: boolean;
  onSidebarCollapsedChange?: (collapsed: boolean) => void;
}) {
  const uid = useId(); const toggle = useRef<HTMLButtonElement>(null); const body = useRef<HTMLDivElement>(null);
  const [width, setWidth] = useState(() => { try { const stored = storageKey ? localStorage.getItem(`${storageKey}:width`) : null; const value = Number(stored); return stored !== null && Number.isFinite(value) ? Math.max(0, value) : initialSidebarWidth; } catch { return initialSidebarWidth; } });
  const [ownCollapsed, setOwnCollapsed] = useState(() => { try { return !!storageKey && localStorage.getItem(`${storageKey}:collapsed`) === 'true'; } catch { return false; } });
  const collapsedState = sidebarCollapsed ?? ownCollapsed;
  const setCollapsed = (value: boolean) => { if (sidebarCollapsed === undefined) setOwnCollapsed(value); onSidebarCollapsedChange?.(value); };
  const [maximum, setMaximum] = useState(initialSidebarWidth);
  const hasSidebar = sidebar !== undefined && sidebar !== null && sidebar !== false;
  const collapsed = collapsible ? collapsedState : false;
  const shown = hasSidebar && !collapsed;
  useEffect(() => {
    const node = body.current; if (!node) return;
    const update = () => {
      const railWidth = node.querySelector('.sj-workspace-rail')?.getBoundingClientRect().width || 0;
      const toggleWidth = toggle.current?.getBoundingClientRect().width || 28;
      setMaximum(Math.max(0, node.clientWidth - railWidth - 5 - toggleWidth - 20));
    };
    const observer = new ResizeObserver(update); observer.observe(node); update();
    return () => observer.disconnect();
  }, [rail]);
  const visibleWidth = Math.min(resizable ? width : initialSidebarWidth, maximum);
  useEffect(() => { if (storageKey) try { localStorage.setItem(`${storageKey}:width`, String(width)); localStorage.setItem(`${storageKey}:collapsed`, String(collapsedState)); } catch { /* Optional storage. */ } }, [storageKey, width, collapsedState]);
  const sidebarToggle = hasSidebar && collapsible ? <button ref={toggle} type="button" className="sj-icon-button sj-button sj-button--quiet sj-sidebar-toggle" aria-controls={uid} aria-expanded={shown} aria-label={shown ? 'Hide navigation' : 'Show navigation'} title={shown ? 'Hide navigation' : 'Show navigation'} onClick={() => setCollapsed(!collapsedState)}><Icon name="menu" /></button> : null;
  return <div className="sj-workspace" style={{ '--sj-sidebar-width': `${visibleWidth}px`, '--sj-workspace-rail-width': rail ? 'var(--sj-density-railWidth)' : '0px' } as CSSProperties}>
    <header className="sj-workspace-header">{header}</header>
    <div ref={body} className="sj-workspace-body" data-sidebar={shown ? 'open' : 'closed'} data-resizable={resizable}>
      {rail && <aside className="sj-workspace-rail" aria-label="Sections">{sidebarToggle}<div className="sj-workspace-rail-scroll">{rail}</div>{navigationFooter && <footer className="sj-navigation-footer">{navigationFooter}</footer>}</aside>}
      {hasSidebar && <aside id={uid} className="sj-workspace-sidebar" hidden={!shown} aria-label={sidebarTitle}>
        <header className="sj-workspace-bar"><strong>{sidebarTitle}</strong></header>
        <div className="sj-workspace-sidebar-body"><div className="sj-workspace-navigation">{sidebar}</div>{!rail && navigationFooter && <footer className="sj-navigation-footer">{navigationFooter}</footer>}</div>
      </aside>}
      {hasSidebar && <div hidden={!shown} className="sj-workspace-resize" role="separator" tabIndex={resizable ? 0 : -1} aria-label={`Resize ${sidebarTitle.toLowerCase()} sidebar`} aria-controls={uid} aria-orientation="vertical" aria-valuemin={0} aria-valuemax={Math.round(maximum)} aria-valuenow={Math.round(visibleWidth)} aria-disabled={!resizable || undefined}
        onKeyDown={e => { if (resizable && ['ArrowLeft', 'ArrowRight', 'Home', 'End'].includes(e.key)) { e.preventDefault(); setWidth(e.key === 'Home' ? 0 : e.key === 'End' ? maximum : Math.max(0, Math.min(maximum, visibleWidth + (e.key === 'ArrowRight' ? 1 : -1) * (e.shiftKey ? 32 : 8)))); } }}
        onPointerDown={e => { if (!resizable) return; e.preventDefault(); e.currentTarget.setPointerCapture(e.pointerId); }}
        onPointerMove={e => { if (e.currentTarget.hasPointerCapture(e.pointerId)) { const panel = document.getElementById(uid)!; setWidth(Math.max(0, Math.min(maximum, e.clientX - panel.getBoundingClientRect().left))); } }}
        onPointerUp={e => { if (e.currentTarget.hasPointerCapture(e.pointerId)) e.currentTarget.releasePointerCapture(e.pointerId); }} />}
      <main id={mainId} tabIndex={-1} className={`sj-workspace-main ${contentOwnsHeader && rail ? 'sj-workspace-main--integrated' : ''}`}>
        {(!contentOwnsHeader || !rail) && ((!rail && sidebarToggle) || toolbar) && <header className="sj-workspace-bar">{!rail && sidebarToggle}{toolbar}</header>}
        <div className="sj-workspace-content">{typeof children === 'function' ? children({ sidebarCollapsed: !shown }): children}</div>
      </main>
    </div>
    {!rail && !shown && navigationFooter && <footer className="sj-navigation-footer">{navigationFooter}</footer>}
    {footer}
  </div>;
}

export function Stack({ children, gap = 16, horizontal = false, className = '', ...props }: HTMLAttributes<HTMLDivElement> & { gap?: number; horizontal?: boolean }) { return <div {...props} className={`sj-stack ${horizontal ? 'sj-stack--horizontal' : ''} ${className}`} style={{ gap, ...props.style }}>{children}</div>; }
export function Toolbar({ children, label, ...props }: HTMLAttributes<HTMLDivElement> & { label: string }) { return <div {...props} className={`sj-toolbar ${props.className || ''}`} aria-label={label}>{children}</div>; }
export function Panel({ title, children, actions, className = '', ...props }: Omit<HTMLAttributes<HTMLElement>, 'title'> & { title?: string; actions?: ReactNode }) { return <section {...props} className={`sj-panel ${className}`}>{(title || actions) && <header className="sj-panel-heading">{title && <h3>{title}</h3>}{actions}</header>}<div className="sj-panel-body">{children}</div></section>; }
export function Disclosure({ label, children, defaultOpen = false }: { label: string; children: ReactNode; defaultOpen?: boolean }) { return <details className="sj-disclosure" open={defaultOpen || undefined}><summary><Icon name="chevron-right" size={16} />{label}</summary><div>{children}</div></details>; }
export function SettingsRow({ label, description, children }: { label: string; description?: string; children: ReactNode }) { return <div className="sj-settings-row"><div><strong>{label}</strong>{description && <p className="sj-muted">{description}</p>}</div><div>{children}</div></div>; }
export function SplitPane({ primary, secondary, secondaryLabel = 'Inspector', initialSize = 32 }: { primary: ReactNode; secondary: ReactNode; secondaryLabel?: string; initialSize?: number }) {
  const [shown, setShown] = useState(true); const [size, setSize] = useState(initialSize); const uid = useId();
  return <section className="sj-split"><div className="sj-split-toolbar"><Button variant="quiet" aria-expanded={shown} aria-controls={uid} onClick={() => setShown(!shown)}><Icon name="settings" />{shown ? `Hide ${secondaryLabel.toLowerCase()}` : `Show ${secondaryLabel.toLowerCase()}`}</Button></div><div className={`sj-split-body ${shown ? '' : 'is-collapsed'}`} style={{ gridTemplateColumns: shown ? `minmax(0, ${100 - size}fr) 12px minmax(160px, ${size}fr)` : 'minmax(0, 1fr)' }}><div className="sj-split-primary">{primary}</div>{shown && <div role="separator" tabIndex={0} aria-label={`Resize ${secondaryLabel.toLowerCase()}`} aria-orientation="vertical" aria-valuenow={size} aria-valuemin={20} aria-valuemax={50} className="sj-split-handle" onKeyDown={e => { if (e.key === 'ArrowLeft' || e.key === 'ArrowRight') { e.preventDefault(); setSize(Math.max(20, Math.min(50, size + (e.key === 'ArrowLeft' ? 2 : -2)))); } else if (e.key === 'Home') setSize(20); else if (e.key === 'End') setSize(50); }} onPointerDown={e => { e.preventDefault(); e.currentTarget.setPointerCapture(e.pointerId); }} onPointerMove={e => { if (e.currentTarget.hasPointerCapture(e.pointerId)) { const rect = e.currentTarget.parentElement!.getBoundingClientRect(); setSize(Math.max(20, Math.min(50, ((rect.right - e.clientX) / rect.width) * 100))); } }} onPointerUp={e => { if (e.currentTarget.hasPointerCapture(e.pointerId)) e.currentTarget.releasePointerCapture(e.pointerId); }} /> }<aside id={uid} hidden={!shown} className="sj-split-secondary" aria-label={secondaryLabel}>{secondary}</aside></div></section>;
}
