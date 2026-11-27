# Stack Junkie Style

A reusable design system for app and site interfaces, so a new project inherits its look, layout and behavior instead of rebuilding them.

## Language

### Appearance

**Clean**:
The single visual direction of the system: true black dark mode, a white light mode, thin borders for structure, and a blue accent.
_Avoid_: Clean utility, theme, direction

**True black**:
Dark mode whose ground is pure black, with structure drawn by thin borders rather than gray fills.
_Avoid_: Near black, dark gray

### Configuration

**Preset**:
A named starting set of App frame choices for one kind of project (Web app, Desktop app, Document editor, Website). Its choices can be switched individually, and the result saved as a custom preset.
_Avoid_: Template, profile, mode

**Builder**:
The full-window showcase screen where you pick a Preset, switch its choices on a blank App frame (or Website layout), and export exactly what it shows. The one place for presets, custom presets and Export.
_Avoid_: Wizard, configurator, starter preview

**Website**:
The Preset for reading sites: a top header with logo and navigation, a footer, a reading-width column and a small theme toggle, instead of the App frame.
_Avoid_: Site layout, blog template

**Look-alike app**:
An app built to deliberately imitate an existing app (such as SJ Paint imitating Microsoft Paint). It follows that app's look instead of this system.
_Avoid_: Clone, special case

### Frame and navigation

**App frame**:
The standard outer shell every app inherits: rail, sidebar, title bar, and the gear that opens the Settings window.
_Avoid_: Shell, layout, chrome, wrapper

**Rail**:
The narrow column of icons on the far left that switches between an app's main areas.
_Avoid_: Activity bar, icon bar, sidebar

**Sidebar**:
The panel beside the rail listing the pages of the current area, shown as a collapsible tree when an app has many pages.
_Avoid_: Subsidebar, contextual sidebar, nav panel

**Tab row**:
A single row of tabs. A page never shows one tab row stacked under or inside another.
_Avoid_: Tab bar, sub-tabs

**Menu bar**:
The top row of menus in a Document editor: File, Edit, View, the app's own menus, then Help. Settings sits under File and About under Help.
_Avoid_: Toolbar, top bar

**Status bar**:
The thin bottom row of a desktop app: the current item's state on the left, small facts on the right. Never ads or tips.
_Avoid_: Footer, info bar

**Settings window**:
The pop-up window, opened from the gear or Control comma, that holds all settings. Settings are never a page in the app's navigation.
_Avoid_: Settings page, preferences page

### Input

**Stub**:
Development stand-in content (sample text, fake data, unfinished labels) marked so it can never ship: it is visibly striped during development, and production builds refuse to finish while any remain.
_Avoid_: Placeholder, dummy content, lorem ipsum

**Dashboard**:
A home page made of tiles, where each tile shows a real number that answers a question the user actually asks and links to where they act on it.
_Avoid_: Overview, stats page

**Hint text**:
Short grey example text inside an empty input that shows what valid input looks like. It never replaces the input's label.
_Avoid_: Placeholder (ambiguous with stub content)
