# Diagrams

Draw.io source files for the `vne::xwin` module documentation.

## Export to PNG

The `xwin.md` document references PNG images. Export the `.drawio` files to PNG using one of these methods:

### Option 1: draw.io Desktop (macOS/Windows/Linux)

If [draw.io Desktop](https://github.com/jgraph/drawio-desktop/releases) is installed:

```bash
drawio -x -f png -o context.png context.drawio
drawio -x -f png -o architecture.png architecture.drawio
drawio -x -f png -o event_loop.png event_loop.drawio
```

Or export all at once:

```bash
drawio -x -f png -o . .
```

### Option 2: draw.io Web

1. Open [app.diagrams.net](https://app.diagrams.net)
2. File → Open from → Device → select each `.drawio` file
3. File → Export as → PNG
4. Save to this `diagrams/` folder

### Files

| Source | Output | Used in xwin.md | Contents |
|--------|--------|-----------------|----------|
| context.drawio | context.png | Overview | Application ↔ xwin ↔ backends / vneevents |
| architecture.drawio | architecture.png | Architecture | Factory → Manager → Window / Descriptor / handles |
| event_loop.drawio | event_loop.png | Event bridge | processEvents → EventManager → frame → Input::nextFrame |
