# Stack Junkie web UI

Reusable React components and a local, interactive catalog. Clean utility is the only design direction. The showcase imports the same components exported to applications.

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

What the frame draws today: the rail; the sidebar as a tree, a flat list or nothing; closable document tabs (beside a sidebar); the default theme, where `system` follows the device; and compact or comfortable density. The frame does not own a command palette, so `overlays` can be a function that receives the choices, and `useShellChoices()` reads them anywhere inside the frame. The Menu bar, Status bar and bottom panel choices are accepted but drawn in a later preview. Website uses the App frame with rail and sidebar off until it gets its own layout.

In the showcase, Settings > Layout holds the preset picker and every choice. Changes apply to the showcase frame at once, are saved in local storage, and Export writes them into `App.tsx`.

The declarations and built library do not require the source `shared` directory in consuming applications. Keep the distributed font licenses with the package. Browser appearance and assistive-technology behavior still require owner review; a successful build alone does not establish that review.
