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

/** @file time_step.h Fixed timestep helper for polling loops. */

#include <chrono>
#include <cstdint>
#include <limits>

namespace vne::xwin {

class TimeStep {
   public:
    TimeStep() noexcept;
    explicit TimeStep(double target_fps) noexcept;
    ~TimeStep() noexcept = default;

    TimeStep(const TimeStep&) = delete;
    TimeStep& operator=(const TimeStep&) = delete;
    TimeStep(TimeStep&&) = delete;
    TimeStep& operator=(TimeStep&&) = delete;

    [[nodiscard]] bool update() noexcept;
    void reset() noexcept;

    [[nodiscard]] double getDeltaTime() const noexcept { return delta_time_; }
    [[nodiscard]] double getDeltaTimeMs() const noexcept { return delta_time_ * 1000.0; }
    [[nodiscard]] double getElapsedTime() const noexcept;
    [[nodiscard]] double getFrameRate() const noexcept;
    [[nodiscard]] double getAverageFrameRate(uint32_t frame_count = 60) const noexcept;

    void setTargetFrameRate(double target_fps) noexcept;
    [[nodiscard]] double getTargetFrameRate() const noexcept { return target_fps_; }
    [[nodiscard]] bool shouldRender() const noexcept;
    void setFrameRateLimitEnabled(bool enabled) noexcept { frame_rate_limit_enabled_ = enabled; }
    [[nodiscard]] bool isFrameRateLimitEnabled() const noexcept { return frame_rate_limit_enabled_; }

    [[nodiscard]] double getMinDeltaTime() const noexcept { return min_delta_time_; }
    [[nodiscard]] double getMaxDeltaTime() const noexcept { return max_delta_time_; }
    [[nodiscard]] uint64_t getFrameCount() const noexcept { return frame_count_; }

    void setMaxDeltaTime(double max_delta) noexcept { max_delta_time_limit_ = max_delta; }
    void setSmoothingEnabled(bool enabled) noexcept { smoothing_enabled_ = enabled; }
    [[nodiscard]] bool isSmoothingEnabled() const noexcept { return smoothing_enabled_; }
    void setSleepPacingEnabled(bool enabled) noexcept { sleep_pacing_enabled_ = enabled; }
    [[nodiscard]] bool isSleepPacingEnabled() const noexcept { return sleep_pacing_enabled_; }

   private:
    [[nodiscard]] double calculateSmoothedDeltaTime(double raw_delta) noexcept;
    [[nodiscard]] double clampDeltaTime(double delta) const noexcept;
    void sleepRemainder(double seconds) const noexcept;

    using Clock_T = std::chrono::steady_clock;
    Clock_T::time_point last_frame_time_;
    Clock_T::time_point start_time_;
    double delta_time_ = 0.016;
    double elapsed_time_ = 0.0;
    double target_fps_ = 60.0;
    double target_frame_time_ = 1.0 / 60.0;
    bool frame_rate_limit_enabled_ = true;
    Clock_T::time_point last_render_time_;
    double min_delta_time_ = std::numeric_limits<double>::max();
    double max_delta_time_ = 0.0;
    uint64_t frame_count_ = 0;
    double max_delta_time_limit_ = 0.1;
    bool smoothing_enabled_ = true;
    bool sleep_pacing_enabled_ = false;
    static constexpr size_t SMOOTHING_SAMPLES = 10;
    double delta_time_history_[SMOOTHING_SAMPLES];
    size_t history_index_ = 0;
    bool history_filled_ = false;
};

}  // namespace vne::xwin
