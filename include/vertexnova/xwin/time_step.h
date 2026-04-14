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

#include <chrono>
#include <cstdint>
#include <limits>

namespace vne::xwin {

class TimeStep_C {
   public:
    TimeStep_C() noexcept;
    explicit TimeStep_C(double target_fps) noexcept;
    ~TimeStep_C() noexcept = default;

    TimeStep_C(const TimeStep_C&) = delete;
    TimeStep_C& operator=(const TimeStep_C&) = delete;
    TimeStep_C(TimeStep_C&&) = delete;
    TimeStep_C& operator=(TimeStep_C&&) = delete;

    [[nodiscard]] bool Update() noexcept;
    void Reset() noexcept;

    [[nodiscard]] double GetDeltaTime() const noexcept { return _delta_time; }
    [[nodiscard]] double GetDeltaTimeMs() const noexcept { return _delta_time * 1000.0; }
    [[nodiscard]] double GetElapsedTime() const noexcept;
    [[nodiscard]] double GetFrameRate() const noexcept;
    [[nodiscard]] double GetAverageFrameRate(uint32_t frame_count = 60) const noexcept;

    void SetTargetFrameRate(double target_fps) noexcept;
    [[nodiscard]] double GetTargetFrameRate() const noexcept { return _target_fps; }
    [[nodiscard]] bool ShouldRender() const noexcept;
    void SetFrameRateLimitEnabled(bool enabled) noexcept { _frame_rate_limit_enabled = enabled; }
    [[nodiscard]] bool IsFrameRateLimitEnabled() const noexcept { return _frame_rate_limit_enabled; }

    [[nodiscard]] double GetMinDeltaTime() const noexcept { return _min_delta_time; }
    [[nodiscard]] double GetMaxDeltaTime() const noexcept { return _max_delta_time; }
    [[nodiscard]] uint64_t GetFrameCount() const noexcept { return _frame_count; }

    void SetMaxDeltaTime(double max_delta) noexcept { _max_delta_time_limit = max_delta; }
    void SetSmoothingEnabled(bool enabled) noexcept { _smoothing_enabled = enabled; }
    [[nodiscard]] bool IsSmoothingEnabled() const noexcept { return _smoothing_enabled; }
    void SetSleepPacingEnabled(bool enabled) noexcept { _sleep_pacing_enabled = enabled; }
    [[nodiscard]] bool IsSleepPacingEnabled() const noexcept { return _sleep_pacing_enabled; }

   private:
    [[nodiscard]] double CalculateSmoothedDeltaTime(double raw_delta) noexcept;
    [[nodiscard]] double ClampDeltaTime(double delta) const noexcept;
    void SleepRemainder(double seconds) const noexcept;

    using Clock_T = std::chrono::steady_clock;
    Clock_T::time_point _last_frame_time;
    Clock_T::time_point _start_time;
    double _delta_time = 0.016;
    double _elapsed_time = 0.0;
    double _target_fps = 60.0;
    double _target_frame_time = 1.0 / 60.0;
    bool _frame_rate_limit_enabled = true;
    Clock_T::time_point _last_render_time;
    double _min_delta_time = std::numeric_limits<double>::max();
    double _max_delta_time = 0.0;
    uint64_t _frame_count = 0;
    double _max_delta_time_limit = 0.1;
    bool _smoothing_enabled = true;
    bool _sleep_pacing_enabled = false;
    static constexpr size_t SMOOTHING_SAMPLES = 10;
    double _delta_time_history[SMOOTHING_SAMPLES];
    size_t _history_index = 0;
    bool _history_filled = false;
};

}  // namespace vne::xwin
