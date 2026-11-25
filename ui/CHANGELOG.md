# Stack Junkie UI changelog

## 3.0.0-preview.9

- Make the header SJ mark open Home, with Foundations accessible there instead of the top rail icon.
- Place one navigation toggle at the fixed top of the activity rail. Remove the contextual collapse arrow and redundant main-area toggle.
- Move web component tabs into the top content row. Show the component name alongside tabs only when the contextual sidebar is collapsed; preserve mounted page and tab state.
- Share the navigation behavior and collapsed tab identity in the native library and showcase.

## 3.0.0-preview.8

- Pin Settings below the scrolling activity icons. Reusable shells place it at the contextual-sidebar footer when the rail is absent.
- Move theme, design comparisons and About into Settings; remove the web header search action while retaining command search through Control K.
- Keep contextual sidebars in the page layout at narrow widths and browser zoom. Remove fixed width caps and native automatic collapse.
- Add reusable SettingsPage, AboutPage and branding support. Product identity stays prominent above compact persistent style attribution and copyright.
- Expose the style-system version from the single VERSION source in About and imported libraries. Mark showcase-only controls explicitly and keep them out of library exports.

## 3.0.0-preview.7

- Reduce the SJ corner mark from 28px to 25px through one shared size token, following owner review of the blue J in the web preview.

## 3.0.0-preview.6

- Remove the logo's white square. Render the S in the theme text color and the J in its accent color using one shared transparent asset.
- Follow the active palette in both showcases, including light mode and alternate directions. The accent placement is a trial, not a final brand-color decision.

## 3.0.0-preview.5

- Use the owner's selected F logo, a black SJ monogram on a white square, in both web and Windows showcase headers.
- Share one standalone logo asset across themes and toolkits. Preserve the previous site assets and private selection history.

## 3.0.0-preview.4

- Use the platform UI font in Clean utility, with quieter shell labels, compact rows and shared density values across web and Qt.
- Separate reading body, subheading and heading roles; keep reading content comfortably sized and limit its line length.
- Align web contextual headers, retain adjustable sidebar state, and preserve the neutral black palette with softer ordinary text.
- Use the existing site SJ favicon as a small corner mark. New generated refinements remain candidates; the first six-logo sheet was rejected.
- Save primary-source typography, layout and accessibility research in the skill references.

## 3.0.0-preview.3

- Darken Clean utility to neutral black surfaces and remove the blue tint from selection backgrounds. Keep the light palette separate.
- Replace the invented initials badge with the site's Stack-Junkie header wordmark, using shared brand values in the web and native showcases.
- Clean utility is the owner's closest candidate; its overall appearance remains under review.

## 3.0.0-preview.2

- Make both showcases use a shared section rail, adjustable contextual sidebar and connected Example / Usage tabs.
- Preserve per-section selection, mounted example state and sidebar width through collapse; retain existing native sidebar preferences on migration.
- Restore the clean dark baseline to near-black neutral surfaces while keeping the other directions and light themes available.
- Keep headers and tabs outside independently scrolling example panels; use compact shell titles and controls.

Fonts, accents and owner-operated interaction acceptance remain open. This update does not install the skill or change ART.

## 3.0.0-preview.1

- Introduce shared light/dark tokens for Clean utility, Warm editorial and Precise technical.
- Add reusable web and Qt widget libraries, working showcases, 51 catalog entries and eight product layout patterns.
- Derive showcase and distribution versions from the skill's `VERSION`.
- Bundle fonts, licenses and icons for offline use.
- Replace the independently styled legacy HTML specimen with a route to the living showcase.

This is a working comparison preview. The final visual direction and owner interaction acceptance remain open.
