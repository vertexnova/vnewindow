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

#include "vertexnova/xwin/time_step.h"

#include <algorithm>
#include <numeric>
#include <thread>

namespace vne::xwin {

TimeStep_C::TimeStep_C() noexcept
    : _last_frame_time(Clock_T::now())
    , _start_time(_last_frame_time)
    , _last_render_time(_last_frame_time) {
    std::fill(_delta_time_history, _delta_time_history + SMOOTHING_SAMPLES, 0.016);
}

TimeStep_C::TimeStep_C(double target_fps) noexcept
    : _last_frame_time(Clock_T::now())
    , _start_time(_last_frame_time)
    , _target_fps(target_fps)
    , _target_frame_time(target_fps > 0.0 ? 1.0 / target_fps : 0.0)
    , _last_render_time(_last_frame_time) {
    std::fill(_delta_time_history, _delta_time_history + SMOOTHING_SAMPLES, 0.016);
}

bool TimeStep_C::Update() noexcept {
    const auto now = Clock_T::now();

    if (_frame_rate_limit_enabled && _target_fps > 0.0) {
        const double since_last_render = std::chrono::duration<double>(now - _last_render_time).count();
        const double remain = _target_frame_time - since_last_render;
        if (remain > 0.0) {
            if (_sleep_pacing_enabled) {
                SleepRemainder(remain);
            }
            return false;
        }
    }

    double raw_delta = std::chrono::duration<double>(now - _last_frame_time).count();
    raw_delta = ClampDeltaTime(raw_delta);
    _delta_time = _smoothing_enabled ? CalculateSmoothedDeltaTime(raw_delta) : raw_delta;

    _last_frame_time = now;
    _last_render_time = now;
    _elapsed_time += _delta_time;
    _frame_count++;

    _min_delta_time = std::min(_min_delta_time, _delta_time);
    _max_delta_time = std::max(_max_delta_time, _delta_time);
    return true;
}

void TimeStep_C::Reset() noexcept {
    _last_frame_time = Clock_T::now();
    _start_time = _last_frame_time;
    _last_render_time = _last_frame_time;
    _delta_time = 0.016;
    _elapsed_time = 0.0;
    _frame_count = 0;
    _min_delta_time = std::numeric_limits<double>::max();
    _max_delta_time = 0.0;
    std::fill(_delta_time_history, _delta_time_history + SMOOTHING_SAMPLES, 0.016);
    _history_index = 0;
    _history_filled = false;
}

double TimeStep_C::GetElapsedTime() const noexcept {
    const auto now = Clock_T::now();
    return std::chrono::duration<double>(now - _start_time).count();
}

double TimeStep_C::GetFrameRate() const noexcept {
    return _delta_time > 0.0 ? 1.0 / _delta_time : 0.0;
}

double TimeStep_C::GetAverageFrameRate(uint32_t frame_count) const noexcept {
    if (frame_count == 0) {
        return 0.0;
    }
    double total_time = 0.0;
    uint32_t samples = std::min(frame_count, static_cast<uint32_t>(SMOOTHING_SAMPLES));
    for (uint32_t i = 0; i < samples; ++i) {
        total_time += _delta_time_history[i];
    }
    return samples > 0 ? samples / total_time : 0.0;
}

void TimeStep_C::SetTargetFrameRate(double target_fps) noexcept {
    _target_fps = target_fps;
    _target_frame_time = target_fps > 0.0 ? 1.0 / target_fps : 0.0;
}

bool TimeStep_C::ShouldRender() const noexcept {
    if (!_frame_rate_limit_enabled || _target_fps <= 0.0) {
        return true;
    }
    const auto now = Clock_T::now();
    const double time_since_last_render = std::chrono::duration<double>(now - _last_render_time).count();
    return time_since_last_render >= _target_frame_time;
}

double TimeStep_C::CalculateSmoothedDeltaTime(double raw_delta) noexcept {
    _delta_time_history[_history_index] = raw_delta;
    _history_index = (_history_index + 1) % SMOOTHING_SAMPLES;
    if (!_history_filled && _history_index == 0) {
        _history_filled = true;
    }
    size_t sample_count = _history_filled ? SMOOTHING_SAMPLES : _history_index;
    double sum = std::accumulate(_delta_time_history, _delta_time_history + sample_count, 0.0);
    return sum / static_cast<double>(sample_count);
}

double TimeStep_C::ClampDeltaTime(double delta) const noexcept {
    return std::clamp(delta, 0.0, _max_delta_time_limit);
}

void TimeStep_C::SleepRemainder(double seconds) const noexcept {
    using namespace std::chrono;
    if (seconds <= 0.0) {
        return;
    }
    const auto dur_ns = duration_cast<nanoseconds>(duration<double>(seconds));
    std::this_thread::sleep_for(dur_ns);
}

}  // namespace vne::xwin
