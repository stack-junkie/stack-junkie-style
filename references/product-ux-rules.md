# Product UX rules

Use this reference for content, disclosure, feedback and interaction. Choose structure with [layouts](layouts.md) and reuse the [components](components.md) that implement the behavior.

## Preserve the requested result

Reconcile the requested outcome, existing operations, approved mockup details and later amendments. Separate owner choices from assistant proposals. Preserve capability when simplifying and keep platform-specific scope intact. A mobile change is not permission to redesign desktop behavior.

An approved composition includes placement and state behavior. A collapsed context field must actually be absent when that is the chosen behavior. A prototype, application and packaged deliverable are different requests; deliver the one requested at the required interaction depth.

## Visible content and optional detail

Keep the current object, required input, next action, consequential state and blocking information visible. Place per-run choices beside their operation; persistent defaults belong in Settings. Make secondary operations discoverable without competing with the primary task.

Do not invent metrics, activity, categories, controls or filler copy to complete a layout. A quiet empty area is preferable to invented content. Empty states explain the missing object and a real next action. Distinguish an empty collection, no filtered matches, unavailable capability and failed operation.

### Standalone titles

Titles stand alone by default. Do not automatically add subtitles, eyebrow labels, taglines or introductory paragraphs. Supporting text must add necessary context, a consequence or help with acting. If navigation already names the page, a duplicate heading needs a separate purpose. Do not promote the selected object into a large title merely to fill a summary panel.

Keep labels, values, prerequisites, errors, warnings and recovery instructions visible when they matter. Disclose occasional explanations or advanced controls through a predictable label. Avoid unnecessary nested disclosures. Removing a redundant summary must leave its actual controls and consequences available.

## Contextual help

Provide a named question-mark control where a real explanation is useful. Hover or keyboard focus previews help; click or tap pins it. Escape or an outside click dismisses it. Keep it available while the pointer enters the help and inside the available window. Restore focus appropriately after dismissal. Clicking another meaningful control should keep focus on that control.

Use a nonmodal popover for pinned, linked or interactive help. A simple tooltip is supplemental and noninteractive. Never put essential instructions, state or errors only in either surface. Do not repeat a visible label as its entire explanation. Support keyboard and touch without requiring hover. Give the control a usable target even when the symbol is small.

## Operations, feedback and recovery

An action needs a perceivable response proportionate to its duration. Local changes can confirm themselves; longer work needs busy or progress state and appropriate cancellation. Preserve input on failure, explain the failure near its consequence and offer recovery. Prevent duplicate submission while the operation is pending. Reserve confirmation for consequential, hard-to-reverse actions.

Derive persistent status and Stop or Cancel from the running operation. Switching pages or tabs must not change the reported operation, input ownership or stop action. A page named Hold cannot claim the pointer is free while foreground playback is using it. Keep operation state separate from the selected view.

Preserve edits, selections, filters, scroll position and relevant pane state across navigation and disclosure. Resolve unsaved work before discarding it. Failed saves preserve the draft and prevent pending navigation that would lose it. Distinguish paused, failed, disconnected and completed states.

## Reflow, text and access

At narrow sizes, reduce excess spacing, wrap text, collapse secondary navigation or stack regions before making controls unreadable. Keep primary actions, important state and a route to secondary capabilities available. Scroll the appropriate content region. A no-scroll requirement belongs to the product that requested it; plots, maps and data grids may need two-dimensional navigation.

Wrap natural language at word boundaries. Break long identifiers only when necessary. Do not truncate primary actions, errors or required instructions. Constrained paths and table values may use ellipsis if the full value is reachable through focus, selection or detail. Exercise long names and larger text through permitted checks and owner review.

Use semantic controls with accessible names, roles, values and state. Preserve keyboard activation, logical focus order, visible focus, text scaling and reduced motion. Do not rely on color or sound alone. Native widgets must expose native accessibility semantics. Do not claim screen-reader or keyboard acceptance from source inspection alone.

## Containers and inspection

Choose a tooltip for brief supplemental text, a popover for contextual help, an inspector for persistent secondary controls, a modal for a blocking decision, and a page for long material. Modal focus and dismissal must be intentional. Avoid visually competing background controls when a true modal is active.

Explain hierarchy and narrow behavior briefly before a material redesign. Use permitted source, API and CLI checks. The owner operates the GUI; do not use browser or desktop automation. Report unreviewed appearance, keyboard, assistive-technology or native behavior accurately. Do not add reports, validators or checking infrastructure to ordinary UI work.
