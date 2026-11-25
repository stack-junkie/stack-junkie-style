# Product layouts

Choose by the task, objects people move between and frequency of switching. A web app can be a workbench; a native app can be a one-action utility. Classify distinct surfaces separately when one product contains a reading site, workspace and editor.

These eight IDs are shared catalog entries and showcase compositions, not eight mandatory pages. Use only regions that earn their place and compose the same library controls used elsewhere.

| Catalog ID | Fits | Starting composition |
| --- | --- | --- |
| `focused-utility` | One recurring operation with a few choices | Input, action and result; optional controls beside their purpose |
| `multi-tool` | Distinct operations with their own setup or saved items | Section rail, contextual page navigation and work area; persistent active status and Stop |
| `embedded` | A tool inside another product | Host navigation, dimensions and state; local actions without a competing app shell |
| `data` | Durable areas operating on related records | Destinations, filters, collection and detail/inspector; preserved selection |
| `workbench` | Switching among tool families, resources and open work | Section rail, contextual sidebar and dominant workspace; document tabs or peer views |
| `publication` | Browsing, searching and long reading | Site navigation, readable content and useful local contents; preserved links and reading position |
| `guided` | Ordered steps to complete a task | Position, required inputs, consequences and safe backtracking; preserved drafts |
| `monitoring` | Noticing changes, comparing signals and investigating | Meaningful status and exceptions, then evidence; retained filters, units and timestamps |

## Navigation levels

The narrow icon sidebar is the activity rail. The adjacent contextual sidebar is the owner's "subsidebar". Settings is pinned to the fixed bottom of the activity rail, outside its scrolling navigation. Without a rail, it belongs at the bottom of the contextual sidebar. Do not move Settings into the header. If both navigation regions are hidden, retain a bottom-edge route to Settings and to reopen navigation.

A rail selects a top-level section. Its contextual sidebar selects pages or objects within that section. A tab row switches peer views of the selected page. Document tabs instead represent separate open objects. Do not let two controls silently select the same destination or mix tab meanings without a clear distinction.

The shared Workspace composes the rail and contextual sidebar simultaneously. Collapse hides the contextual pane without replacing the rail or losing its width. The showcase uses families in the rail, components in the sidebar, and Example / Usage and states as peer views. Connected tabs touch, use a top active edge and meet the content surface; segmented choices remain a different control.

Keep one navigation hamburger at the fixed top of the activity rail, in the same position when the contextual sidebar is open or closed. Do not add a second collapse arrow or a second main-area hamburger. Without an activity rail, retain a reachable toggle in the workspace bar. The contextual heading names the group, such as Actions. Its selected item names the page, such as Button. Put that page's tabs at the top of the main area, aligned with the group heading, without a redundant title row. When the sidebar is collapsed, retain a compact page name alongside the tabs. Keep accessible names and keyboard focus intact. The header product logo returns to that product's Home screen, not the style provider's website. In the showcase, Home supplies access to Foundations after removal of its top rail icon.

Planning can be a section, Budgets and Forecasts its pages, and Plan and Actual views of one budget. Returning to Budgets should restore the appropriate object and view. Labels, active treatment and keyboard focus must make location apparent.

Two sidebars can be useful when they answer different questions. They are unnecessary when they duplicate destinations or consume the task's working space. Keep rail icons named and discoverable; use labels when ambiguous. Group Settings by purpose and keep per-run options beside the operation.

## Splits and constrained space

Resizable panes need pointer and keyboard resizing, sensible limits, collapse/reopen controls and restoration of width and selection. Collapsing must not discard edits or strand focus. Preserve relevant scroll position and a stable route to recover a hidden inspector.

Expanding the contextual sidebar takes space in the layout and moves the working area. Browser zoom or a narrow viewport must not silently convert it into an overlay or change the owner's collapse choice. Allow resizing across the available container space rather than arbitrary fixed width bands. Keep the collapse/reopen control reachable, preserve the chosen width when temporary viewport limits constrain it, and allow local content scrolling. A small utility need not grow a rail; a multi-tool utility can need a larger window.

## State belongs to the product

Keep active work independent of the selected page. A tab switch does not stop an operation or change who owns input. Persistent status and Stop or Cancel describe the running operation on every relevant view. Keep drafts, selections and filters stable, and resolve unsaved edits before discarding them.

When navigation names the page, begin content at the first meaningful choice unless a heading adds identity. The Macro Recorder rejection of a repeated Run Hold summary illustrates this: removing redundant copy must preserve window, input, duration and action controls. Its exact arrangement is a product decision, not a layout for every utility.

## Adapting and reviewing

Use the catalog and showcase implementation for the selected composition. Supply real content and omit unused slots. Preserve an approved composition unless a later instruction changes it. Keep mockup decisions scoped, including fixed-page requirements or intentionally compact controls.

Check composition and state through permitted source or CLI inspection, then give the owner the runnable GUI. Do not automate their browser or desktop. Report what the owner has and has not accepted; historical rejected examples and candidate styles are not approvals.
