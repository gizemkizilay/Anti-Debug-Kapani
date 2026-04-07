#include "TimingDetector.h"

#include <cstdint>
#include <windows.h>

// Platform-independent RDTSC / memory fence wrappers
#if defined(_MSC_VER)
#  include <intrin.h>    // __rdtsc(), _mm_lfence()
#  define RDTSC_FENCE() _mm_lfence()
#  define RDTSC()       __rdtsc()
#elif defined(__GNUC__) || defined(__clang__)
#  include <x86intrin.h>  // __rdtsc(), _mm_lfence()
#  define RDTSC_FENCE() _mm_lfence()
#  define RDTSC()       __rdtsc()
#else
#  error "Unsupported compiler"
#endif

namespace AntiDebug {

// ─────────────────────────────────────────────────────────────────────────────
// Public entry point
// ─────────────────────────────────────────────────────────────────────────────
bool TimingDetector::IsTimingAnomaly() const {
    // Either check firing is sufficient evidence of debugging.
    if (CheckRdtscDelta())  return true;
    if (CheckQpcLatency())   return true;
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// RDTSC delta check
//
// __rdtsc() reads the CPU's Time Stamp Counter.  We serialize the instruction
// stream with _mm_lfence() before each read so out-of-order execution doesn't
// collapse the two readings into one.
//
// Under normal execution the delta between two consecutive reads is tiny
// (typically < 100 cycles).  A debugger that is single-stepping, handling
// breakpoint exceptions, or running heavy instrumentation will push this delta
// well above kRdtscThresholdCycles.
// ─────────────────────────────────────────────────────────────────────────────
bool TimingDetector::CheckRdtscDelta() const {
    RDTSC_FENCE();
    const uint64_t t1 = RDTSC();
    RDTSC_FENCE();

    // A lightweight operation that a debugger must still process.
    // We use a volatile store so the compiler cannot eliminate the code.
    volatile uint64_t sink = 0;
    for (int i = 0; i < 10; ++i) {
        sink += static_cast<uint64_t>(i);
    }
    (void)sink;

    RDTSC_FENCE();
    const uint64_t t2 = RDTSC();
    RDTSC_FENCE();

    const uint64_t delta = t2 - t1;
    return (delta > kRdtscThresholdCycles);
}

// ─────────────────────────────────────────────────────────────────────────────
// QPC latency check
//
// QueryPerformanceCounter provides a high-resolution wall-clock timestamp.
// We measure the time elapsed during a trivial loop.  In a non-debugged run
// this takes nanoseconds; under a debugger the overhead is measurable in
// hundreds of microseconds or more.
// ─────────────────────────────────────────────────────────────────────────────
bool TimingDetector::CheckQpcLatency() const {
    LARGE_INTEGER freq = {};
    if (!QueryPerformanceFrequency(&freq) || freq.QuadPart == 0) {
        return false; // QPC not available – cannot determine
    }

    LARGE_INTEGER start = {};
    QueryPerformanceCounter(&start);

    // Same lightweight volatile loop as above
    volatile uint64_t sink = 0;
    for (int i = 0; i < 100; ++i) {
        sink += static_cast<uint64_t>(i);
    }
    (void)sink;

    LARGE_INTEGER end = {};
    QueryPerformanceCounter(&end);

    // Convert to milliseconds
    const double elapsedMs =
        static_cast<double>(end.QuadPart - start.QuadPart) * 1000.0 /
        static_cast<double>(freq.QuadPart);

    return (elapsedMs > kQpcThresholdMs);
}

} // namespace AntiDebug
