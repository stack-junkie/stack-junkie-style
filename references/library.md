# Library

Two libraries share one design: React and TypeScript on web (`@stack-junkie/ui-web`), Qt 6 Widgets and C++17 on native (`StackJunkie::Widgets`). They share tokens, the catalog and Presets from [ui/shared](../ui/shared/), not executable widgets. Build and run commands: [UI README](../ui/README.md), [web README](../ui/web/README.md), [native README](../ui/native/README.md).

## Install

**New project:** one command, straight from the GitHub repo (no npm registry):

```shell
npx github:stack-junkie/stack-junkie-style#v<version> new my-app --platform native --preset desktop-app --choice sidebar=tree
```

`--platform` is `web` (default) or `native`; `--preset` or `--preset-file` (a custom preset file); `--choice name=value` repeats; `--logo` copies an image into the logo slot. It writes a new folder and refuses one that is not empty. The showcases' Builder exports the same starter as a zip (web) or folder (native) and copies the same command; the native showcase also writes one with `--export-starter <folder> --starter native --preset desktop-app`. All of them fill the templates in [ui/shared/starters](../ui/shared/starters/) the same way (`starters.mjs`, and `showcase/starter.cpp` for native), so they write identical files. The Website Preset has no native starter.

A starter opens with two areas of three Stub pages each and a Stub bottom panel tab (the Website: Home, Articles, About and three Stub articles). Replace each with the app's own: release builds stop at every Stub that remains.

**Existing project:** install from a tagged release of `github.com/stack-junkie/stack-junkie-style` ([ADR 0003](../docs/adr/0003-install-from-github-tags.md)), pinned to one version:

- Web: depend on the library file attached to the release, `https://github.com/stack-junkie/stack-junkie-style/releases/download/v<version>/stack-junkie-ui-web-<version>.tgz`, plus `import '@stack-junkie/ui-web/styles.css'`.
- Native: `find_package(StackJunkie CONFIG REQUIRED)` and `target_link_libraries(my_app PRIVATE StackJunkie::Widgets)`, or FetchContent of the tag with `SOURCE_SUBDIR ui/native`.

Upgrade by moving the pin on purpose. Publishing to npm is deferred.

## Use

- Compose named components with explicit properties, state and events. Pages supply domain content; the library owns repeated behavior and styling. Read the actual exports before importing: web [index.ts](../ui/web/src/library/index.ts), native [components.hpp](../ui/native/include/stackjunkie/components.hpp) and [presets.hpp](../ui/native/include/stackjunkie/presets.hpp).
- A missing component or variant goes into the library (with its catalog entry and showcase example), not into the project. Anything used more than once is one shared component.
- Qt 5, WPF and other toolkits cannot import these controls. Keep the toolkit and adapt the tokens and behavior through its native controls; never migrate an app just to use this package.
- The web version is `systemVersion`; native is `STACKJUNKIE_VERSION`. Both come from [VERSION](../VERSION). The app's own version comes from the app's version source.
- Showcase-only controls and sample content live under `showcase`, are labeled as not part of the style, and are never exported or imported. The old static specimen in `assets/style-guide` is history, not a source.

## Stubs and the Stub guard

A Stub marks development stand-in content: sample text, fake data, unfinished labels. It renders striped in development. Release builds fail while any Stub remains and name each location. Every piece of stand-in content is a Stub; nothing unmarked stands in.

Web:

```tsx
import { Stub, stub } from '@stack-junkie/ui-web';

<h1><Stub>Project name</Stub></h1>
<Stub block><ProjectList projects={stub(sampleProjects)} /></Stub>
<Button aria-label={stub('Open project')}>Open</Button>
```

Add the guard to the app's Vite config: `plugins: [react(), stackJunkieStubGuard()]` from `@stack-junkie/ui-web/vite`. `vite build` then stops and lists each Stub by file, line and column. The web starter has it on already.

Native:

```cpp
layout->addWidget(STACKJUNKIE_STUB(QStringLiteral("Summary text goes here.")));
layout->addWidget(STACKJUNKIE_STUB(new QLabel(QStringLiteral("42 open items"))));
```

Create native Stubs only with the macro. Release, RelWithDebInfo and MinSizeRel builds fail to compile at each remaining Stub's file and line.

Hint text (an input's `placeholder` on web, `setPlaceholderText` on native) is a short example of valid input. It never replaces the label, and the guard never reports it.

## Changing the library

- Change the shared value or component first, then its catalog entry in [catalog.json](../ui/shared/catalog.json), both showcases, and the affected skill text, in the same release.
- The showcase imports the same component an app uses. Fix appearance or behavior in the library, never with a private fix in the example.
- Examples show realistic, labeled demonstration content and every applicable state: default, hover, focus, selected, disabled, pending, empty, invalid, failed.
- Bump [VERSION](../VERSION) once per delivered preview or release, not per planning edit, and add the change to [CHANGELOG](../ui/CHANGELOG.md).
- Tests guard real behavior only: `npm run test:starters` (every Preset to a building starter whose release build stops at its Stubs), `npm run test:new` (the `new` command and the Builder's command write the generator's files), `npm run test:presets` (custom preset file round trip and rejection) and `npm run test:stub-guard` in `ui/web`, and `tests/stub-guard/run.ps1` in `ui/native`. Look and layout are checked with screenshots, not tests.
