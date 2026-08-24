# Diagrams

Draw.io source for `vne::xwin`. Markdown on GitHub uses the exported SVG files.

Band colors match vnerhi / vnegfx architecture diagrams (consumers blue, entry/backends orange, host teal, surfaces green, support indigo, events/native purple).

## Export to SVG

If [draw.io Desktop](https://github.com/jgraph/drawio-desktop/releases) is installed:

```bash
drawio -x -f svg -o context.svg context.drawio
drawio -x -f svg -o architecture.svg architecture.drawio
drawio -x -f svg -o class_diagram.svg class_diagram.drawio
drawio -x -f svg -o event_loop.svg event_loop.drawio
```

Or export every diagram in this folder:

```bash
drawio -x -f svg -o . .
```

Web: open [app.diagrams.net](https://app.diagrams.net), File -> Open from -> Device, File -> Export as -> SVG.

## Files

| Source | Output | Used in | Contents |
|--------|--------|---------|----------|
| context.drawio | context.svg | xwin.md overview | Application, xwin, backends, vneevents |
| architecture.drawio | architecture.svg | xwin.md, architecture/README.md | Layered bands (no arrows) |
| class_diagram.drawio | class_diagram.svg | xwin.md, architecture/README.md | UML class diagram: interfaces, emitter, value types, enumerations |
| backend_classes.drawio | backend_classes.svg | xwin.md, architecture/README.md | All 8 backend pairs, ObjC helpers, class flow only (no members) |
| event_loop.drawio | event_loop.svg | xwin.md event bridge | processEvents -> EventManager -> frame -> Input::nextFrame |

## Which class diagram to read

| Want | Read |
|------|------|
| The API shape - what members and signatures exist | `class_diagram.svg` (Cocoa as exemplar backend) |
| The structure - every backend and how the classes connect | `backend_classes.svg` (names only, no members) |

Both use the same UML notation and the same palette, so they read as one pair rather than two
unrelated pictures.

## class_diagram.drawio conventions

Standard UML notation, with a legend on the canvas:

| Relationship | Notation |
|---|---|
| Realization (implements an interface) | dashed line, hollow triangle |
| Generalization (inheritance) | solid line, hollow triangle |
| Composition (by-value member) | filled diamond at the owner |
| Aggregation (`shared_ptr` / reference) | hollow diamond at the owner |
| Directed association | solid line, open arrow |
| Dependency (`«use»`, `«create»`) | dashed line, open arrow |

Stereotypes: `«interface»`, `«utility»`, `«struct»`, `«enumeration»`, `«internal»`, `«external»`.
Static members are underlined. Multiplicities (`1`, `0..*`, `0..1`) sit at the owned end.

Cocoa is drawn as the exemplar backend; the other seven realize the same two interfaces with the
same three members (`id_`, `desc_`, `events_`), so drawing all eight would add rows without adding
information.

**Gotcha:** avoid generic `mxCell` ids such as `map` — they collide with mxGraph's internal lookup
and make `drawio -x` fail with a bare `Error: Export failed` and no further detail. Namespace ids
(`cls_inputmap`) instead.
