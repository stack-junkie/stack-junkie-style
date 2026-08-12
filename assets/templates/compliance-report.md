# Stack Junkie visual compliance report

Project: TODO
Scope: TODO
Date: TODO
Implementer: TODO
Audit level: TODO
Coverage tier: TODO
Coverage tier reason: TODO
Independent verifier: TODO
Independent verification: TODO
Theme source: TODO
Evidence directory: TODO

## UI intent brief

Primary task: TODO

Front and center: TODO

Contextual or tucked away: TODO

Removed or intentionally omitted: TODO

Narrow behavior: TODO

## Screenshot manifest

Add one row per evidence image. Paths may be absolute or relative to this report.

| ID | File | Phase | Theme | Surface | State | Size | Viewport or window | Purpose |
|---|---|---|---|---|---|---|---|---|
| E01 | TODO | TODO | TODO | TODO | TODO | TODO | TODO | TODO |

## Runtime evidence manifest

Add one row for each behavioral claim that screenshots cannot prove. Use an inspectable test, browser trace, accessibility-tree inspection, or command. Remove the example row when runtime evidence is legitimately unnecessary.

| ID | Method | Target behavior | Result | Artifact or command |
|---|---|---|---|---|
| R01 | TODO | TODO | TODO | TODO |

## Checklist

Set every result to `PASS`, `FAIL`, or `N/A`. Mark the checkbox when reviewed. A visual `PASS` cites one or more `E` IDs. Behavioral rows cite both screenshot and `R` IDs when the validator requires runtime proof. An `N/A` needs a specific reason in Notes and uses `Evidence: N/A`.

### Scope and truthfulness

- [ ] SJ-SCOPE-01 The report names the exact changed surfaces and leaves unrelated surfaces out of scope. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-SCOPE-02 Before evidence exists for each redesigned existing surface. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-SCOPE-03 After evidence comes from the real running product or its normal preview, not a mockup. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-SCOPE-04 The theme source and reusable primitives are identified. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-SCOPE-05 Any intentionally unchanged or unverified surface is disclosed. | Result: TODO | Evidence: N/A | Notes: N/A

### Foundations

- [ ] SJ-FOUND-01 Dark mode uses the canonical near-black neutral layers or a documented platform-equivalent mapping. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-FOUND-02 Light mode uses the canonical warm off-white and white layers or a documented platform-equivalent mapping. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-FOUND-03 Primary and secondary text remain clearly distinguishable and readable in every implemented theme. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-FOUND-04 Blue is controlled and consistent for primary action, focus, links, and active navigation. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-FOUND-05 Success, warning, danger, and data-series colors retain their own semantic meaning. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-FOUND-06 Surfaces use flat one-pixel borders, modest radii, and rare restrained elevation. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-FOUND-07 Gradients, glass effects, neon glow, heavy shadows, excessive pills, and random accents are absent. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-FOUND-08 Legacy framework defaults and conflicting visual rules are absent from the changed surfaces. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-FOUND-09 An explicit theme choice persists and system preference is the fallback when no explicit choice exists. | Result: TODO | Evidence: N/A | Notes: N/A

### Typography and text integrity

- [ ] SJ-TYPE-01 Manrope, Lora, and Fira Code have deliberate roles, or any platform substitution is documented and visually coherent. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-TYPE-02 Dense controls and technical data do not use decorative serif typography. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-TYPE-03 Reading text, labels, controls, and metadata remain above the defined readability floors. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-TYPE-04 Natural-language text wraps at word boundaries without broken words. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-TYPE-05 Required labels, errors, values, instructions, and actions are not clipped or ellipsized. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-TYPE-06 Any permitted identifier or table-cell truncation exposes the full value on focus, hover, selection, or detail view. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-TYPE-07 Long realistic content and narrow-width text have been visually stress-tested. | Result: TODO | Evidence: N/A | Notes: N/A

### Hierarchy and anti-slop

- [ ] SJ-HIER-01 The primary object, current state, and primary action are identifiable within five seconds. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-HIER-02 One action is visually primary within each local task region. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-HIER-03 Frequently needed controls are frontloaded and rare or advanced controls use clear progressive disclosure. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-HIER-04 Essential instructions, errors, warnings, and current status are not hidden in tooltips or secondary disclosure. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-HIER-05 Every visible panel, card, label, metric, and control maps to a real user goal or required system state. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-HIER-06 No speculative features, fake metrics, sample activity, decorative dashboards, or nonfunctional controls were invented. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-HIER-07 Explanatory prose is concise and contextual rather than permanently frontloaded across the surface. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-HIER-08 Empty space is allowed when it improves focus; filler content was not added merely to occupy it. | Result: TODO | Evidence: N/A | Notes: N/A

### Components and interaction

- [ ] SJ-COMP-01 Navigation and tabs show clear default, hover, focus, active, and disabled treatment where applicable. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-COMP-02 Primary, secondary, icon, and destructive actions use the correct hierarchy and semantic treatment. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-COMP-03 Fields, selects, text areas, choices, switches, and sliders use coherent surfaces, borders, labels, and focus states. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-COMP-04 Cards, panels, lists, trees, tables, and data grids use restrained separation and appropriate density. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-COMP-05 Tags and compact statuses are used sparingly and ordinary controls have not been turned into pills. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-COMP-06 Help cues are small, accessible, contextual, keyboard-reachable, and use tooltip or popover behavior appropriate to their content. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-COMP-07 Tooltips are concise and supplemental; longer or interactive help uses a popover, flyout, or details surface. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-COMP-08 Dialogs are reserved for blocking or focused workflows and do not contain sprawling reports or tutorials over a competing workbench. | Result: TODO | Evidence: N/A | Notes: N/A

### Feedback and workflow states

- [ ] SJ-STATE-01 User actions produce immediate visible feedback near the affected control or object. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-STATE-02 Pending actions prevent accidental duplicate submission without unnecessarily blocking unrelated work. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-STATE-03 Known-duration work uses determinate progress and unknown-duration work uses an appropriate indeterminate treatment. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-STATE-04 Loading, success, empty, and error states that exist in scope are designed and evidenced. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-STATE-05 Errors preserve recoverable user input, explain the failure, and offer a next step. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-STATE-06 Selected, focused, disabled, and destructive states remain distinguishable without color alone. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-STATE-07 Motion is brief and functional, and reduced-motion behavior exists when the platform supports it. | Result: TODO | Evidence: N/A | Notes: N/A

### Responsive behavior

- [ ] SJ-RESP-01 Normal, narrow, and minimum-supported sizes are defined and evidenced for resizable surfaces. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-RESP-02 The layout reflows before secondary regions collapse and before any type compaction reaches its floor. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-RESP-03 The primary workflow remains directly usable at the minimum supported size. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-RESP-04 Secondary panes move behind clear tabs, drawers, menus, or disclosures instead of being squeezed into illegibility. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-RESP-05 No required content or control overlaps, clips, disappears, or becomes unreachable at tested sizes. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-RESP-06 The interface does not use whole-surface transform scaling or microscopic text as a substitute for responsive layout. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-RESP-07 Two-dimensional scrolling is confined to intrinsically two-dimensional domain surfaces. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-RESP-08 Scrollable regions are intentional, visually understandable, and do not create nested-scroll traps. | Result: TODO | Evidence: N/A | Notes: N/A

### Accessibility and evidence integrity

- [ ] SJ-A11Y-01 Keyboard focus is visible and logical across the changed surface. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-A11Y-02 Pointer targets meet the 24 by 24 CSS pixel minimum or a documented platform exception; touch targets follow the platform target size. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-A11Y-03 Hover or focus content is dismissible, hoverable, and persistent when custom behavior is used. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-A11Y-04 Text enlargement or platform text scaling does not remove content or functionality. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-A11Y-05 Contrast, labels, roles, names, and state semantics remain valid after styling. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-EVID-01 Every changed surface appears in the screenshot manifest. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-EVID-02 Every implemented theme appears in the screenshot manifest. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-EVID-03 Every required state and overlay appears in evidence or has a legitimate N/A reason. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-EVID-04 Every screenshot was examined at full resolution rather than inferred from filenames or build success. | Result: TODO | Evidence: N/A | Notes: N/A
- [ ] SJ-EVID-05 The independent verifier is different from the implementer and has no unresolved failures. | Result: TODO | Evidence: N/A | Notes: N/A

## Verifier findings

Verifier verdict: TODO

Blocking findings: TODO

Corrections and recaptured evidence: TODO

## Final summary

Changed surfaces: TODO

Frontloaded content: TODO

Disclosed or removed content: TODO

Themes, states, and viewports checked: TODO

Intentionally unchanged: TODO
