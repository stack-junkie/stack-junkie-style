---
name: stack-junkie-style
description: Apply Christopher Kvamme's Stack Junkie design system across web, desktop, mobile, and application interfaces. Use when the user asks for Stack Junkie styling, stack-junkie.com styling, the Stack Junkie light and dark themes, a restrained editorial-technical UI, or a decluttered responsive redesign.
---

# Stack Junkie Style

Apply the Stack Junkie identity to the real product while preserving its native interaction model and primary user task.

## Required resources

Read these before editing an interface:

1. [references/design-system.md](references/design-system.md) for visual tokens and component rules.
2. [references/product-ux-rules.md](references/product-ux-rules.md) for hierarchy, disclosure, responsive behavior, feedback, accessibility, and anti-slop rules.

Open [assets/style-guide/index.html](assets/style-guide/index.html) in a real browser before implementation. Treat it as the visual specimen, not as a component library to copy blindly.

## Workflow

### Inspect the product

- Identify the framework, theme source, reusable primitives, screens in scope, real user tasks, and native platform conventions.
- Launch or render the current product before editing when practical.
- Preserve functionality, user-authored assets, accessibility semantics, semantic status colors, and unrelated dirty work.
- Do not invent screens, metrics, cards, copy, settings, examples, or controls merely to make the interface look complete.

### State the design intent

Tell the user concisely what task should dominate, what secondary material will move or disappear, how narrow layouts will reflow, and which surfaces are changing. Ask only when a genuine product decision cannot be inferred safely.

### Apply the system

- Centralize semantic colors and typography in the product's existing theme system.
- Use quiet neutral layers, controlled blue accents, thin borders, modest radii, and restrained elevation.
- Preserve semantic success, warning, danger, and data-series colors.
- Use Lora for strong editorial identity moments, Manrope for controls and application surfaces, and Fira Code for code or technical data where appropriate.
- Organize the primary task before decorating. Remove decorative, duplicative, speculative, or low-value material.
- Preserve readable text, usable targets, focus treatment, reduced-motion preferences, and native interaction behavior.
- Reflow at narrow sizes instead of shrinking the interface into unreadability.
- Change only the requested surface and shared primitives it legitimately depends on.

### Inspect the real result

Open the changed product and inspect the affected surfaces at the sizes and themes relevant to the change. Fix visible clipping, overlap, broken wrapping, unreadable density, contradictory legacy styling, or broken interaction introduced by the work. Show the user the rendered result when the environment supports it.

Do not create compliance reports, evidence matrices, checklist artifacts, validator scripts, audit levels, or independent-review loops. Direct product inspection and owner feedback are the acceptance boundary.

## Handoff

Report the changed surfaces, the main hierarchy decision, what was removed or disclosed, and any limitation the owner still needs to judge. Keep the handoff proportional to the design change.
