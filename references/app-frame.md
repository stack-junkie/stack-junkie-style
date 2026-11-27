# App frame and layouts

The App frame is `AppShell` on web and native. Apps supply sections, pages and their own Settings sections; the frame owns the Rail, Sidebar, Tab row, Settings window, theme and About. Drop to `Workspace` only for a frame `AppShell` cannot express. The exact props and methods are in the [web README](../ui/web/README.md) and [native README](../ui/native/README.md).

## Presets

[presets.json](../ui/shared/presets.json) is the one source for the four Presets and every choice (rail, sidebar, menuBar, statusBar, bottomPanel, documentTabs, commandPalette, theme, density, dashboardHome). Read the values from it.

- Web app: browser tools. Desktop app: Windows tools. Document editor: editors. Website: reading sites.
- Pass the Preset to `AppShell` and override single choices (web `preset` and `choices`; native `setPreset(id, overrides)`). Keep only the choices that differ from the Preset, so deleting one returns to the Preset.
- The showcase Settings window's Layout section holds the Preset picker, every choice, and Export starter.
- A custom Preset is a name, a base Preset and its changed choices. The showcase saves, renames, updates, deletes, exports and imports them; one file format for web and native ([PRESET-FILE.md](../ui/shared/PRESET-FILE.md)), read with `parseCustomPresetFile` or `CustomPreset::parse`.
- The frame does not own a command palette. An app that has one reads the `commandPalette` choice.

## Frame parts

**Rail.** Selects a top-level section. Icons have accessible names; add labels where an icon is ambiguous.

**Sidebar.** Lists the pages of the current section: a `NavTree` of collapsible groups when there are many, a flat list when there are few. Its heading names the section (such as Actions); the selected item names the page. It is resizable across the available space with pointer and keyboard, collapses completely, and restores its width and selection. Expanded, it pushes the work area over; a narrow window or browser zoom never turns it into an overlay or changes the user's collapse choice. One toggle shows and hides it, fixed at the top of the Rail, in the same place whether the Sidebar is open or closed. Without a Rail, the toggle stays reachable in the header.

**Tab row.** Switches peer views of the selected page, or holds closable document tabs (separate open objects). Put it at the top of the work area, aligned with the Sidebar heading, with no page title row above it. While the Sidebar is collapsed, show a compact page name beside the tabs.

- One Tab row per page. Never place one directly above, below or inside another. Groups stay in the same row.
- More depth than one row carries goes into the Sidebar tree or a toolbar toggle.
- A real visual break (a framed specimen, an embedded app window, the bottom panel) may hold its own tabs. Mark it with `TabBoundary`. Debug builds warn about stacked rows.
- Without a Sidebar, the section's pages become the one Tab row and pages render without their own tabs (`ShellPageState.tabbed` on web). Without a Rail either, every page shares one grouped row.
- Tab rows scroll natively at narrow widths and keep full labels.
- Closing a tab is immediate (close control, Delete or Control W). No confirmation, no unsaved dot.

**Settings window.** `SettingsWindow`: a dialog with search and a section list, where changes apply at once. It opens from the gear fixed at the bottom of the Rail (the bottom of the Sidebar when there is no Rail) and from Control comma; with the Menu bar on, also from File. If both Rail and Sidebar are hidden, keep a route to Settings and to reopen navigation. Settings is never a page in the navigation and never in the header.

- Appearance (the theme) and About are built-in sections. The app's own sections come first. Group settings by purpose. Per-run options stay beside their operation, not in Settings.
- About leads with the app's name, description, logo and version (from the app's own version source). Beneath it sit the smaller Stack Junkie Style mark, the library version, project link and copyright. Keep the library version separate from the app's version.
- Import `AboutSection` or `ThemeSetting` only when replacing a built-in section.

**Menu bar.** Document editor Preset only. File, Edit, View, the app's menus, then Help. The frame owns Settings (File), About (Help) and the View toggles for Sidebar (Control B), bottom panel (Control J), Status bar and theme. Edit holds only the app's items and is left out without them.

**Status bar.** A thin bottom row: the current item's state on the left, small facts (position, encoding, version) on the right. Never ads or tips.

**Bottom panel.** A resizable panel under the work area with its own Tab row, for output, problems or a terminal. It is drawn only when it has content.

**Logo slot.** The app's logo sits in the App frame's logo slot (web `AppShell` `logo`) and clicking it returns to the app's Home, never the style's website. Until the app has its own logo, use the interim SJ mark (web `BrandMark`; shared images in `ui/shared/brand`). Work tools otherwise carry no personal branding: plain functional names, no "Stack Junkie" prefix.

## Website

The Website Preset uses `WebsiteLayout`, a separate component from the App frame: a header with the logo, site navigation and a dark/light toggle, one reading-width column, and a footer with site links and copyright. No Rail, Sidebar or Settings window. The default theme follows the device until the visitor picks one. Below 600 pixels the navigation folds into a menu button. It is web only; the Website Preset has no native starter.

## Product layouts

The catalog's eight layout IDs ([catalog.json](../ui/shared/catalog.json) `layouts`) are starting compositions, not mandatory pages. Choose by the task, the objects people move between and how often they switch. Classify each surface of a product separately. Use only regions that earn their place.

| Layout ID | Fits | Starting composition |
| --- | --- | --- |
| `focused-utility` | One recurring operation with a few choices | Input, action and result; optional controls beside their purpose |
| `multi-tool` | Distinct operations with their own setup or saved items | Rail, Sidebar and work area; persistent active status and Stop |
| `embedded` | A tool inside another product | The host's navigation, size and state; local actions, no competing App frame |
| `data` | Areas operating on related records | Destinations, filters, collection and detail inspector; preserved selection |
| `workbench` | Many pages, tool families and open work | Rail, Sidebar tree of every page, one row of open documents, optional bottom panel |
| `publication` | Browsing, searching and long reading | `WebsiteLayout`; readable content and local contents; preserved reading position |
| `guided` | Ordered steps to finish a task | Position, required inputs, consequences and safe backtracking; preserved drafts |
| `monitoring` | Noticing changes and investigating | Status and exceptions first, then evidence; retained filters, units and timestamps |

The `workbench` is the reference for apps with many pages, like ART.

## Navigation sense

- Each control selects one thing. The Rail picks a section, the Sidebar a page, the Tab row a view or document. Two controls never silently select the same destination.
- Returning to a page restores its object and view. Labels, the active treatment and keyboard focus make the location obvious.
- A second sidebar earns its place only when it answers a different question than the first.
- When navigation already names the page, content starts at the first meaningful choice. No repeated page title.

## Splits and narrow space

- Resizable panes take pointer and keyboard resizing, collapse and reopen controls, and restore width and selection. Collapsing never discards edits or strands focus.
- Allow resizing across the available space, not fixed width bands. When the window temporarily limits a pane, keep the chosen width for later.
- A small utility does not grow a Rail; a multi-tool utility may need a larger window.

## State belongs to the product

Active work is independent of the selected page. Switching pages or tabs never stops an operation or changes who owns input; persistent status and Stop or Cancel follow the running operation on every view. Drafts, selections, filters, scroll position and pane state survive navigation.
