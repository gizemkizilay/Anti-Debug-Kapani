#pragma once

#ifndef TIMING_DETECTOR_H
#define TIMING_DETECTOR_H

#include <windows.h>
#include <cstdint>

namespace AntiDebug {

class TimingDetector {
public:
    TimingDetector() = default;
    ~TimingDetector() = default;

    // Deletes copy/move to prevent accidental misuse
    TimingDetector(const TimingDetector&)            = delete;
    TimingDetector& operator=(const TimingDetector&) = delete;

    // Returns true if a timing anomaly (debugger-induced slowdown) is detected.
    // Combines two independent methods: RDTSC delta and QueryPerformanceCounter.
    bool IsTimingAnomaly() const;

private:
    // RDTSC-based check: measures CPU cycles between two serialised reads.
    // A debugger introduces significant overhead, pushing the delta above the threshold.
    bool CheckRdtscDelta() const;

    // QPC-based check: measures wall-clock time between two tight operations.
    // Single-stepping or heavy instrumentation causes measurable latency spikes.
    bool CheckQpcLatency() const;

    // Threshold constants  ─────────────────────────────────────────────────────
    // Adjust these per your target hardware / use-case.
    static constexpr uint64_t kRdtscThresholdCycles = 500ULL;   // ~500 cycles @ ≥3 GHz ≈ 167 ns
    static constexpr double   kQpcThresholdMs       = 0.5;      // 0.5 ms – far above a normal NOP loop
};

} // namespace AntiDebug

#endif // TIMING_DETECTOR_H
