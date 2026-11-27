---
name: stack-junkie-style
description: Apply Christopher Kvamme's Stack Junkie UI system by default when building, editing, or reviewing web, desktop, mobile, and application interfaces, unless the user explicitly chooses another design system. Reuse its components and layouts where the toolkit fits; preserve established conventions for narrow edits.
---

# Stack Junkie Style

Build the requested interface around its real work. Keep existing capabilities, approved compositions and later corrections in their original scope. A clean screen still fails if it loses a required operation or misrepresents what is running.

## Start with the product

Identify the task, toolkit, shared controls, theme, affected surfaces and state that must survive navigation. Read the original request and material later corrections before changing their result. Briefly explain the primary task, what stays visible, what becomes optional and how a narrow layout behaves. A small change needs only a small explanation.

Load the references needed for the change:

- Appearance or theme work: [visual system](references/design-system.md), then the relevant values in [shared tokens](ui/shared/tokens.json).
- Font hierarchy, reading surfaces or compact application sizing: [typography and density](references/typography-and-density.md).
- Controls or component reuse: [components](references/components.md), the applicable [catalog](ui/shared/catalog.json) entries and the matching toolkit implementation.
- Page structure, navigation or responsive work: [layouts](references/layouts.md).
- Content, disclosure, feedback, help or interaction work: [product UX rules](references/product-ux-rules.md).
- Repository setup or delivery documentation: [application delivery preferences](references/application-delivery-preferences.md).

## Use the working system

Consuming products import the shared library and its `AppShell` frame rather than copying components or styling. Settings always opens as a window (`SettingsWindow`) from the gear at the bottom of the activity rail, or of the sidebar when the rail is absent, and from Control comma. It is never a page in the app's navigation. Theme and About are sections of that window; keep the compact style attribution and library-sourced version beneath the consuming product's own identity. Showcase-only controls and explanatory content remain outside consumer imports.

[UI package instructions](ui/README.md) describe the React/TypeScript web and Qt 6 Widgets native libraries and their showcase commands. The showcases import those libraries. Compose named components and layout primitives; do not recreate their styling in each page or maintain a separate showcase theme. Other toolkits reuse the decisions and tokens through an appropriate adapter, not incompatible executable controls.

The working showcase runs on `AppShell`: a section rail, an adjustable sidebar, and one row of tabs. Clean utility is the only design direction. Its dark mode is true black (`#000000`) with structure from thin borders, near-white text and the blue accent. Light mode is one white ground with thin borders. Avoid gray fills used as structure in either mode. Retained user theme selections still win. The accent is blue. A default preview is not an approved design. Follow an explicit product choice when one exists; show a concrete comparison when a new aesthetic choice is needed. The old static specimen is not a second source of truth.

## Check and hand off

Use permitted source, API and command-line checks. Never take control of the owner's PC or use browser or desktop UI automation. The owner operates the GUI for visual and interaction review. Prepare usable launch instructions and report what still needs that review; a successful build does not establish visual acceptance.

Fix observed mismatches without adding validators, compliance reports, audit loops or permanent checking machinery. When changing this system, keep the component, its example and affected guidance in the same release, using the package [VERSION](VERSION). Report the changed surfaces, behavior and actual verification. Do not install, publish or change another product merely because this skill was invoked.
