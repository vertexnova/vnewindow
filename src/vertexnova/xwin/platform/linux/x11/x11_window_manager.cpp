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

#include "x11_window_manager.h"

#include "x11_window.h"

#include "vertexnova/logging/logging.h"

#include <algorithm>
#include <csignal>
#include <csetjmp>
#include <chrono>
#include <mutex>
#include <thread>

namespace vne::xwin {

namespace {
CREATE_VNE_LOGGER_CATEGORY("vne.xwin.x11_window_manager");

std::mutex g_xclose_mutex;
sigjmp_buf g_xclose_display_jmp{};

void handleXCloseDisplaySignal(int /*signal*/) {
    siglongjmp(g_xclose_display_jmp, 1);
}

bool closeDisplaySafely(Display* display) {
    std::lock_guard<std::mutex> lock(g_xclose_mutex);

    struct sigaction crash_handler = {};
    crash_handler.sa_handler = handleXCloseDisplaySignal;
    sigemptyset(&crash_handler.sa_mask);
    crash_handler.sa_flags = 0;

    struct sigaction previous_sigsegv = {};
    struct sigaction previous_sigabrt = {};
    struct sigaction previous_sigbus = {};
    sigaction(SIGSEGV, &crash_handler, &previous_sigsegv);
    sigaction(SIGABRT, &crash_handler, &previous_sigabrt);
#ifdef SIGBUS
    sigaction(SIGBUS, &crash_handler, &previous_sigbus);
#endif

    bool close_succeeded = true;
    if (sigsetjmp(g_xclose_display_jmp, 1) == 0) {
        XCloseDisplay(display);
    } else {
        close_succeeded = false;
    }

    sigaction(SIGSEGV, &previous_sigsegv, nullptr);
    sigaction(SIGABRT, &previous_sigabrt, nullptr);
#ifdef SIGBUS
    sigaction(SIGBUS, &previous_sigbus, nullptr);
#endif
    return close_succeeded;
}
}  // namespace

X11WindowManager::X11WindowManager() = default;

X11WindowManager::~X11WindowManager() {
    shutdown();
}

bool X11WindowManager::initialize() {
    static std::once_flag s_x11_threads_init_once;
    std::call_once(s_x11_threads_init_once, []() { (void)XInitThreads(); });

    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        return false;
    }
    screen_ = DefaultScreen(display_);
    root_ = RootWindow(display_, screen_);
    xcb_connection_ = nullptr;
    initialized_ = true;
    return true;
}

void X11WindowManager::shutdown() {
    if (!initialized_ && display_ == nullptr) {
        return;
    }
    destroyAllWindows();
    if (display_) {
        XSync(display_, False);
        if (!closeDisplaySafely(display_)) {
            VNE_LOG_DEBUG << "X11WindowManager::shutdown: XCloseDisplay crashed; display handle detached defensively";
        }
        display_ = nullptr;
    }
    xcb_connection_ = nullptr;
    screen_ = 0;
    root_ = 0;
    initialized_ = false;
}

bool X11WindowManager::isInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> X11WindowManager::openWindow(const WindowDescriptor& descriptor) {
    if (!initialized_ || !display_) {
        return nullptr;
    }
    auto w = std::make_shared<X11Window>();
    w->setEventOwner(this);
    w->setDisplay(display_, screen_, root_, xcb_connection_);
    w->initialize(descriptor);
    if (!w->isOpen()) {
        return nullptr;
    }
    windows_.push_back(w);
    if (!primary_) {
        primary_ = w;
    }
    focused_ = w;
    return w;
}

std::shared_ptr<IWindow> X11WindowManager::openWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor descriptor(title, width, height);
    return openWindow(descriptor);
}

void X11WindowManager::removeWindow(std::shared_ptr<IWindow> window) {
    if (!window) {
        return;
    }
    window->close();
    auto it = std::find(windows_.begin(), windows_.end(), window);
    if (it != windows_.end()) {
        windows_.erase(it);
    }
    if (primary_ == window) {
        primary_ = windows_.empty() ? nullptr : windows_.front();
    }
    if (focused_ == window) {
        focused_ = primary_;
    }
}

void X11WindowManager::destroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->close();
            if (auto x11_window = std::dynamic_pointer_cast<X11Window>(w)) {
                x11_window->setDisplay(nullptr, 0, 0, nullptr);
            }
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t X11WindowManager::getWindowCount() const noexcept {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> X11WindowManager::getWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> X11WindowManager::getPrimaryWindow() const noexcept {
    return primary_;
}

std::shared_ptr<IWindow> X11WindowManager::getFocusedWindow() const noexcept {
    return focused_;
}

void X11WindowManager::setPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void X11WindowManager::focusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void X11WindowManager::processEvents() {
    for (auto& w : windows_) {
        if (w) {
            w->pollEvents();
        }
    }
}

bool X11WindowManager::shouldClose() const noexcept {
    for (const auto& w : windows_) {
        if (w && !w->isOpen()) {
            return true;
        }
    }
    return false;
}

bool X11WindowManager::shouldCloseAll() const noexcept {
    if (windows_.empty()) {
        return false;
    }
    for (const auto& w : windows_) {
        if (w && w->isOpen()) {
            return false;
        }
    }
    return true;
}

WindowAPI X11WindowManager::getWindowAPI() const noexcept {
    return WindowAPI::eX11Window;
}

std::string X11WindowManager::getPlatformInfo() const {
    return "Linux X11 (Xlib)";
}

bool X11WindowManager::isFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "decorated" || feature == "clipboard" || feature == "icon";
}

std::string X11WindowManager::getProperties() const {
    return properties_;
}

void X11WindowManager::setProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t X11WindowManager::getCurrentTime() const noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void X11WindowManager::sleep(uint32_t milliseconds) const noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double X11WindowManager::getPlatformTime() const noexcept {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
