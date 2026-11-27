import { useState, type ReactNode } from 'react';
import { BrandMark } from './BrandMark';
import { Dialog } from './overlays';
import { TextInput } from './inputs';
import release from '../../../generated/version.json';
import attribution from '../../../shared/attribution.json';

export const systemVersion = release.version;

export function SettingsPage({ title, children }: { title: string; children: ReactNode }) {
  return <section className="sj-settings-page" aria-label={title}><div className="sj-settings-page-inner"><h2>{title}</h2>{children}</div></section>;
}

// One section of the Settings window. The highlighted section list already names it, so it carries no repeated heading.
export function SettingsSection({ label, children }: { label: string; children: ReactNode }) {
  return <section className="sj-settings-section" aria-label={label}>{children}</section>;
}

export type SettingsWindowSection = { id: string; label: string; keywords?: string; content: ReactNode };
// Settings opens as a window over the app, never as a page in the app's navigation. Sections on the left, search above them,
// changes apply immediately, so the only action is closing the window.
export function SettingsWindow({ open, onClose, sections, section, onSectionChange, title = 'Settings' }: { open: boolean; onClose: () => void; sections: SettingsWindowSection[]; section: string; onSectionChange: (id: string) => void; title?: string }) {
  const [query, setQuery] = useState(''); const needle = query.trim().toLowerCase();
  const matches = needle ? sections.filter(s => `${s.label} ${s.keywords || ''}`.toLowerCase().includes(needle)): sections;
  const current = matches.some(s => s.id === section) ? section : matches[0]?.id;
  return <Dialog open={open} onClose={onClose} title={title} size="large">
    <div className="sj-settings-window">
      <nav className="sj-settings-window-nav" aria-label="Settings sections">
        <TextInput label="Search settings" type="search" placeholder="Search settings" value={query} onChange={e => setQuery(e.target.value)} />
        <div className="sj-side-nav">{matches.map(s => <button type="button" key={s.id} className="sj-nav-item" aria-current={current === s.id ? 'page' : undefined} onClick={() => onSectionChange(s.id)}>{s.label}</button>)}</div>
        {matches.length === 0 && <p className="sj-muted" role="status">No settings match.</p>}
      </nav>
      <div className="sj-settings-window-body">{sections.map(s => <div key={s.id} hidden={s.id !== current}>{s.content}</div>)}</div>
    </div>
  </Dialog>;
}

export function StyleAttribution() {
  return <footer className="sj-about-attribution" aria-label="UI system attribution">
    <BrandMark size={22} /><div><p><strong>{attribution.name}</strong> · {systemVersion}</p>
      <p><a href={attribution.website}>{attribution.projectLine}</a></p>
      <p>© {new Date().getFullYear()} {attribution.copyrightHolder}</p>
    </div>
  </footer>;
}

export function AboutPage({ name, description, version, logo, children }: { name: string; description: string; version?: string; logo?: ReactNode; children?: ReactNode }) {
  return <SettingsPage title="About"><AboutContent name={name} description={description} version={version} logo={logo}>{children}</AboutContent></SettingsPage>;
}
export function AboutSection({ name, description, version, logo, children }: { name: string; description: string; version?: string; logo?: ReactNode; children?: ReactNode }) {
  return <SettingsSection label="About"><AboutContent name={name} description={description} version={version} logo={logo}>{children}</AboutContent></SettingsSection>;
}
function AboutContent({ name, description, version, logo, children }: { name: string; description: string; version?: string; logo?: ReactNode; children?: ReactNode }) {
  return <><div className="sj-about-product"><div className="sj-about-product-name">{logo}<h2>{name}</h2></div>
    <p>{description}</p>{version && <p className="sj-muted">Version {version}</p>}{children}
  </div><StyleAttribution /></>;
}
