# Projects install the library from tagged GitHub releases

Projects install Stack Junkie Style straight from a tagged release in its public GitHub repo: web projects from the built library file attached to the release (a git dependency cannot work, because the package sits in `ui/web` and its build output is not committed), native projects through CMake fetching the tag. Publishing to the public npm registry was the alternative. It was deferred because it adds a public release step that tagged releases do not need. It can be added later. Decided on 2026-11-26.
