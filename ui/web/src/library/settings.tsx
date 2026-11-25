import type { ReactNode } from 'react';
import { BrandMark } from './BrandMark';
import release from '../../../generated/version.json';
import attribution from '../../../shared/attribution.json';

export const systemVersion = release.version;

export function SettingsPage({ title, children }: { title: string; children: ReactNode }) {
  return <section className="sj-settings-page" aria-label={title}><div className="sj-settings-page-inner"><h2>{title}</h2>{children}</div></section>;
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
  return <SettingsPage title="About">
    <div className="sj-about-product"><div className="sj-about-product-name">{logo}<h2>{name}</h2></div>
      <p>{description}</p>{version && <p className="sj-muted">Version {version}</p>}{children}
    </div><StyleAttribution />
  </SettingsPage>;
}
