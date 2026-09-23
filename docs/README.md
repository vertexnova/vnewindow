# VneCrossWindow Documentation

Documentation for the VertexNova windowing library (`vne::xwin`).

## Architecture

- [Window System](vertexnova/xwin/xwin.md) — Design, components, platforms, and usage
- [Architecture](vertexnova/xwin/architecture/README.md) — Layered stack (vnerhi colors) and class diagram

## Testing

- [Testing strategy](TESTING.md) — Unit layers, CI, null/smoke vs desktop validation

## Generating API Documentation

Generate API documentation with Doxygen:

```bash
cmake -B build/shared -DENABLE_DOXYGEN=ON
cmake --build build/shared --target vnexwin_doc_doxygen
```

Documentation will be available at `build/shared/docs/html/index.html`.

Or from the project root:

```bash
./scripts/generate-docs.sh
```

## Requirements

- Doxygen 1.8.13 or higher
- Graphviz (for class diagrams)
