#include "Logger.h"

// Logger is only compiled in DEBUG builds. The Release path is an inline no-op
// defined entirely in the header, so this translation unit contributes nothing
// in that configuration.

#ifdef DEBUG

#include <windows.h>   // GetLocalTime(), SYSTEMTIME
#include <fstream>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <string>

namespace AntiDebug {

// ─────────────────────────────────────────────────────────────────────────────
// Internal helpers (anonymous namespace – not visible outside this TU)
// ─────────────────────────────────────────────────────────────────────────────
namespace {

// Guards concurrent writes from multiple threads.
std::mutex g_logMutex;

// Returns the current local time as "YYYY-MM-DD HH:MM:SS".
std::string CurrentTimestamp() {
    SYSTEMTIME st = {};
    GetLocalTime(&st);

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << st.wYear   << '-'
        << std::setw(2) << st.wMonth  << '-'
        << std::setw(2) << st.wDay    << ' '
        << std::setw(2) << st.wHour   << ':'
        << std::setw(2) << st.wMinute << ':'
        << std::setw(2) << st.wSecond;
    return oss.str();
}

} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
// Logger::LogEvent
//
// Thread-safe append to "debug_log.txt" (relative to the process working dir).
// Format:  [YYYY-MM-DD HH:MM:SS] <msg>\n
// ─────────────────────────────────────────────────────────────────────────────
void Logger::LogEvent(const std::string& msg) {
    std::lock_guard<std::mutex> lock(g_logMutex);

    std::ofstream ofs(LogFilePath(), std::ios::app);
    if (!ofs.is_open()) return; // Silently fail – we're in a stealth tool

    ofs << '[' << CurrentTimestamp() << "] " << msg << '\n';
}

} // namespace AntiDebug

#endif // DEBUG
