# Stack Junkie visual QA protocol

## Contents

1. [What this protocol can and cannot prove](#what-this-protocol-can-and-cannot-prove)
2. [Evidence package](#evidence-package)
3. [Screenshot matrix](#screenshot-matrix)
4. [How to inspect](#how-to-inspect)
5. [Independent verifier](#independent-verifier)
6. [Fail-closed rules](#fail-closed-rules)
7. [Completion language](#completion-language)

## What this protocol can and cannot prove

Screenshots can prove rendered hierarchy, palette, typography, spacing, clipping, wrapping, state presentation, density, and narrow-layout behavior at captured moments. They cannot alone prove keyboard behavior, screen-reader semantics, persistence, animation timing, or every dynamic state. Use runtime inspection and normal tests for those properties, then cite the relevant result in the checklist notes.

The deterministic validator proves report completeness and evidence linkage. It does not judge visual quality. The independent visual verifier judges the images and may reject an implementer's claims.

## Evidence package

Create one temporary evidence folder or use the project's existing artifact directory. Include:

- the completed compliance report copied from `assets/templates/compliance-report.md`;
- before screenshots for each changed surface when modifying an existing interface;
- after screenshots following the matrix below;
- a short runtime or build note when behavior cannot be shown in a still image;
- no unrelated screenshots, generated filler, or duplicate captures.

Use stable evidence IDs such as `E01`, `E02`, and `E03`. Record the exact file, before or after phase, theme, surface, state, size class, viewport or window size, and purpose in the report manifest.

Screenshots must show the real running product or its normal preview. Do not submit a design mockup as implementation evidence. Do not crop away the window edges when the claim concerns responsive fit, clipping, dialogs, or overall hierarchy.

## Screenshot matrix

Capture the smallest set that proves all changed behavior:

| Dimension | Required coverage |
|---|---|
| Surface | Every changed surface and every shared primitive family affected |
| Theme | Dark and light when both are implemented |
| Size | Normal, narrow collapse breakpoint, and minimum supported size when resizable |
| Workflow | Idle or ready, one active or loading state, one completed or success state, and one empty or error state when those states exist |
| Interaction | Visible focus, selected, disabled, and help disclosure examples when changed |
| Overlay | Every changed dialog, drawer, popover, tooltip, toast, or menu in an open state |
| Content stress | Representative long label, long user value, localization expansion, or narrow table content when the surface accepts variable text |

Use `N/A` only when the product or scoped surface genuinely lacks the dimension. Example: a fixed-size kiosk can mark narrow resize evidence `N/A` with the fixed-size platform constraint. “Did not have time,” “not requested,” and “looks fine in the main screenshot” are not legitimate reasons when the skill requires the state.

## How to inspect

Examine each after screenshot at full resolution. Do not rely only on file dimensions or automated pixel checks.

Check in this order:

1. **Task hierarchy:** identify the primary object, current state, and primary action within five seconds. If several regions demand equal attention, fail hierarchy.
2. **Information necessity:** identify every visible panel, label, card, metric, and explanation. Fail anything that has no real user purpose or duplicates nearby information.
3. **Responsive fit:** inspect all edges and panes for clipping, overlap, accidental horizontal scrolling, split words, microscopic text, and unreachable controls.
4. **Typography:** confirm deliberate roles for Manrope, Lora, and Fira Code or an approved substitution. Check readable size, line length, weight, and contrast.
5. **Foundations:** confirm three quiet neutral layers, controlled blue, thin borders, modest radii, and restrained elevation.
6. **Components and states:** compare relevant primitives to the specimen page. Check hover or focus, selected, disabled, loading, success, empty, and error treatments.
7. **Feedback:** confirm that user actions visibly change state and that pending, success, and failure treatments appear near the affected object.
8. **Disclosure:** confirm that secondary content is reachable and clearly labeled, and that essential content is not hidden in help.
9. **Legacy conflicts:** look for default framework colors, gradients, excessive pills, heavy shadows, mismatched radii, and isolated old components.
10. **Screenshot sufficiency:** reject a pass when the image does not actually show the claimed element or state.

## Independent verifier

When subagents are available, use a fresh read-only verifier after implementation.

The verifier receives only:

- the user's styling request and named scope;
- this skill folder, including the specimen page and references;
- the before and after screenshots;
- a blank compliance checklist;
- necessary runtime notes that describe facts, not conclusions.

Do not show the verifier the implementer's filled checklist, final summary, intended verdict, or suspected failures before its first pass. This prevents conclusion leakage.

Suggested verifier task:

```text
Act as the read-only visual verifier for a Stack Junkie style implementation.
Inspect the skill references and visual specimen, then independently evaluate the
provided real-product screenshots against the blank compliance checklist. Do not
edit files. Return PASS or FAIL for every applicable checklist ID, cite the evidence
image for each decision, reject unsupported N/A claims, and list the smallest set of
blocking corrections. Do not infer that an element exists when it is not visible.
```

After the first pass, compare the verifier's decisions with the implementer's report. Resolve every conflict through code changes, new evidence, or a specific legitimate `N/A`. Re-run the verifier on affected items.

The implementer must not sign the verifier field. A verifier that contributed implementation edits is no longer independent and must be replaced for the final pass.

## Fail-closed rules

The result is not visually verified when any of these conditions is true:

- a checklist item remains `TODO` or `FAIL`;
- a required checklist ID is missing or duplicated;
- a `PASS` lacks screenshot evidence that visibly supports it;
- an `N/A` lacks a narrow and credible reason;
- a changed surface, implemented theme, required state, or required viewport is absent;
- the deterministic validator fails;
- the independent verifier reports an unresolved failure;
- the implementer and verifier are the same agent;
- screenshots are mockups rather than the running product;
- visual inspection was skipped because the build or preview succeeded.

If the app cannot be rendered, report the blocker. Do not substitute source inspection and do not claim visual completion.

## Completion language

Use one of these honest labels:

- **Implemented, not visually checked:** source changed, but rendered inspection did not occur.
- **Implemented and self-audited:** the implementer examined evidence and `validate_compliance.py <report> --audit-level self` passed, but no independent verifier was available. The report must say `Audit level: self`, `Independent verifier: Unavailable`, and `Independent verification: NOT RUN`.
- **Visually verified:** the deterministic report passed and an independent visual verifier approved every applicable item.
- **Verification failed:** one or more required items remain unresolved.

Include the evidence folder or report path in the handoff when it is appropriate to retain it. If evidence is intentionally temporary, show the relevant screenshots to the user in the final response and state that no permanent audit artifacts were added.
