export type StarterPlatform = 'web' | 'native';
export type StarterFile = { path: string; text: string };
export type StarterResult = { files: StarterFile[]; unsupported?: undefined } | { files?: undefined; unsupported: string };
export type StarterKind = { label: string; description: string; files: Record<string, string>; presetFiles?: Record<string, Record<string, string>>; unsupported?: Record<string, string> };
export type StarterPage = { id: string; label: string; content: string };
export type StarterArea = { id: string; label: string; icon: string; pages: StarterPage[] };
export type StarterManifest = { repository: string; structure: { note?: string; areas: StarterArea[] }; web: StarterKind; native: StarterKind };
export type StarterPresets = {
  choices: Record<string, { options: readonly (string | boolean)[] }>;
  presets: readonly { id: string; label: string; choices: Record<string, string | boolean> }[];
};
export type StarterRequest = { platform: StarterPlatform; preset: string; overrides?: Record<string, string | boolean | undefined>; version: string };
export type StarterSource = { manifest: StarterManifest; presets: StarterPresets; template: (path: string) => string };

/** The overrides that are valid and differ from the preset, in presets.json choice order. */
export function starterOverrides(presets: StarterPresets, presetId: string, overrides?: StarterRequest['overrides']): [string, string | boolean][];
/** Generates a starter project, or says why the preset has none on that platform. */
export function generateStarter(request: StarterRequest, source: StarterSource): StarterResult;
