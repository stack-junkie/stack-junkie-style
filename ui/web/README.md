# Stack Junkie web UI

Reusable React components and a local, interactive catalog. Clean is the only look. The showcase imports the same components exported to applications.

## Setup

Use Node.js 22.12 or newer. From this directory in PowerShell:

```powershell
npm install
npm run dev
```

Open the local address printed by Vite. The server binds to localhost and fails if its port is already occupied. It does not open or control a browser.

To build the showcase and the library:

```powershell
npm run build
npm run build:library
```

The showcase is written to `dist`. The library is written to `dist-library`, with declarations in `dist-types`. Shared preparation reads the package version from the skill's `VERSION` and emits application metadata. Fonts, icons, tokens and examples are local assets.

## How to use

Choose a family in the section rail and a component in the sidebar. The page shows the example, with Usage and states below it. The gear at the bottom of the rail, or Control comma, opens the Settings window: theme, layout, keyboard and About. Search spans all component families. The theme control applies to the entire catalog.

Example edits remain when changing sections or tabs. Reloading resets example edits; theme, per-family component selection, sidebar width and collapse preference are saved in local storage. The expanded sidebar always takes layout space; at narrow widths or high zoom, the work area moves rather than being covered. File selection reads names only. The catalog has no backend and sends no specimen data to a service.

Keyboard: Tab follows the controls, arrow keys move between tabs, and Escape dismisses dialogs and help. Split-pane dividers accept left/right arrows. Control K (Command K on macOS) opens the command palette. Question-mark help previews on hover/focus and pins on click.

### Use the library in an application

Build the library before installing this directory as a local npm dependency. Install React and React DOM in the consuming application. Import the public entry and stylesheet:

```tsx
import { Button, TextInput, ThemeProvider } from '@stack-junkie/ui-web';
import '@stack-junkie/ui-web/styles.css';

export function SaveForm() {
  return (
    <ThemeProvider direction="clean" mode="light">
      <form onSubmit={event => event.preventDefault()}>
        <TextInput label="Document name" name="name" required />
        <Button type="submit" variant="primary">Save</Button>
      </form>
    </ThemeProvider>
  );
}
```

Components use native HTML controls and native dialog behavior. Their props provide normal React controlled/uncontrolled input contracts. `ThemeProvider` scopes shared semantic tokens, typography and spacing, so light and dark surfaces can coexist. Its `direction` prop accepts only `clean` and is optional. `Workspace`, `SideNavigation`, `Tabs`, `DataTable`, `Canvas` and similar components accept application-owned values and change handlers.

### Stubs

A Stub is development stand-in content: sample text, fake data or an unfinished label. Wrap it in `Stub` and it renders striped, so it is easy to spot. Use `block` around larger content. For a value that cannot show stripes (attribute text, fake data), wrap it in `stub()`:

```tsx
import { Stub, stub } from '@stack-junkie/ui-web';

<h1><Stub>Project name</Stub></h1>
<Stub block><ProjectList projects={stub(sampleProjects)} /></Stub>
<Button aria-label={stub('Open project')}>Open</Button>
```

Add the release guard to the application's Vite config. `vite build` then stops while any `Stub` or `stub` remains in the application and lists each one by file, line and column. `vite dev` is unaffected.

```ts
import { stackJunkieStubGuard } from '@stack-junkie/ui-web/vite';

export default defineConfig({ plugins: [react(), stackJunkieStubGuard()] });
```

The guard reads application modules only, so the library itself and `node_modules` are never reported. It counts named, renamed and namespace imports (`UI.Stub`) from `@stack-junkie/ui-web` or a relative path into the library. The showcase does not use the guard, so its Stub page can show real Stubs. Hint text (an input's `placeholder`) is not a Stub and is never reported.

`npm run test:stub-guard` builds a small sample app in `tests/stub-guard` with a Stub (it must fail and name each location) and without one (it must pass), against the source and, after `npm run build:library`, the built package.

### Presets

A Preset is a named starting set of App frame choices. The four presets live in `../shared/presets.json`, the one source that web, native and the skill read:

| Choice | Web app | Desktop app | Document editor | Website |
|---|---|---|---|---|
| `rail` | on | on | on | off |
| `sidebar` | tree | list | tree | off |
| `menuBar` | off | off | on | off |
| `statusBar` | off | on | on | off |
| `bottomPanel` | off | off | on | off |
| `documentTabs` | off | off | on | off |
| `commandPalette` | on | on | on | off |
| `theme` | dark | dark | dark | system |
| `density` | compact | compact | compact | comfortable |
| `dashboardHome` | off | off | off | off |

Pass a preset to `AppShell`, and override single choices with `choices`:

```tsx
<AppShell preset="desktop-app" choices={{ sidebar: 'tree' }} name="My app" description="What it does." version="0.1.0" sections={sections} />
```

`resolvePreset(id, overrides)` returns the full set of choices, and `presets` lists every preset with its label and description. Without `preset` or `choices`, `AppShell` keeps reading `rail`, `contextualSidebar` and `mode` from `config` as before.

What the frame draws today: the rail; the sidebar as a tree, a flat list or nothing; closable document tabs (beside a sidebar); the default theme, where `system` follows the device; and compact or comfortable density. The frame does not own a command palette, so `overlays` can be a function that receives the choices, and `useShellChoices()` reads them anywhere inside the frame. The Menu bar, Status bar and bottom panel are drawn when their choices are on (see below). The Website preset is not drawn by `AppShell`; use `WebsiteLayout` (below). Given `preset="website"`, `AppShell` only turns its rail and sidebar off.

In the showcase, Settings > Layout holds the preset picker and every choice. Changes apply to the showcase frame at once, are saved in local storage, and Export writes them into a starter project (see Export below). Preview the starter app shows the frame a starter gets, including the Website layout.

### Custom presets

A custom preset is a name, a base preset and only the choices that differ from it. The file format is shared with native and described in `../shared/PRESET-FILE.md`:

```json
{ "format": "stack-junkie-preset", "version": 1, "name": "My editor", "base": "desktop-app", "choices": { "sidebar": "tree" } }
```

The library reads and writes it, so an app can offer the same feature:

- `customPresetFile(preset)` returns the file text: two-space JSON, choices in `presets.json` order.
- `parseCustomPresetFile(text)` returns `{ preset }` or `{ error }`. Any unknown base, choice name or value, wrong type (`"true"` for `true`), missing or overlong name, other format or version, or extra field rejects the whole file with one plain message.
- `checkCustomPreset({ name, base, choices })` checks a preset built in code and trims it to the changed choices. `customPresetNameError(name)` checks a name alone.
- Apply one with `<AppShell preset={custom.base} choices={custom.choices} />`.

In the showcase, Settings > Layout > Save as custom preset names the current preset and choices. Custom presets are kept in local storage (`sj-v3-custom-presets`) and listed in the preset picker after the four presets, marked "(custom)". With one selected, its row offers Update (save the current choices into it), Rename, Export file (downloads `<name>.preset.json`) and Delete. Import file adds a preset from a file and selects it; a preset with the same name is replaced. Export starter uses the selected preset's base and choices.

`npm run test:presets` writes a preset file with the shared web logic, has the native showcase import it and export it again (`--import-preset`, `--export-preset`), and checks the bytes match and web reads it back. It also checks the native `--preset` and `--choice` route writes the same file, that a starter exported from an imported preset carries its choices, and that nine bad files are rejected by web and by native (non-zero exit, a message, no file written). It uses the newest Release showcase under `../native/build*/`, or `-- --showcase <exe>`; Qt comes from `-- --qt <prefix>`, `STACKJUNKIE_QT` or `../.tools/Qt/6.8.3/msvc2022_64`. The showcase runs offscreen.

### Export

Settings > Layout > Export starter writes a new project that matches the chosen preset and changed choices, as one zip (`my-app-web-starter.zip` or `my-app-native-starter.zip`). Pick Web starter or Native starter and preview any file before downloading.

- Web starter: a Vite, React and TypeScript project. `package.json` depends on `@stack-junkie/ui-web` as a git dependency pinned to this version's tag (`git+https://github.com/stack-junkie/stack-junkie-style.git#v<version>`, ADR 0003). It has `index.html`, `src/main.tsx`, `src/App.tsx` (`AppShell` with the preset and choices, or `WebsiteLayout` for the Website preset), `vite.config.ts` with `stackJunkieStubGuard()`, `tsconfig.json`, `.gitignore` and a short README.
- Native starter: `CMakeLists.txt` and `main.cpp` (see the native README). The Website preset has no native starter, and the dialog says so.
- `src/App.tsx` lists only the choices that differ from the preset, so deleting one goes back to the preset.

The file list and templates live in `../shared/starters`. `starters.mjs` beside them is the one generator: given a platform, a preset id and overrides, it returns the files. The native showcase fills the same templates the same way, so both Exports write identical starters.

`npm run test:starters` is the preset-to-starter test. It builds the library (`npm run build:library`), then for every preset generates the web starter into a temporary folder and typechecks and builds it against the local package (the dependency is pointed at this folder with `file:`). For the three app presets it builds and installs the native library into a temporary prefix, then configures, builds and starts (offscreen) each native starter with CMake, plus once through the FetchContent fallback with a local source. A desktop-app case with custom choices checks they reach the starter, and every starter is compared with the native showcase's `--export-starter` output. Each case prints PASS or FAIL; any failure exits non-zero. Native cases need CMake, Visual Studio 2022 C++ tools and Qt 6: pass `-- --qt <prefix>`, set `STACKJUNKIE_QT`, or keep Qt in `../.tools/Qt/6.8.3/msvc2022_64`. `-- --only web` or `-- --only native` runs one side, and `-- --keep` keeps the generated starters.

### Menu bar, Status bar and bottom panel

The Document editor preset turns all three on. `AppShell` draws them from the `menuBar`, `statusBar` and `bottomPanel` choices; the app supplies their content:

```tsx
<AppShell preset="document-editor" name="My editor" description="What it does." version="0.1.0" sections={sections}
  menuItems={{ file: [{ id: 'open', label: 'Open…', shortcut: 'Ctrl+O', run: openFile }], edit: [{ id: 'undo', label: 'Undo', shortcut: 'Ctrl+Z', run: undo }] }}
  menus={[{ id: 'format', label: 'Format', items: [{ id: 'bold', label: 'Bold', run: bold }] }]}
  status={{ state: 'Notes.md, saved', facts: ['Ln 12, Col 4', 'UTF-8'] }}
  panels={[{ id: 'output', label: 'Output', content: <Output /> }, { id: 'problems', label: 'Problems', content: <Problems /> }]} />
```

- Menu bar: File, Edit, View, the app's `menus`, then Help, in the header beside the app's name. The frame owns Settings… (File, Control comma), About (Help, opens Settings at About), and in View the Sidebar (Control B), Bottom panel (Control J) and Status bar toggles and Dark theme. `menuItems` adds the app's items above the frame's; Edit holds only app items and is left out without them. An item's `shortcut` is shown text, so the app owns its own keys. Arrow keys move between menus and items, Enter or Space runs an item, Escape closes the menu. `MenuBar` also works on its own.
- Status bar: a thin bottom row with `status.state` on the left (the current page's name when omitted) and `status.facts` on the right. Never ads or tips. `StatusBar` also works on its own.
- Bottom panel: a resizable panel under the work area with its own tab row of `panels`. Drag or arrow-key its top edge; the close control and View > Bottom panel hide it for the session. Its height is saved with `storageKey`. No panel is drawn without `panels`. `BottomPanel` also works on its own as the last child of a flex column.

The View toggles hide and show the parts for the session; the choices decide whether they exist.

### Website

`WebsiteLayout` is the layout for reading sites and the one the Website preset uses. It is a separate component, not a mode of the App frame: a header with the logo, site navigation and a theme toggle, one reading-width column, and a footer with site links and a copyright line. It has no rail, sidebar or Settings window.

```tsx
import { BrandMark, Prose, WebsiteLayout, type WebsitePage } from '@stack-junkie/ui-web';

const pages: WebsitePage[] = [
  { id: 'home', label: 'Home', content: <Prose><h1>My site</h1><p>Start here.</p></Prose> },
  { id: 'about', label: 'About', content: <Prose><h1>About</h1></Prose> },
];

<WebsiteLayout name="My site" logo={<BrandMark size={20} />} pages={pages} storageKey="my-site-theme" style={{ minHeight: '100dvh' }} />
```

- `pages` are the header navigation, in order. The current page is marked with `aria-current="page"`. A page without `href` switches in place; with one, the link is followed and `onNavigate` still reports it. `page` and `onNavigate` make the current page controlled, and `children` replace the current page's content.
- The main column is limited to the reading measure (`readingMeasure` at the reading size) and uses the reading line height. Wrap text in `Prose` for headings, lists and quotes.
- `theme` sets what a new visitor sees and defaults to `system`, which follows the device. The toggle switches dark and light and saves the choice under `storageKey` (default `sj-website`; an empty string keeps it for the visit only). Storage errors are ignored. `mode` and `onModeChange` make the theme controlled.
- `density` defaults to `comfortable`. `footerLinks` replace the page links in the footer, and `copyright` replaces the default year and site name.
- When the layout is narrower than 600 pixels, the navigation folds into a menu button and the gutters shrink to 16 pixels. It uses a container query, so it also folds inside a narrow frame.
- The layout fills its parent and does not reset the page. Give `html` and `body` a zero margin in the site's own stylesheet.

The declarations and built library do not require the source `shared` directory in consuming applications. Keep the distributed font licenses with the package. Check browser appearance and assistive-technology behavior by hand; a passing build does not cover them.
