# Components and showcases

Use this reference when building or changing controls. [catalog.json](../ui/shared/catalog.json) names component and layout coverage, states and style-selection status. [UI instructions](../ui/README.md) provide entrypoints and commands. Read the actual exported API before importing it.

## Reuse within the right toolkit

Import the shared implementation instead of copying components or styles into consuming projects. Change repeated details in the library and use supported configuration for product differences.

The web library targets React and TypeScript. The native library targets Qt 6 Widgets and C++17. Each showcase imports its own component library and consumes the shared semantic tokens and catalog. They share design decisions, not executable widgets.

Qt 5, WPF, mobile and other applications cannot import React or Qt 6 controls unchanged. Preserve the toolkit and adapt shared roles and behavior through native controls. Do not migrate an application solely to use this package. Reuse a compatible existing component before introducing another implementation.

Compose named primitives with explicit properties, state and events. Pages supply domain content and orchestrate the product. A shell arranges navigation and content; it must not become a giant class implementing every control and workflow. Extract meaningful repeated combinations rather than adding generic components for every wrapper.

Use the library's theme and tokens. Do not copy buttons, fields, tabs or stylesheets into each page to change appearance. Add a legitimate variant to the shared component and demonstrate it. Keep semantic intent, such as destructive action or invalid input, separate from visual styling.

## Coverage by family

| Family | Provides | Preserve |
| --- | --- | --- |
| Foundations | Theme, typography, spacing, surfaces and icons | Readable role pairs, scaling and semantic status colors |
| Actions | Buttons, icon actions, links and command groups | Names, keyboard activation, disabled and pending states |
| Inputs | Text/numeric entry, selection, toggles, ranges and search | Labels, values, units, validation and drafts |
| Navigation | Section rail, contextual sidebar, tabs and location cues | Selection scope, focus and distinct selected/hover states |
| Containers | Panels, disclosures, split panes and inspectors | Truly optional regions; resize, collapse and restore |
| Collections | Lists, tables, trees, grids and selection | Stable identity, filtering and empty/result distinctions |
| Feedback | Status, progress, notifications and empty/error surfaces | Accurate operation state, consequences and recovery |
| Overlays | Tooltip, help, popover, menu and dialog | Dismissal, focus return and appropriate modality |
| Content | Reading text, technical output and supporting media | Natural wrapping, hierarchy and access to full values |
| Layouts | Eight product compositions in [layouts](layouts.md) | Shared controls, persistent state and narrow behavior |

The catalog and exported implementation determine available named components. A family label does not establish that every possible widget or platform variant is implemented. When a requested control is missing, implement the needed component or adapter and its example within the authorized change.

## Examples use the implementation

`SettingsWindow` is the reusable Settings surface: a dialog with search and a section list, where changes apply immediately. `AboutSection` and `ThemeSetting` fill its built-in sections. `AppShell` adds them automatically. Theme selection belongs in Settings. About leads with the consuming product's identity and optional version, followed by the smaller persistent Stack Junkie Style mark, library version, project link and copyright. The library's version is generated from VERSION; product versions come from the consuming application's version source. Keep showcase explanations, sample data and configuration/export tools outside the public library exports and visibly mark their scope.

Show realistic, clearly labeled demonstration content and applicable states: default, hover, focus, selected, disabled, pending, empty, invalid and failed. Not every state applies to every component. Examples make actual behavior operable, including disclosure, cancellation and recovery; a painted image is not an interactive component.

`AppShell` (web and native) is the standard frame. Applications supply sections, pages and their own settings sections; the frame owns the rail, sidebar, Settings window, theme and About. It drops to `Workspace` only for frames `AppShell` cannot express. Web `Tabs` offers connected and underline variants, groups in one row (group chips collapse their tabs), and closable document tabs (`onCloseTab`, immediate, Delete or Control W). `NavTree` gives the sidebar collapsible page groups.

The showcase imports the same component an application uses. Keep sample data and showcase navigation separate from library behavior. Change appearance or behavior in that implementation, not through a private fix in the example.

For a component change, update implementation, relevant catalog entry, example and affected guidance in the same release. Both showcases report the package [VERSION](../VERSION). Use focused CLI checks for changed behavior and prepare owner-operated GUI review. State which interactions remain unreviewed. Do not add validators, test frameworks or review machinery unless explicitly requested.
