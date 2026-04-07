#include "HeapFlagDetector.h"

#include <windows.h>
#include <cstdint>

// ─── PEB accessor helpers (compiler-portable) ────────────────────────────────
// MSVC exposes __readgsqword / __readfsdword as intrinsics.
// GCC/Clang on Windows needs inline asm or the __readgsbyte family via
// winnt.h (with __x86_64__) – use inline asm for maximum portability.
//
// On x64 :  GS:[0x60] → PEB*
// On x86 :  FS:[0x30] → PEB*
//
static inline const uint8_t* GetPEB() noexcept {
#if defined(_MSC_VER)
#  if defined(_WIN64)
    return reinterpret_cast<const uint8_t*>(__readgsqword(0x60));
#  else
    return reinterpret_cast<const uint8_t*>(__readfsdword(0x30));
#  endif
#elif (defined(__GNUC__) || defined(__clang__))
#  if defined(__x86_64__)
    uintptr_t peb;
    __asm__ volatile("mov %%gs:0x60, %0" : "=r"(peb));
    return reinterpret_cast<const uint8_t*>(peb);
#  else
    uintptr_t peb;
    __asm__ volatile("mov %%fs:0x30, %0" : "=r"(peb));
    return reinterpret_cast<const uint8_t*>(peb);
#  endif
#endif
}

namespace AntiDebug {

// ─────────────────────────────────────────────────────────────────────────────
// Public entry point
// ─────────────────────────────────────────────────────────────────────────────
bool HeapFlagDetector::IsHeapFlagSet() const {
    if (CheckNtGlobalFlag())    return true;
    if (CheckHeapHeaderFlags()) return true;
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// CheckNtGlobalFlag
//
// PEB layout (x64):
//   Offset 0x000 : InheritedAddressSpace (BYTE)
//   Offset 0x002 : BeingDebugged         (BYTE)  ← also useful
//   Offset 0x0BC : NtGlobalFlag          (ULONG) ← we read this
//
// We use __readgsqword(0x60) to get the PEB pointer without importing any
// symbol, keeping the intent opaque to simple static tracers.
// ─────────────────────────────────────────────────────────────────────────────
bool HeapFlagDetector::CheckNtGlobalFlag() const {
    const uint8_t* peb = GetPEB();
    if (!peb) return false;

#if defined(_WIN64) || defined(__x86_64__)
    // NtGlobalFlag is at PEB offset 0xBC (x64)
    const ULONG ntGlobalFlag = *reinterpret_cast<const ULONG*>(peb + 0xBC);
#else
    // NtGlobalFlag is at PEB offset 0x68 (x86)
    const ULONG ntGlobalFlag = *reinterpret_cast<const ULONG*>(peb + 0x68);
#endif

    // Check for HEAP_TAIL_CHECKING_ENABLED (0x20) OR HEAP_FREE_CHECKING_ENABLED (0x40)
    return (ntGlobalFlag & kNtGlobalFlagDebugMask) != 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// CheckHeapHeaderFlags
//
// The default process heap's header contains two relevant fields:
//   Flags     – normally 0x00000002 (HEAP_GROWABLE)
//   ForceFlags– normally 0x00000000
//
// When NtGlobalFlag activates debug heap flags, ntdll sets additional bits
// in both fields.  We compare against the expected clean-run values.
//
// Heap structure layout (x64):
//   Offset 0x70 : Flags      (ULONG)
//   Offset 0x74 : ForceFlags (ULONG)
//
// Heap structure layout (x86):
//   Offset 0x40 : Flags      (ULONG)
//   Offset 0x44 : ForceFlags (ULONG)
// ─────────────────────────────────────────────────────────────────────────────
bool HeapFlagDetector::CheckHeapHeaderFlags() const {
    const uint8_t* heap = reinterpret_cast<const uint8_t*>(GetProcessHeap());
    if (!heap) return false;

#if defined(_WIN64)
    const ULONG flags      = *reinterpret_cast<const ULONG*>(heap + 0x70);
    const ULONG forceFlags = *reinterpret_cast<const ULONG*>(heap + 0x74);
#else
    const ULONG flags      = *reinterpret_cast<const ULONG*>(heap + 0x40);
    const ULONG forceFlags = *reinterpret_cast<const ULONG*>(heap + 0x44);
#endif

    // In a clean (non-debugged) run:
    //   flags      == 0x00000002 (HEAP_GROWABLE only)
    //   forceFlags == 0x00000000
    // Under a debugger or gflags, extra bits are set.
    if (flags != kHeapDebugFlags)       return true;  // Unexpected bits present
    if (forceFlags != 0)                return true;  // Any force-flag is suspicious
    return false;
}

} // namespace AntiDebug
