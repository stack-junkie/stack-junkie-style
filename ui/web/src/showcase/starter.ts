import { systemVersion, type PresetChoices, type PresetId } from '../library';
import manifest from '../../../shared/starters/starters.json';
import presets from '../../../shared/presets.json';
import { generateStarter, logoFileName, starterCommand, type StarterPlatform, type StarterResult } from '../../../shared/starters/starters.mjs';

// Showcase only: the Builder hands the shared starter generator this preset, its changed choices and the logo.
const templates = import.meta.glob<string>('../../../shared/starters/**/*.tmpl', { query: '?raw', import: 'default', eager: true });
const template = (path: string) => {
  const text = templates[`../../../shared/starters/${path}`];
  if (text === undefined) throw new Error(`Missing starter template ${path}`);
  return text;
};

export { manifest as starterManifest, logoFileName };
export type StarterLogo = { name: string; data: Uint8Array };
export const starterKinds = (['web', 'native'] as const).map(platform => ({ platform, label: manifest[platform].label, description: manifest[platform].description }));
export function starter(platform: StarterPlatform, preset: PresetId, overrides: Partial<PresetChoices>, logo?: StarterLogo): StarterResult {
  return generateStarter({ platform, preset, overrides, version: systemVersion, logo }, { manifest, presets, template });
}
/** The `new` command that writes the same starter as `starter` for the same input. */
export function command(platform: StarterPlatform, preset: PresetId, overrides: Partial<PresetChoices>, logoName?: string): string {
  return starterCommand({ platform, preset, overrides, version: systemVersion, logoName }, { manifest, presets });
}
