import { systemVersion, type ShellConfig } from '../library';

// Showcase only: writes the starter file a new project imports. It references the library; it never copies component code.
export function exportApp(config: ShellConfig) {
  return `// App.tsx, exported from the Stack Junkie UI showcase.
// It imports the shared library instead of copying it, so every app stays on the same components.
// Pin the library in package.json: "@stack-junkie/ui-web": "${systemVersion}"
import { AppShell, BrandMark, type ShellConfig, type ShellSection } from '@stack-junkie/ui-web';
import '@stack-junkie/ui-web/styles.css';

export const shellConfig: ShellConfig = ${JSON.stringify(config, null, 2).replace(/"(\w+)":/g, '$1:').replace(/"/g, "'")};

// Your app's sections (rail icons) and their pages (sidebar entries).
const sections: ShellSection[] = [
  { id: 'home', label: 'Home', icon: 'grid', pages: [{ id: 'overview', label: 'Overview', content: <p>Start here.</p> }] },
];

export default function App() {
  // Replace BrandMark with the app's own logo when it has one.
  return <AppShell name="My app" description="What this app does." version="0.1.0" logo={<BrandMark size={20} />}
    config={shellConfig} sections={sections} storageKey="my-app" style={{ height: '100dvh' }} />;
}
`;
}
