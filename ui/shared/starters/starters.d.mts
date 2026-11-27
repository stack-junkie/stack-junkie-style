export type StarterPlatform = 'web' | 'native';
/** A text file, or the logo image's bytes. */
export type StarterFile = { path: string; text: string; data?: undefined } | { path: string; data: Uint8Array; text?: undefined };
export type StarterResult = { files: StarterFile[]; unsupported?: undefined } | { files?: undefined; unsupported: string };
export type StarterKind = { label: string; description: string; files: Record<string, string>; presetFiles?: Record<string, Record<string, string>>; unsupported?: Record<string, string> };
export type StarterPage = { id: string; label: string; content: string };
export type StarterArea = { id: string; label: string; icon: string; pages: StarterPage[] };
export type StarterArticle = { id: string; title: string; date: string; summary: string };
export type StarterManifest = { repository: string; structure: { note?: string; areas: StarterArea[]; panels: StarterPage[] }; website: { note?: string; articles: StarterArticle[] }; web: StarterKind; native: StarterKind };
export type StarterPresets = {
  choices: Record<string, { options: readonly (string | boolean)[] }>;
  presets: readonly { id: string; label: string; choices: Record<string, string | boolean> }[];
};
export type StarterLogo = { name: string; data: Uint8Array };
export type StarterRequest = { platform: StarterPlatform; preset: string; overrides?: Record<string, string | boolean | undefined>; version: string; logo?: StarterLogo };
export type StarterSource = { manifest: StarterManifest; presets: StarterPresets; template: (path: string) => string };

/** The overrides that are valid and differ from the preset, in presets.json choice order. */
export function starterOverrides(presets: StarterPresets, presetId: string, overrides?: StarterRequest['overrides']): [string, string | boolean][];
/** Image types the logo slot takes, by file extension. */
export const logoTypes: string[];
/** The starter's file name for a logo file name, such as logo.png, or an error message. */
export function logoFileName(name: string): { file: string; error?: undefined } | { file?: undefined; error: string };
/** Generates a starter project, or says why the preset has none on that platform. Throws on a logo of the wrong type. */
export function generateStarter(request: StarterRequest, source: StarterSource): StarterResult;
/** The `new` command that creates the same starter, pinned to this version's tag. logoName is the logo file's name, if any. */
export function starterCommand(request: { platform: StarterPlatform; preset: string; overrides?: StarterRequest['overrides']; version: string; logoName?: string; folder?: string }, source: Pick<StarterSource, 'manifest' | 'presets'>): string;
