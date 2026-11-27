import { useEffect, useId, useState, type CSSProperties, type MouseEvent, type ReactNode } from 'react';
import { ThemeProvider, type Density, type ThemeMode } from './theme';
import { IconButton } from './core';
import { useSystemTheme } from './shell';
import type { ThemeChoice } from './presets';

// The Website layout: a header with the logo and site navigation, one reading-width column, a footer and a small theme toggle.
// It is the layout the Website preset uses instead of the App frame. It has no rail, no sidebar and no Settings window.
export type WebsitePage = { id: string; label: string;
  /** A real address for the page. Without one the link stays on this page and the layout switches content itself. */ href?: string;
  /** What the main column shows while this page is current. Children replace it. */ content?: ReactNode };
export type WebsiteLink = { label: string; href: string };

function readSaved(key: string | undefined): ThemeMode | undefined {
  if (!key) return undefined;
  try { const value = localStorage.getItem(key); return value === 'dark' || value === 'light' ? value : undefined; } catch { return undefined; }
}

export function WebsiteLayout({ name, logo, brand, pages, page: controlledPage, onNavigate, children, footerLinks, copyright, theme = 'system',
  mode: controlledMode, onModeChange, storageKey = 'sj-website', density = 'comfortable', mainId = 'main', skipLabel = 'Skip to content', className = '', style }: {
  name: string; logo?: ReactNode;
  /** Replaces the default logo-and-name link in the header. */ brand?: ReactNode;
  /** The site's pages, in navigation order. The header and, by default, the footer link to them. */ pages: WebsitePage[];
  /** Controlled current page id. Without it the layout remembers the current page itself. */ page?: string;
  onNavigate?: (id: string) => void;
  /** Replaces the current page's content in the main column. */ children?: ReactNode;
  /** Footer links. Defaults to the pages. */ footerLinks?: WebsiteLink[];
  /** The footer's last line. Defaults to the year and the site name. */ copyright?: ReactNode;
  /** The theme a new visitor sees. System follows the device until the visitor picks one with the toggle. */ theme?: ThemeChoice;
  mode?: ThemeMode; onModeChange?: (mode: ThemeMode) => void;
  /** Local storage key for the visitor's theme choice. Pass an empty string to keep the choice for this visit only. */ storageKey?: string;
  density?: Density; mainId?: string; skipLabel?: string; className?: string; style?: CSSProperties;
}) {
  const systemMode = useSystemTheme();
  const [picked, setPicked] = useState<ThemeMode | undefined>(() => readSaved(storageKey));
  const mode = controlledMode ?? picked ?? (theme === 'system' ? systemMode : theme);
  function toggleTheme() {
    const next: ThemeMode = mode === 'dark' ? 'light' : 'dark';
    if (controlledMode === undefined) setPicked(next);
    if (storageKey) { try { localStorage.setItem(storageKey, next); } catch { /* Optional storage. */ } }
    onModeChange?.(next);
  }

  const [ownPage, setOwnPage] = useState(pages[0]?.id ?? '');
  const current = pages.find(p => p.id === (controlledPage ?? ownPage)) ?? pages[0];
  const [menuOpen, setMenuOpen] = useState(false);
  const navId = useId();
  useEffect(() => {
    if (!menuOpen) return;
    function key(e: KeyboardEvent) { if (e.key === 'Escape') setMenuOpen(false); }
    document.addEventListener('keydown', key); return () => document.removeEventListener('keydown', key);
  }, [menuOpen]);
  function go(target: WebsitePage, e: MouseEvent<HTMLAnchorElement>) {
    // Pages without an address switch in place; pages with one follow the link and still report the change.
    if (!target.href) e.preventDefault();
    if (controlledPage === undefined) setOwnPage(target.id);
    setMenuOpen(false);
    onNavigate?.(target.id);
  }
  const link = (p: WebsitePage) => <a href={p.href ?? `#${p.id}`} aria-current={p.id === current?.id ? 'page' : undefined} onClick={e => go(p, e)}>{p.label}</a>;
  const home = pages[0];
  const brandLink = brand ?? <a className="sj-website-brand" href={home?.href ?? `#${home?.id ?? ''}`} onClick={e => home && go(home, e)}>{logo}<strong>{name}</strong></a>;
  const footer = footerLinks ?? pages.map(p => ({ label: p.label, href: p.href ?? `#${p.id}` }));

  return <ThemeProvider mode={mode} density={density} className={`sj-website ${className}`} style={style}>
    <a className="sj-skip-link" href={`#${mainId}`}>{skipLabel}</a>
    <header className="sj-website-header"><div className="sj-website-column sj-website-bar">
      {brandLink}
      <nav id={navId} className="sj-website-nav" aria-label={`${name} pages`} data-open={menuOpen || undefined}>
        <ul>{pages.map(p => <li key={p.id}>{link(p)}</li>)}</ul>
      </nav>
      <IconButton className="sj-website-theme" variant="quiet" icon={mode === 'dark' ? 'sun' : 'moon'} label={mode === 'dark' ? 'Use light theme' : 'Use dark theme'} onClick={toggleTheme} />
      <IconButton className="sj-website-menu" variant="quiet" icon={menuOpen ? 'close' : 'menu'} label={menuOpen ? 'Close menu' : 'Open menu'} aria-expanded={menuOpen} aria-controls={navId} onClick={() => setMenuOpen(o => !o)} />
    </div></header>
    <main id={mainId} className="sj-website-column sj-website-main" tabIndex={-1}>{children ?? current?.content}</main>
    <footer className="sj-website-footer"><div className="sj-website-column">
      {footer.length > 0 && <nav aria-label="Footer"><ul>{footer.map(l => <li key={l.href + l.label}><a href={l.href} onClick={e => { const p = pages.find(x => (x.href ?? `#${x.id}`) === l.href); if (p) go(p, e); }}>{l.label}</a></li>)}</ul></nav>}
      <p className="sj-muted">{copyright ?? <>&copy; {new Date().getFullYear()} {name}</>}</p>
    </div></footer>
  </ThemeProvider>;
}
