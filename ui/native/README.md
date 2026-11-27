# Stack Junkie native widgets

A Qt 6 Widgets library and a separate component showcase for Windows and Linux. Clean utility is the only design direction. It supports light and dark themes.

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
- `menuBar`, `statusBar`, `bottomPanel` and `dashboardHome` are accepted but drawn in a later preview (#9).

When the frame shows its own tab row, a page that is itself a `QTabWidget` has its tab row hidden, so tab rows never stack.

In the showcase, Settings > Layout holds the preset picker and one control per choice, generated from `presets.json`. Changes apply to the showcase at once and are saved in `QSettings`; "Reset to" the preset appears once a choice differs.
## Stub

A Stub marks development stand-in content: sample text, fake data, unfinished labels. Create one only with the macro, which records its file and line:

```cpp
layout->addWidget(STACKJUNKIE_STUB(QStringLiteral("Summary text goes here.")));
layout->addWidget(STACKJUNKIE_STUB(new QLabel(QStringLiteral("42 open items"))));
```

- Development builds paint a Stub with diagonal warning stripes and a dashed border, in both themes. A wrapped widget gets a striped band around it. The tooltip names the location.
- Release builds refuse to compile while any Stub remains. Each remaining Stub is a compile error at its own file and line: `main.cpp(11,23): error C2338: static_assert failed: 'Stub remains at ...main.cpp(11): ...'`.
- The guard is on when `NDEBUG` or `STACKJUNKIE_RELEASE` is defined. `StackJunkie::Widgets` adds `STACKJUNKIE_RELEASE` to consumers in Release, RelWithDebInfo and MinSizeRel, including through the installed CMake package, so removing `NDEBUG` does not bypass it.
- `STACKJUNKIE_STUBS_ALLOWED` is 1 when Stubs compile. The showcase uses it to add a development-only `stub` page.
- Hint text inside an empty input (`setPlaceholderText`) is not a Stub and is unaffected.

`tests/stub-guard/run.ps1` builds a small sample app against the installed package in Release, with and without its Stubs. The first build must fail and name each Stub's line; the second must pass.

## Review the showcase

The rail selects families from `../shared/catalog.json`; the contextual list selects components. Each component has Example and Usage and states tabs. Edits remain while switching pages or themes. The state selector demonstrates disabled, read-only and invalid treatments; pointer hover and keyboard focus are real interactions, not painted screenshots.

Eight layout compositions use the same controls: focused utility, multi-tool utility, embedded utility, data application, workbench, publication/reference, guided workflow and monitoring/analysis. Local sample records, chart values and simulated progress are explicitly demonstration content. They are not live project metrics, server state or verification results.

The shared catalog describes the intended component range. Preview labels and style attributes do not establish screen-reader acceptance, native runtime behavior on every platform File/color pickers, focus restoration, help behavior and display scaling still deserve review in the target product environment.

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
