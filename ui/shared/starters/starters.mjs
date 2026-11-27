// The starter generator: a preset id plus overrides in, a new project's files out.
// The file list (starters.json) and the templates sit beside this file. The web showcase and
// the test script call this module; the native showcase fills the same templates the same way
// (showcase/starter.cpp), so both Exports write identical starters.

const placeholder = /\{\{(\w+)\}\}/g;

/** The overrides that are valid and differ from the preset, in presets.json choice order. */
export function starterOverrides(presets, presetId, overrides = {}) {
  const preset = presets.presets.find(p => p.id === presetId);
  if (!preset) throw new Error(`Unknown preset: ${presetId}`);
  return Object.entries(presets.choices)
    .filter(([key, info]) => key in overrides && info.options.includes(overrides[key]) && overrides[key] !== preset.choices[key])
    .map(([key]) => [key, overrides[key]]);
}

function literal(platform, changed) {
  if (!changed.length) return '{}';
  if (platform === 'web') return `{ ${changed.map(([k, v]) => `${k}: ${typeof v === 'string' ? `'${v}'` : v}`).join(', ')} }`;
  return `{${changed.map(([k, v]) => `{QStringLiteral("${k}"), ${typeof v === 'string' ? `QStringLiteral("${v}")` : v}}`).join(', ')}}`;
}

/** The starter's areas and pages as code: AppShell sections for web, addSection and addPage calls for native. */
function sectionsCode(platform, structure) {
  if (platform === 'web') return structure.areas.map(area => [
    `  { id: '${area.id}', label: stub('${area.label}'), icon: '${area.icon}', pages: [`,
    ...area.pages.map(page => `    { id: '${page.id}', label: stub('${page.label}'), content: <Stub block>${page.content}</Stub> },`),
    '  ] },',
  ].join('\n')).join('\n');
  return structure.areas.map((area, i) => [
    `    const int area${i + 1} = shell.addSection(STACKJUNKIE_STUB_TEXT("${area.label}"), QStringLiteral("${area.icon}"));`,
    ...area.pages.map(page => `    shell.addPage(STACKJUNKIE_STUB_TEXT("${page.label}"), STACKJUNKIE_STUB("${page.content}"), QStringLiteral("file"), area${i + 1});`),
  ].join('\n')).join('\n');
}

/**
 * Generates a starter project.
 * request: { platform: 'web' | 'native', preset, overrides, version }
 * source: { manifest (starters.json), presets (presets.json), template(path) returning a template's text }
 * Returns { files: [{ path, text }] }, or { unsupported: reason } when the preset has no starter on that platform.
 */
export function generateStarter({ platform, preset, overrides = {}, version }, { manifest, presets, template }) {
  const kind = manifest[platform];
  if (!kind) throw new Error(`Unknown starter platform: ${platform}`);
  const changed = starterOverrides(presets, preset, overrides);
  const unsupported = kind.unsupported?.[preset];
  if (unsupported) return { unsupported };
  const choices = { ...presets.presets.find(p => p.id === preset).choices, ...Object.fromEntries(changed) };
  const values = {
    version, tag: `v${version}`, cmakeVersion: /^\d+\.\d+\.\d+/.exec(version)?.[0] ?? version,
    repository: manifest.repository, preset, presetLabel: presets.presets.find(p => p.id === preset).label,
    overrides: literal(platform, changed), theme: choices.theme, density: choices.density,
    sections: sectionsCode(platform, manifest.structure),
  };
  const files = { ...kind.files, ...kind.presetFiles?.[preset] };
  return {
    files: Object.entries(files).map(([path, source]) => ({
      path,
      text: template(source).replace(/\r/g, '').replace(placeholder, (_, name) => {
        if (!(name in values)) throw new Error(`Unknown placeholder {{${name}}} in ${source}`);
        return String(values[name]);
      }),
    })),
  };
}
