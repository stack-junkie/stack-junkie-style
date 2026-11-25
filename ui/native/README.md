# Stack Junkie native widgets

A Qt 6 Widgets library and a separate component showcase for Windows and Linux. Three provisional directions, Clean utility, Warm editorial and Precise technical, share the same components and behavior. Each supports light and dark themes. The directions remain alternatives for owner review.

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

Apply the theme after composing a surface, and reapply after adding custom icon buttons or changing direction. Applying a theme preserves widget values and selection. Scope a different theme to a child surface for comparisons. Do not recreate pages merely to switch themes.

## Reusable API

- `Theme`: embedded tokens, catalog, spacing, type sizes, scoped palette/stylesheet application and direction lookup.
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

## Review the showcase

The rail selects families from `../shared/catalog.json`; the contextual list selects components. Each component has Example and Usage and states tabs. Edits remain while switching pages or themes. Use the comparison page to see the same controls in all three directions. The state selector demonstrates disabled, read-only and invalid treatments; pointer hover and keyboard focus are real interactions, not painted screenshots.

Eight layout compositions use the same controls: focused utility, multi-tool utility, embedded utility, data application, workbench, publication/reference, guided workflow and monitoring/analysis. Local sample records, chart values and simulated progress are explicitly demonstration content. They are not live project metrics, server state or verification results.

The shared catalog describes the intended component range. Preview labels and style attributes do not establish screen-reader acceptance, native runtime behavior on every platform, or owner approval of a direction. File/color pickers, focus restoration, help behavior and display scaling still deserve review in the target product environment.

## Export a native preview

The export option selects Qt's offscreen platform before the application starts, renders the actual showcase and exits. It does not take control of the desktop or operate another program.

```sh
StackJunkieShowcase --export-preview compare-dark.png --theme dark --page compare
StackJunkieShowcase --export-preview inputs-warm.png --direction warm --theme light --page inputs
StackJunkieShowcase --export-preview inputs-narrow.png --page inputs --width 540 --height 900
```

Page IDs are the catalog family IDs, `family/component` (for example `navigation/tabs`), or `compare`. Exported previews capture the visible viewport, not every scroll position. Screenshot generation is a product artifact, not proof that all interactions work.

## Shared source

Visual values originate in `../shared/tokens.json`, including shell density, text sizes and weights. Catalog metadata originates in `../shared/catalog.json`. `StackJunkie::icon` renders symbols directly from `../shared/icons.svg` using QtSvg. Checkbox check glyphs are generated from that sprite at configure time using each theme's `onAccent` token. CMake regenerates them when shared tokens change. Font files are embedded from `../shared/fonts`; their nominal token names resolve to the actual families reported by the active Qt font database, including the offscreen platform. The `system-ui` token resolves to Qt's system general font. The showcase uses the existing SJ favicon images from `../shared/brand` beside a compact title. Toolkit-specific geometry and behavior stay in the native adapter.

The library does not alter global application styles automatically. The host decides which root to theme. The showcase alone uses Fusion for consistent comparison and stores preferences under its own `StackJunkie/ComponentShowcase` settings namespace.
