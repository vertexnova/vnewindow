/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   August 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include "vertexnova/xwin/window_manager.h"

#include "event_emitter.h"

namespace vne::xwin {

void IWindowManager::notifyApplicationLifecycle(ApplicationLifecycle transition) {
    EventEmitter::applicationLifecycle(transition);
}

}  // namespace vne::xwin
