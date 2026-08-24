# vnewindow Skills

Task playbooks for working in vnewindow (`vne::xwin`). Each folder holds a
self-contained `SKILL.md` (YAML frontmatter plus a short body) that any AI
coding tool or human can read. They capture conventions already documented in
`CODING_GUIDELINES.md`, `CONTRIBUTING.md`, `scripts/README.md`, and
`.github/copilot-instructions.md`, distilled into a checklist to apply while
working.

These were adapted from the vnegfx skills set. vnewindow is a native windowing
library, not a renderer: there is no GPU shader / `gpu_layout.yaml` pipeline
here. Do not copy `vne-shader-pipeline` into this repo.

## Skills

| Skill | Enforces | Fires when |
|-------|----------|------------|
| [vne-coding-style](vne-coding-style/SKILL.md) | Naming, formatting, initialization, modern C++ | Writing or editing C++ / Objective-C++ |
| [plain-ascii-authoring](plain-ascii-authoring/SKILL.md) | ASCII-only text, no AI glyphs or filler phrasing | Producing any code, comment, doc, or message |
| [vne-header-hygiene](vne-header-hygiene/SKILL.md) | Include order, header self-containment, forward decls | Editing includes or creating headers |
| [vne-build-verify](vne-build-verify/SKILL.md) | Format, build, and test pipeline via scripts | Before finishing any source change |
| [vne-testing](vne-testing/SKILL.md) | GoogleTest layout, determinism, null-backend tests | Adding or changing tests |
| [vne-xwin-platforms](vne-xwin-platforms/SKILL.md) | Backend layout, native handles, thread rules | Editing platform backends or public window APIs |

## Using these skills

- Claude Code: the same skills are symlinked under `.claude/skills/`, so they are
  auto-discovered and invocable by name.
- Cursor: a pointer rule under `.cursor/rules/` references this directory.
- GitHub Copilot and others: `.github/copilot-instructions.md` points here; read the
  matching `SKILL.md` before build, test, header, or style work.

## Self-check

Keep this directory ASCII-clean per
[plain-ascii-authoring](plain-ascii-authoring/SKILL.md):

```bash
rg -nP "[^\x00-\x7F]" skills
```

A clean run prints nothing.
