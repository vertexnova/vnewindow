#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   September 2026
 *
 * Autodoc:   yes
 * ---------------------------------------------------------------------- */

/**
 * @file wasm_canvas_transfer.h
 * @brief One authoritative answer to "has the page canvas been handed to a worker?".
 *
 * After @c canvas.transferControlToOffscreen() the HTML canvas becomes a placeholder. Reading or
 * writing its @c width / @c height throws @c InvalidStateError on Safari, and writing it through
 * @c emscripten_set_canvas_element_size re-enters the canvas-size path until the JS stack
 * overflows. Every writer of canvas geometry therefore has to know whether the transfer has
 * happened.
 *
 * That fact is *published by whoever performs the transfer*, never sniffed, because every
 * feature probe fails in at least one place that matters:
 *
 * - @c document does not exist in the worker realm, which is precisely the realm that renders.
 * - @c GL.offscreenCanvases only exists when the GL library is linked; WebGPU builds do not.
 * - A URL or sample-name heuristic is wrong for every sample it was not written for.
 *
 * The flag lives in wasm linear memory, which @c -pthread builds share across every thread, so
 * the host thread and the render worker read the same value. @c markCanvasControlTransferred()
 * additionally mirrors it onto the main thread's @c Module so the HTML shell agrees: each
 * pthread worker gets its own JS @c Module object, so a JS-side flag alone would be invisible to
 * exactly the thread that needs it.
 */

#include "xwin_export.h"

namespace vne::xwin {

/**
 * @brief Record that the page canvas has been transferred to a worker.
 *
 * Call once, on the main thread, immediately before creating the thread that receives the
 * canvas (the @c emscripten_pthread_attr_settransferredcanvases call). Idempotent.
 *
 * No-op off Emscripten.
 */
VNE_XWIN_API void markCanvasControlTransferred() noexcept;

/**
 * @brief Whether the page canvas has been transferred to a worker.
 *
 * Safe from any thread. Callers that write canvas CSS or drawing-buffer size must check this
 * first and skip the write when it returns true.
 *
 * @return @c false off Emscripten, and before @c markCanvasControlTransferred() is called.
 */
[[nodiscard]] VNE_XWIN_API bool canvasControlTransferred() noexcept;

}  // namespace vne::xwin
