# Third-party resources

Stack Junkie code follows the adjacent MIT license. Third-party resources retain their own licenses.

- Manrope, Lora and Fira Code: SIL Open Font License 1.1. Original texts are in `shared/fonts`; provenance is in `shared/ASSETS.md`.
- React and React DOM: MIT. The installed dependency licenses accompany packaged web artifacts. Exact dependency versions are in `web/package-lock.json`.
- Qt Core, Gui, Widgets and Svg: the Windows preview is built against Qt 6.8.3 and dynamically links the corresponding Qt libraries. License texts are in `shared/licenses`. Qt source is available from the [Qt 6.8.3 source archive](https://download.qt.io/archive/qt/6.8/6.8.3/submodules/) and the [Qt source repositories](https://code.qt.io/). Linux builds use the operator's installed Qt packages.
- Microsoft Visual C++ runtime: a prerequisite for the Windows MSVC build, supplied by Microsoft rather than copied into this source package.

Bundled Qt libraries remain replaceable shared libraries. The complete Stack Junkie native source and CMake build files are included. This preview does not install or modify system runtimes.
