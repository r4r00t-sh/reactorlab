# Phase 0 — Definition of Done

Phase 0 is complete when all of the following are true:

1. **Repository skeleton** matches the polyglot layout (`core/`, `bindings/`, `python/`, `gui/`, `docs/`, `data/`, `scenarios/`, CI).
2. **CMake ≥ 3.25** configures with Ninja presets (`dev`, `release`, `asan`, `core-only`).
3. **Dependencies:** `vcpkg.json` present; FetchContent fallback builds fmt/spdlog/GTest without system packages.
4. **`libreactorlab`** builds as a static library with `version` / `hello` / educational disclaimer APIs.
5. **GoogleTest** suite passes (`ctest --preset dev`).
6. **nanobind** module `reactorlab_core` imports and passes `python/tests/test_bindings_smoke.py`.
7. **Qt 6 Widgets** app `reactorlab_gui` launches and shows the disclaimer / About content (no physics).
8. **Tooling:** `.clang-format`, `.clang-tidy`, `.pre-commit-config.yaml`, `pyproject.toml` (ruff/pytest).
9. **CI:** `.github/workflows/ci.yml` covers build+test, ASan/UBSan, and clang-format check.
10. **Docs:** README disclaimer, `docs/ubuntu-setup.md`, ADR 0001, empty `data/PROVENANCE.md` policy.
11. **Sanitizer preset** builds and runs C++ tests without bindings/GUI.

Out of scope for Phase 0: physics modules, QCustomPlot vendoring, nuclear data, packaging.
