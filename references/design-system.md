# Visual system

Use this reference for appearance, themes and typography. Exact values live in [tokens.json](../ui/shared/tokens.json); component coverage and style-selection status live in [catalog.json](../ui/shared/catalog.json). Do not copy an old website palette into a second authoritative token table.

## Current design status

Clean utility is the only design direction. Its dark mode is true black with structure from thin borders; light mode is one white ground with thin borders. The accent is blue. The owner selected logo F on 2026-11-22, then clarified that only the SJ letters should remain, without the square background. Both showcases use shared brand/sj-transparent-mask.png, mapping S to the theme text color and J to its accent. The owner accepted the blue-J appearance and requested a smaller 25px mark on 2026-11-22. Logo selection does not establish acceptance of the overall interface.

Preserve an explicitly selected product identity during narrow edits. Earlier mockup approvals are evidence about their named product and version; they do not select every token for all future products.

## Authority and reuse

The latest applicable owner instruction controls the task. Within this package, tokens define shared values, the catalog identifies component and layout coverage, and the toolkit library defines executable behavior and appearance. The showcase imports that implementation. Repair mismatches among them instead of choosing whichever example is easiest to copy.

Consume semantic roles for surfaces, text, borders, focus, selection, actions and status. Keep success, warning, danger and data-series meaning distinct from the decorative accent. Do not communicate state by color alone. Check actual foreground/background pairs, including hover, selected and disabled states; a familiar color does not establish readable contrast.

Adapters turn these roles into each toolkit's theme mechanism. Web components consume shared theme properties. Qt controls use a shared palette, styles and component implementation. Pages supply content and application state, not copies of hex values, fonts or widget styling. A platform-specific measurement belongs in the adapter when it cannot be shared sensibly.

## Typography and density

Clean utility uses the platform UI font, with Segoe UI Variable Text or Segoe UI on Windows. Manrope remains bundled for branding and as the fallback, and Fira Code serves code and aligned values. See the researched [typography and density guidance](typography-and-density.md) for role sizes, weights, compact shell geometry and readable document layout. Honor platform text scaling and accessibility settings.

Labels, values and controls that do the same job share a text role. Use weight and spacing for emphasis before adding new type sizes. Reading surfaces need a reading scale and line length; inspectors need compact, readable controls. Do not shrink the entire interface or cut off required text to force a fit.

Spacing, borders, radius and elevation come from the active tokens. Group related controls and distinguish interactive states. Decoration needs a purpose. Cards, pills, sidebars, tabs and shadows are not universally prohibited; their use must fit the task.

## Shared resources and release

Use shared fonts and icons through the toolkit adapter. Icons need accessible names where their meaning is not supplied by a label. Decorative icons must not interrupt assistive reading. Reusing this system does not require copying Stack Junkie logos, site prose or unrelated imagery.

Use the package [VERSION](../VERSION) for both libraries and showcases. Update a changed component, its demonstrable states and affected guidance together. Keep [UI instructions](../ui/README.md) accurate. CLI checks establish source and build behavior; owner-operated GUI review determines visual preference and real interaction acceptance.
