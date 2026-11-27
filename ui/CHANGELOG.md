# Stack Junkie UI changelog

## 3.0.1

Polish from the first look at exported starters, in the web and native libraries, so every starter picks it up.

- About gives the app the spotlight: a large logo, the name with a version chip, the description and a links row (website, repository, license, each optional). The Stack Junkie Style credit is small grey text in the bottom right. The native About text shows proper characters.
- Native apps use the style's fonts everywhere, including menus, dialogs and tooltips, with no extra setup.
- Everything clickable shows the pointing hand; disabled controls keep the arrow.
- Tooltips sit beside their element: to the right of Rail items, below toolbar buttons.
- The Sidebar toggle is a panel icon (new `sidebar` icon), and the Settings gear matches the Rail icons.
- The Sidebar tree is tighter, with about 30% less indent, and group rows show no counts.
- View > Appearance holds the Sidebar, Status bar and Panel toggles, Zoom in, Zoom out, Reset zoom and Dark theme. Native apps zoom with Control plus, minus and 0. Web Menu bar entries can be submenus.
- Native apps show the logo and name in a logo slot at the top left; clicking it goes Home.

## 3.0.0

The first full release of Stack Junkie Style as its own product and repo.

- One look, Clean: True black dark mode, white light mode, thin borders for structure and a blue accent. Warm and Technical are gone.
- One web library (React) and one native library (Qt 6 Widgets), each with a showcase. Both showcases demo every catalog entry.
- Four Presets set the App frame: Web app, Desktop app, Document editor and Website. Every choice can be switched, saved as a custom preset, and exported or imported as a file.
- Export writes a web starter or a native CMake starter for the chosen preset and choices, installing the library from this version's tag.
- The App frame: Rail, Sidebar, one Tab row and a Settings window opened from the gear or Control comma. The Document editor adds the Menu bar, Status bar, bottom panel and closable document tabs. The Website preset uses its own layout.
- Stubs mark development stand-in content. They are striped in development, and release builds fail while any remain, on web and native.
- The first Dashboard, "Projects using Stack Junkie Style", lives in `apps/projects`.
- The skill is rewritten around the glossary and the settled rules, with a short "no slop" content page.

Since preview.10: the native showcase shows Content > Stub in every build, the web Settings window scrolls once instead of twice, and stale preview wording is gone. Details from the last preview follow.

## 3.0.0-preview.10

- Custom presets on web and native: Settings > Layout saves the current preset and choices under a name, lists them in the preset picker, and can update, rename, delete, export and import them. One file format (`shared/PRESET-FILE.md`) for both, checked by `parseCustomPresetFile` on web and `CustomPreset::parse` on native; a bad file is rejected whole with a plain message. Native adds `--import-preset`, `--export-preset` and `--preset-name`, and `npm run test:presets` checks a web to native round trip and bad files on both.
- Swap the next web and native versions in. There is one web library and one native library again.
- Settings is a window (`SettingsWindow`) opened from the gear or Control comma, never a page.
- No stacked tab rows: groups sit in one row, document tabs close immediately, and debug builds warn about a tab row inside another. `TabBoundary` marks real visual breaks.
- Add `NavTree` for the sidebar tree and `AppShell` as the standard frame on web and native. The web showcase runs on `AppShell`.
- Clean's dark mode is true black with thin borders and near-white text; light mode is one white ground with thin borders. The values now live in the shared tokens.
- Product layouts use their own `layout` icon. The workbench is the reference for many-page apps: rail, sidebar tree, one row of open documents and a bottom panel.
- The expanded sidebar always pushes the work area and never becomes an overlay.
- Remove the Warm editorial and Precise technical directions. Clean is the only direction.
- Native: add `StackJunkie::Stub` and `STACKJUNKIE_STUB`. Stubs are striped in development builds; release builds fail to compile at each remaining Stub's file and line. Fix a debug-build assert from closable document tabs.
- Add Presets (Web app, Desktop app, Document editor, Website) in `shared/presets.json`. The web `AppShell` takes `preset` and `choices` and draws the rail, tree or list sidebar, closable document tabs, default theme (including system) and density. The showcase Settings window picks the preset and switches every choice, and Export writes them into a starter.
- Native presets: the library embeds `shared/presets.json` and adds `PresetChoices`, `Preset::resolve` and `Preset::overrides`. `Workspace::setChoices` and `AppShell::setPreset` draw the rail, tree or list sidebar or none, closable document tabs and density; `AppShell` also applies the default theme, with `system` following the device. The comfortable density values move into the shared `density` tokens for both platforms. The native showcase Settings window gains a Layout section with the preset picker and every choice, and the CLI takes `--preset`, `--choice` and `--page settings/<section>`.
- Add `WebsiteLayout` on web for the Website preset: a header with logo, navigation (a menu button at narrow widths) and a dark or light toggle that defaults to the system and is saved, one reading-width column, and a footer. It is separate from `AppShell`. Export writes a Website `App.tsx` that uses it, and Product layouts has a Website page.
- Add the first Dashboard, "Projects using Stack Junkie Style", as a local app in `apps/projects`. A project scan reads 02 Projects plus GitHub and GitLab repos (through `gh` and `glab`) and finds each project's version from its package.json, imports, CMake or copied skill. Tiles show how many projects use the style, are behind the latest version, have no readable version, or are front ends not using it yet, and each opens the matching list of projects with links to open them.
- Add web Stubs. `Stub` (and `stub()` for plain values) renders development stand-in content striped. `stackJunkieStubGuard()` from `@stack-junkie/ui-web/vite` stops an application's release build while any Stub remains and names each one by file:line. `npm run test:stub-guard` checks it on a sample app. Hint text is unaffected.
- Add the Menu bar, Status bar and bottom panel on web and native, drawn by `AppShell` and `Workspace` when the `menuBar`, `statusBar` and `bottomPanel` choices are on. The Menu bar shows File, Edit, View, the app's menus, then Help, with Settings under File and About under Help, and View toggles for the sidebar, bottom panel, Status bar and theme. The Status bar shows the current item's state on the left and small facts on the right. The bottom panel is resizable with its own tab row. Web adds `MenuBar`, `StatusBar` and `BottomPanel`; native adds `StatusBar`, `BottomPanel` and `Workspace` menu methods. Both showcases use them with the Document editor preset, list them in the catalog, and the native preview export takes `--menu`.
- Export writes a starter project for the chosen preset and changed choices, from one generator over shared templates (`shared/starters`). The web starter is a Vite, React and TypeScript app that installs `@stack-junkie/ui-web` from this version's tag and uses `AppShell` (or `WebsiteLayout` for the Website preset) with the Stub guard on. The native starter is a CMake app that uses an installed `StackJunkie` package or fetches the same tag, and calls `AppShell::setPreset`. The Website preset has no native starter. The web showcase downloads a zip; the native showcase writes to a chosen folder and takes `--export-starter <folder> --starter web|native`. `npm run test:starters` builds every starter on both platforms.

## 3.0.0-preview.9

- Make the header SJ mark open Home, with Foundations accessible there instead of the top rail icon.
- Place one navigation toggle at the fixed top of the activity rail. Remove the contextual collapse arrow and redundant main-area toggle.
- Move web component tabs into the top content row. Show the component name alongside tabs only when the contextual sidebar is collapsed; preserve mounted page and tab state.
- Share the navigation behavior and collapsed tab identity in the native library and showcase.

## 3.0.0-preview.8

- Pin Settings below the scrolling activity icons. Reusable shells place it at the contextual-sidebar footer when the rail is absent.
- Move theme, design comparisons and About into Settings; remove the web header search action while retaining command search through Control K.
- Keep contextual sidebars in the page layout at narrow widths and browser zoom. Remove fixed width caps and native automatic collapse.
- Add reusable SettingsPage, AboutPage and branding support. Product identity stays prominent above compact persistent style attribution and copyright.
- Expose the style-system version from the single VERSION source in About and imported libraries. Mark showcase-only controls explicitly and keep them out of library exports.

## 3.0.0-preview.7

- Reduce the SJ corner mark from 28px to 25px through one shared size token, after review of the blue J in the web preview.

## 3.0.0-preview.6

- Remove the logo's white square. Render the S in the theme text color and the J in its accent color using one shared transparent asset.
- Follow the active palette in both showcases, including light mode and alternate directions. The accent placement is a trial, not a final brand-color decision.

## 3.0.0-preview.5

- Use the selected F logo, a black SJ monogram on a white square, in both web and Windows showcase headers.
- Share one standalone logo asset across themes and toolkits. Preserve the previous site assets and private selection history.

## 3.0.0-preview.4

- Use the platform UI font in Clean utility, with quieter shell labels, compact rows and shared density values across web and Qt.
- Separate reading body, subheading and heading roles; keep reading content comfortably sized and limit its line length.
- Align web contextual headers, retain adjustable sidebar state, and preserve the neutral black palette with softer ordinary text.
- Use the existing site SJ favicon as a small corner mark. New generated refinements remain candidates; the first six-logo sheet was rejected.
- Save primary-source typography, layout and accessibility research in the skill references.

## 3.0.0-preview.3

- Darken Clean utility to neutral black surfaces and remove the blue tint from selection backgrounds. Keep the light palette separate.
- Replace the invented initials badge with the site's Stack-Junkie header wordmark, using shared brand values in the web and native showcases.
- Clean utility is the closest candidate; its overall appearance remains under review.

## 3.0.0-preview.2

- Make both showcases use a shared section rail, adjustable contextual sidebar and connected Example / Usage tabs.
- Preserve per-section selection, mounted example state and sidebar width through collapse; retain existing native sidebar preferences on migration.
- Restore the clean dark baseline to near-black neutral surfaces while keeping the other directions and light themes available.
- Keep headers and tabs outside independently scrolling example panels; use compact shell titles and controls.

Fonts, accents and hands-on interaction review remain open. This update does not install the skill or change ART.

## 3.0.0-preview.1

- Introduce shared light/dark tokens for Clean utility, Warm editorial and Precise technical.
- Add reusable web and Qt widget libraries, working showcases, 51 catalog entries and eight product layout patterns.
- Derive showcase and distribution versions from the skill's `VERSION`.
- Bundle fonts, licenses and icons for offline use.
- Replace the independently styled legacy HTML specimen with a route to the living showcase.

This is a working comparison preview. The final visual direction and hands-on interaction review remain open.
