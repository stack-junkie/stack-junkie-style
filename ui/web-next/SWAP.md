# Swap the candidates in

Use this once Chris approves `ui/web-next` and `ui/native-next`. Until then, nothing here is applied.

## Steps

1. **Web:** replace `ui/web/src` with `ui/web-next/src`. Keep `ui/web/package.json`, its lockfile and `node_modules`. Delete `ui/web-next`, which includes this file, so copy the rule text below out first.
2. **Native:** replace `ui/native/{include,src,showcase,CMakeLists.txt,README.md}` with the `ui/native-next` versions, then delete `ui/native-next`, its build folder, and `ui/dist/windows-next`.
3. **Tokens:** move the overrides into `ui/shared/tokens.json` for `directions.clean`:
   - dark: canvas/surface `#000000`, raised `#0e0e0e`, text `#ededed`, muted `#8f8f8f`, border `#1f1f1f`, selection `#141414`, onAccent `#000000`
   - light: canvas/surface `#ffffff`, raised `#f4f5f7`, border `#d5dae1`

   Then delete the override blocks in `src/library/theme.tsx` and `native/src/theme.cpp`.
4. **Catalog** (`ui/shared/catalog.json`):
   - Change the layouts family icon to `layout`, then delete the `familyIcon` override in `App.tsx` and the `iconName` override in native `main.cpp`.
   - Remove the `settings/appearance` and `settings/about` component entries, or describe them as sections of `SettingsWindow`.
   - Change the workbench description to "Section rail, collapsible sidebar tree, one row of open documents and a bottom panel."
   - Change the sidebar and tabs entries to match the rules below.
5. **Version:** bump `VERSION` (for example `3.0.0-preview.10`), then run `ui/scripts/prepare.mjs` to sync `web/package.json`. `package.mjs` needs no change because it already packages `ui/web` and `ui/native`.
6. **`.gitignore`:** remove the `web-next` and `native-next` lines.
7. **Rules:** apply the replacements below to the skill.

## Rule replacements

**`SKILL.md`, "Use the working system", replace the Settings sentences with:**

> Consuming products import the shared library and its `AppShell` frame rather than copying components or styling. Settings always opens as a window (`SettingsWindow`) from the gear at the bottom of the activity rail, or of the sidebar when the rail is absent, and from Control comma. It is never a page in the app's navigation. Theme and About are sections of that window.

**`SKILL.md`, replace the "The working showcase uses…" sentence and the neutral-dark sentence with:**

> The working showcase runs on `AppShell`: a section rail, an adjustable sidebar, and one row of tabs. Clean utility's dark mode is true black (`#000000`) with structure from thin borders, near-white text and the blue accent. Light mode is one white ground with thin borders. Avoid gray fills used as structure in either mode.

**`references/components.md:36`, replace the first sentence with:**

> `SettingsWindow` is the reusable Settings surface: a dialog with search and a section list, where changes apply immediately. `AboutSection` and `ThemeSetting` fill its built-in sections. `AppShell` adds them automatically.

**`references/components.md:40`, replace with:**

> `AppShell` (web and native) is the standard frame. Applications supply sections, pages and their own settings sections; the frame owns the rail, sidebar, Settings window, theme and About. It drops to `Workspace` only for frames `AppShell` cannot express. Web `Tabs` offers connected and underline variants, groups in one row (group chips collapse their tabs), and closable document tabs (`onCloseTab`, immediate, Delete or Control W). `NavTree` gives the sidebar collapsible page groups.

**`references/layouts.md:13`, workbench row:**

> Section rail, collapsible sidebar tree of every page, one row of open documents, optional bottom panel with its own tabs.

**`references/layouts.md:20`, replace the Settings sentences with:**

> Settings is a gear pinned to the fixed bottom of the activity rail, or of the sidebar without a rail, and it opens the Settings window. It is never a navigation page, and it never moves into the header.

**`references/layouts.md:22`, append:**

> Never place a tab row directly under or inside another tab row. When a page has more depth than one row can carry, put the depth in the sidebar tree (`NavTree`) or a toolbar toggle. A framed specimen, an embedded app window, or a separate panel region (such as a bottom panel) is a real visual break and may have its own tabs; mark it with `TabBoundary`. Debug builds warn about stacked rows.

**`references/layouts.md:24`, replace with:**

> `AppShell` composes the rail and sidebar. With many pages, the sidebar is a `NavTree` of collapsible groups. Without a sidebar, a section's pages become the page's one tab row, and pages render their content without their own tabs (`ShellPageState.tabbed`). Without a rail either, every page shares one grouped row.

**`references/typography-and-density.md:35`, replace the first sentence with:**

> Use the rail for top-level areas, the sidebar tree for every page in the area, and one tab row for open documents or peer views.

**`references/typography-and-density.md:46`, replace "Collapse the contextual sidebar into an accessible overlay and let toolbars wrap." with:**

> Let toolbars wrap. The expanded sidebar keeps taking layout space and pushes the work area; it never turns into an overlay on its own.

Chris settled this on 2026-11-25 in `codex://threads/01a1dd06-1a23-70c0-8d73-825d9692133d`, where he complained that when zoomed in, the sidebar sat on top of the page instead of moving it. `layouts.md:36` already says this.

**`ui/README.md`, "Settings and About" section, replace paragraphs two to four with:**

> Settings is the gear at the bottom of the activity rail (or the sidebar without a rail) and opens `SettingsWindow`. Control comma opens it too. Appearance (theme, plus showcase-only direction comparison), Keyboard and About are its sections. The header has no theme or search button.
>
> Apps use `AppShell`: supply `name`, `description`, `version`, `logo`, `sections` (each with pages) and optional `settings`. On web it accepts controlled `location`, `mode` and `settingsState`, plus slots for the brand, sidebar header, sidebar override and overlays. Native `AppShell` derives from `Workspace` and adds the Settings window.
>
> Import `AboutSection` and `ThemeSetting` only when replacing a built-in section. The rule about versions is unchanged.

Also replace its last line, "A configuration download feature is not implemented", with:

> Settings › Layout exports an `App.tsx` that imports the library at the current version and configures `AppShell`.

**`ui/web/README.md:29`, replace "Narrow screens use the same contextual sidebar as a dismissible overlay rather than shrinking the work area." with:**

> The expanded sidebar always takes layout space; at narrow widths or high zoom, the work area moves rather than being covered.
