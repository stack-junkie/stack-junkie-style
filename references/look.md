# Look: color, type and density

Exact values live in [tokens.json](../ui/shared/tokens.json): colors under `directions.clean`, sizes under `typeScale`, weights under `typography`, frame geometry under `density`. The library applies them; pages never carry their own hex values, fonts or widget styling.

## Clean

- Dark mode is True black: a pure black ground, structure from thin borders, near-white text, blue accent. Gray fills used as structure belong to neither mode.
- Light mode is one white ground with thin borders and the same blue accent.

## True black done well

Pure black is kept (it is the look, and it saves power on OLED screens), with the fixes from the products that ship it well, Apple's dark mode and X's Lights out:

- **Black is the ground, not every layer.** The page, Rail, Sidebar and work area are `canvas`/`surface` (#000). Floating layers (menus, dialogs, popovers, tooltips, toasts, the command palette) use `elevated` with a `borderStrong` edge. A shadow cannot show on black, so the lift comes from the lighter fill and the edge.
- **Borders carry the structure, so they must show.** `border` is at least 1.5:1 against black; `borderStrong` is at least 1.5:1 against `elevated`.
- **Hover and selection never look alike.** Hover is a neutral lift (`raised`). Selection is tinted with the accent (`selection`) and keeps the accent text or edge. Two near-identical grays are a bug.
- **Text is off-white, not #fff.** Pure white on black glows at the edges (halation). `text` stays about 17:1, `muted` above 4.5:1 on black and on `elevated`.
- **Type renders heavier on black.** Web uses grayscale font smoothing in dark mode; do not add weight for emphasis that spacing or position already gives.
- **Long scrolling content stays on black.** Filled dark cards scrolling over black smear on OLED screens as their pixels switch on. Separate scrolling items with borders; keep fills for fixed chrome and floating layers.
- **The accent is lighter in dark mode** (`accent` is a light blue), so it holds contrast on black and on `selection`.
- Check dark mode on an OLED phone or laptop while scrolling, not only in screenshots.
- A theme the user picked is kept. A new user gets the Preset's default theme; `system` follows the device.
- Use semantic roles (surface, text, border, focus, selection, action, status). Success, warning, danger and chart series keep their own meanings, distinct from the accent. State never rides on color alone.
- No gradients, glows, heavy shadows or decorative pills. Cards, sidebars and tabs are fine where the task needs them.

## Fonts

- UI text uses the platform font (`system-ui`): Segoe UI Variable Text or Segoe UI on Windows. Manrope is bundled as the fallback. Fira Code is for code, logs and aligned values.
- Use available system fonts; never copy proprietary font files into the package.

## Type roles

Controls doing the same job share one role from `typeScale`:

- `chrome`: navigation, tabs, toolbar controls.
- `label`: ordinary interface and form text.
- `caption`: metadata, version, annotations.
- `section`: short groups inside a tool.
- `reading`, `subheading`, `heading`, `title`: prose surfaces, with the reading line height.

Emphasize with weight and spacing before adding a size. A pane name is not a page heading. Selected items are not bold just for being selected; the background, edge and position already carry selection. Real heading elements carry the outline, not just bigger text.

## Density

- Tools are compact; reading surfaces are comfortable (the Preset's `density` choice). Reduce padding, gaps and redundant header layers before shrinking reading text.
- Frame heights in `density` are minimums, not text-clipping boxes. Touch targets stay larger.
- The App frame stays put while the content pane scrolls. Reading pages use normal page flow and a reading measure around 65 characters; tables and canvases are not width-limited.

## Fit and text scaling

- Everything fits the window. As it narrows, scale down proportionally: reduce spacing, collapse secondary navigation or stack regions, shrink text, then wrap. Nothing is clipped or cut off. Tab rows, plots, maps and data grids may scroll in their own region.
- Wrap at word boundaries; never split a word. Long identifiers break only when they must. Truncate only constrained values whose full text is reachable on focus or in a detail view; never primary actions, errors or instructions.
- Text enlarges to 200% without losing content or operation. At 320 CSS pixels wide, content reflows without whole-page horizontal scrolling.
- Survive user text spacing overrides (1.5 line height, 2em paragraph spacing, 0.12em letter, 0.16em word spacing). No fixed-height text boxes.
- Normal text meets 4.5:1 contrast, large text 3:1. Check the actual pairs, including hover, selected and disabled states.
- Pointer targets are at least 24 by 24 CSS pixels; the library uses 28 for compact icons and 44 for coarse pointers.
