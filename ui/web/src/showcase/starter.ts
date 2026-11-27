import { systemVersion, type PresetChoices, type PresetId } from '../library';
import manifest from '../../../shared/starters/starters.json';
import presets from '../../../shared/presets.json';
import { generateStarter, type StarterPlatform, type StarterResult } from '../../../shared/starters/starters.mjs';

// Showcase only: Export hands the shared starter generator this preset and its changed choices.
const templates = import.meta.glob<string>('../../../shared/starters/**/*.tmpl', { query: '?raw', import: 'default', eager: true });
const template = (path: string) => {
  const text = templates[`../../../shared/starters/${path}`];
  if (text === undefined) throw new Error(`Missing starter template ${path}`);
  return text;
};

export const starterKinds = (['web', 'native'] as const).map(platform => ({ platform, label: manifest[platform].label, description: manifest[platform].description }));
export function starter(platform: StarterPlatform, preset: PresetId, overrides: Partial<PresetChoices>): StarterResult {
  return generateStarter({ platform, preset, overrides, version: systemVersion }, { manifest, presets, template });
}
