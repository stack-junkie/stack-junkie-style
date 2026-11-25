# Typography and workbench density

Use a compact application shell around comfortable working content. This reference records the 2026-11-20 research and resulting preview choices. The choices are starting values for owner review, not a claim that one font or layout works for every product.

## Font roles

Use one proportional UI family for navigation, tabs, forms and toolbars. Clean utility now starts with the platform UI font: Segoe UI Variable Text or Segoe UI on Windows, then the browser or native platform's system family. This follows Fluent's preference for native platform fonts and makes a useful comparison against the owner's VS Code screenshot. Manrope remains bundled for the other visual candidates and existing branding. It was not rejected as universally unsuitable. Keep Fira Code for code, logs and aligned technical values; use Lora only when an editorial surface actually calls for it.

The Qt adapter verifies that the requested system family exists and is proportional. Environments without usable system fonts, including this offscreen renderer, fall back to the registered bundled Manrope family instead of an accidental monospace substitution. The exported image therefore does not establish the exact Segoe rendering on a normal desktop.

Microsoft explains why small interface text and display text have different optical needs. Fluent recommends regular weights for ordinary content and selective stronger emphasis. These support changing roles and density before shopping for another display font. Neither source proves a universally best family. Check ambiguous characters, punctuation, numbers, long labels, supported languages and real text in the target platform. Use available system fonts without copying proprietary font files into the package. [Windows typography](https://learn.microsoft.com/windows/apps/design/signature-experiences/typography), [Fluent typography](https://fluent2.microsoft.design/typography).

## Shared scale

Values come from `ui/shared/tokens.json`. Web and Qt adapt these roles rather than copying independent font tables. CSS sizes below are at normal browser zoom; native values use Qt logical pixels. Device pixels in screenshots are not a reliable measurement of either.

| Role | Size | Weight | Purpose |
| --- | --- | --- | --- |
| Navigation, tabs, toolbar controls | 13 | 400 | Quiet, consistent application controls |
| Ordinary interface and form text | 14 | 400 | Inputs, instructions and field values |
| Context or panel title | 14 | 600 | Compact identity within a pane |
| Metadata | 12 | 400 | Secondary labels, version and annotations |
| Section heading | 16 | 600 | Short groups within a tool |
| Reading body | 16 | 400 | Prose with 1.6 line height |
| Reading subheading | 18 | 600 | Clear grouping above body text |
| Reading heading | 22 | 600 | Document section title |
| Reading title | 28 | 600 | Document identity where needed |

These are our proposed values. Microsoft's Windows Body and Body Strong use the same size, with weight distinguishing them; its layout guidance recommends Body Strong for titles in limited spaces. Do not turn every pane name into a large page heading. Do not make all selected labels bold merely because they are selected. Background, edge treatment and position already carry state. Keep text semantic: real heading elements establish the document outline, not just larger styling. [Content layout](https://learn.microsoft.com/en-us/windows/apps/design/basics/content-basics).

## Density and page structure

Reduce padding, gaps and redundant header layers before reducing reading text. Microsoft specifically supports compact sizing for productivity interfaces with mouse and keyboard input; density can vary by region. In this preview, the top bar starts at 40 logical pixels, context rows at 36, tabs at 32, navigation rows at 28 and icon targets at 28. These are minimums, not rigid text-clipping boxes. Form controls start at 32. Touch-oriented controls remain larger. [Compact sizing](https://learn.microsoft.com/en-us/windows/apps/develop/ui/controls/compact-sizing).

Use the rail for sections, contextual sidebar for the selected section's elements, and tabs for peer views. Keep view names short. Scope controls to the view they affect. The showcase's family label is now Content instead of Content and working surfaces. Web context rows share a grid row so a wrapped title does not break alignment with the main pane. Sidebar resizing, collapse, saved width and selections remain part of the required behavior. These principles are informed by VS Code's scoped navigation and view guidance; its rules for extensions are not blanket restrictions on unrelated apps. [VS Code architecture](https://code.visualstudio.com/api/ux-guidelines/overview), [Views](https://code.visualstudio.com/api/ux-guidelines/views).

Keep workbench chrome stationary while the relevant content pane scrolls. Reading sites and long documents can use ordinary page flow. Use a reading measure around 65 characters as a starting point, not a full-window line of text. Do not apply that width limit to tables, canvases or every kind of work area. WCAG's related 80-character provision is AAA, not an AA rule requiring all pages to use a particular line length. [Visual presentation](https://www.w3.org/WAI/WCAG22/Understanding/visual-presentation.html).

## Dark text and emphasis

The owner chose neutral black backgrounds. Keep those values. Clean dark ordinary text is now a softer neutral gray, with a separate readable muted role. Reserve stronger weight and accent for meaningful distinctions. Secondary text still needs contrast; disabling a control is not a reason to make ordinary instructions faint. Check the actual text and background pair. WCAG AA calls for 4.5:1 for normal text and 3:1 for qualifying large text. Most interface text in this scale is normal text. [Contrast](https://www.w3.org/WAI/WCAG22/Understanding/contrast-minimum.html).

## Enlargement, spacing and input access

- Text must be able to enlarge to 200% without loss of content or operation. Use natural text flow and minimum dimensions. Do not shrink labels automatically to fit. CSS pixel values do not prohibit browser zoom, but using them is not proof that all content survives enlargement. [Resize text](https://www.w3.org/WAI/WCAG22/Understanding/resize-text.html).
- At 320 CSS pixels wide, content should reflow instead of demanding whole-page horizontal scrolling, with exceptions for genuinely two-dimensional content. Collapse the contextual sidebar into an accessible overlay and let toolbars wrap. Keep tables and canvases in their own scroll regions. [Reflow](https://www.w3.org/WAI/WCAG22/Understanding/reflow.html).
- Support user overrides of 1.5 line height, 2em paragraph spacing, .12em letter spacing and .16em word spacing without loss. These are override tolerances, not required default styling. Avoid fixed-height text boxes and clipped labels. [Text spacing](https://www.w3.org/WAI/WCAG22/Understanding/text-spacing.html).
- A compact icon can sit inside a larger click target. WCAG 2.2 AA's minimum target criterion is generally 24 by 24 CSS pixels, with specified exceptions; 44 by 44 belongs to the enhanced AAA criterion. This preview uses 28-pixel compact icon targets and 44-pixel coarse-pointer targets in the workspace. Keyboard focus, accessible names and keyboard resizing remain necessary. [Target size](https://www.w3.org/WAI/WCAG22/Understanding/target-size-minimum.html).

## What verification means here

Source checks and builds establish implementation consistency. Native offscreen exports can show rendered type and geometry. They cannot establish owner acceptance or prove every web zoom, touch, language and text-spacing case. The owner operates the browser. Review the actual workbench at their usual zoom, narrow width and larger text, with long labels and meaningful content. A font choice should survive that review before being treated as final.
