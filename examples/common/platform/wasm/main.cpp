/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   May 2026
 *
 * Autodoc:   yes
 *
 * WebAssembly / Emscripten entry point for vnewindow examples.
 *
 * Emscripten cannot use a blocking while-loop (it would starve the browser
 * event loop). Instead we register a tick callback via
 * emscripten_set_main_loop() which is driven by requestAnimationFrame.
 * ExampleRunner::tick() is called once per frame; when it returns false
 * we cancel the loop and call shutdown().
 * ----------------------------------------------------------------------
 */

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "common/example_runner.h"

#include <memory>

std::unique_ptr<vne::xwin::examples::ExampleBase> createExample();

namespace {

std::unique_ptr<vne::xwin::examples::ExampleRunner> g_runner;

void wasm_tick() {
    if (!g_runner) {
        return;
    }
    if (!g_runner->tick()) {
        g_runner->shutdown();
        g_runner.reset();
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
    }
}

}  // namespace

int main() {
    g_runner = std::make_unique<vne::xwin::examples::ExampleRunner>(createExample());
    if (!g_runner->initialize()) {
        g_runner.reset();
        return 1;
    }

#ifdef __EMSCRIPTEN__
    // fps = 0  → driven by requestAnimationFrame (matches display refresh rate)
    // simulate_infinite_loop = 1  → main() does not return; browser handles lifetime
    emscripten_set_main_loop(wasm_tick, 0, 1);
#else
    // Fallback for non-Emscripten builds that link this file (shouldn't happen
    // in practice; desktop uses platform/desktop/main.cpp instead).
    return g_runner->run();
#endif
    return 0;
}
