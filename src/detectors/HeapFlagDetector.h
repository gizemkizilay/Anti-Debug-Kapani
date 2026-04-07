#pragma once

#ifndef HEAP_FLAG_DETECTOR_H
#define HEAP_FLAG_DETECTOR_H

#include <windows.h>
#include <cstdint>

namespace AntiDebug {

// ──────────────────────────────────────────────────────────────────────────────
// When a process is launched under a debugger (or with image file options set
// by gflags.exe), Windows sets specific bits in the PEB's NtGlobalFlag field.
//
// The three canonical bits to watch are:
//   FLG_HEAP_ENABLE_TAIL_CHECK  (0x10) – "HEAP_TAIL_CHECKING_ENABLED"
//   FLG_HEAP_ENABLE_FREE_CHECK  (0x20) – "HEAP_FREE_CHECKING_ENABLED"
//   FLG_HEAP_VALIDATE_PARAMETERS(0x40) – "HEAP_VALIDATE_PARAMETERS_ENABLED"
//
// Under a debugger ntdll normally sets all three, giving NtGlobalFlag == 0x70.
// We check the two most common ones (0x20 | 0x40) as specified in the task.
//
// Additionally, the heap header itself contains a Flags/ForceFlags field that
// differs between debugged/non-debugged processes – we verify that as well.
// ──────────────────────────────────────────────────────────────────────────────

class HeapFlagDetector {
public:
    HeapFlagDetector() = default;
    ~HeapFlagDetector() = default;

    HeapFlagDetector(const HeapFlagDetector&)            = delete;
    HeapFlagDetector& operator=(const HeapFlagDetector&) = delete;

    // Returns true if debugger-indicative heap flags are found.
    bool IsHeapFlagSet() const;

private:
    // Reads PEB->NtGlobalFlag via inline asm / intrinsic and checks the bits.
    bool CheckNtGlobalFlag() const;

    // Reads the heap's own Flags and ForceFlags fields as a secondary check.
    bool CheckHeapHeaderFlags() const;

    // Bit masks as tasked ─────────────────────────────────────────────────────
    static constexpr ULONG kHeapTailCheckEnabled  = 0x20UL; // HEAP_TAIL_CHECKING_ENABLED
    static constexpr ULONG kHeapFreeCheckEnabled  = 0x40UL; // HEAP_FREE_CHECKING_ENABLED
    static constexpr ULONG kNtGlobalFlagDebugMask = kHeapTailCheckEnabled | kHeapFreeCheckEnabled;

    // Heap header Flags mask that appears under a debugger (non-debug = 2, debug = 0x40000062)
    static constexpr ULONG kHeapDebugFlags = 0x02UL;        // HEAP_GROWABLE (absent under debugger)
    static constexpr ULONG kHeapDebugForceFlags = 0x40000060UL;
};

} // namespace AntiDebug

#endif // HEAP_FLAG_DETECTOR_H
