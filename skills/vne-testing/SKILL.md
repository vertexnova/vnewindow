---
name: vne-testing
description: >
  Conventions for writing and organizing GoogleTest tests in vnewindow
  (null-backend unit tests, factory and event coverage). Use this skill when
  adding or changing tests, or when a behavior change needs test coverage.
---

# Testing Conventions

vnewindow uses GoogleTest. A feature or bug fix ships with a test unless there
is a clear reason not to. Tests must be deterministic and fast.

This is not vnegfx: there is no `tests/whitebox/`, `tests/blackbox/`, or
image-comparison suite. Do not invent that layout here.

## 1. Layout

- `tests/` - unit tests compiled into `vnexwin_tests` (see `tests/CMakeLists.txt`).
- Existing files: `window_factory_test.cpp`, `null_window_test.cpp`,
  `null_window_manager_test.cpp`, `event_emitter_test.cpp`.
- Add new `*_test.cpp` files to `TEST_SOURCES` in `tests/CMakeLists.txt`.
- Shared example runtime lives under `examples/common/`; do not put unit tests
  there.

Prefer the **null** backend (`WindowAPI::eNullWindow`) so CI stays headless.
Platform-specific behavior that needs a real display belongs in examples and
manual checklists (`examples/README.md`), not in the default GoogleTest binary.

## 2. File and case naming

- One test file per unit: `<subject>_test.cpp`.
- Follow project naming in test code too (PascalCase types, camelCase methods,
  snake_case locals); see [vne-coding-style](../vne-coding-style/SKILL.md).

## 3. Structure

- Arrange, Act, Assert, with a blank line between phases.
- One behavior per `TEST` / `TEST_F`; give the case a name that states the behavior.
- Keep mocks minimal and assert on observable behavior, not implementation details.
- Construct managers through `WindowFactory::createWindowManager(WindowAPI::eNullWindow)`
  unless the test is specifically about factory selection.

## 4. Determinism

- No `sleep`, no timeouts, no wall-clock or ordering assumptions.
- Do not depend on a GPU, a window server, or special hardware in unit tests.
- Fix random seeds; avoid data races in threaded tests.

## 5. What not to copy from vnegfx

- No committed reference images or `image_compare.h` helpers.
- No `tests/shared/` tree unless several new test files genuinely need a fixture.
- Unicode or binary fixtures (if ever added) are exempt from the ASCII rule
  (see [plain-ascii-authoring](../plain-ascii-authoring/SKILL.md)).

## 6. Running

```bash
ctest -C Debug --test-dir build/shared --output-on-failure
ctest -C Debug --test-dir build/shared -R <suite_or_name>
./scripts/build_macos.sh -a test
<test-binary> --gtest_filter=Suite.TestName
```

See [vne-build-verify](../vne-build-verify/SKILL.md) for building the test target.

Source of truth: the test-review section of
[.github/copilot-instructions.md](../../.github/copilot-instructions.md),
[examples/README.md](../../examples/README.md) for interactive coverage.
