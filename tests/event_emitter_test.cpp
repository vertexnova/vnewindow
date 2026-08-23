/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ----------------------------------------------------------------------
 */

/* Covers the single event path: one bridge call in, one vne::events event out, plus the
 * Input mirror. Before this refactor neither event path had any test coverage at all.
 */

#include <gtest/gtest.h>

#include "vertexnova/xwin/event_emitter.h"
#include "vertexnova/xwin/window.h"
#include "vertexnova/xwin/window_factory.h"

#include <vertexnova/events/events.h>

#include <memory>
#include <string>
#include <vector>

namespace vne::xwin {
namespace {

namespace ev = vne::events;

/**
 * @brief Minimal IWindow the bridge can take an id from.
 *
 * Deliberately not a real backend: the bridge only reads getId(), so the test stays independent
 * of which platform happens to be compiled in.
 */
class StubWindow final : public IWindow {
   public:
    void initialize(const WindowDescriptor&) override {}
    void pollEvents() override {}
    void swapBuffers() override {}
    void setTitle(const std::string&) override {}
    void setWindowMode(WindowMode) override {}
    [[nodiscard]] WindowMode getWindowMode() const noexcept override { return WindowMode::eWindowed; }
    void setFullscreen(bool) override {}
    [[nodiscard]] bool isFullscreen() const noexcept override { return false; }
    void setPosition(int, int) override {}
    [[nodiscard]] WindowPosition getPosition() const override { return {}; }
    void resize(uint32_t, uint32_t) override {}
    void close() override {}
    [[nodiscard]] bool isOpen() const noexcept override { return true; }
    [[nodiscard]] vne::events::WindowId getId() const noexcept override { return id_; }
    [[nodiscard]] NativeWindowHandle getNativeHandle() const noexcept override { return {}; }
    [[nodiscard]] WindowAPI getWindowAPI() const noexcept override { return WindowAPI::eNullWindow; }
    [[nodiscard]] int getWidth() const noexcept override { return 0; }
    [[nodiscard]] int getHeight() const noexcept override { return 0; }

   private:
    const vne::events::WindowId id_ = IWindow::nextId();
};

/** Captures every dispatched event so a test can assert on type, payload and window id. */
class CaptureListener final : public ev::EventListener {
   public:
    void onEvent(const ev::Event& event) override {
        types.push_back(event.type());
        window_ids.push_back(event.windowId());
        descriptions.push_back(event.toString());
    }

    std::vector<ev::EventType> types;
    std::vector<ev::WindowId> window_ids;
    std::vector<std::string> descriptions;
};

/** Every type the bridge can emit, so a listener sees the full output of one call. */
constexpr ev::EventType kAllBridgeTypes[] = {
    ev::EventType::eKeyPressed,           ev::EventType::eKeyReleased,
    ev::EventType::eKeyRepeat,            ev::EventType::eTextInput,
    ev::EventType::eMouseButtonPressed,   ev::EventType::eMouseButtonReleased,
    ev::EventType::eMouseMoved,           ev::EventType::eMouseScrolled,
    ev::EventType::eTouchPress,           ev::EventType::eTouchRelease,
    ev::EventType::eTouchMove,            ev::EventType::eWindowResize,
    ev::EventType::eWindowClose,          ev::EventType::eWindowFocus,
    ev::EventType::eWindowMinimize,       ev::EventType::eWindowRestore,
    ev::EventType::eWindowMove,           ev::EventType::eWindowDpiChanged,
    ev::EventType::eWindowSafeAreaChanged, ev::EventType::eApplicationPause,
    ev::EventType::eApplicationResume,    ev::EventType::eApplicationLowMemory,
};

class EventEmitterTest : public ::testing::Test {
   protected:
    void SetUp() override {
        window_ = std::make_unique<StubWindow>();
        descriptor_.enable_events = true;
        descriptor_.enable_input = true;
        events_ = std::make_unique<EventEmitter>(window_.get(), descriptor_);

        listener_ = std::make_shared<CaptureListener>();
        auto& manager = ev::EventManager::instance();
        manager.clearPendingEvents();
        for (const ev::EventType type : kAllBridgeTypes) {
            manager.registerListener(type, listener_);
        }
    }

    void TearDown() override {
        auto& manager = ev::EventManager::instance();
        for (const ev::EventType type : kAllBridgeTypes) {
            manager.unregisterListener(type, listener_.get());
        }
        manager.clearPendingEvents();
    }

    /** Drains the queue so the listener has seen everything emitted so far. */
    void drain() { ev::EventManager::instance().processEvents(); }

    [[nodiscard]] ev::WindowId windowId() const { return window_->getId(); }

    std::unique_ptr<StubWindow> window_;
    WindowDescriptor descriptor_{};
    std::unique_ptr<EventEmitter> events_;
    std::shared_ptr<CaptureListener> listener_;
};

// ---------------------------------------------------------------------------
// Window identity
// ---------------------------------------------------------------------------

TEST_F(EventEmitterTest, StampsTheOwningWindowIdOnEveryEvent) {
    events_->keyDown(ev::KeyCode::eA, 0, false);
    events_->mouseMove(1.0, 2.0, 0);
    events_->touch(1U, 3.0, 4.0, TouchPhase::eDown, 0);
    events_->windowResize(800U, 600U);
    drain();

    ASSERT_EQ(listener_->window_ids.size(), 4U);
    for (const ev::WindowId id : listener_->window_ids) {
        EXPECT_EQ(id, windowId());
        EXPECT_NE(id, ev::kInvalidWindowId);
    }
}

// ---------------------------------------------------------------------------
// Descriptor gating — the only place enable_events / enable_input are consulted
// ---------------------------------------------------------------------------

TEST_F(EventEmitterTest, EnableEventsFalseSuppressesEventsButKeepsInputMirror) {
    descriptor_.enable_events = false;

    events_->keyDown(ev::KeyCode::eB, 0, false);
    drain();

    EXPECT_TRUE(listener_->types.empty());
    EXPECT_TRUE(ev::Input::isKeyPressed(static_cast<int>(ev::KeyCode::eB)));
    events_->keyUp(ev::KeyCode::eB, 0);
}

TEST_F(EventEmitterTest, EnableInputFalseSuppressesMirrorButKeepsEvents) {
    descriptor_.enable_input = false;

    events_->keyDown(ev::KeyCode::eC, 0, false);
    drain();

    ASSERT_EQ(listener_->types.size(), 1U);
    EXPECT_EQ(listener_->types[0], ev::EventType::eKeyPressed);
    EXPECT_FALSE(ev::Input::isKeyPressed(static_cast<int>(ev::KeyCode::eC)));
}

// ---------------------------------------------------------------------------
// Keyboard and text
// ---------------------------------------------------------------------------

TEST_F(EventEmitterTest, KeyDownRepeatEmitsRepeatNotPressed) {
    events_->keyDown(ev::KeyCode::eD, 0, false);
    events_->keyDown(ev::KeyCode::eD, 0, true);
    events_->keyUp(ev::KeyCode::eD, 0);
    drain();

    ASSERT_EQ(listener_->types.size(), 3U);
    EXPECT_EQ(listener_->types[0], ev::EventType::eKeyPressed);
    EXPECT_EQ(listener_->types[1], ev::EventType::eKeyRepeat);
    EXPECT_EQ(listener_->types[2], ev::EventType::eKeyReleased);
}

TEST_F(EventEmitterTest, UnknownKeyIsDropped) {
    events_->keyDown(ev::KeyCode::eUnknown, 0, false);
    events_->keyUp(ev::KeyCode::eUnknown, 0);
    drain();

    EXPECT_TRUE(listener_->types.empty());
}

TEST_F(EventEmitterTest, TextInputCarriesUtf8AndDropsEmpty) {
    events_->textInput(nullptr);
    events_->textInput("");
    drain();
    EXPECT_TRUE(listener_->types.empty());

    // Multi-byte text must survive intact; the old KeyCode path could not represent it.
    events_->textInput("\xE3\x81\x82");
    drain();
    ASSERT_EQ(listener_->types.size(), 1U);
    EXPECT_EQ(listener_->types[0], ev::EventType::eTextInput);
    EXPECT_NE(listener_->descriptions[0].find("\xE3\x81\x82"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Pointer
// ---------------------------------------------------------------------------

TEST_F(EventEmitterTest, UnmappedMouseButtonIsDropped) {
    events_->mouseButton(ev::MouseButton::eUnknown, true, 1.0, 2.0, 0);
    drain();

    EXPECT_TRUE(listener_->types.empty());
}

TEST_F(EventEmitterTest, MouseButtonPressAndReleaseMirrorIntoInput) {
    events_->mouseButton(ev::MouseButton::eLeft, true, 5.0, 6.0, 0);
    drain();
    EXPECT_TRUE(ev::Input::isMouseButtonPressed(static_cast<int>(ev::MouseButton::eLeft)));

    events_->mouseButton(ev::MouseButton::eLeft, false, 5.0, 6.0, 0);
    drain();
    EXPECT_FALSE(ev::Input::isMouseButtonPressed(static_cast<int>(ev::MouseButton::eLeft)));

    ASSERT_EQ(listener_->types.size(), 2U);
    EXPECT_EQ(listener_->types[0], ev::EventType::eMouseButtonPressed);
    EXPECT_EQ(listener_->types[1], ev::EventType::eMouseButtonReleased);
}

TEST_F(EventEmitterTest, ScrollCarriesCursorPosition) {
    // Consumers previously had to recover this from the Input:: global to route scroll.
    events_->mouseScroll(0.0F, 1.0F, 320.0, 240.0, ev::ModifierKey::eModCtrl);
    drain();

    ASSERT_EQ(listener_->types.size(), 1U);
    EXPECT_EQ(listener_->types[0], ev::EventType::eMouseScrolled);
    EXPECT_NE(listener_->descriptions[0].find("at (320, 240)"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Touch
// ---------------------------------------------------------------------------

TEST_F(EventEmitterTest, TouchPhasesMapToDistinctEventTypes) {
    events_->touch(7U, 1.0, 2.0, TouchPhase::eDown, 0);
    events_->touch(7U, 3.0, 4.0, TouchPhase::eMove, 0);
    events_->touch(7U, 5.0, 6.0, TouchPhase::eUp, 0);
    drain();

    ASSERT_EQ(listener_->types.size(), 3U);
    EXPECT_EQ(listener_->types[0], ev::EventType::eTouchPress);
    EXPECT_EQ(listener_->types[1], ev::EventType::eTouchMove);
    EXPECT_EQ(listener_->types[2], ev::EventType::eTouchRelease);
}

TEST_F(EventEmitterTest, TouchIsSuppressedWhenEventsDisabled) {
    descriptor_.enable_events = false;

    events_->touch(1U, 1.0, 2.0, TouchPhase::eDown, 0);
    drain();

    EXPECT_TRUE(listener_->types.empty());
}

// ---------------------------------------------------------------------------
// Window state
// ---------------------------------------------------------------------------

TEST_F(EventEmitterTest, ResizeMirrorsIntoInputWindowSize) {
    events_->windowResize(1024U, 768U);
    drain();

    ASSERT_EQ(listener_->types.size(), 1U);
    EXPECT_EQ(listener_->types[0], ev::EventType::eWindowResize);
    const auto [w, h] = ev::Input::windowSize();
    EXPECT_EQ(w, 1024);
    EXPECT_EQ(h, 768);
}

TEST_F(EventEmitterTest, EmitsEveryWindowStateTransition) {
    // Before the refactor minimize/restore/move existed only on X11's dead callback path,
    // and DPI / safe-area had no representation at all.
    events_->windowClose();
    events_->windowFocus(true);
    events_->windowMinimize();
    events_->windowRestore();
    events_->windowMove(10, 20);
    events_->windowDpiChanged(2.0F);
    events_->windowSafeAreaChanged(59.0F, 0.0F, 34.0F, 0.0F);
    drain();

    const std::vector<ev::EventType> expected = {
        ev::EventType::eWindowClose,      ev::EventType::eWindowFocus,
        ev::EventType::eWindowMinimize,   ev::EventType::eWindowRestore,
        ev::EventType::eWindowMove,       ev::EventType::eWindowDpiChanged,
        ev::EventType::eWindowSafeAreaChanged,
    };
    EXPECT_EQ(listener_->types, expected);
}

// ---------------------------------------------------------------------------
// Application lifecycle
// ---------------------------------------------------------------------------

TEST_F(EventEmitterTest, ApplicationLifecycleIsProcessScopedAndCarriesNoWindowId) {
    auto manager = WindowFactory::createWindowManager(WindowAPI::eNullWindow);
    ASSERT_NE(manager, nullptr);
    manager->notifyApplicationLifecycle(ApplicationLifecycle::ePause);
    manager->notifyApplicationLifecycle(ApplicationLifecycle::eResume);
    manager->notifyApplicationLifecycle(ApplicationLifecycle::eLowMemory);
    drain();

    const std::vector<ev::EventType> expected = {
        ev::EventType::eApplicationPause,
        ev::EventType::eApplicationResume,
        ev::EventType::eApplicationLowMemory,
    };
    EXPECT_EQ(listener_->types, expected);
    for (const ev::WindowId id : listener_->window_ids) {
        EXPECT_EQ(id, ev::kInvalidWindowId);
    }
}

}  // namespace
}  // namespace vne::xwin
