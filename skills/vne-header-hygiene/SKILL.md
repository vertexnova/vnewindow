---
name: vne-header-hygiene
description: >
  Enforce include ordering, header self-containment, and forward-declaration
  rules in vnewindow. Use this skill when adding or editing #include / #import
  directives or creating C++ / Objective-C++ headers.
---

# Header Hygiene

Rules for includes and headers in vnewindow. Full context is in the Header Files
section of [CODING_GUIDELINES.md](../../CODING_GUIDELINES.md).

## 1. Include group order

Group includes, one blank line between groups, in this order:

1. The corresponding header (in a `.cpp`/`.mm`): `#include "window.h"` or
   `#include "cocoa_window.h"`.
2. Project headers: `#include "vertexnova/xwin/..."`.
3. Other VertexNova libraries (often angle brackets):
   `#include <vertexnova/events/types.h>`.
4. System headers: `#include <vector>`, `<string>`, `<memory>`.
5. Third-party headers: `#include <gtest/gtest.h>`.

clang-format sorts alphabetically *within* each group. Do not hand-sort or
reorder across groups; keep the blank-line separators so the grouping survives.

Public headers live under `include/vertexnova/xwin/` and must remain
self-contained. Implementation-only headers live next to their `.cpp`/`.mm`
under `src/vertexnova/xwin/`.

## 2. Header self-containment

Every `.h` must compile on its own. Include what you use directly in the header;
never rely on a type being pulled in transitively by another include or by the
`.cpp` that includes it. If a header names `IWindow` or `WindowDescriptor`, that
header includes or forward-declares it, not its callers.

## 3. Forward declarations

Prefer a forward declaration over an include when only a pointer or reference is
used in the header. Include the full definition only when the layout is needed
(by value, base class, container element, or member access).

## 4. Header structure

- `#pragma once` first, then the copyright banner block.
- Then: includes (grouped as above), forward declarations, `namespace`, then the
  class / struct / enum. Close with `}  // namespace vne::xwin`.

## 5. Objective-C++ import order

In Cocoa/UIKit `.mm` files: `#include` C++ headers first, then `#import`
Objective-C headers, then system frameworks (guarded by platform macros where
needed). Keep ObjC helper `@interface` types in the `.mm` when they are not part
of the public C++ API.

## 6. After editing includes

Run `python3 scripts/clang_formatter.py all` (or add `--dry-run` to check), then
build. See [vne-build-verify](../vne-build-verify/SKILL.md).

Source of truth: [CODING_GUIDELINES.md](../../CODING_GUIDELINES.md) (Header Files).
