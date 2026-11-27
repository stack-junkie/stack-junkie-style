# Projects dashboard

A local Dashboard: which projects use Stack Junkie Style, which version, and which are behind. It runs on the library's own `AppShell` with the Web app preset, imported from source. It is not part of the public showcase, because it lists private projects.

## Run

```
npm install
npm run dev
```

`npm run dev` scans, then serves the app at http://127.0.0.1:5183. The Scan page reruns the scan.

- `npm run scan` writes `data/projects.json` only. `node scan.mjs --local` skips GitHub and GitLab.
- `npm test` runs the scan on the sample folders in `test/fixtures`.
- `npm run build` typechecks and builds. The build holds no scan data.

## What the scan reads

Folders, skipped folder names, look-alike apps (`exempt`) and the remote call limit are in `scan.config.json`.

A project uses the style when it has any of:

- a `package.json` dependency on `@stack-junkie/ui-web` (version from the installed copy, the tag or the range)
- an import of `@stack-junkie/ui-web` in its source (no version)
- CMake `find_package(StackJunkie...)`, or `FetchContent_Declare` or `add_subdirectory` of the style repo (version from `GIT_TAG`)
- a copied `stack-junkie-style` skill folder (version from its `VERSION` file, if any)

Each git repo is one project. A local clone of a GitHub or GitLab repo is one project with links to both. The style's own source tree is reported as the source and is not counted.

GitHub repos are listed with `gh` and GitLab repos with `glab`. Each repo costs one call for its root files plus one per marker file, and two GitHub code searches cover imports and copied skills below the root. A missing or signed-out CLI shows as "not connected".

`data/` is gitignored and stays on this machine.
