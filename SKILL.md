---
name: stack-junkie-style
description: Apply and visually verify Christopher Kvamme's Stack Junkie design system across web, desktop, mobile, and application interfaces. Use when the user asks for Stack Junkie styling, stack-junkie.com styling, the Stack Junkie light and dark themes, a restrained editorial-technical UI, a decluttered responsive redesign, or evidence-backed visual QA against the Stack Junkie style guide.
---

# Stack Junkie Style

Implement the Stack Junkie identity in the real product, preserve the product's native interaction model, and prove the result with rendered evidence. Do not stop at tokens, mockups, recommendations, or unchecked self-reporting.

## Required resources

Read these completely before editing an interface:

1. [references/design-system.md](references/design-system.md) for canonical visual tokens and component rules.
2. [references/product-ux-rules.md](references/product-ux-rules.md) for hierarchy, progressive disclosure, responsive behavior, feedback, accessibility, and anti-slop rules.
3. [references/visual-qa-protocol.md](references/visual-qa-protocol.md) for screenshot evidence and independent verification.

Open [assets/style-guide/index.html](assets/style-guide/index.html) in a real browser before implementation. Treat it as the visual specimen, not as a component library to copy blindly. Copy [assets/templates/compliance-report.md](assets/templates/compliance-report.md) into the target project's existing artifact or temporary-output location and complete it during the run. Do not add the report to the project permanently unless the user requests durable documentation.

## Workflow

### 1. Inspect the product

- Identify the framework, theme source, reusable primitives, existing themes, screens in scope, real user tasks, native platform conventions, and relevant instruction files.
- Render or launch the current product before editing when practical. Preserve before screenshots of the affected surfaces.
- Preserve functionality, user-authored assets, accessibility semantics, semantic status colors, and unrelated dirty work.
- Locate conflicting legacy styles and unused visual rules. Do not invent replacement screens, metrics, cards, copy, settings, examples, or controls merely to make the interface look complete.

### 2. Produce a UI intent brief

Before substantial edits, tell the user concisely:

- the primary task and object that should dominate the surface;
- what will be frontloaded and why;
- what will move behind contextual disclosure, a secondary pane, a tab, or settings, and why;
- what decorative, duplicative, speculative, or low-value material will be removed or left out;
- how the layout will reflow at narrow sizes;
- which rendered states and viewports will be captured for proof.

Use current product evidence and established guidance from the UX reference. Ask only when a genuine product decision cannot be inferred safely.

### 3. Choose the translation

- Editorial sites: reproduce the reference closely, including Lora headings and its reading rhythm.
- Product websites and general applications: preserve the palette, typography roles, flat surfaces, restrained borders, modest radii, focus treatment, and short functional motion while keeping workflows compact.
- Dense professional tools: use Manrope for controls and data surfaces; reserve Lora for the strongest identity moments. Keep plots, tables, inspectors, trees, and workbenches native to their category.
- Mobile and touch: retain the identity while meeting platform target sizes, safe areas, input behavior, and touch-first disclosure.
- Native apps: map semantic tokens into the existing theme or palette system. Preserve native behavior where custom styling would reduce usability.

When the user names one surface, change that surface and shared primitives it legitimately depends on. Do not silently redesign unrelated areas.

### 4. Establish shared foundations

- Centralize semantic tokens. Do not scatter literal colors through components.
- Implement canonical light and dark foundations unless the user explicitly limits theme scope.
- Preserve a usable theme switcher. When theme selection is in scope and none exists, support light, dark, and system preference with persisted explicit choice.
- Deliver Lora, Manrope, and Fira Code through licensed project dependencies or standard font delivery. For offline apps, bundle or package fonts lawfully, or record the approved system-font substitution. Never copy font binaries from stack-junkie.com.
- Keep success, warning, danger, and data-series colors separate from the blue accent.

### 5. Organize before decorating

- Apply the frontload test from the UX reference before adding visual polish.
- Keep the primary task, current system state, required inputs, and next primary action immediately available.
- Move advanced, rare, explanatory, or diagnostic content behind a clearly labeled secondary disclosure.
- Remove any element that lacks a user goal, required status, safety purpose, or requested product function.
- Use a small circled help control only for genuinely supplemental information. Support hover, keyboard focus, and click or tap. Use a nonmodal popover for longer or interactive help.
- Never hide essential instructions, errors, warnings, current status, or required actions in a tooltip.

### 6. Apply reusable primitives and real states

- Update the shell, typography, surfaces, navigation, buttons, icon buttons, fields, choices, tabs, tags, tables, trees, lists, overlays, help cues, focus, selection, disabled treatment, and relevant data surfaces.
- Implement the states that exist in the workflow: loading, determinate progress, indeterminate progress, success, empty, error, selected, hover, focus, disabled, and narrow-window behavior.
- Provide immediate visual feedback for user actions. Prevent duplicate submissions while an action is pending. Prefer local feedback near the affected object.
- Keep motion functional, 150 to 200 ms where appropriate, and respect reduced-motion preferences.

### 7. Reflow without microscopic UI

- Preserve information and functionality as the window narrows. Reflow first, collapse secondary regions second, then allow one-direction scrolling in the appropriate content region.
- Wrap prose and labels at word boundaries. Never clip, overlap, or split words to make a layout fit.
- Do not solve narrow layouts by shrinking everything indefinitely. Maintain readable text and usable targets. Compact spacing and type only within the floors in the UX reference.
- Allow two-dimensional scrolling only for content whose meaning requires it, such as plots, maps, diagrams, and data grids. Keep their surrounding controls independently usable.
- Define and test a minimum supported window size. Below it, preserve the primary workflow and expose secondary regions through an explicit drawer, tab, menu, or disclosure.

### 8. Capture evidence and verify independently

- Select and justify the highest applicable coverage tier from the visual QA protocol: `full`, `focused`, or `token`. Never use a lower tier merely to reduce evidence work.
- Run the real build or preview and capture the smallest screenshot matrix that completely proves the selected tier.
- Complete every checklist row as `PASS`, `FAIL`, or `N/A`. Every visual `PASS` must cite screenshot evidence. Behavioral rows identified by the validator must also cite runtime evidence. Every `N/A` must contain a specific reason.
- Record runtime evidence for keyboard behavior, help dismissal and persistence, feedback behavior, input recovery, reduced motion, accessibility semantics, text scaling, and theme persistence. Runtime evidence supplements rather than replaces screenshots.
- Run `python scripts/validate_compliance.py <completed-report.md> --coverage-tier <full|focused|token>` from this skill folder when an independent verifier is available. Fix every deterministic failure.
- When subagents are available, require a fresh read-only visual verifier that has not seen the implementer's conclusions. Give it the user request, style guide, completed app screenshots, and blank checklist. Do not give it the implementer's filled checklist until its first pass is complete.
- Fix verifier findings, recapture affected screenshots, and repeat verification. The implementer and verifier must not be the same agent.
- If independent verification is unavailable, set `Audit level: self`, `Independent verifier: Unavailable`, and `Independent verification: NOT RUN`, then run `python scripts/validate_compliance.py <completed-report.md> --audit-level self --coverage-tier <full|focused|token>`. Report the interface as implemented and self-audited, never as independently verified.

## Completion contract

Call the requested surface **Stack Junkie styled** only when it uses shared semantic tokens, behaves coherently in every implemented theme, preserves the product's task hierarchy, and has no known clipping, overlap, broken-word wrapping, unreadable shrinkage, contradictory legacy styling, or decorative filler.

Call it **visually verified** only when:

1. the compliance report passes the deterministic validator;
2. all required screenshots exist and were visually examined;
3. an independent verifier returns no unresolved failure;
4. any `N/A` entry is narrow, specific, and legitimate.

Report the theme source, changed surfaces, frontloaded and disclosed content decisions, rendered states and viewports checked, verifier result, and anything intentionally left unchanged.
