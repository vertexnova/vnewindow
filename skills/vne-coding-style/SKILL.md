---
name: vne-coding-style
description: >
  Enforce VertexNova C++ and Objective-C++ conventions (naming, formatting,
  initialization, modern C++) for vnewindow. Use this skill when writing or
  modifying any code under src/, include/, examples/, or tests/.
---

# VertexNova Coding Style (vnewindow)

A compact checklist for `vne::xwin` code. The full rules with rationale live in
[CODING_GUIDELINES.md](../../CODING_GUIDELINES.md); this skill is the quick
reference to apply while editing. C++ standard is C++20.

Public API: `include/vertexnova/xwin/`. Implementation: `src/vertexnova/xwin/`.
Examples: `examples/`. Do not edit `deps/` unless the change is intentionally
about a submodule.

## 1. Naming (apply exactly)

| Construct | Style | Example |
|-----------|-------|---------|
| Classes / Structs | PascalCase | `WindowFactory`, `NativeWindowHandle` |
| Interface classes | `I` + PascalCase | `IWindow`, `IWindowManager` |
| Enums | PascalCase | `WindowAPI`, `WindowMode` |
| Enum values | `e` + PascalCase, explicit value | `eNullWindow = 0`, `eCocoaWindow = 21` |
| Type aliases | PascalCase, no `T` prefix | `WindowPosition` |
| Functions / Methods | camelCase | `createWindowManager()`, `getNativeHandle()` |
| Constants | `k` + PascalCase | `kMaxWindowCount` |
| Private / Protected members | snake_case + `_` | `is_open_`, `descriptor_` |
| Public members | snake_case | `hwnd`, `ns_view`, `canvas_id` |
| Locals / Parameters | snake_case | `window_api`, `width` |
| Static (private) | `s_` + snake_case + `_` | `s_next_id_` |
| Global | `g_` + snake_case | `g_last_error` |
| Booleans | `is_` / `has_` / `can_` / `should_` | `is_open_`, `has_focus_` |
| Macros | ALL_CAPS, `VNE_` prefix | `VNE_XWIN_API`, `VNE_ASSERT` |
| Namespaces | lowercase | `vne`, `xwin` |
| File names | snake_case | `window_factory.h`, `cocoa_window.mm` |

CMake and export macros for this library use `VNE_XWIN_*` (not `VNE_GFX_*`).

## 2. Files and headers

- `#pragma once` for every header (never include guards).
- Start each file with the project copyright banner block (see any existing
  header under `include/vertexnova/xwin/`).
- Include ordering and self-containment: see [vne-header-hygiene](../vne-header-hygiene/SKILL.md).
- Platform backends: see [vne-xwin-platforms](../vne-xwin-platforms/SKILL.md).

## 3. Types and members

- `enum class` only, `e`-prefixed values with explicit integer values.
- Rule of Zero first; when a special member is needed, declare the full Rule of Five.
- `explicit` on single-argument constructors.
- Prefer brace initialization `{}`; give members in-class defaults
  (`bool is_open_{false};`) and use the initializer list only for injected values.
- Struct = data container; class = invariants plus behavior. Order members:
  public types/constants, constructors/destructor, public methods, protected, private.

## 4. Functions

- Return values over out-parameters; `[[nodiscard]]` when the result must be used.
- `const` on methods that do not mutate; `noexcept` on non-throwing methods and moves.
- References for non-nullable parameters, pointers only when null is meaningful.

## 5. Modern C++ to prefer

- `std::optional<T>` for "may be absent", `std::string_view` for non-owning string
  params, `std::span<T>` for non-owning array views.
- `nullptr` (never `NULL`/`0`), `constexpr` / `if constexpr`, structured bindings,
  range-based `for`, `auto` only when the type is obvious.
- Smart pointers for ownership (`std::unique_ptr` exclusive, `std::shared_ptr` shared,
  `std::weak_ptr` to break cycles); RAII for every resource.

## 6. Objective-C++

- C++ interface in `.h`, implementation in `.mm` (Cocoa and UIKit backends).
- Import order: `#include` C++ headers, then `#import` Objective-C headers, then
  system frameworks. ARC is on; no manual `retain`/`release`.
- Native pointers stored in `NativeWindowHandle` are non-owning; the backend
  `IWindow` retains the AppKit/UIKit objects.

## 7. Documentation and prose

- Doxygen `@brief` / `@param` / `@return` on public APIs whose behavior is not obvious.
- Document thread safety with `@threadsafe` or `@warning Not thread-safe`.
  AppKit/UIKit backends are main-thread-only unless a method says otherwise.
- Keep all comments, identifiers, and docs ASCII-only and free of AI phrasing:
  see [plain-ascii-authoring](../plain-ascii-authoring/SKILL.md).

## 8. Before you finish

Run formatting and the build/test pipeline: see
[vne-build-verify](../vne-build-verify/SKILL.md). Formatting and static analysis are
CI-enforced via `.clang-format` and `.clang-tidy`, so do not hand-format.

Source of truth: [CODING_GUIDELINES.md](../../CODING_GUIDELINES.md),
[.github/copilot-instructions.md](../../.github/copilot-instructions.md).
