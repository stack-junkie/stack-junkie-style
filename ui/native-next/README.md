# Stack Junkie UI, next candidate (Qt native)

The native side of the `../web-next` candidate. `../native` is unchanged.

## Build and run

The script `../scripts/build-native.ps1` still builds `../native`. Build this one with the same steps pointed here:

```powershell
$ui = "<repo>\skills\stack-junkie-style\ui"
cmake -S "$ui\native-next" -B "$ui\native-next\build-windows" -G "Visual Studio 17 2022" -A x64 "-DCMAKE_PREFIX_PATH=$ui\.tools\Qt\6.8.3\msvc2022_64"
cmake --build "$ui\native-next\build-windows" --config Release
cmake --install "$ui\native-next\build-windows" --config Release --prefix "$ui\dist\windows-next"
& "$ui\.tools\Qt\6.8.3\msvc2022_64\bin\windeployqt.exe" --release --no-translations --no-compiler-runtime "$ui\dist\windows-next\bin\StackJunkieShowcase.exe"
```

Copy `plugins\platforms\qoffscreen.dll` into `dist\windows-next\bin\platforms` for screenshot export. `--page settings` exports the Settings window.

## What changed from preview.9

- **True black, blue accent.** Clean utility's dark mode uses `#000000` grounds, `#1f1f1f` borders and near-white text. Light mode uses one white ground with thin borders. Splitters are 1px lines, and scrollbars are 8px. The overrides live in `src/theme.cpp` until the shared `tokens.json` moves over.
- **`SettingsWindow`.** A dialog with search and a section list, where changes apply immediately. `Workspace::setSettingsHandler` makes the rail's gear open it, and Settings is no longer a Workspace section. The showcase opens it with the gear or Control comma.
- **No stacked tab rows.** In debug builds, `Tabs` warns when it sits inside another tab page. `TabBoundary` frames specimens and embedded apps so their tabs are allowed.
- **`GroupedTabs` is one row.** Group chips sit in the row and collapse their group, replacing the row of groups above a row of tabs.
- **Closable document tabs.** `Tabs::setDocumentTabs(true)` closes immediately, with no confirmation, using the library's close icon. Control W closes the current tab.
- **`NavTree`.** Collapsible page groups with counts, VS Code style.
- **`Workspace` options.** `setResizable` and `setCollapsible`.
- **`AppShell`.** A Workspace with a built-in Settings window (product sections, then Appearance and About) and Control comma.
- **Showcase.**
  - Product layouts use their own `layout` icon, so the gear belongs to Settings.
  - The Compare page no longer adds a rail icon.
  - The workbench is the reference for many-page apps: rail, tree, one row of closable documents, and a resizable bottom panel with its own tabs.
