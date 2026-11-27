import { presetOverrides, resolvePreset, systemVersion } from '../library';
import type { FrameSettings } from './layouts';

const literal = (value: unknown) => JSON.stringify(value, null, 2).replace(/"(\w+)":/g, '$1:').replace(/"/g, "'");

// Showcase only: writes the starter file a new project imports. It references the library; it never copies component code.
export function exportApp({ preset, choices, config }: FrameSettings) {
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
