// llms.txt for the showcase site: what Stack Junkie Style is, how an agent installs the skill and creates a project,
// and every Preset with its choices. Generated from presets.json and starters.json at build time, never hand-edited.

const value = v => String(v);
/** The llms.txt text for this version. */
export function llmsText({ version, presets, manifest, site = 'https://style.stack-junkie.com' }) {
  const repo = manifest.repository;
  const slug = repo.replace(/^https:\/\/github\.com\//, '');
  const command = `npx github:${slug}#v${version} new`;
  const groupOf = id => Object.entries(presets.groups).find(([, g]) => g.choices.includes(id))?.[1].label ?? 'Both';
  const lines = [
    '# Stack Junkie Style',
    '',
    `> A design system for web apps, desktop apps, document editors and websites: a React library, a Qt 6 Widgets library, Presets that set the App frame, starters that install the library from a tagged release, and an agent skill. Version ${version}.`,
    '',
    '## Install the skill',
    '',
    `npx skills add ${slug}`,
    '',
    'The skill (SKILL.md) tells a coding agent the settled rules: the App frame, the Settings window, Stubs, the look and the content rules.',
    '',
    '## Create a project',
    '',
    `${command} <folder> [--platform web|native] [--preset <id>] [--choice <name>=<value>]... [--preset-file <file>] [--logo <image>]`,
    '',
    '- Runs straight from the GitHub repo. No npm registry or account.',
    '- Writes a new folder and refuses one that is not empty.',
    '- --platform: web (Vite, React, TypeScript; needs Node.js 20 or later) or native (CMake and Qt 6.4 or later). Default web. The Website preset is web only.',
    '- --preset-file: a custom preset file exported from the Builder. Use it or --preset, not both.',
    '- --choice: change one choice; repeat it for more.',
    '- --logo: a PNG, JPEG, SVG or WebP file for the logo slot. Default: the interim SJ mark.',
    '- An app starter opens with two areas of three pages each; the Website starter with Home, Articles, About and three articles. Names and content are Stubs: striped in development, and a release build fails and names each one until it is replaced.',
    '',
    'Examples:',
    '',
    `${command} my-tool --preset web-app`,
    `${command} my-utility --platform native --preset desktop-app --choice sidebar=tree`,
    `${command} my-editor --platform native --preset document-editor --choice theme=light --logo ./logo.png`,
    `${command} my-blog --preset website --choice siteNavigation=sidebar --choice headerButton=false`,
    '',
    `The Builder on ${site} ("Build an app") shows each Preset and choice live and copies the matching command.`,
    '',
    '## Presets',
    '',
    ...presets.presets.flatMap(p => [
      `### ${p.label} (${p.id})`,
      '',
      `${p.description} ${p.madeFor}`,
      '',
      `Choices: ${Object.entries(p.choices).filter(([k]) => p.id === 'website' ? groupOf(k) !== 'App frame' : groupOf(k) !== 'Website').map(([k, v]) => `${k}=${value(v)}`).join(', ')}`,
      '',
    ]),
    '## Choices',
    '',
    ...Object.entries(presets.choices).map(([id, c]) => `- ${id} (${c.label}, ${groupOf(id)}): ${c.description} Values: ${c.options.map(value).join(', ')}.`),
    '',
    '## Links',
    '',
    `- Skill: ${repo}/blob/v${version}/SKILL.md`,
    `- Release ${version}: ${repo}/releases/tag/v${version}`,
    `- Repository: ${repo}`,
    `- Showcase and Builder: ${site}`,
    '',
  ];
  return lines.join('\n');
}
