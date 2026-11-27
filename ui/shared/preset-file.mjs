// The custom preset file: a name, a base preset and only the choices that differ from it.
// The format is described in PRESET-FILE.md. The web library and the test script call this module;
// the native library (src/presets.cpp, CustomPreset) applies the same rules and writes the same text.
// Allowed preset ids, choice names and values always come from presets.json.

export const presetFileFormat = 'stack-junkie-preset';
export const presetFileVersion = 1;
export const presetNameMaxLength = 60;
const fileKeys = ['format', 'version', 'name', 'base', 'choices'];

/** A plain error message for a bad name, or an empty string when the name is usable. Names are trimmed first. */
export function presetNameError(name) {
  if (typeof name !== 'string' || !name.trim()) return 'The preset needs a name.';
  if (name.trim().length > presetNameMaxLength) return `The preset name is longer than ${presetNameMaxLength} characters.`;
  if (/[\u0000-\u001f\u007f]/.test(name)) return 'The preset name cannot contain line breaks or control characters.';
  return '';
}

const shown = value => typeof value === 'string' ? `"${value}"` : JSON.stringify(value);

/**
 * Checks a custom preset and returns it in canonical form: the name trimmed, and only the choices that differ
 * from the base, in presets.json order. Nothing is changed or applied on failure.
 * Returns { preset: { name, base, choices } } or { error }.
 */
export function checkCustomPreset({ name, base, choices }, presets) {
  const nameError = presetNameError(name);
  if (nameError) return { error: nameError };
  const basePreset = presets.presets.find(p => p.id === base);
  if (typeof base !== 'string') return { error: `The preset needs a base preset: one of ${presets.presets.map(p => p.id).join(', ')}.` };
  if (!basePreset) return { error: `Unknown base preset ${shown(base)}. Use one of: ${presets.presets.map(p => p.id).join(', ')}.` };
  if (choices === null || typeof choices !== 'object' || Array.isArray(choices)) return { error: 'The preset choices must be an object.' };
  for (const [key, value] of Object.entries(choices)) {
    const info = presets.choices[key];
    if (!info) return { error: `Unknown choice ${shown(key)}.` };
    if (!info.options.includes(value)) return { error: `The ${key} choice cannot be ${shown(value)}. Use one of: ${info.options.map(String).join(', ')}.` };
  }
  const changed = Object.keys(presets.choices).filter(key => key in choices && choices[key] !== basePreset.choices[key]);
  return { preset: { name: name.trim(), base, choices: Object.fromEntries(changed.map(key => [key, choices[key]])) } };
}

/** The file text for a custom preset: two-space JSON with a final newline. Throws on an invalid preset. */
export function presetFileText(preset, presets) {
  const checked = checkCustomPreset(preset, presets);
  if (checked.error) throw new Error(checked.error);
  const { name, base, choices } = checked.preset;
  return `${JSON.stringify({ format: presetFileFormat, version: presetFileVersion, name, base, choices }, null, 2)}\n`;
}

/** Reads a preset file's text. Returns { preset: { name, base, choices } } or { error } with a plain message. */
export function parsePresetFile(text, presets) {
  let data;
  try { data = JSON.parse(String(text).replace(/^﻿/, '')); } catch { return { error: 'The file is not valid JSON.' }; }
  if (data === null || typeof data !== 'object' || Array.isArray(data) || data.format !== presetFileFormat) return { error: 'This is not a Stack Junkie preset file.' };
  if (data.version !== presetFileVersion) return { error: `The preset file is version ${shown(data.version)}. This version reads version ${presetFileVersion}.` };
  const extra = Object.keys(data).find(key => !fileKeys.includes(key));
  if (extra) return { error: `Unknown field ${shown(extra)} in the preset file.` };
  if (!('choices' in data)) return { error: 'The preset file has no choices.' };
  return checkCustomPreset(data, presets);
}
