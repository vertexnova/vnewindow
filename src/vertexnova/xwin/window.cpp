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
#include "vertexnova/xwin/window_factory.h"

#include <atomic>
#include <exception>
#include <limits>
#include <memory>

namespace vne::xwin {
namespace {

class ManagedWindow final : public IWindow {
   public:
    ManagedWindow(std::shared_ptr<IWindowManager> manager, std::shared_ptr<IWindow> window) noexcept
        : manager_(std::move(manager))
        , window_(std::move(window)) {}

    void initialize(const WindowDescriptor& descriptor) override { window_->initialize(descriptor); }
    void swapBuffers() override { window_->swapBuffers(); }
    void setTitle(const std::string& title) override { window_->setTitle(title); }
    void setWindowMode(WindowMode mode) override { window_->setWindowMode(mode); }
    [[nodiscard]] WindowMode getWindowMode() const noexcept override { return window_->getWindowMode(); }
    void setFullscreen(bool enabled) override { window_->setFullscreen(enabled); }
    [[nodiscard]] bool isFullscreen() const noexcept override { return window_->isFullscreen(); }
    void setPosition(int x, int y) override { window_->setPosition(x, y); }
    [[nodiscard]] WindowPosition getPosition() const override { return window_->getPosition(); }
    void resize(uint32_t width, uint32_t height) override { window_->resize(width, height); }
    void close() override { window_->close(); }
    [[nodiscard]] bool isOpen() const noexcept override { return window_->isOpen(); }
    [[nodiscard]] vne::events::WindowId getId() const noexcept override { return window_->getId(); }
    [[nodiscard]] NativeWindowHandle getNativeHandle() const noexcept override { return window_->getNativeHandle(); }
    [[nodiscard]] WindowAPI getWindowAPI() const noexcept override { return window_->getWindowAPI(); }
    [[nodiscard]] int getWidth() const noexcept override { return window_->getWidth(); }
    [[nodiscard]] int getHeight() const noexcept override { return window_->getHeight(); }
    void setWindowLimits(const WindowLimits& limits) override { window_->setWindowLimits(limits); }
    void setCursor(WindowCursor cursor) override { window_->setCursor(cursor); }
    void setMonitor(uint32_t monitor_index) override { window_->setMonitor(monitor_index); }
    [[nodiscard]] uint32_t getMonitor() const noexcept override { return window_->getMonitor(); }
    [[nodiscard]] float getDpiScale() const noexcept override { return window_->getDpiScale(); }
    void setTransparent(bool enabled) override { window_->setTransparent(enabled); }
    [[nodiscard]] bool isTransparent() const noexcept override { return window_->isTransparent(); }
    void setVSync(bool enabled) override { window_->setVSync(enabled); }
    [[nodiscard]] bool isVSyncEnabled() const noexcept override { return window_->isVSyncEnabled(); }
    void minimize() override { window_->minimize(); }
    void maximize() override { window_->maximize(); }
    void restore() override { window_->restore(); }
    [[nodiscard]] std::string getClipboardText() const override { return window_->getClipboardText(); }
    void setClipboardText(const std::string& text) override { window_->setClipboardText(text); }
    void setWindowIcon(std::span<const uint8_t> rgba_pixels, uint32_t width, uint32_t height) override {
        window_->setWindowIcon(rgba_pixels, width, height);
    }

   private:
    std::shared_ptr<IWindowManager> manager_;
    std::shared_ptr<IWindow> window_;
};

}  // namespace

vne::events::WindowId IWindow::nextId() noexcept {
    // Starts at 1 so 0 stays vne::events::kInvalidWindowId. Never reused: a window recreated after
    // a platform teardown is a different window, and stale ids must not silently resolve to it.
    static std::atomic<vne::events::WindowId> s_next_window_id{1};
    constexpr vne::events::WindowId kMaxId = std::numeric_limits<vne::events::WindowId>::max();

    for (;;) {
        vne::events::WindowId id = s_next_window_id.load(std::memory_order_relaxed);
        // At max, the next increment would wrap to kInvalidWindowId (0) and eventually reuse IDs.
        if (id == kMaxId || id == vne::events::kInvalidWindowId) {
            std::terminate();
        }
        if (s_next_window_id.compare_exchange_weak(id, id + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {
            return id;
        }
    }
}

std::unique_ptr<IWindow> IWindow::create(const WindowDescriptor& descriptor) {
    auto manager = WindowFactory::createWindowManager();
    if (!manager || !manager->initialize()) {
        return nullptr;
    }
    std::shared_ptr<IWindow> window = manager->openWindow(descriptor);
    if (!window) {
        manager->shutdown();
        return nullptr;
    }
    return std::make_unique<ManagedWindow>(std::move(manager), std::move(window));
}

}  // namespace vne::xwin
