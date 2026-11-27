# Stack Junkie UI changelog

## 3.0.0-preview.10

- Swap the next web and native versions in. There is one web library and one native library again.
- Settings is a window (`SettingsWindow`) opened from the gear or Control comma, never a page.
- No stacked tab rows: groups sit in one row, document tabs close immediately, and debug builds warn about a tab row inside another. `TabBoundary` marks real visual breaks.
- Add `NavTree` for the sidebar tree and `AppShell` as the standard frame on web and native. The web showcase runs on `AppShell`.
- Clean utility's dark mode is true black with thin borders and near-white text; light mode is one white ground with thin borders. The values now live in the shared tokens.
- Product layouts use their own `layout` icon. The workbench is the reference for many-page apps: rail, sidebar tree, one row of open documents and a bottom panel.
- The expanded sidebar always pushes the work area and never becomes an overlay.
- Remove the Warm editorial and Precise technical directions. Clean utility is the only direction.
- Native: add `StackJunkie::Stub` and `STACKJUNKIE_STUB`. Stubs are striped in development builds; release builds fail to compile at each remaining Stub's file and line. Fix a debug-build assert from closable document tabs.
- Add Presets (Web app, Desktop app, Document editor, Website) in `shared/presets.json`. The web `AppShell` takes `preset` and `choices` and draws the rail, tree or list sidebar, closable document tabs, default theme (including system) and density. The showcase Settings window picks the preset and switches every choice, and Export writes them into `App.tsx`.
- Add `WebsiteLayout` on web for the Website preset: a header with logo, navigation (a menu button at narrow widths) and a dark or light toggle that defaults to the system and is saved, one reading-width column, and a footer. It is separate from `AppShell`. Export writes a Website `App.tsx` that uses it, and Product layouts has a Website page.
- Add web Stubs. `Stub` (and `stub()` for plain values) renders development stand-in content striped. `stackJunkieStubGuard()` from `@stack-junkie/ui-web/vite` stops an application's release build while any Stub remains and names each one by file:line. `npm run test:stub-guard` checks it on a sample app. Hint text is unaffected.

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
