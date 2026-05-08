#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Common logging configuration for vne::xwin examples
 * ----------------------------------------------------------------------
 */

#include "vertexnova/logging/logging.h"

#include <cstdio>

// Create a logger category for examples
CREATE_VNE_LOGGER_CATEGORY("vnewindow.examples")

namespace vne::xwin::examples {

/**
 * @class LoggingGuard
 * @brief RAII guard for console logging configuration in examples.
 *
 * Initializes the logging system with console output in its constructor
 * and shuts it down in its destructor. Use at the start of main() in example programs.
 *
 * Usage:
 * @code
 * int main() {
 *     LoggingGuard logging_guard;
 *     // ... example code ...
 *     return 0;
 * }
 * @endcode
 */
class LoggingGuard {
   public:
    LoggingGuard() {
        // TODO(vnewindow): Temporary workaround for buffered IDE/run-wrapper output.
        // Investigate root cause in vnelogging/console sink flush behavior and/or
        // Qt Creator launch wrapper stream handling, then remove this override.
        // Example-only runtime behavior: disable stdio buffering so IDE wrappers
        // (e.g. Qt Creator launch helpers) show logs immediately.
        std::setvbuf(stdout, nullptr, _IONBF, 0);
        std::setvbuf(stderr, nullptr, _IONBF, 0);

        vne::log::LoggerConfig config{};
        config.name = vne::log::kDefaultLoggerName;
        config.sink = vne::log::LogSinkType::eConsole;
        config.console_pattern = "[%l] [%n] %v";
        config.log_level = vne::log::LogLevel::eInfo;
        // TODO(vnewindow): Keep at INFO until flush semantics are verified end-to-end.
        // Flush info logs immediately so interactive input/event output appears live.
        config.flush_level = vne::log::LogLevel::eInfo;
        config.async = false;

        vne::log::Logging::configureLogger(config);
    }

    ~LoggingGuard() { vne::log::Logging::shutdown(); }

    LoggingGuard(const LoggingGuard&) = delete;
    LoggingGuard& operator=(const LoggingGuard&) = delete;
};

}  // namespace vne::xwin::examples
