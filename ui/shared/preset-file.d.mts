export type PresetFileChoices = Record<string, string | boolean>;
export type PresetFilePreset = { name: string; base: string; choices: PresetFileChoices };
export type PresetFileResult = { preset: PresetFilePreset; error?: undefined } | { preset?: undefined; error: string };
export type PresetFileSource = {
  choices: Record<string, { options: readonly (string | boolean)[] }>;
  presets: readonly { id: string; choices: Record<string, string | boolean> }[];
};

export const presetFileFormat: 'stack-junkie-preset';
export const presetFileVersion: 1;
export const presetNameMaxLength: number;
/** A plain error message for a bad name, or an empty string when the name is usable. */
export function presetNameError(name: unknown): string;
/** Checks a custom preset and returns it trimmed, with only the choices that differ from the base. */
export function checkCustomPreset(preset: { name: unknown; base: unknown; choices: unknown }, presets: PresetFileSource): PresetFileResult;
/** The file text for a custom preset. Throws on an invalid preset. */
export function presetFileText(preset: { name: string; base: string; choices: Partial<PresetFileChoices> }, presets: PresetFileSource): string;
/** Reads a preset file's text, or says plainly why it cannot be used. */
export function parsePresetFile(text: string, presets: PresetFileSource): PresetFileResult;
