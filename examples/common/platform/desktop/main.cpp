/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Desktop (macOS / Windows / Linux) entry point for vnewindow examples.
 *
 * This file is injected automatically by vne_add_example() for non-iOS
 * platforms. Example source files must NOT contain a main() function.
 * ----------------------------------------------------------------------
 */

#include "common/example_runner.h"

// Declared in each example's example.cpp
std::unique_ptr<vne::xwin::examples::ExampleBase> createExample();

int main() {
    vne::xwin::examples::ExampleRunner runner{createExample()};
    if (!runner.initialize()) {
        return 1;
    }
    return runner.run();
}
