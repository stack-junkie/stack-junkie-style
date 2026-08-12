# Stack Junkie visual system

Canonical implementation snapshot derived from the live `https://www.stack-junkie.com/` page and stylesheet on 2026-08-09. This reference captures the visual system, not the site's prose voice, content, logo, photographs, or other copyrighted assets.

## Design intent

Stack Junkie is restrained editorial technology design: readable, high-contrast, flat, and calm. It combines literary headings with utilitarian interface typography, muted neutral surfaces, thin borders, and a controlled blue accent. Whitespace, type scale, and alignment do most of the visual work.

The system should feel deliberately curated, not like a themed component library. Preserve this identity when adapting it to a different framework or product category.

## Canonical color tokens

Use semantic names in the target project. These values are exact from the source snapshot.

### Dark theme

```css
:root,
[data-theme="dark"] {
  --bg-primary: #0a0a0a;
  --bg-secondary: #111111;
  --bg-card: #161616;
  --text-primary: #e0e0e0;
  --text-secondary: #999999;
  --accent: #3b82f6;
  --accent-hover: #60a5fa;
  --border: #2e2e2e;
  --tag-bg: #1a2540;
  --tag-text: #60a5fa;
  --code-btn-bg: #ffffff14;
  --code-btn-bg-hover: #ffffff1f;
  --code-btn-text: #9ca3af;
  --code-btn-border: #ffffff1f;
  --toc-inactive: #b0b0b0;
}
```

### Light theme

```css
[data-theme="light"] {
  --bg-primary: #f7f7f7;
  --bg-secondary: #ffffff;
  --bg-card: #ffffff;
  --text-primary: #1a1a1a;
  --text-secondary: #666666;
  --accent: #0369a1;
  --accent-hover: #0284c7;
  --border: #e0e0e0;
  --tag-bg: #e0f2fe;
  --tag-text: #0369a1;
  --code-btn-bg: #0000000a;
  --code-btn-bg-hover: #00000014;
  --code-btn-text: #6b7280;
  --code-btn-border: #0000001a;
  --toc-inactive: #777777;
}
```

Use white text on filled accent buttons. Add separate semantic success, warning, danger, and data-series palettes when the product needs them; ensure they work in both themes and do not redefine the core Stack Junkie tokens.

## Typography

```css
--font-serif: "Lora", Georgia, "Times New Roman", serif;
--font-sans: "Manrope", system-ui, -apple-system, sans-serif;
--font-mono: "Fira Code", "Cascadia Code", Consolas, monospace;
```

- Manrope is the base interface, body, label, navigation, metadata, and control face.
- Lora carries editorial titles and major content headings. Use 700 for prominent titles and approximately 1.2-1.3 line height.
- Fira Code is reserved for source code, values that benefit from character alignment, and technical identifiers.
- Body copy is 16 px with 1.8 line height on reading surfaces.
- Standard card titles are 18 px, Lora 700, line-height 1.3.
- Hero/editorial titles use `clamp(18px, 2.5vw, 26px)`, Lora 700, line-height 1.2.
- Metadata is 13 px Manrope in secondary text.
- Compact interface labels and pills use 11-14 px with 500-600 weight.

For dense application UI, do not set every heading in Lora. A panel title, field label, table header, menu, plot label, and toolbar control are interface elements and should remain Manrope. Reserve Lora for the app's strongest identity moments.

## Spatial rhythm and geometry

- Base spacing rhythm: 4 px.
- Common gaps/padding: 4, 6, 8, 12, 16, 24, 32, and 48 px.
- Content container: 1280 px maximum, centered, 24 px horizontal padding; reduce to 16 px on small screens.
- Borders: 1 px using `--border`.
- Radii: 4 px for tags, 6 px for navigation controls, 8 px for buttons/images/blockquote surfaces, and 10 px for cards.
- Avoid inflated whitespace inside dense tools. Preserve the rhythm while respecting information density.
- Avoid heavy elevation. Use a border or a nearby background tone to separate layers. Reserve subtle shadow for temporary overlays that genuinely require elevation.

## Global interaction rules

```css
body {
  min-height: 100dvh;
  margin: 0;
  overflow-x: clip;
  background: var(--bg-primary);
  color: var(--text-primary);
  font-family: var(--font-sans);
  font-size: 16px;
  line-height: 1.8;
  transition: background-color 200ms, color 200ms;
}

::selection {
  background: var(--accent);
  color: #ffffff;
}

:focus-visible {
  outline: 2px solid var(--accent);
  outline-offset: 2px;
  border-radius: 2px;
}

@media (prefers-reduced-motion: reduce) {
  *, *::before, *::after {
    animation-duration: 0.01ms !important;
    animation-iteration-count: 1 !important;
    scroll-behavior: auto !important;
    transition-duration: 0.01ms !important;
  }
}
```

- Standard hover and selection transitions are 150-200 ms.
- Focus is always visible; do not rely on color shift alone.
- Icon buttons have accessible names and a practical target size. The source theme toggle is 36 x 36 px.
- Scrollbars are slim: 6 px, secondary track, bordered thumb, 3 px radius; the thumb moves toward secondary text on hover.
- Respect explicit theme choice first, system preference second. The source website stores the choice under `sj-theme`; use a project-appropriate key elsewhere.
- Include a skip link or equivalent navigation aid on document-like web pages.

## Component language

### Cards and panels

- Background: `--bg-card`.
- Border: 1 px `--border`.
- Radius: 10 px.
- Hover: accent border and, where appropriate, a very small positional response. Do not add a dramatic shadow.
- Use `--bg-secondary` for inset wells, code blocks, image placeholders, sidebars, and alternating layers.

### Navigation

- Default text: `--text-secondary`.
- Control radius: 6 px; typical padding 6 x 12 px; 14 px Manrope 400.
- Hover: primary text on card background.
- Active: accent text on tag background, weight 600.
- Sticky site navigation may use top 0 and z-index 50. Desktop application menu and tool bars should follow native docking behavior.

### Primary actions

- White text on `--accent`.
- Radius: 8 px; typical padding 8 x 20 px; 14 px Manrope 600.
- Hover: `--accent-hover`.
- Secondary actions use a transparent or card background with a border and primary text.
- Destructive actions require a separate danger treatment.

### Tags and compact status chips

- Background: `--tag-bg`; text: `--tag-text`.
- Radius: 4 px; padding 4 x 10 px; 11 px Manrope 500; letter spacing about 0.04 em.
- Hover opacity may reduce to 0.8 for interactive tags.
- Do not turn ordinary buttons, fields, tabs, or every piece of metadata into pills.

### Fields and data controls

- Use card or secondary background, 1 px border, primary value text, secondary placeholder text, and accent focus.
- Maintain compact native sizing in dense tools.
- Align numeric or code-like data with the mono face only where it improves scanning.
- Selected rows and focused regions use restrained tag/accent backgrounds; preserve contrast.

### Tables

- Collapse adjacent borders on documents; use the platform's efficient grid treatment in applications.
- Typical document size: 15 px.
- Cell padding: 8 x 12 px.
- Header background: `--bg-secondary`, weight 600.
- Border: `--border`.

### Editorial prose

- Headings use Lora 700 and primary text; general line-height 1.3.
- H2 is about 24 px with a top border, 48 px top margin, and 24 px top padding.
- H3 is intentionally more utilitarian: Manrope 18 px, 700.
- H4 is Manrope 16 px, 600.
- Paragraphs have 24 px bottom margin.
- Lists have 28 px left padding and about 1.7 line height.
- Links use accent, underline, and 3 px underline offset; hover uses accent-hover.
- Blockquotes use a 3 px accent left border, card background, secondary italic text, 12 x 24 px padding, and `0 8px 8px 0` radius.
- Images use an 8 px radius.
- Horizontal rules use the border color with 40 px vertical margin.

## Responsive behavior

- Primary breakpoints are approximately 480, 640, and 768 px.
- Collapse multi-column content to one column before it becomes cramped.
- The source hero moves from a stacked image/text card to a `2fr 3fr` grid at 768 px and grows from roughly 280 to 360 px minimum height.
- Two-up supporting content becomes one column below 768 px.
- Reduce horizontal page padding from 24 to 16 px on small screens.
- For resizable desktop windows, use the same principle: collapse or hide secondary panes intelligently rather than squeezing all controls into illegibility.

## Native application mapping

Map semantics, not CSS syntax:

- Window/background -> `--bg-primary`
- Sidebar, inset, alternate row -> `--bg-secondary`
- Card, dock panel, dialog surface -> `--bg-card`
- Primary/disabled/secondary text -> primary and secondary tokens with state-appropriate opacity
- Selection/focus/default action -> accent tokens
- Separators and input outlines -> border token

For Qt, centralize these values in the existing theme manager, palette, and QSS generation rather than duplicating hex literals in `.ui` files. Map `Window`, `Base`, `AlternateBase`, `Text`, `PlaceholderText`, `Highlight`, and `HighlightedText` deliberately. Keep plot canvases and data-series colors separately tuned for legibility.

For web frameworks, expose the semantic properties at the theme root and bind components to them. Do not leave framework-default blues, grays, shadows, or radii mixed into the final surface.

## Anti-pattern check

Reject or remove these when applying the system:

- gradients, glass panels, neon glow, heavy drop shadows;
- huge marketing typography or long six-line display headlines;
- excessive rounded pills and floating cards;
- decorative charts, fake metrics, or UI controls without product purpose;
- multiple competing accent colors;
- serif text in dense controls or technical data grids;
- low-contrast gray-on-gray text;
- hover-only affordances, invisible keyboard focus, or motion without reduced-motion handling;
- token files that are not actually connected to the rendered components;
- broad redesigns of unrelated workflows merely to demonstrate the style.

## Practical fidelity check

A successful implementation should be recognizable without the logo. In both themes, confirm:

1. backgrounds form three quiet neutral layers;
2. primary and secondary text remain clearly distinct and readable;
3. blue is used consistently for actions, focus, active navigation, and links;
4. Manrope, Lora, and Fira Code have deliberate jobs;
5. borders and spacing create hierarchy without decorative effects;
6. hover, focus, selected, disabled, empty, and error states fit the same system;
7. the product still behaves like its category rather than like a copy of the Stack Junkie homepage.
