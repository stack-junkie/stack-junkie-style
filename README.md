# Stack Junkie Style

A design system for web apps, desktop apps, document editors and websites: a React library, a Qt 6 Widgets library, shared tokens and presets, showcases for both, and an agent skill (`SKILL.md`) that tells coding agents how to use it.

## Start a project

One command creates a project from a Preset (Web app, Desktop app, Document editor or Website) and any choices, straight from this repo:

```shell
npx github:stack-junkie/stack-junkie-style new my-app --platform web --preset web-app --choice theme=light
```

Run it with `--help` for every option, or pin a version with `github:stack-junkie/stack-junkie-style#v<version>`. The Builder in either showcase ("Build an app") shows the starter live and exports the same files as a zip or folder, or copies this command. Coding agents can read everything they need from [llms.txt](https://style.stack-junkie.com/llms.txt). The starter installs the library from a tagged release.

- Web: depend on `https://github.com/stack-junkie/stack-junkie-style/releases/download/v<version>/stack-junkie-ui-web-<version>.tgz`.
- Native: `find_package(StackJunkie CONFIG REQUIRED)`, or FetchContent of the tag with `SOURCE_SUBDIR ui/native`.

## Layout

- `SKILL.md`, `references/`, `GLOSSARY.md`, `docs/adr/`: the skill and the decisions behind it.
- `ui/shared/`: tokens, presets, catalog, icons, fonts, starter templates and the one generator.
- `bin/` and `package.json`: the `new` command.
- `ui/web/`: the React library and web showcase. See its README.
- `ui/native/`: the Qt library and Windows showcase. See its README.
- `ui/apps/projects/`: a local dashboard of projects using the style.
- `ui/CHANGELOG.md`: release notes.

## Release

1. Bump `VERSION` and add a `ui/CHANGELOG.md` section.
2. In `ui/web`: `npm run pack:release` writes `ui/dist/stack-junkie-ui-web-<version>.tgz`.
3. Tag `v<version>`, push, and create a GitHub release with that file attached.

## License

See `ui/LICENSE` and `ui/THIRD_PARTY_NOTICES.md`.
