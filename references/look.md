# Look: color, type and density

Exact values live in [tokens.json](../ui/shared/tokens.json): colors under `directions.clean`, sizes under `typeScale`, weights under `typography`, frame geometry under `density`. The library applies them; pages never carry their own hex values, fonts or widget styling.

## Clean

- Dark mode is True black: a pure black ground, structure from thin borders, near-white text, blue accent. Gray fills used as structure belong to neither mode.
- Light mode is one white ground with thin borders and the same blue accent.
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
