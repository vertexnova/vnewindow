/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include "vertexnova/xwin/window.h"

#include "platform/null/null_window.h"

#include <memory>

namespace vne::xwin {

std::unique_ptr<Window_I> Window_I::Create(const WindowDescriptor_C& descriptor) {
    auto w = std::make_unique<NullWindow_C>();
    w->Initialize(descriptor);
    return w;
}

}  // namespace vne::xwin
