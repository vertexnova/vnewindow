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

TimeStep::TimeStep() noexcept
    : last_frame_time_(Clock_T::now())
    , start_time_(last_frame_time_)
    , last_render_time_(last_frame_time_) {
    std::fill(delta_time_history_, delta_time_history_ + SMOOTHING_SAMPLES, 0.016);
}

TimeStep::TimeStep(double target_fps) noexcept
    : last_frame_time_(Clock_T::now())
    , start_time_(last_frame_time_)
    , target_fps_(target_fps)
    , target_frame_time_(target_fps > 0.0 ? 1.0 / target_fps : 0.0)
    , last_render_time_(last_frame_time_) {
    std::fill(delta_time_history_, delta_time_history_ + SMOOTHING_SAMPLES, 0.016);
}

bool TimeStep::Update() noexcept {
    const auto now = Clock_T::now();

    if (frame_rate_limit_enabled_ && target_fps_ > 0.0) {
        const double since_last_render = std::chrono::duration<double>(now - last_render_time_).count();
        const double remain = target_frame_time_ - since_last_render;
        if (remain > 0.0) {
            if (sleep_pacing_enabled_) {
                SleepRemainder(remain);
            }
            return false;
        }
    }

    double raw_delta = std::chrono::duration<double>(now - last_frame_time_).count();
    raw_delta = ClampDeltaTime(raw_delta);
    delta_time_ = smoothing_enabled_ ? CalculateSmoothedDeltaTime(raw_delta) : raw_delta;

    last_frame_time_ = now;
    last_render_time_ = now;
    elapsed_time_ += delta_time_;
    frame_count_++;

    min_delta_time_ = std::min(min_delta_time_, delta_time_);
    max_delta_time_ = std::max(max_delta_time_, delta_time_);
    return true;
}

void TimeStep::Reset() noexcept {
    last_frame_time_ = Clock_T::now();
    start_time_ = last_frame_time_;
    last_render_time_ = last_frame_time_;
    delta_time_ = 0.016;
    elapsed_time_ = 0.0;
    frame_count_ = 0;
    min_delta_time_ = std::numeric_limits<double>::max();
    max_delta_time_ = 0.0;
    std::fill(delta_time_history_, delta_time_history_ + SMOOTHING_SAMPLES, 0.016);
    history_index_ = 0;
    history_filled_ = false;
}

double TimeStep::GetElapsedTime() const noexcept {
    const auto now = Clock_T::now();
    return std::chrono::duration<double>(now - start_time_).count();
}

double TimeStep::GetFrameRate() const noexcept {
    return delta_time_ > 0.0 ? 1.0 / delta_time_ : 0.0;
}

double TimeStep::GetAverageFrameRate(uint32_t frame_count) const noexcept {
    if (frame_count == 0) {
        return 0.0;
    }
    double total_time = 0.0;
    uint32_t samples = std::min(frame_count, static_cast<uint32_t>(SMOOTHING_SAMPLES));
    for (uint32_t i = 0; i < samples; ++i) {
        total_time += delta_time_history_[i];
    }
    return samples > 0 ? samples / total_time : 0.0;
}

void TimeStep::SetTargetFrameRate(double target_fps) noexcept {
    target_fps_ = target_fps;
    target_frame_time_ = target_fps > 0.0 ? 1.0 / target_fps : 0.0;
}

bool TimeStep::ShouldRender() const noexcept {
    if (!frame_rate_limit_enabled_ || target_fps_ <= 0.0) {
        return true;
    }
    const auto now = Clock_T::now();
    const double time_since_last_render = std::chrono::duration<double>(now - last_render_time_).count();
    return time_since_last_render >= target_frame_time_;
}

double TimeStep::CalculateSmoothedDeltaTime(double raw_delta) noexcept {
    delta_time_history_[history_index_] = raw_delta;
    history_index_ = (history_index_ + 1) % SMOOTHING_SAMPLES;
    if (!history_filled_ && history_index_ == 0) {
        history_filled_ = true;
    }
    size_t sample_count = history_filled_ ? SMOOTHING_SAMPLES : history_index_;
    double sum = std::accumulate(delta_time_history_, delta_time_history_ + sample_count, 0.0);
    return sum / static_cast<double>(sample_count);
}

double TimeStep::ClampDeltaTime(double delta) const noexcept {
    return std::clamp(delta, 0.0, max_delta_time_limit_);
}

void TimeStep::SleepRemainder(double seconds) const noexcept {
    using namespace std::chrono;
    if (seconds <= 0.0) {
        return;
    }
    const auto dur_ns = duration_cast<nanoseconds>(duration<double>(seconds));
    std::this_thread::sleep_for(dur_ns);
}

}  // namespace vne::xwin
