# Stack Junkie web UI

Reusable React components and a local, interactive catalog. Clean utility, Warm editorial and Precise technical are provisional directions. The showcase imports the same components exported to applications.

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

Choose a family in the section rail and a component in the sidebar. The page shows the example, with Usage and states below it. The gear at the bottom of the rail, or Control comma, opens the Settings window: theme, layout, keyboard and About. Search spans all component families. Compare directions retains the original shared-input comparison. The comparison uses the same working inputs and results in all three columns. The theme and direction controls apply to the entire catalog. Every direction remains a preview pending owner selection.

Example edits remain when changing sections or tabs. Reloading resets example edits; direction, theme, per-family component selection, sidebar width and collapse preference are saved in local storage. The expanded sidebar always takes layout space; at narrow widths or high zoom, the work area moves rather than being covered. File selection reads names only. The catalog has no backend and sends no specimen data to a service.

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

Components use native HTML controls and native dialog behavior. Their props provide normal React controlled/uncontrolled input contracts. `ThemeProvider` scopes shared semantic tokens, typography and spacing, so multiple directions can coexist. `Workspace`, `SideNavigation`, `Tabs`, `DataTable`, `Canvas` and similar components accept application-owned values and change handlers.

The declarations and built library do not require the source `shared` directory in consuming applications. Keep the distributed font licenses with the package. Browser appearance and assistive-technology behavior still require owner review; a successful build alone does not establish that review.
