# Stack Junkie product UX rules

## Contents

1. [Purpose](#purpose)
2. [Frontload test](#frontload-test)
3. [Anti-slop element test](#anti-slop-element-test)
4. [Progressive disclosure](#progressive-disclosure)
5. [Help cues](#help-cues)
6. [Feedback and system status](#feedback-and-system-status)
7. [Responsive and narrow-window behavior](#responsive-and-narrow-window-behavior)
8. [Typography and target floors](#typography-and-target-floors)
9. [Text integrity](#text-integrity)
10. [Dense workbenches](#dense-workbenches)
11. [Overlays and dialogs](#overlays-and-dialogs)
12. [Required UI intent brief](#required-ui-intent-brief)
13. [Authoritative guidance](#authoritative-guidance)

## Purpose

Use these rules to make the product easier to operate, not merely more organized. The Stack Junkie style depends on hierarchy, restraint, and quiet surfaces. Adding containers, labels, metrics, tabs, explanatory prose, or dashboards without a user need violates the system even when the colors are correct.

## Frontload test

Frontload only what the user needs to understand and perform the current task:

1. **Primary object:** the document, recording, plot, file, message, project, or data selection being acted on.
2. **Current state:** ready, recording, processing, unsaved, disconnected, failed, complete, or other state that changes the user's next decision.
3. **Required input:** only information necessary to begin or complete the current action.
4. **Primary action:** the most likely next action. Prefer one visually dominant action per local task region.
5. **Blocking information:** errors, safety constraints, missing requirements, or consequences that must be understood now.

Everything else must earn its position. Frequency, urgency, reversibility, and task dependency determine prominence. Do not frontload something merely because implementation data exists for it.

Use three disclosure tiers:

- **Primary:** visible without searching because most users need it now.
- **Secondary:** one clearly labeled step away because it is occasional, advanced, or contextual.
- **Tertiary:** settings, diagnostics, documentation, or a dedicated detail surface for rare expert needs.

Avoid more than two nested disclosure steps. If a feature needs deeper nesting, simplify or regroup the information architecture.

## Anti-slop element test

Every visible element must satisfy at least one purpose:

- advances the active user task;
- communicates current or changed system state;
- prevents an error or communicates a required consequence;
- provides navigation to a real, requested capability;
- displays user data or a domain artifact needed for the task;
- provides supplemental help that users can request contextually.

Remove or do not create the element when its only purpose is to:

- fill empty space;
- make a screen look more complete;
- imitate a dashboard or component gallery;
- restate nearby text;
- expose implementation details with no user decision attached;
- advertise a feature that is not implemented;
- manufacture metrics, sample activity, categories, cards, or controls;
- organize information that does not need to exist.

Prefer an honest quiet area or focused empty state over filler. A useful empty state explains what is absent, why it matters, and the real next action. It does not contain decorative statistics, fake recent activity, or speculative shortcuts.

## Progressive disclosure

Initially show the small set of options that most users need. Reveal specialized or rare features only when requested. The disclosure control must clearly predict what it reveals.

Good candidates for secondary disclosure:

- advanced settings;
- infrequent formatting or export controls;
- diagnostics and raw technical details;
- long explanations and examples;
- secondary metadata;
- alternate views that are not part of the primary workflow;
- expert controls whose accidental use is costly.

Do not hide:

- the primary action;
- required fields or constraints;
- errors, warnings, or current status;
- information needed to choose safely;
- frequently used controls;
- the only route to completing the task.

Use a labeled disclosure such as `Advanced settings`, `Show diagnostics`, or `More export options`. Avoid vague labels such as `More`, unexplained chevrons, and mystery icon buttons.

## Help cues

Use a small circled question-mark control next to the term or setting only when inline wording cannot remain both concise and clear.

Requirements:

- Give the control an accessible name such as `About wake phrase detection`.
- Support pointer hover, keyboard focus, and click or tap.
- Keep the target at least 24 by 24 CSS pixels, even if the visible circle is smaller.
- Use a concise tooltip for supplemental, nonessential text that needs no interaction.
- Use a nonmodal popover for longer explanations, examples, links, or interactive content.
- Make hover or focus content dismissible, hoverable, and persistent long enough to read. Support `Escape` where the platform allows.
- Keep the popup within the window and avoid covering the field or value it explains.

Do not use a modal dialog for ordinary field help. Do not place essential instructions, errors, warnings, or status only inside help content. Do not repeat the visible label in its tooltip.

## Feedback and system status

Every user action needs a perceivable response appropriate to its duration and consequence.

- **Immediate action:** change the control state, selection, value, or nearby object immediately.
- **Short asynchronous action:** show a local busy state and disable duplicate submission.
- **Known-duration work:** use determinate progress and describe the operation.
- **Unknown-duration nonblocking work:** use an indeterminate progress bar or compact local activity cue.
- **Unknown-duration blocking work:** use a blocking progress treatment only when the user truly cannot continue.
- **Success:** confirm close to the affected object. Use a transient toast only when local confirmation would be unclear.
- **Failure:** preserve the user's input, explain what failed, and provide a recovery action.
- **Destructive action:** distinguish it visually and confirm only when the consequence is difficult to reverse or easy to trigger accidentally.

Avoid permanent success banners for routine actions, a spinner on every list row, duplicate toast plus inline confirmation, and progress indicators for imperceptibly short work.

## Responsive and narrow-window behavior

Reflow is the default. Content and functionality must remain available without overlap or two-direction page scrolling, except for domain surfaces that require two dimensions for meaning.

Use this order:

1. Reduce nonessential outer whitespace within the 4 px rhythm.
2. Allow flexible regions to grow or shrink.
3. Wrap prose and control labels at word boundaries.
4. Move secondary actions into a clearly labeled overflow menu.
5. Collapse secondary panes into tabs, drawers, or on-demand inspectors.
6. Stack independent regions into one column.
7. Scroll the appropriate content region in one direction.
8. Apply modest type compaction without crossing readability floors.

Never shrink the entire interface with a transform. Never compress interactive targets below their floor. Never keep every dock visible merely because the wide layout has room for them.

Define three evidence sizes when the platform permits:

- **Normal:** the intended everyday working size.
- **Narrow:** the first major collapse breakpoint.
- **Minimum supported:** the smallest size at which the primary workflow remains usable.

At minimum supported size, the primary object, current state, and primary action remain directly available. Secondary panes may move behind one explicit disclosure. If a platform imposes a fixed minimum window size, verify that size and explain why.

Two-dimensional exceptions include plots, maps, diagrams, media timelines, games, and data grids whose relationships require both axes. Keep surrounding labels and controls reflowable, and confine scrolling to the domain surface.

## Typography and target floors

Use the canonical sizes in `design-system.md` first. These floors prevent the narrow layout from becoming technically complete but practically unreadable:

- Reading body: prefer 16 px or platform equivalent.
- Standard controls and labels: prefer 13 to 14 px in dense desktop tools, 14 to 16 px elsewhere.
- Compact metadata: 12 px minimum, used sparingly and never for required instructions.
- Long-form line height: 1.6 to 1.8.
- Dense control line height: at least 1.35.
- Pointer target: at least 24 by 24 CSS px under WCAG 2.2, with larger platform-native targets for touch.
- Touch target: follow the platform recommendation, commonly 44 to 48 logical px.

Do not reduce required text below these floors to avoid layout work. Reflow or disclose instead. Preserve user text scaling and verify 200 percent text resize for web surfaces when applicable.

## Text integrity

- Allow prose, descriptions, and labels to wrap at normal word boundaries.
- Use `overflow-wrap: break-word` only for unavoidable long tokens such as URLs or identifiers.
- Do not use `word-break: break-all` for natural language.
- Do not clip required text.
- Do not use ellipsis for primary labels, errors, required values, or instructions.
- Ellipsis is acceptable for constrained identifiers, file paths, table cells, and replaceable metadata only when the full value is available on focus, hover, selection, or an adjacent detail view.
- Let buttons grow, wrap a short label, or switch to an icon-plus-accessible-label treatment before truncating the action name.
- Test realistic long content, localized expansion, and user-provided names.

## Dense workbenches

Dense tools need hierarchy more than they need extra containers.

- Keep one primary work canvas or data object visually dominant.
- Group controls by the object they affect, not by implementation module.
- Show only the active inspector when multiple inspectors compete for the same space.
- Let users recover hidden panes through a stable menu, tab, or layout command.
- Keep transport controls together and place local status near them.
- Avoid explanatory paragraphs inside permanent sidebars. Use concise labels and contextual help.
- Use modal dialogs only for blocking decisions or focused short workflows. Do not place a long report, tutorial, and action console in one dialog over an already dense workbench.
- Dim or simplify the inactive background when a true modal is open so the dialog has an unmistakable focus.

The EtherPlot anti-example in the specimen page fails because the dialog, background workbench, side panels, status messages, controls, and diagram all remain visually competitive. The repair is prioritization and disclosure, not smaller text alone.

## Overlays and dialogs

Choose the least disruptive container that fits the task:

- **Tooltip:** one short supplemental explanation, no interaction.
- **Popover or flyout:** contextual help, a short choice set, or interactive supplemental content.
- **Drawer or inspector:** persistent secondary controls tied to the current object.
- **Modal dialog:** a blocking decision, destructive confirmation, authentication, or short focused workflow that must finish before returning.
- **Dedicated page or workspace:** long reports, tutorials, multi-section configuration, complex analysis, or content users need to compare with the main work.

A dialog needs a clear title, concise purpose, obvious primary action, secondary dismissal, focus containment, keyboard dismissal where appropriate, and a size that leaves context without exposing a visually active competing interface.

## Required UI intent brief

Use this compact structure before implementation:

```markdown
Primary task: ...
Front and center: ... because ...
Contextual or tucked away: ... through ... because ...
Removed or intentionally omitted: ... because ...
Narrow behavior: ...
Evidence plan: surfaces ..., themes ..., states ..., viewports ...
```

Do not turn this into project documentation unless requested. It is a decision checkpoint and user-facing progress update.

## Authoritative guidance

- [W3C WCAG 2.2, Reflow](https://www.w3.org/WAI/WCAG22/Understanding/reflow.html): preserve information and functionality at narrow effective widths, with limited exceptions for intrinsically two-dimensional content.
- [W3C WCAG 2.2, Resize Text](https://www.w3.org/WAI/WCAG22/Understanding/resize-text.html): support 200 percent text enlargement without losing content or functionality.
- [W3C WCAG 2.2, Content on Hover or Focus](https://www.w3.org/WAI/WCAG22/Understanding/content-on-hover-or-focus.html): make supplemental hover and focus content dismissible, hoverable, and persistent.
- [W3C WCAG 2.2, Target Size Minimum](https://www.w3.org/WAI/WCAG22/Understanding/target-size-minimum.html): provide targets of at least 24 by 24 CSS pixels or sufficient spacing under the listed exceptions.
- [Microsoft Windows app tooltip guidance](https://learn.microsoft.com/en-us/windows/apps/develop/ui/controls/tooltips): use tooltips sparingly, keep them concise and supplemental, and use another control for essential or interactive content.
- [Microsoft Windows app progress guidance](https://learn.microsoft.com/en-us/windows/apps/develop/ui/controls/progress-controls): choose determinate, indeterminate, blocking, and nonblocking feedback based on the operation.
- [GOV.UK Details component](https://design-system.service.gov.uk/components/details/): use disclosure for secondary help, not information most users need.
- [Nielsen Norman Group, Progressive Disclosure](https://www.nngroup.com/articles/progressive-disclosure/): keep common features visible and defer specialized features through an obvious, well-labeled path.
