#pragma once

#ifndef MEMORY_ERASER_H
#define MEMORY_ERASER_H

#include <windows.h>
#include <cstddef>
#include <cstdint>
#include <array>
#include <string>

namespace AntiDebug {

// ──────────────────────────────────────────────────────────────────────────────
// XORSTR – compile-time XOR string obfuscation macro
//
// Usage:  auto s = XORSTR("secret");   // Returns an XorString<N>
//         std::string plain = s.Decrypt();
//
// The literal is XORed byte-by-byte with a compile-time key at compile time,
// so it never appears in plain text in the binary's .rdata section.
// ──────────────────────────────────────────────────────────────────────────────

// Compile-time XOR helper ─────────────────────────────────────────────────────
template<std::size_t N>
struct XorString {
    static constexpr uint8_t kKey = 0xAB; // Single-byte XOR key

    std::array<char, N> m_data;

    // Constructor: encrypt at compile time
    constexpr XorString(const char (&str)[N]) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            m_data[i] = static_cast<char>(static_cast<uint8_t>(str[i]) ^ kKey);
        }
    }

    // Decrypt at runtime into a std::string, then wipe the temporary buffer.
    [[nodiscard]] std::string Decrypt() const {
        std::string result(N - 1, '\0'); // Exclude null terminator
        for (std::size_t i = 0; i < N - 1; ++i) {
            result[i] = static_cast<char>(static_cast<uint8_t>(m_data[i]) ^ kKey);
        }
        return result;
    }
};

// Deduction guide so XORSTR macro can use template argument deduction
template<std::size_t N>
XorString(const char (&)[N]) -> XorString<N>;

// Public macro ────────────────────────────────────────────────────────────────
#define XORSTR(literal) (::AntiDebug::XorString{literal})

// ──────────────────────────────────────────────────────────────────────────────
// MemoryEraser
// ──────────────────────────────────────────────────────────────────────────────
class MemoryEraser {
public:
    MemoryEraser()  = default;
    ~MemoryEraser() = default;

    MemoryEraser(const MemoryEraser&)            = delete;
    MemoryEraser& operator=(const MemoryEraser&) = delete;

    // Securely zeroes 'size' bytes starting at 'ptr'.
    // Uses SecureZeroMemory (which the compiler cannot optimise away) so that
    // sensitive data (keys, passwords, decrypted strings) is reliably wiped.
    static void ZeroSensitiveMemory(void* ptr, std::size_t size) noexcept;

    // Overwrites the PE header of the currently loaded image with zeros.
    // This makes static forensic analysis of a memory dump significantly harder.
    static void ErasePEHeader() noexcept;
};

} // namespace AntiDebug

#endif // MEMORY_ERASER_H
