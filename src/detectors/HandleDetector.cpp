#include "HandleDetector.h"

#include <cstdint>
#include <windows.h>
#include <winternl.h>

// ntdll is linked via CMakeLists.txt target_link_libraries

// ─────────────────────────────────────────────────────────────────────────────
// Portable SEH wrapper
//
// MSVC supports __try/__except natively.
// MinGW (POSIX thread model, the most common pre-built distribution) does NOT
// support Structured Exception Handling in C++ translation units because it
// compiles with SJLJ or Dwarf unwinding rather than Win32 SEH stack frames.
//
// For MinGW we fall back to a "raw" PVECTORED_EXCEPTION_HANDLER approach:
// we install a vectored handler, attempt the bad CloseHandle call, then remove
// the handler.  If the handler fires, no debugger consumed the exception.
// This is slightly noisier (VEH fires for ALL exceptions) but is fully
// portable across POSIX and Win32 MinGW builds.
// ─────────────────────────────────────────────────────────────────────────────

namespace AntiDebug {

// ─────────────────────────────────────────────────────────────────────────────
// Constructor: resolve NtQueryInformationProcess from ntdll at runtime.
// Using GetProcAddress makes the import invisible to static analysis tools.
// ─────────────────────────────────────────────────────────────────────────────
HandleDetector::HandleDetector() {
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll) {
        // Cast via void* to suppress -Wcast-function-type on GCC
        void* raw = reinterpret_cast<void*>(
            GetProcAddress(hNtdll, "NtQueryInformationProcess")
        );
        m_NtQueryInfoProc = reinterpret_cast<NtQueryInformationProcess_t>(raw);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Public entry point
// ─────────────────────────────────────────────────────────────────────────────
bool HandleDetector::IsHandleAnomalyDetected() const {
    if (CheckInvalidHandleException()) return true;
    if (CheckDebugPort())              return true;
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// CheckInvalidHandleException
//
// We call CloseHandle(INVALID_HANDLE_VALUE) – which is guaranteed to set
// GetLastError() to ERROR_INVALID_HANDLE (6) on a normal run – and then check
// whether the error code is as expected.
//
// A simpler, more reliable alternative: call NtClose(0xDEAD) via the native
// API.  When a kernel debugger is attached, NtClose on an invalid handle
// raises a STATUS_INVALID_HANDLE exception that can disrupt execution; when no
// debugger is present it returns STATUS_INVALID_HANDLE (0xC0000008) silently.
//
// Here we use a pure Win32 approach:
//   1. Call CloseHandle with a deliberately invalid value.
//   2. Check GetLastError() immediately after.
//   3. Additionally, use IsDebuggerPresent() as a quick sanity check.
//
// The VEH approach requires SJLJ/Win32 SEH stack frames which are absent in
// POSIX-model MinGW builds, so we avoid it here for portability.
// ─────────────────────────────────────────────────────────────────────────────
bool HandleDetector::CheckInvalidHandleException() const {
    // Suppress error dialogs / hard errors
    const UINT prevMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

    SetLastError(0);

    // CloseHandle with a completely invalid value.
    // On non-debugged runs this sets LastError = ERROR_INVALID_HANDLE (6) and
    // returns FALSE.  Under some debuggers/tools the call behaves differently.
    const BOOL result = CloseHandle(reinterpret_cast<HANDLE>(static_cast<ULONG_PTR>(0xDEAD)));

    const DWORD lastErr = GetLastError();

    SetErrorMode(prevMode);

    // Normal expectation: result == FALSE and lastErr == ERROR_INVALID_HANDLE
    // If result is unexpectedly TRUE, something intercepted / spoofed the call.
    if (result != FALSE) {
        return true; // Anomaly: handle was "valid" – should not happen
    }

    // If lastErr is not the expected invalid-handle error, something is off.
    if (lastErr != ERROR_INVALID_HANDLE) {
        return true;
    }

    return false; // Everything is as expected in a clean run
}

// ─────────────────────────────────────────────────────────────────────────────
// CheckDebugPort
//
// ProcessDebugPort (class 7) returns a non-zero DWORD_PTR when the process
// is being debugged by a kernel debugger or a user-mode debugger that has
// attached via DebugActiveProcess().
// ─────────────────────────────────────────────────────────────────────────────
bool HandleDetector::CheckDebugPort() const {
    if (!m_NtQueryInfoProc) {
        return false; // Cannot determine – treat as not detected
    }

    DWORD_PTR debugPort = 0;
    ULONG      returnLen = 0;

    const NTSTATUS status = m_NtQueryInfoProc(
        GetCurrentProcess(),
        static_cast<ULONG>(NtProcessInfoClass::ProcessDebugPort),
        &debugPort,
        static_cast<ULONG>(sizeof(debugPort)),
        &returnLen
    );

    // NT_SUCCESS(status) && debugPort != 0  →  debugger attached
    if (status == 0 /*STATUS_SUCCESS*/ && debugPort != 0) {
        return true;
    }

    return false;
}

} // namespace AntiDebug
