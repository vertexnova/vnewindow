/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   September 2026
 *
 * Autodoc:   yes
 * ---------------------------------------------------------------------- */

#include "vertexnova/xwin/wasm_canvas_transfer.h"

#include <atomic>

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

namespace vne::xwin {
namespace {

/// Lives in wasm linear memory, which -pthread builds share, so every thread sees one value.
std::atomic<bool> g_canvas_control_transferred{false};

}  // namespace

void markCanvasControlTransferred() noexcept {
    g_canvas_control_transferred.store(true, std::memory_order_release);
#if defined(__EMSCRIPTEN__)
    // Mirror to the main thread's Module so the HTML shell sees the same fact. The shell reads
    // this before touching canvas.width / canvas.height.
    //
    // Loose `!=` is deliberate. clang-format lexes JS `!==` as C++ `!=` followed by `=` and
    // reflows them apart into the invalid `!= =`, silently breaking the block at runtime. Against
    // `typeof`, which always yields a string, `!=` and `!==` are equivalent, so this sidesteps the
    // formatter instead of relying on a guard comment that is easy to disturb.
    EM_ASM({ if (typeof Module != 'undefined') { Module.vneCanvasTransferred = true; } });
#endif
}

bool canvasControlTransferred() noexcept {
    return g_canvas_control_transferred.load(std::memory_order_acquire);
}

}  // namespace vne::xwin
