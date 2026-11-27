import { useEffect, useId, useState, type CSSProperties, type MouseEvent, type ReactNode } from 'react';
import { ThemeProvider, type Density, type ThemeMode } from './theme';
import { Button, IconButton } from './core';
import { useSystemTheme } from './shell';
import type { SiteNavigationChoice, ThemeChoice } from './presets';

// The Website layout: a header with the logo, the site's navigation, header actions and a small theme toggle; one
// reading-width column; and a Site footer that links to every page. It is the layout the Website preset uses instead
// of the App frame. It has no rail and no Settings window. Navigation sits in the top bar or in a sidebar beside the column.
export type WebsitePage = { id: string; label: string;
  /** A real address for the page. Without one the link stays on this page and the layout switches content itself. */ href?: string;
  /** What the main column shows while this page is current. Children replace it. */ content?: ReactNode;
  /** A heading the page sits under in the sidebar navigation and the Site footer, such as Articles. */ group?: string;
  /** Leave the page out of the top bar, as for single articles. The sidebar and Site footer still list it. */ topBar?: boolean };
export type WebsiteLink = { label: string; href: string };
/** The header's one main button, such as Subscribe. */
export type WebsiteAction = { label: string; href?: string; onClick?: () => void };

function readSaved(key: string | undefined): ThemeMode | undefined {
  if (!key) return undefined;
  try { const value = localStorage.getItem(key); return value === 'dark' || value === 'light' ? value : undefined; } catch { return undefined; }
}
const linkOf = (p: WebsitePage) => p.href ?? `#${p.id}`;
// Pages in order, grouped by their group; ungrouped pages come first under no heading.
function grouped(pages: WebsitePage[]) {
  const groups: { label?: string; pages: WebsitePage[] }[] = [];
  for (const page of pages) {
    const group = groups.find(g => g.label === page.group);
    if (group) group.pages.push(page); else groups.push({ label: page.group, pages: [page] });
  }
  return groups;
}

export function WebsiteLayout({ name, logo, brand, pages, page: controlledPage, onNavigate, children, footerLinks, copyright, theme = 'system',
  mode: controlledMode, onModeChange, storageKey = 'sj-website', density = 'comfortable', mainId = 'main', skipLabel = 'Skip to content', className = '', style,
  navigation = 'top', footer = true, onSearch, searchLabel = 'Search', mainAction, actions }: {
  name: string; logo?: ReactNode;
  /** Replaces the default logo-and-name link in the header. */ brand?: ReactNode;
  /** The site's pages, in navigation order. The navigation and the Site footer link to them. */ pages: WebsitePage[];
  /** Controlled current page id. Without it the layout remembers the current page itself. */ page?: string;
  onNavigate?: (id: string) => void;
  /** Replaces the current page's content in the main column. */ children?: ReactNode;
  /** Site footer links. Defaults to every page, grouped. */ footerLinks?: WebsiteLink[];
  /** The Site footer's last line. Defaults to the year and the site name. */ copyright?: ReactNode;
  /** The theme a new visitor sees. System follows the device until the visitor picks one with the toggle. */ theme?: ThemeChoice;
  mode?: ThemeMode; onModeChange?: (mode: ThemeMode) => void;
  /** Local storage key for the visitor's theme choice. Pass an empty string to keep the choice for this visit only. */ storageKey?: string;
  density?: Density; mainId?: string; skipLabel?: string; className?: string; style?: CSSProperties;
  /** Where the pages are listed: a top bar in the header, or a sidebar beside the reading column. */ navigation?: SiteNavigationChoice;
  /** The Site footer at the end of every page. Off leaves only the reading column. */ footer?: boolean;
  /** Shows a search button in the header that calls this. */ onSearch?: () => void; searchLabel?: string;
  /** The header's one main button, such as Subscribe. */ mainAction?: WebsiteAction;
  /** More header actions, placed before the main button. */ actions?: ReactNode;
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
  function go(target: WebsitePage, e?: MouseEvent<HTMLElement>) {
    // Pages without an address switch in place; pages with one follow the link and still report the change.
    if (!target.href) e?.preventDefault();
    if (controlledPage === undefined) setOwnPage(target.id);
    setMenuOpen(false);
    onNavigate?.(target.id);
  }
  // Links to a page inside the content, such as an article on the home page, switch pages the same way.
  function followContentLink(e: MouseEvent<HTMLElement>) {
    const anchor = (e.target as Element).closest('a[href]');
    const target = anchor && pages.find(p => linkOf(p) === anchor.getAttribute('href'));
    if (target) { go(target, e); document.getElementById(mainId)?.focus({ preventScroll: true }); }
  }
  const link = (p: WebsitePage) => <a href={linkOf(p)} aria-current={p.id === current?.id ? 'page' : undefined} onClick={e => go(p, e)}>{p.label}</a>;
  const home = pages[0];
  const brandLink = brand ?? <a className="sj-website-brand" href={home ? linkOf(home) : '#'} onClick={e => home && go(home, e)}>{logo}<strong>{name}</strong></a>;
  const topPages = pages.filter(p => p.topBar !== false);
  const footerGroups = footerLinks ? [{ label: undefined, links: footerLinks }] : grouped(pages).map(g => ({ label: g.label, links: g.pages.map(p => ({ label: p.label, href: linkOf(p) })) }));
  const sidebarNav = navigation === 'sidebar';

  const navList = sidebarNav
    ? grouped(pages).map((g, i) => <div key={g.label ?? `pages-${i}`} className="sj-website-nav-group">{g.label && <p className="sj-website-nav-heading">{g.label}</p>}<ul>{g.pages.map(p => <li key={p.id}>{link(p)}</li>)}</ul></div>)
    : <ul>{topPages.map(p => <li key={p.id}>{link(p)}</li>)}</ul>;
  const nav = <nav id={navId} className="sj-website-nav" aria-label={`${name} pages`} data-open={menuOpen || undefined}>{navList}</nav>;

  return <ThemeProvider mode={mode} density={density} className={`sj-website ${className}`} style={style} data-navigation={navigation}>
    <a className="sj-skip-link" href={`#${mainId}`}>{skipLabel}</a>
    <header className="sj-website-header"><div className="sj-website-bar">
      {brandLink}
      {!sidebarNav && nav}
      <div className="sj-website-actions">
        {onSearch && <IconButton variant="quiet" icon="search" label={searchLabel} onClick={onSearch} />}
        {actions}
        <IconButton className="sj-website-theme" variant="quiet" icon={mode === 'dark' ? 'sun' : 'moon'} label={mode === 'dark' ? 'Use light theme' : 'Use dark theme'} onClick={toggleTheme} />
        {mainAction && (mainAction.href
          ? <a className="sj-button sj-button--primary sj-website-main-action" href={mainAction.href} onClick={mainAction.onClick}>{mainAction.label}</a>
          : <Button variant="primary" className="sj-website-main-action" onClick={mainAction.onClick}>{mainAction.label}</Button>)}
        <IconButton className="sj-website-menu" variant="quiet" icon={menuOpen ? 'close' : 'menu'} label={menuOpen ? 'Close menu' : 'Open menu'} aria-expanded={menuOpen} aria-controls={navId} onClick={() => setMenuOpen(o => !o)} />
      </div>
    </div></header>
    <div className="sj-website-body">
      {sidebarNav && <aside className="sj-website-sidebar" aria-label={`${name} sections`}>{nav}</aside>}
      <main id={mainId} className="sj-website-column sj-website-main" tabIndex={-1} onClick={followContentLink}>{children ?? current?.content}</main>
    </div>
    {footer && <footer className="sj-website-footer" aria-label="Site footer"><div className="sj-website-footer-inner">
      {footerGroups.some(g => g.links.length) && <nav aria-label="All pages" className="sj-website-footer-nav">{footerGroups.map((g, i) => <div key={g.label ?? `links-${i}`}>
        {g.label && <p className="sj-website-nav-heading">{g.label}</p>}
        <ul>{g.links.map(l => <li key={l.href + l.label}><a href={l.href} onClick={e => { const p = pages.find(x => linkOf(x) === l.href); if (p) go(p, e); }}>{l.label}</a></li>)}</ul>
      </div>)}</nav>}
      <p className="sj-muted">{copyright ?? <>&copy; {new Date().getFullYear()} {name}</>}</p>
    </div></footer>}
  </ThemeProvider>;
}

/** One article in a list: its title, a short summary and when it was published. Link it with href, such as `#first-article`. */
export type ArticleSummary = { id: string; title: ReactNode; summary?: ReactNode; date?: ReactNode; href?: string };

/** A Website home page: one featured article, then a list of recent ones. */
export function WebsiteHome({ featured, recent = [], recentTitle = 'Recent', intro }: { featured?: ArticleSummary; recent?: ArticleSummary[]; recentTitle?: string; intro?: ReactNode }) {
  return <div className="sj-website-home">
    {intro && <div className="sj-website-intro">{intro}</div>}
    {featured && <article className="sj-website-featured">
      {featured.date && <p className="sj-website-date">{featured.date}</p>}
      <h1><a href={featured.href ?? `#${featured.id}`}>{featured.title}</a></h1>
      {featured.summary && <p className="sj-website-summary">{featured.summary}</p>}
      <a className="sj-website-more" href={featured.href ?? `#${featured.id}`}>Read the article</a>
    </article>}
    {recent.length > 0 && <section aria-label={recentTitle} className="sj-website-recent"><h2>{recentTitle}</h2><WebsiteArticleList articles={recent} /></section>}
  </div>;
}

/** A list of articles, newest first: title, date and summary for each. */
export function WebsiteArticleList({ articles }: { articles: ArticleSummary[] }) {
  return <ul className="sj-website-articles">{articles.map(a => <li key={a.id}>
    <a href={a.href ?? `#${a.id}`}>{a.title}</a>{a.date && <span className="sj-website-date">{a.date}</span>}{a.summary && <p className="sj-website-summary">{a.summary}</p>}
  </li>)}</ul>;
}

/** A Website article page: the title, its date and byline, then the article in the reading column. */
export function WebsiteArticle({ title, date, byline, children }: { title: ReactNode; date?: ReactNode; byline?: ReactNode; children: ReactNode }) {
  return <article className="sj-website-article sj-prose">
    <header><h1>{title}</h1>{(date || byline) && <p className="sj-website-date">{date}{date && byline ? ' · ' : ''}{byline}</p>}</header>
    {children}
  </article>;
}
