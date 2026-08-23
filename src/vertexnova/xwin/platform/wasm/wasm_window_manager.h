#pragma once
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

#include "vertexnova/xwin/window_manager.h"

#include <memory>
#include <string>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

namespace vne::xwin {

class WasmWindow;

class WasmWindowManager final : public IWindowManager {
   public:
    WasmWindowManager();
    ~WasmWindowManager() override;

    [[nodiscard]] bool initialize() override;
    void shutdown() override;
    [[nodiscard]] bool isInitialized() const noexcept override;

    [[nodiscard]] std::shared_ptr<IWindow> openWindow(const WindowDescriptor& descriptor) override;
    [[nodiscard]] std::shared_ptr<IWindow> openWindow(const std::string& title,
                                                      uint32_t width,
                                                      uint32_t height) override;
    void removeWindow(std::shared_ptr<IWindow> window) override;
    void destroyAllWindows() override;

    [[nodiscard]] size_t getWindowCount() const noexcept override;
    [[nodiscard]] std::vector<std::shared_ptr<IWindow>> getWindows() const override;
    [[nodiscard]] std::shared_ptr<IWindow> getPrimaryWindow() const noexcept override;
    [[nodiscard]] std::shared_ptr<IWindow> getFocusedWindow() const noexcept override;
    void setPrimaryWindow(std::shared_ptr<IWindow> window) override;
    void focusWindow(std::shared_ptr<IWindow> window) override;

    void processEvents() override;
    /** Only when a host shell (window.VneShell) provides additional canvases; one canvas otherwise. */
    [[nodiscard]] bool supportsMultipleWindows() const noexcept override;
    [[nodiscard]] bool shouldClose() const noexcept override;
    [[nodiscard]] bool shouldCloseAll() const noexcept override;

    [[nodiscard]] WindowAPI getWindowAPI() const noexcept override;
    [[nodiscard]] std::string getPlatformInfo() const override;
    [[nodiscard]] bool isFeatureSupported(const std::string& feature) const override;
    [[nodiscard]] std::string getProperties() const override;
    void setProperties(const std::string& properties) override;

    [[nodiscard]] uint64_t getCurrentTime() const noexcept override;
    void sleep(uint32_t milliseconds) const noexcept override;
    [[nodiscard]] double getPlatformTime() const noexcept override;

    void focusWindowFromCanvas(WasmWindow* window);

   private:
    void registerGlobalCallbacks();
    void unregisterGlobalCallbacks();

#ifdef __EMSCRIPTEN__
    static EM_BOOL GlobalKeyDownCallback(int event_type, const EmscriptenKeyboardEvent* ev, void* user_data);
    static EM_BOOL GlobalKeyUpCallback(int event_type, const EmscriptenKeyboardEvent* ev, void* user_data);
    static EM_BOOL GlobalVisibilityCallback(int event_type, const EmscriptenVisibilityChangeEvent* ev, void* user_data);
    static EM_BOOL GlobalResizeCallback(int event_type, const EmscriptenUiEvent* event, void* user_data);
#endif

    std::vector<std::shared_ptr<IWindow>> windows_;
    std::shared_ptr<IWindow> primary_;
    std::shared_ptr<IWindow> focused_;
    bool initialized_ = false;
    bool global_callbacks_registered_ = false;
    std::string properties_;
};

}  // namespace vne::xwin
