# Stack Junkie native widgets

A Qt 6 Widgets library and a separate component showcase for Windows and Linux. Clean is the only look. It supports light and dark themes.

`StackJunkie::Widgets` is the reusable library. `StackJunkieShowcase` consumes it. The showcase does not define a second stylesheet. Ordinary Qt widgets retain their keyboard, focus, selection and platform accessibility semantics under the shared theme.

## Build

Requires a C++17 compiler, CMake 3.21 or later, and Qt 6.4 or later with Widgets and Svg. Use a Qt build matching the compiler. The bundled fonts and shared JSON/SVG resources are embedded in the library, so running the built program does not fetch fonts or tokens from the network.

From this directory, with Qt discoverable through `CMAKE_PREFIX_PATH`:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --config Release --prefix install
```

The default library is static. Add `-DBUILD_SHARED_LIBS=ON` for a shared library. `-DSTACKJUNKIE_BUILD_SHOWCASE=OFF` builds just the library. The full display version comes from `../../VERSION`; the CMake package version uses its numeric semantic-version core.

The installed showcase still requires the matching Qt runtime and platform plugins. Use the parent UI package's distribution commands to bundle those dependencies. A successful Windows build does not establish Linux runtime verification. No Linux-specific installation or runtime result is claimed here.

## Consume the library

```cmake
find_package(StackJunkie CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE StackJunkie::Widgets)
```

```cpp
#include <stackjunkie/components.hpp>

auto *save = new StackJunkie::Button(
    QStringLiteral("Save"), StackJunkie::ButtonVariant::Primary, window);
auto theme = StackJunkie::Theme::load(QStringLiteral("clean"), false);
theme.apply(window);
```

Apply the theme after composing a surface, and reapply after adding custom icon buttons or changing between light and dark. Applying a theme preserves widget values and selection. Scope a different theme to a child surface when needed. Do not recreate pages merely to switch themes.

## Reusable API

- `Theme`: embedded tokens, catalog, spacing, type sizes, scoped palette/stylesheet application.
- `Button` and `IconButton`: semantic variants, accessible names, busy state and shared SVG icons.
- `Field` and `SearchField`: visible labels, native editing, contextual help and inline validation.
- `SegmentedControl`: exclusive options using native toggle buttons.
- `HelpButton`: hover/focus preview, click-to-pin, hoverable help, outside-click and Escape dismissal.
- `Disclosure`: labeled expandable content with preserved child state.
- `Notice`, `Tag` and `EmptyState`: explicit textual state and local outcomes.
- `CodeBlock`: selectable read-only code and clipboard copying.
- `ReadingView`: shared document typography with explicit title, heading, subheading and body sizes, reading line height and a measured reading width. Theme changes preserve the text selection and scroll position.
- `Workspace`: simultaneous section rail and contextual page navigation, retained pages, per-section selection, resizable sidebar, manual collapse/restore, narrow-width collapse and optional persisted sidebar/page settings. Use addSection, then addPage with its section index. AddPage-only callers retain a single Pages section.
- `SplitPane`: shared pointer and keyboard resizing, with optional vertical stacking below a chosen width. Tab to its named handle; use arrows, Shift plus arrows, Home or End. The rail selects sections while the sidebar selects pages; collapsing the sidebar keeps the rail available.
- `Tabs`: the shared QTabWidget subclass with connected top-accent tabs and retained native panels.
- `GroupedTabs`: groups containing independent peer-document tabs. Each document retains its widget state.
- `Drawer`: real left or right detail overlay with Escape/outside-click dismissal and focus restoration.
- `Chart`: reusable labeled bar/line chart accepting caller-owned values and units, with a readable accessibility description.
- `Canvas`: reusable selectable/movable rectangle and ellipse canvas with native selection, position inputs, zoom and themed graphics. The showcase supplies its sample shapes.
- `heading`, `paragraph`, `row` and `wrap`: shared typography and spacing helpers. Wrapping toolbars and option groups retain their controls at narrow widths.

Use the themed native Qt controls for text, passwords, dates, times, numbers, sliders, checkboxes, radio choices, selects, tabs, menus, dialogs, tables, trees, lists, grids and scroll areas. This avoids replacing mature native behavior with decorative copies. The showcase contains compositions for local data filtering/sorting, editable documents, canvas selection/zoom, progress with pause/cancel, file/color selection and focused dialogs.

## Presets

A Preset is a named starting set of App frame choices. The four presets live in `../shared/presets.json`, the one source that web, native and the skill read; the library embeds it like the tokens and catalog.

| Choice | Web app | Desktop app | Document editor | Website |
|---|---|---|---|---|
| `rail` | on | on | on | off |
| `sidebar` | tree | list | tree | off |
| `menuBar` | off | off | on | off |
| `statusBar` | off | on | on | off |
| `bottomPanel` | off | off | on | off |
| `documentTabs` | off | off | on | off |
| `commandPalette` | on | on | on | off |
| `theme` | dark | dark | dark | system |
| `density` | compact | compact | compact | comfortable |
| `dashboardHome` | off | off | off | off |

Start `AppShell` from a preset and override single choices:

```cpp
#include <stackjunkie/components.hpp>
StackJunkie::AppShell shell(QStringLiteral("My app"), QStringLiteral("What it does."), QStringLiteral("0.1.0"));
// add sections and pages, then:
shell.setPreset(QStringLiteral("desktop-app"), {{QStringLiteral("sidebar"), QStringLiteral("tree")}});
```

`Preset::resolve(id, overrides)` returns the full `PresetChoices`, `Preset::overrides(id, choices)` returns only the changed ones for saving, `Preset::all()` lists every preset with its label and description, and `Preset::choiceInfo()` lists every choice with its label, description and allowed values. Unknown ids fall back to Web app, and invalid overrides are ignored. A plain `Workspace` takes `setChoices(choices)`; without it, it keeps the rail and list sidebar it always had.

What the frame draws today:

- `rail`: the section rail, or none. Without the rail the sidebar lists every page, grouped by section, and Settings moves to the sidebar or, with no sidebar, the end of the header.
- `sidebar`: a `NavTree`, a flat list, or nothing. With no sidebar the pages become the one tab row: the current section's pages beside the rail, every page without it.
- `documentTabs`: beside a sidebar, every page opened from it stays in one row of closable tabs until closed (Control W closes the focused one). With none open the work area says so.
- `theme`: `AppShell::setPreset` applies dark or light; `system` follows `QStyleHints::colorScheme` live until someone picks a theme in Settings. The offscreen platform reports no scheme, which counts as light.
- `density`: compact or comfortable, both from the `density` tokens. `Theme::setDensityMode` holds one mode for the app; `setChoices` sets it, resizes the frame and re-applies the frame's theme. Content areas marked with the `sjContentPadding` property follow the padding.
- `commandPalette`: the frame does not own a palette, so an app that has one reads `choices().commandPalette` or listens to `choicesChanged`.
- `menuBar`, `statusBar`, `bottomPanel`: the Menu bar, Status bar and bottom panel (below).
- `dashboardHome`: the frame does not draw a Dashboard, so an app that has one reads `choices().dashboardHome` and opens on it.

When the frame shows its own tab row, a page that is itself a `QTabWidget` has its tab row hidden, so tab rows never stack.

In the showcase, Settings > Layout holds the preset picker and one control per choice, generated from `presets.json`. Changes apply to the showcase at once and are saved in `QSettings`; "Reset to" the preset appears once a choice differs.

### Custom presets

A custom preset is a name, a base preset and only the choices that differ from it. Web and native save, export and import the same file, described in `../shared/PRESET-FILE.md`. `CustomPreset` (in `presets.hpp`) applies the same rules as web, reading the allowed values from the embedded `presets.json`:

```cpp
CustomPreset preset;
const QString error = CustomPreset::parse(file.readAll(), &preset); // empty on success; nothing is set on failure
shell.setPreset(preset.base, preset.choices);
QByteArray text = CustomPreset{QStringLiteral("My editor"), QStringLiteral("desktop-app"), Preset::overrides(id, choices)}.toFile();
```

`toFile()` writes the same bytes as the web library. `check(preset, &checked)` checks a preset built in code and trims it to the changed choices, and `nameError(name)` checks a name alone. Booleans must be `bool`, not text, as in the file.

In the showcase, Settings > Layout > Save as custom preset names the current preset and choices. Custom presets are kept in `QSettings` (`layout/customPresets`, as file text) and listed in the preset picker after the four presets, marked "(custom)". With one selected, its row offers Update, Rename, Export file and Delete. Import file adds a preset from a file and selects it; a preset with the same name is replaced. Export starter uses the selected preset's base and choices.

For testing without dialogs:

```powershell
StackJunkieShowcase -platform offscreen --import-preset my-editor.preset.json --export-preset copy.preset.json
StackJunkieShowcase -platform offscreen --preset desktop-app --choice sidebar=tree --preset-name "My editor" --export-preset my-editor.preset.json
StackJunkieShowcase -platform offscreen --import-preset my-editor.preset.json --export-starter my-app --starter native
StackJunkieShowcase -platform offscreen --import-preset my-editor.preset.json --page settings/layout --export-preview layout.png --height 1300
```

`--import-preset <file>` checks the file first: a bad file prints "Preset file not imported" with the reason and exits with code 2 before anything is applied or written. It cannot be combined with `--preset`. `--export-preset <file>` writes the current choices (imported, `--preset`, or saved, then each `--choice`) and exits; `--preset-name` names it, defaulting to the imported or selected custom preset's name, else "Custom preset". During an export nothing is saved to `QSettings`. `npm run test:presets` in `../web` runs the web and native round trip.

### Menu bar, Status bar and bottom panel

The Document editor preset turns all three on. `Workspace` draws them from the choices; the app supplies their content:

```cpp
shell.addMenuItem(shell.fileMenu(), QStringLiteral("Open\u2026"), [] { /* open */ }, QKeySequence(Qt::CTRL | Qt::Key_O));
auto *format = shell.addMenu(QStringLiteral("Format"));
shell.addMenuItem(format, QStringLiteral("Bold"), [] { /* bold */ });
shell.statusBar()->setState(QStringLiteral("Notes.md, saved"));
shell.statusBar()->setFacts({QStringLiteral("Ln 12, Col 4"), QStringLiteral("UTF-8")});
shell.bottomPanel()->addTab(output, QStringLiteral("Output"));
```

- Menu bar: a `QMenuBar` in the header beside the app's name, styled by the theme: File, Edit, View, the app's menus (`addMenu`), then Help. The frame owns Settings (File, calls the settings handler), About (Help, calls `setAboutHandler`; `AppShell` opens Settings at About) and in View the Sidebar (Control B), Bottom panel (Control J) and Status bar toggles. `AppShell` adds Dark theme. `addMenuItem` puts an app item above the frame's items; its shortcut works while the Menu bar is shown. Edit holds only app items and is hidden without them. F10 opens the first menu; arrow keys and Escape work as usual.
- `StatusBar`: a thin bottom row with `setState` on the left and `setFacts` on the right. Never ads or tips.
- `BottomPanel`: a panel under the work area in a vertical splitter, with its own tab row (`addTab`) and a close control. It is drawn only once it has a tab. View > Bottom panel or `setBottomPanelShown` hides it for the session, as `setStatusBarShown` does for the Status bar.

The native showcase uses all three: the current component on the left of the Status bar, its family and the library version on the right, and Output and Problems in the bottom panel. Export one menu open with `--menu file|edit|view|library|help`.

## Export

Settings > Layout > Export picks Web starter or Native starter, and Export starter writes that project into a folder you choose (it asks before replacing files). The starter matches the chosen preset and changed choices.

- Native starter: `CMakeLists.txt`, `main.cpp`, `.gitignore` and a short README. `CMakeLists.txt` uses an installed package through `find_package(StackJunkie)` when `CMAKE_PREFIX_PATH` finds one, and otherwise fetches this version's tag with FetchContent (`https://github.com/stack-junkie/stack-junkie-style.git`, `GIT_TAG v<version>`, `SOURCE_SUBDIR ui/native`, ADR 0003). `main.cpp` builds an `AppShell` and calls `AppShell::setPreset(id, choices)` with only the choices that differ from the preset.
- Web starter: the same Vite, React and TypeScript project the web showcase exports (see the web README).
- The Website preset has no native starter, because native has no Website layout; the row says so and Export starter is disabled.

The templates and file list live in `../shared/starters`; `showcase/starter.cpp` fills them exactly as `starters.mjs` does on web. For scripts and tests:

```sh
StackJunkieShowcase --export-starter my-app --starter native --preset desktop-app --choice sidebar=tree
```

`--export-starter <folder>` writes the starter and exits without opening a window (it uses the offscreen platform). `--starter` is `web` (the default) or `native`; `--preset` and `--choice` work as for `--export-preview`. Without `--preset` it uses the saved preset and choices. The website preset with `--starter native` exits with code 2 and says why. `npm run test:starters` in `../web` builds every starter (see the web README).

## Stub

A Stub marks development stand-in content: sample text, fake data, unfinished labels. Create one only with the macro, which records its file and line:

```cpp
layout->addWidget(STACKJUNKIE_STUB(QStringLiteral("Summary text goes here.")));
layout->addWidget(STACKJUNKIE_STUB(new QLabel(QStringLiteral("42 open items"))));
```

- A Stub paints with diagonal warning stripes and a dashed border, in both themes. A wrapped widget gets a striped band around it. The tooltip names the location.
- Release builds refuse to compile while any Stub remains. Each remaining Stub is a compile error at its own file and line: `main.cpp(11,23): error C2338: static_assert failed: 'Stub remains at ...main.cpp(11): ...'`.
- The guard is on when `NDEBUG` or `STACKJUNKIE_RELEASE` is defined. `StackJunkie::Widgets` adds `STACKJUNKIE_RELEASE` to consumers in Release, RelWithDebInfo and MinSizeRel, including through the installed CMake package, so removing `NDEBUG` does not bypass it.
- `STACKJUNKIE_STUBS_ALLOWED` is 1 when Stubs compile.
- The showcase's Content > Stub page (`--page content/stub`) builds the `Stub` class directly as its specimen, so it shows the striped look in a Release build too. Apps use only the macro.
- Hint text inside an empty input (`setPlaceholderText`) is not a Stub and is unaffected.

`tests/stub-guard/run.ps1` builds a small sample app against the installed package in Release, with and without its Stubs. The first build must fail and name each Stub's line; the second must pass.

## Review the showcase

The rail selects families from `../shared/catalog.json`; the contextual list selects components. Each component has Example and Usage and states tabs. Edits remain while switching pages or themes. The state selector demonstrates disabled, read-only and invalid treatments; pointer hover and keyboard focus are real interactions, not painted screenshots.

Eight layout compositions use the same controls: focused utility, multi-tool utility, embedded utility, data application, workbench, publication/reference, guided workflow and monitoring/analysis. Local sample records, chart values and simulated progress are explicitly demonstration content. They are not live project metrics, server state or verification results.

Every catalog entry has a native example, as on web. Accessible names and style attributes do not establish screen-reader acceptance or native behavior on every platform. File and color pickers, focus restoration, help behavior and display scaling still deserve review in the target product environment.

## Export a native preview

The export option selects Qt's offscreen platform before the application starts, renders the actual showcase and exits. It does not take control of the desktop or operate another program.

```sh
StackJunkieShowcase --export-preview home-dark.png --theme dark --page home
StackJunkieShowcase --export-preview settings-light.png --theme light --page settings
StackJunkieShowcase --export-preview inputs-narrow.png --page inputs --width 540 --height 900
StackJunkieShowcase --export-preview website-light.png --theme light --page actions --preset website
StackJunkieShowcase --export-preview layout-dark.png --theme dark --page settings/layout --height 900 --preset desktop-app --choice sidebar=tree
```

Page IDs are the catalog family IDs, `family/component` (for example `navigation/tabs`), `home`, `settings` for the Settings window, or `settings/<section>` (`appearance`, `layout`, `keyboard`, `about`) for one section at the export height. `--preset <id>` starts from that preset alone, ignoring saved choices, and `--choice name=value` (repeatable) changes one choice the way Settings does. Exported previews capture the visible viewport, not every scroll position. Screenshot generation is a product artifact, not proof that all interactions work.

## Shared source

Visual values originate in `../shared/tokens.json`, including shell density, text sizes and weights. Catalog metadata originates in `../shared/catalog.json`. `StackJunkie::icon` renders symbols directly from `../shared/icons.svg` using QtSvg. Checkbox check glyphs are generated from that sprite at configure time using each theme's `onAccent` token. CMake regenerates them when shared tokens change. Font files are embedded from `../shared/fonts`; their nominal token names resolve to the actual families reported by the active Qt font database, including the offscreen platform. The `system-ui` token resolves to Qt's system general font. The showcase uses the existing SJ favicon images from `../shared/brand` beside a compact title. Toolkit-specific geometry and behavior stay in the native adapter.

The library does not alter global application styles automatically. The host decides which root to theme. The showcase alone uses Fusion for consistent rendering and stores preferences under its own `StackJunkie/ComponentShowcase` settings namespace.
