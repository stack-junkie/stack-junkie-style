import data from '../../../shared/presets.json';

// Presets are read from the shared definition, the only source of preset choices for web, native and the skill.
export type PresetId = 'web-app' | 'desktop-app' | 'document-editor' | 'website';
export type SidebarChoice = 'tree' | 'list' | 'off';
export type ThemeChoice = 'dark' | 'light' | 'system';
export type DensityChoice = 'compact' | 'comfortable';
export type PresetChoices = {
  rail: boolean; sidebar: SidebarChoice; menuBar: boolean; statusBar: boolean; bottomPanel: boolean;
  documentTabs: boolean; commandPalette: boolean; theme: ThemeChoice; density: DensityChoice; dashboardHome: boolean;
};
export type PresetChoice = keyof PresetChoices;
export type Preset = { id: PresetId; label: string; description: string; choices: PresetChoices };
/** Label, description and allowed values of each choice, for building settings controls. */
export type PresetChoiceInfo = { label: string; description: string; options: readonly (string | boolean)[] };

export const presets = data.presets as Preset[];
export const presetChoices = data.choices as Record<PresetChoice, PresetChoiceInfo>;
export const presetIds = presets.map(p => p.id);
export const isPresetId = (id: unknown): id is PresetId => presetIds.includes(id as PresetId);

/** The preset's choices with any overrides applied. Unknown ids fall back to Web app. */
export function resolvePreset(id: PresetId, overrides: Partial<PresetChoices> = {}): PresetChoices {
  const preset = presets.find(p => p.id === id) ?? presets[0];
  const choices = { ...preset.choices };
  for (const [key, value] of Object.entries(overrides) as [PresetChoice, PresetChoices[PresetChoice]][]) {
    if (value !== undefined && presetChoices[key]?.options.includes(value)) (choices as Record<PresetChoice, unknown>)[key] = value;
  }
  return choices;
}

/** Only the choices that differ from the preset, for saving or exporting a compact set of overrides. */
export function presetOverrides(id: PresetId, choices: Partial<PresetChoices>): Partial<PresetChoices> {
  const base = resolvePreset(id);
  return Object.fromEntries(Object.entries(choices).filter(([key, value]) => base[key as PresetChoice] !== value)) as Partial<PresetChoices>;
}
