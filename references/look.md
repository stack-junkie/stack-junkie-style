# Look: color, type and density

Exact values live in [tokens.json](../ui/shared/tokens.json): colors under `directions.clean`, sizes under `typeScale`, weights under `typography`, frame geometry under `density`. The library applies them; pages never carry their own hex values, fonts or widget styling.

## Clean

- Dark mode is True black: a pure black ground, structure from thin borders, near-white text, blue accent. Gray fills used as structure belong to neither mode.
- Light mode is one white ground with thin borders and the same blue accent.

## True black done well

Pure black is kept, with the fixes from the products that ship it well (Apple's base and elevated backgrounds, X's Lights out) and the WCAG contrast rules. Every value is a token in `tokens.json`; never hand-pick a gray.

- **Black is the ground.** The page, Rail, Sidebar, work area and large windows such as Settings are `canvas`/`surface` (#000). Small floating layers (menus, popovers, tooltips, toasts, small dialogs, the command palette, drawers) use `elevated` with a `borderStrong` edge: a shadow cannot show on black, so the lift comes from the fill and the edge. A large dialog or separate window stays black with the `borderStrong` edge.
- **Inside an elevated layer, steps move up with it.** Hover uses `raisedOnElevated` and dividers use `borderStrong`, so they still show on the lighter fill. The library sets this; do not override it.
- **Controls meet 3:1.** The border that shows where a text box, select, menu button or switch is uses `controlBorder` (at least 3:1 against black and against `elevated`, WCAG 1.4.11). `border` is for divider lines only and stays quiet.
- **Selection is never carried by the fill alone.** Persistent selection (nav rows, list rows, table rows, toggles) uses the accent-tinted `selection` fill plus accent text or an accent edge. Menus highlight the item under the pointer or keyboard with `selection`; that is the menu convention and a menu has no persistent selection. Hover is the neutral `raised`.
- **Text is off-white, not #fff**, which glows at the edges on black. Selected text uses `textSelection`.
- **Disabled controls keep their outline** (`border`) and fade their text (`disabledText`); they are not faded as a whole, which makes them vanish on black.
- **Danger in a menu is danger-colored text**, never a filled block.
- **Prefer borders to fills in long scrolling content.** Fills that scroll over black can smear on some OLED screens (user reports, mostly phones, panel-dependent). Table headers and row stripes use `stripe`, which is black in dark mode.
- **Font rendering:** web dark mode uses grayscale smoothing, which only macOS browsers apply. Windows ClearType is not affected and needs nothing. Keep body weight at 400 and muted text at 12 pixels or more.
- Check dark mode by eye, and on an OLED screen while scrolling where you can. Screenshots and contrast numbers are not the whole look.

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
