---
name: stack-junkie-style
description: A design system for web apps, desktop apps, document editors and websites, with React and Qt libraries, Presets and starters. Use when planning, building, editing or reviewing an interface that uses Stack Junkie Style, or when the user asks for it. Not for a Look-alike app.
---

# Stack Junkie Style

Terms follow [GLOSSARY.md](GLOSSARY.md). Decisions behind them are in [docs/adr](docs/adr/).

## When it applies

A **Look-alike app** (built to imitate an existing app, like SJ Paint imitating Microsoft Paint) follows the imitated app's look and layout. It may still use library components where they fit. Keep it out of the App frame ([ADR 0002](docs/adr/0002-look-alike-apps-exempt.md)).

For a narrow edit in an existing project, keep that project's established conventions and change only what was asked.

## Start a project

1. Pick the Preset that matches the project from [presets.json](ui/shared/presets.json): Web app, Desktop app, Document editor or Website. Each carries a "made for" note.
2. Switch individual choices where the project needs it.
3. Create the starter with one command. It runs straight from the GitHub repo, installs the library from a tagged release ([ADR 0003](docs/adr/0003-install-from-github-tags.md)) and refuses a folder that is not empty:
   `npx github:stack-junkie/stack-junkie-style#v<version> new <folder> --platform web|native --preset <id> --choice <name>=<value> --logo <image>`
   The showcases' Builder ("Build an app") shows the same starter live and copies the same command; [llms.txt](https://style.stack-junkie.com/llms.txt) lists every Preset and choice.
4. Replace the starter's Stub areas, pages and panel with the app's own. Import the library and compose its components. Pages supply the app's own content, state and actions.

Every component and style comes from the library import. Copying a component, stylesheet or token value into a project is the drift this system exists to stop. Commands and APIs: [library](references/library.md).

## Settled rules

- **Clean** is the only look: True black dark mode, white light mode, thin borders for structure, blue accent. Values live in [tokens.json](ui/shared/tokens.json).
- **App frame**: Rail, Sidebar and one Tab row, drawn by `AppShell` on web and native. The Sidebar pushes the content over; it never overlays it, at any width or zoom.
- **Settings window** opens from the gear at the bottom of the Rail (the Sidebar when there is no Rail) and from Control comma. Settings is never a page in the navigation and never sits in the header.
- **One Tab row** per page, never stacked under or inside another.
- **Menu bar** only in the Document editor Preset: File, Edit, View, the app's menus, then Help. Settings sits under File and About under Help.
- **Status bar**: the current item's state on the left, small facts on the right. Never ads or tips.
- **Website** Preset: `WebsiteLayout` instead of the App frame: navigation in a top bar or a sidebar, header search and one main button, a reading-width column, and a **Site footer** that links to every page (switchable). `WebsiteHome` (featured article and recent list) and `WebsiteArticle` are its page templates.
- **Stub** marks all development stand-in content. It is striped in development and release builds fail while any remain, on web and native. **Hint text** is a short example inside an empty input; it never replaces the label and is not a Stub.
- **Dashboard** only when each tile shows a real number that answers a question the user asks and links to where they act on it. No Preset opens on one by default.
- **Logo slot**: the app's logo goes in the App frame's logo slot, and clicking it goes Home. Until the app has its own logo, use the interim SJ mark. Native apps merge it with the title bar: one top row for the logo, name, menus and window buttons.
- **Page content** stays app-specific. The skill never invents it. Write it to the [no slop](references/no-slop.md) rules.

Details for each rule: [App frame](references/app-frame.md).

## Verify

Look at screenshots of every changed page in dark and light before calling visual work done. A hex value, a token diff or a passing build is not a look. Take the screenshots in your own isolated preview (the built-in browser pane for web, the native showcase's offscreen `--export-preview`). Driving the user's desktop needs their yes first. Report what you looked at and what you did not check. Fix what you see; add no validators, compliance reports or audit loops.

## References

- Building screens, navigation, Presets, Settings or layouts: [App frame](references/app-frame.md).
- Colors, fonts, sizes, density or text scaling: [look](references/look.md).
- Installing, importing, Stubs, the Stub guard, or changing the library itself: [library](references/library.md).
- Content, help, feedback, errors, forms, Dashboards or accessibility: [product UX](references/product-ux-rules.md).
- Any visible text: [no slop](references/no-slop.md).
- README, versions or release conventions of an app repo: [delivery](references/delivery.md).
