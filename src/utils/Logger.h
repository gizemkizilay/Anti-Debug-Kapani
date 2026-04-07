#pragma once

#ifndef LOGGER_H
#define LOGGER_H

#include <string>

// ──────────────────────────────────────────────────────────────────────────────
// Logger
//
// In DEBUG builds  : writes timestamped events to "debug_log.txt".
// In RELEASE builds: every call compiles away to a no-op (zero overhead).
//
// Usage:
//   LOG_EVENT("HandleDetector: CloseHandle trap fired");
// ──────────────────────────────────────────────────────────────────────────────

#ifdef DEBUG

namespace AntiDebug {

class Logger {
public:
    // Appends a single timestamped line to debug_log.txt.
    static void LogEvent(const std::string& msg);

    // Returns the path of the log file (always "debug_log.txt" relative to CWD).
    static const char* LogFilePath() noexcept { return "debug_log.txt"; }
};

} // namespace AntiDebug

// Active macro – forwarded to the real implementation.
#define LOG_EVENT(msg) ::AntiDebug::Logger::LogEvent(msg)

#else // RELEASE ────────────────────────────────────────────────────────────────

// In release builds the entire logger body disappears.
// We still expose a no-op LogEvent so call-sites compile without #ifdefs.
namespace AntiDebug {
    namespace Logger {
        [[maybe_unused]] inline void LogEvent(const std::string& /*msg*/) noexcept {}
    }
}

// Macro expands to nothing → zero overhead, zero binary footprint.
#define LOG_EVENT(msg) ((void)0)

#endif // DEBUG

#endif // LOGGER_H
