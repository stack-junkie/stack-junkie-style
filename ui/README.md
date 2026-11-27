# Stack Junkie living showcase

One shared design source, reusable web and Qt components, and working examples built from those components. Version comes from the parent `VERSION` file.

This preview starts from a dark neutral workbench with a section rail, adjustable contextual sidebar and connected page tabs. **Clean utility** is the only design direction, with darker neutral black surfaces and a platform UI font, in light and dark. It preserves stored theme choices. Fonts and accents remain under review. Both showcases use the selected SJ letterforms without a background. The shared transparent brand/sj-transparent-mask.png maps S to text and J to accent at render time, following the active theme. The owner accepted the blue J and requested the smaller 25px mark. Shared typography and density values distinguish compact controls from readable content; see [research and decisions](../references/typography-and-density.md). The current catalog is an interactive showcase. The requested feature configurator and configured-project export remain planned work.

## Run the web showcase

Requirements: Node.js 22.12+ and npm. From this `ui` directory:

```shell
cd web
npm ci
npm run dev
```

Open the printed local URL yourself. The server binds to localhost and does not launch or control a browser. The catalog uses the same library. Inputs and page state remain mounted when changing catalog families, context pages or Example / Usage tabs. Drag or keyboard-resize the context divider; collapse and reopen it without losing its width.

To build the showcase and importable library:

```shell
npm run build
npm run build:library
```

Serve the built showcase from this `ui` directory, without development dependencies:

```shell
node scripts/serve.mjs
```

## Run the Windows showcase

Requirements: Visual Studio 2022 C++ build tools, CMake 3.21+, and a matching Qt 6 x64 MSVC SDK with Widgets and Svg. From this `ui` directory:

```powershell
./scripts/build-native.ps1 -QtPrefix C:/Qt/6.8.3/msvc2022_64
```

The script also recognizes the SDK in this checkout's ignored `.tools/Qt/6.8.3/msvc2022_64` directory. It builds and packages without launching the GUI. Run `dist/windows/bin/StackJunkieShowcase.exe` yourself. Microsoft Visual C++ 2015-2022 x64 Runtime is required on the destination computer.

Native screenshot export uses Qt's offscreen renderer and does not control the desktop:

```powershell
./dist/windows/bin/StackJunkieShowcase.exe --export-preview ./dist/settings-light.png --page settings --theme light
```

## Build on Linux

Requirements: C++17 compiler, CMake 3.21+, Qt 6.4+ development packages for Widgets and Svg. With those dependencies already installed, from this `ui` directory:

```shell
cmake -S native -B native/build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build native/build-linux --parallel 2
cmake --install native/build-linux --prefix "$PWD/dist/linux"
./dist/linux/bin/StackJunkieShowcase
```

The Linux install uses the system Qt runtime. It is not an AppImage. This checkout's existing WSL distribution has a missing virtual disk, so Linux compilation and runtime review remain unverified here. Windows results must not be treated as Linux results.

## Reuse the components

For React applications, build `web/dist-library` and import the package's exported components and stylesheet. For Qt applications, use `add_subdirectory` or the installed `StackJunkie` CMake package and link `StackJunkie::Widgets`. See [native usage](native/README.md) and the web library exports in `web/src/library/index.ts`.

Pages supply content, state and actions. Shared components own repeated behavior and styling. Use composition for layouts; inheritance is useful for native controls when it genuinely removes duplication. Existing Qt 5, WPF or other toolkit applications need an adapter, not an automatic framework migration.

The shared inputs are:

- `shared/tokens.json`: semantic design values and the Clean utility direction.
- `shared/catalog.json`: 52 component entries, applicable variants/states and eight product layouts.
- `shared/fonts` and `shared/icons.svg`: local assets for both implementations, with font licenses.
- `../VERSION`: the only authored release version.

The native library embeds those inputs through Qt resources. The web library consumes them at build time. Neither showcase maintains a second set of visual values. Toolkit-specific layout mechanics remain within their adapters.

## Update and package

Change shared values or a library component first, then its relevant example and skill guidance. Increment `../VERSION` for the next delivered preview or release; planning edits do not need their own release. The web build prepares derived version metadata and native CMake reads the same source. Rebuild both showcases for each delivered update. Consumers pin a version and upgrade explicitly.

After the web and native builds, from this `ui` directory:

```shell
node scripts/package.mjs
```

The explicit package input list excludes private research/transcripts, `.tools`, dependency folders and intermediate builds. Bundles are local outputs under `dist`, not published releases. The old static specimen is preserved in the ignored history collection and its former entrypoint links to this showcase.

The final appearance, owner-operated keyboard/pointer review and Linux runtime acceptance remain open. Compilation and offscreen exports do not establish those outcomes. No desktop or browser automation, permanent tests, validators or release hooks are introduced.

## Settings and About

The header SJ mark opens Home. Foundations is accessible from Home and command search. One fixed hamburger at the top of the activity rail shows or hides contextual navigation. Group names remain in the sidebar; component tabs occupy the top content row. A compact component name appears alongside tabs only while navigation is collapsed. On web, Workspace's contentOwnsHeader option lets page tabs own this row, and its children render function supplies sidebarCollapsed to Tabs.contextLabel. Native Workspace handles the equivalent label for tabbed pages. Page contents remain mounted across navigation.

Settings is the gear at the bottom of the activity rail (or the sidebar without a rail) and opens `SettingsWindow`. Control comma opens it too. Appearance (theme), Keyboard and About are its sections. The header has no theme or search button.

Apps use `AppShell`: supply `name`, `description`, `version`, `logo`, `sections` (each with pages) and optional `settings`. On web it accepts controlled `location`, `mode` and `settingsState`, plus slots for the brand, sidebar header, sidebar override and overlays. Native `AppShell` derives from `Workspace` and adds the Settings window.

Import `AboutSection` and `ThemeSetting` only when replacing a built-in section. Supply the product name, description, logo and product version from that project's version source. Web exports systemVersion; native exposes STACKJUNKIE_VERSION. Both are generated from the parent VERSION file. Never replace the style version with the consuming application's version.

Showcase-only controls are labeled and implemented under showcase, not exported from the component library. Settings › Layout exports an `App.tsx` that imports the library at the current version and configures `AppShell`.
