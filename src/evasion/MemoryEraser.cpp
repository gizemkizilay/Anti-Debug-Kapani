#include "MemoryEraser.h"

#include <windows.h>
#include <cstring>
#include <cstddef>
#include <cstdint>

// Link against DbgHelp at compile time only if needed; here we only need
// VirtualProtect which is in kernel32, already linked by default.

namespace AntiDebug {

// ─────────────────────────────────────────────────────────────────────────────
// ZeroSensitiveMemory
//
// Uses SecureZeroMemory() – a Windows API that is guaranteed not to be
// optimised away by the compiler, unlike memset().  This is critical when
// zeroing cryptographic keys, decrypted strings, or passwords so that they
// don't linger in heap/stack memory after use.
// ─────────────────────────────────────────────────────────────────────────────
void MemoryEraser::ZeroSensitiveMemory(void* ptr, std::size_t size) noexcept {
    if (!ptr || size == 0) return;
    ::SecureZeroMemory(ptr, size);
}

// ─────────────────────────────────────────────────────────────────────────────
// ErasePEHeader
//
// The PE header (MZ + PE signature + COFF + Optional header + section table)
// lives in the first page(s) of the mapped image.  Wiping it makes it much
// harder for a memory forensics tool (e.g. Volatility, PE-sieve) to:
//  - Identify the module as a PE image
//  - Reconstruct imports / exports / section layout from a memory dump
//
// Steps:
//  1. Locate our module base via GetModuleHandleW(nullptr).
//  2. Read e_lfanew to find where the PE header ends.
//  3. Temporarily grant write access with VirtualProtect.
//  4. SecureZeroMemory over the header region.
//  5. Restore original page protection.
//
// WARNING: After this call, Windows and the CRT may no longer be able to
//          walk the PE header for things like resource enumeration or
//          delay-loading.  Call this only after all initialisation is done.
// ─────────────────────────────────────────────────────────────────────────────
void MemoryEraser::ErasePEHeader() noexcept {
    // --- Step 1: locate base address ----------------------------------------
    const HMODULE hModule = GetModuleHandleW(nullptr); // nullptr → current EXE
    if (!hModule) return;

    const uint8_t* base = reinterpret_cast<const uint8_t*>(hModule);

    // --- Step 2: parse the DOS header to find the PE header size -------------
    // IMAGE_DOS_HEADER.e_magic == 'MZ' (0x5A4D)
    const auto* dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return; // Not an MZ image

    const auto* ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(
        base + dosHeader->e_lfanew
    );
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return; // Not a PE image

    // Total header size is given by OptionalHeader.SizeOfHeaders, which covers
    // the DOS stub, PE signature, COFF header, optional header, and section table.
    const DWORD headerSize = ntHeaders->OptionalHeader.SizeOfHeaders;
    if (headerSize == 0) return;

    // --- Step 3: temporarily make the page writable --------------------------
    DWORD oldProtect = 0;
    if (!VirtualProtect(
            const_cast<uint8_t*>(base),
            headerSize,
            PAGE_READWRITE,
            &oldProtect))
    {
        return; // Cannot change protection – skip silently
    }

    // --- Step 4: wipe ---------------------------------------------------------
    SecureZeroMemory(const_cast<uint8_t*>(base), headerSize);

    // --- Step 5: restore protection -------------------------------------------
    DWORD dummy = 0;
    VirtualProtect(const_cast<uint8_t*>(base), headerSize, oldProtect, &dummy);
}

} // namespace AntiDebug
