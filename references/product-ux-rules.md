# Product UX rules

Structure comes from the [App frame](app-frame.md); text follows [no slop](no-slop.md).

## Build what was asked

- Build exactly the requested result: add nothing, remove nothing. Propose anything extra and wait for Chris's yes.
- Before a material redesign, show something Chris can look at (a coded mockup or the change in the real app) and wait for approval. Match the approved mockup, including placement and state behavior.
- Never regress. Simplifying keeps every existing operation, control and fixed behavior. A change on one platform stays on that platform.
- A prototype, an app and a packaged release are different requests; deliver the one asked for.

## What stays visible

- Show only what the core task needs: the current object, required input, next action, consequential state and blocking information. Everything else goes in tabs, Settings or a small circled "?" help control.
- Per-run choices sit beside their operation; persistent defaults go in the Settings window.
- Put things where people expect them. Follow platform conventions.
- A quiet empty area beats invented content. Empty states name what is missing and offer a real next action. Distinguish an empty collection, no filter matches, an unavailable capability and a failed operation.

## Dashboards

A Dashboard is allowed only when each tile shows a real number that answers a question the user actually asks and links to where they act on it. No Preset turns `dashboardHome` on by default, so a Dashboard is always a deliberate choice. No big summary cards on top of ordinary pages. The reference is the Projects dashboard in [ui/apps/projects](../ui/apps/projects/).

## Help

- A named "?" control sits where a real explanation helps. Hover or keyboard focus previews it; click or tap pins it; Escape or an outside click dismisses it and returns focus sensibly.
- Pinned or interactive help is a nonmodal popover; a tooltip is brief and noninteractive. Essential instructions, state and errors are visible on the page, never only in either.
- Help never just repeats the visible label.

## Operations and feedback

- Every action gets a response matched to its duration: local changes show themselves; longer work shows busy or progress state and offers Cancel.
- Persistent status and Stop or Cancel come from the running operation, not the selected page.
- On failure, keep the input, explain the failure next to its cause and offer recovery. Block duplicate submission while pending.
- Confirm only destructive actions that cannot be undone. Routine actions, including closing a tab, never ask "Are you sure?".
- Distinguish paused, failed, disconnected and completed.

## Containers

Tooltip for brief extra text, popover for contextual help, inspector for persistent secondary controls, modal for a blocking decision, page for long material. Overlays and notices always have a visible way to close them and never cover controls without one.

## Accessibility

Semantic controls with accessible names, roles, values and state. Keyboard activation, logical focus order, visible focus, text scaling and reduced motion all work. Native widgets keep native accessibility. Never rely on color or sound alone. Sizing and contrast rules are in [look](look.md). Report screen-reader and keyboard behavior you did not actually exercise as unchecked.
