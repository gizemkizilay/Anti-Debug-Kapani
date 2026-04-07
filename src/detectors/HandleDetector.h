#pragma once

#ifndef HANDLE_DETECTOR_H
#define HANDLE_DETECTOR_H

#include <windows.h>
#include <winternl.h>  // PROCESS_BASIC_INFORMATION, NtQueryInformationProcess prototype

namespace AntiDebug {

// ──────────────────────────────────────────────────────────────────────────────
// NtQueryInformationProcess is not fully declared in public SDK headers for all
// ProcessInformationClass values, so we redeclare the function pointer type and
// the ProcessDebugPort class ourselves.
// ──────────────────────────────────────────────────────────────────────────────
enum class NtProcessInfoClass : ULONG {
    ProcessDebugPort = 7UL
};

using NtQueryInformationProcess_t = NTSTATUS(NTAPI*)(
    HANDLE           ProcessHandle,
    ULONG            ProcessInformationClass,
    PVOID            ProcessInformation,
    ULONG            ProcessInformationLength,
    PULONG           ReturnLength
);

class HandleDetector {
public:
    HandleDetector();
    ~HandleDetector() = default;

    HandleDetector(const HandleDetector&)            = delete;
    HandleDetector& operator=(const HandleDetector&) = delete;

    // Returns true when at least one of the two sub-checks fires.
    bool IsHandleAnomalyDetected() const;

private:
    // Triggers a structured exception by closing the invalid handle 0xDEAD.
    // A debugger swallows the first-chance exception, so we never reach the
    // handler – the function returns true only in a non-debugged context.
    bool CheckInvalidHandleException() const;

    // Queries this process's debug port via the NT native API.
    // A non-zero value means a kernel-mode or user-mode debugger is attached.
    bool CheckDebugPort() const;

    // Resolved once in the constructor from ntdll.dll
    NtQueryInformationProcess_t m_NtQueryInfoProc = nullptr;
};

} // namespace AntiDebug

#endif // HANDLE_DETECTOR_H
