# Stack Junkie UI, next candidate (web)

A side-by-side candidate of the web showcase and library. The current showcase in `../web` is unchanged. Compare them in two tabs:

- Current: `npm run dev` in `../web` → http://127.0.0.1:5173/
- Candidate: `npx vite --host 127.0.0.1` here → http://127.0.0.1:5174/

`node_modules` is a junction to `../web/node_modules`. Saved showcase choices use their own `sj-next-*` browser storage, so the two versions don't overwrite each other.

## What changed from preview.9

- **Settings is a window, not a page.** `SettingsWindow` has a section list, search, and changes that apply immediately. It contains Theme, About, Keyboard and, in the showcase only, Layout and the comparison controls. Control comma opens it.
- **No stacked tab rows.** `Tabs` warns in development when a tab row sits inside another tab panel. `TabBoundary` marks a real visual break, such as a framed specimen or an app window.
- **Grouped tabs in one row.** Tab groups work like browser tab groups: a group chip sits in the same row and collapses the group. This replaces the old row of sub-tabs under a row of tabs.
- **Closable tabs.** Close happens immediately, with no confirmation and no unsaved dot. Delete or Control W closes the focused tab.
- **Sidebar tree (`NavTree`).** Collapsible page groups, VS Code style. This is where depth goes in apps with many pages.
- **`Workspace` options.** The contextual sidebar is optional, and `resizable` and `collapsible` can be turned off.
- **`AppShell`.** The standard app frame products import. It owns where the rail, sidebar, Settings, theme and About go. Products supply only sections, pages and their own settings.
- **Showcase Layout settings and Export.** Switches for the rail, contextual sidebar, resizing and collapsing reconfigure the showcase live. Export downloads an `App.tsx` that imports the library at the current version and uses `AppShell` with those settings.
- **Product layouts are sidebar pages.** Each layout is its own page shown in a frame, with its description behind "?". The workbench is rebuilt as the reference organization for many-page apps: rail, sidebar tree, one row of open documents, and a resizable bottom panel with its own tabs. A Starter app page previews exactly what Export produces.
- **True black dark mode, blue accent.** Clean utility's dark mode uses #000000 for the canvas and surfaces. Structure comes from thin #1f1f1f borders instead of gray fills, and text is near white (#ededed). The sidebar edge is a 1px line instead of a thick gray bar. These overrides live in `src/library/theme.tsx` until the shared `tokens.json` moves over. Blue stays the accent, chosen over teal on 2026-11-26.
- **Small fixes.** Settings rows no longer repeat their label above the control. The rail no longer shows a horizontal scrollbar. Scrollbars are thin, and tab rows show theirs only on hover. Product layouts get their own icon, so the gear belongs to Settings alone.

- **The showcase runs on `AppShell`.**
  - The frame now supports controlled location, mode and Settings state.
  - It has slots for the brand, sidebar header, sidebar override and overlays.
  - Sections can be hidden or left out of the rail.
  - Pages can be functions that receive `{ sidebarCollapsed, tabbed }`.
  - Settings sections can replace the built-in Appearance and About.
  - A `mainId` adds a skip link, and `shortcuts={false}` is available for embedded previews.
  - The showcase builds nothing of its own frame, so anything wrong in the showcase frame is wrong for every app.
- **Light mode follows the same rule.** It uses one white ground, thin borders, and gray only for raised controls. The selected tab scrolls into view when navigation changes it.

The Qt side is in `../native-next`. The shared `catalog.json` and `tokens.json` changes, the skill's rule rewrites and the swap steps are in [SWAP.md](SWAP.md).
