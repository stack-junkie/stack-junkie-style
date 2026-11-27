import { presetOverrides, resolvePreset, systemVersion } from '../library';
import type { FrameSettings } from './layouts';

const literal = (value: unknown) => JSON.stringify(value, null, 2).replace(/"(\w+)":/g, '$1:').replace(/"/g, "'");

// Showcase only: writes the starter file a new project imports. It references the library; it never copies component code.
export function exportApp({ preset, choices, config }: FrameSettings) {
  if (preset === 'website') return exportWebsite(choices);
  const changed = presetOverrides(preset, choices);
  const resolved = resolvePreset(preset, choices);
  return `// App.tsx, exported from the Stack Junkie UI showcase.
// It imports the shared library instead of copying it, so every app stays on the same components.
// Pin the library in package.json: "@stack-junkie/ui-web": "${systemVersion}"
import { AppShell, BrandMark, defaultShellConfig, type PresetChoices, type PresetId, type ShellConfig, type ShellSection } from '@stack-junkie/ui-web';
import '@stack-junkie/ui-web/styles.css';

// The preset sets the App frame. Choices list only what was changed from it; delete one to go back to the preset.
export const preset: PresetId = '${preset}';
export const choices: Partial<PresetChoices> = ${literal(changed)};
export const shellConfig: ShellConfig = { ...defaultShellConfig, resizableSidebar: ${config.resizableSidebar}, collapsibleSidebar: ${config.collapsibleSidebar} };

// Your app's sections (rail icons) and their pages (sidebar entries).
const sections: ShellSection[] = [
  { id: 'home', label: 'Home', icon: 'grid', pages: [{ id: 'overview', label: 'Overview', content: <p>Start here.</p> }] },
];

export default function App() {
  // Replace BrandMark with the app's own logo when it has one.${resolved.commandPalette ? `
  // The command palette is on: pass a CommandPalette in overlays, opened with Control K.` : ''}
  return <AppShell name="My app" description="What this app does." version="0.1.0" logo={<BrandMark size={20} />}
    preset={preset} choices={choices} config={shellConfig} sections={sections} storageKey="my-app" style={{ height: '100dvh' }} />;
}
`;
}

// The Website preset gets its own layout instead of the App frame. Only its theme and density choices apply.
function exportWebsite(choices: FrameSettings['choices']) {
  const { theme, density } = resolvePreset('website', choices);
  return `// App.tsx, exported from the Stack Junkie UI showcase.
// It imports the shared library instead of copying it, so every site stays on the same components.
// Pin the library in package.json: "@stack-junkie/ui-web": "${systemVersion}"
import { BrandMark, Prose, WebsiteLayout, type WebsitePage } from '@stack-junkie/ui-web';
import '@stack-junkie/ui-web/styles.css';

// The Website preset uses WebsiteLayout, not the App frame: a header with navigation, one reading column, a footer and a theme toggle.
// Your site's pages, in navigation order. Give a page an href when the site has real addresses.
const pages: WebsitePage[] = [
  { id: 'home', label: 'Home', content: <Prose><h1>My site</h1><p>Start here.</p></Prose> },
  { id: 'about', label: 'About', content: <Prose><h1>About</h1><p>Who runs this site.</p></Prose> },
];

export default function App() {
  // Replace BrandMark with the site's own logo when it has one.
  return <WebsiteLayout name="My site" logo={<BrandMark size={20} />} pages={pages} theme="${theme}" density="${density}" storageKey="my-site-theme" style={{ minHeight: '100dvh' }} />;
}
`;
}
