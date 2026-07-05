---
description: Keep Markdown documentation synchronized with C++ and build-related changes.
applies_to:
  - "**/*.cpp"
  - "**/*.hpp"
  - "**/*.h"
  - "**/*.c"
  - "**/CMakeLists.txt"
  - "**/*.cmake"
  - "**/meson.build"
  - "**/*.ini"
  - "**/*.json"
---

# Sync Markdown With Code Changes

Whenever you modify C++ code, build files, configuration files, packaging files, or C++ project conventions, check whether Markdown documentation must be updated.

Update relevant Markdown in the same change when the code change affects any of these:

1. Public API, headers, or user-facing behavior
2. Build steps, dependencies, supported platforms, or toolchain requirements
3. Configuration file format, defaults, examples, or generated output
4. Tests, demos, packaging, release process, or installation instructions
5. Project conventions, steering rules, or contributor guidance
6. Architecture, design decisions, or implementation details documented under `docs/`

Common Markdown locations to check:

1. `README.md`
2. `README_ZH.md`
3. `docs/*.md`
4. `.ai/steering/**/*.md`

If no Markdown update is needed, leave the docs untouched. Do not edit Markdown just to mention an internal refactor that has no documented behavior or workflow impact.

Before finishing, report whether documentation was updated or why no Markdown change was needed.
