// ─────────────────────────────────────────────────────────────────────────────
// tests/test_detectors.cpp
//
// Minimal self-test suite for the Anti-Debug detector modules.
//
// Expected behaviour in a NORMAL (non-debugged) Release build:
//   TimingDetector  → false  (no timing anomaly)
//   HandleDetector  → false  (no handle anomaly)
//   HeapFlagDetector→ false  (no debug heap flags)
//   MemoryEraser    → passes (no crash)
//   XORSTR macro    → round-trips correctly
//
// Build target: TestAntiDebug  (see CMakeLists.txt)
// Run with    : ctest  or  TestAntiDebug.exe  directly
// ─────────────────────────────────────────────────────────────────────────────

// Add project root to include path via CMake target_include_directories
#include "detectors/TimingDetector.h"
#include "detectors/HandleDetector.h"
#include "detectors/HeapFlagDetector.h"
#include "evasion/MemoryEraser.h"
#include "utils/Logger.h"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// Simple test framework (no Catch2 / GoogleTest dependency)
// ─────────────────────────────────────────────────────────────────────────────
static int g_passed = 0;
static int g_failed = 0;

static void EXPECT_FALSE(bool value, const char* label) {
    if (!value) {
        printf("  [PASS]  %s → false (no debugger artefact)\n", label);
        ++g_passed;
    } else {
        printf("  [FAIL]  %s → TRUE  (unexpected – are you running under a debugger?)\n", label);
        ++g_failed;
    }
}

static void EXPECT_TRUE(bool value, const char* label) {
    if (value) {
        printf("  [PASS]  %s → true\n", label);
        ++g_passed;
    } else {
        printf("  [FAIL]  %s → false\n", label);
        ++g_failed;
    }
}

static void EXPECT_EQ_STR(const std::string& a, const std::string& b, const char* label) {
    if (a == b) {
        printf("  [PASS]  %s → \"%s\"\n", label, a.c_str());
        ++g_passed;
    } else {
        printf("  [FAIL]  %s → got \"%s\", expected \"%s\"\n", label, a.c_str(), b.c_str());
        ++g_failed;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Individual test functions
// ─────────────────────────────────────────────────────────────────────────────

// 1. TimingDetector ───────────────────────────────────────────────────────────
static void TestTimingDetector() {
    printf("\n[TEST] TimingDetector\n");

    AntiDebug::TimingDetector detector;

    // In a normal Release run the trivial loop should take far less than
    // 500 cycles (RDTSC) and less than 0.5 ms (QPC).
    bool result = detector.IsTimingAnomaly();
    EXPECT_FALSE(result, "IsTimingAnomaly()");

    LOG_EVENT("TestTimingDetector: IsTimingAnomaly() = " + std::to_string(result));
}

// 2. HandleDetector ───────────────────────────────────────────────────────────
static void TestHandleDetector() {
    printf("\n[TEST] HandleDetector\n");

    AntiDebug::HandleDetector detector;

    // CloseHandle(0xDEAD) should raise an exception that we catch ourselves.
    // NtQueryInformationProcess should return debugPort == 0.
    bool result = detector.IsHandleAnomalyDetected();
    EXPECT_FALSE(result, "IsHandleAnomalyDetected()");

    LOG_EVENT("TestHandleDetector: IsHandleAnomalyDetected() = " + std::to_string(result));
}

// 3. HeapFlagDetector ─────────────────────────────────────────────────────────
static void TestHeapFlagDetector() {
    printf("\n[TEST] HeapFlagDetector\n");

    AntiDebug::HeapFlagDetector detector;

    // Without a debugger / gflags, NtGlobalFlag should NOT have bits 0x20 | 0x40,
    // and the heap header should show normal Flags == 0x02, ForceFlags == 0x00.
    bool result = detector.IsHeapFlagSet();
    EXPECT_FALSE(result, "IsHeapFlagSet()");

    LOG_EVENT("TestHeapFlagDetector: IsHeapFlagSet() = " + std::to_string(result));
}

// 4. MemoryEraser – ZeroSensitiveMemory ───────────────────────────────────────
static void TestZeroSensitiveMemory() {
    printf("\n[TEST] MemoryEraser::ZeroSensitiveMemory\n");

    char secret[32] = "TopSecretPassword123";
    AntiDebug::MemoryEraser::ZeroSensitiveMemory(secret, sizeof(secret));

    // Verify every byte is now 0
    bool allZero = true;
    for (std::size_t i = 0; i < sizeof(secret); ++i) {
        if (secret[i] != '\0') { allZero = false; break; }
    }
    EXPECT_TRUE(allZero, "Buffer is zeroed after ZeroSensitiveMemory()");

    LOG_EVENT("TestZeroSensitiveMemory: allZero = " + std::to_string(allZero));
}

// 5. MemoryEraser – ErasePEHeader ─────────────────────────────────────────────
static void TestErasePEHeader() {
    printf("\n[TEST] MemoryEraser::ErasePEHeader\n");

    // Calling ErasePEHeader() should not crash.
    // After erasing we confirm the MZ signature is gone.
    HMODULE hMod = GetModuleHandleW(nullptr);
    const uint8_t* base = reinterpret_cast<const uint8_t*>(hMod);

    AntiDebug::MemoryEraser::ErasePEHeader();

    // The first two bytes should now be 0x00 0x00 (MZ wiped).
    bool mzGone = (base[0] == 0x00 && base[1] == 0x00);
    EXPECT_TRUE(mzGone, "PE MZ signature erased from memory");

    LOG_EVENT("TestErasePEHeader: mzGone = " + std::to_string(mzGone));
}

// 6. XORSTR macro – round-trip ────────────────────────────────────────────────
static void TestXorStr() {
    printf("\n[TEST] XORSTR macro\n");

    // The literal is XOR-obfuscated at compile time; Decrypt() recovers it.
    auto encoded = XORSTR("AntiDebugTrap");
    std::string decoded = encoded.Decrypt();

    EXPECT_EQ_STR(decoded, "AntiDebugTrap", "XORSTR round-trip");

    // Confirm a different string
    auto encoded2 = XORSTR("Hello, World!");
    EXPECT_EQ_STR(encoded2.Decrypt(), "Hello, World!", "XORSTR round-trip 2");

    LOG_EVENT("TestXorStr: completed");
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    printf("====================================================\n");
    printf("  AntiDebugTrap – Detector Self-Test Suite\n");
    printf("====================================================\n");
    printf("  NOTE: All tests should PASS (return false) when\n");
    printf("        run outside a debugger in Release mode.\n");

    // -- Run all tests ---------------------------------------------------------
    TestTimingDetector();
    TestHandleDetector();
    TestHeapFlagDetector();
    TestZeroSensitiveMemory();
    TestXorStr();
    // ErasePEHeader is intentionally run last because it invalidates the
    // in-memory PE header; keep it at the end of the test sequence.
    TestErasePEHeader();

    // -- Summary ---------------------------------------------------------------
    printf("\n====================================================\n");
    printf("  Results: %d passed, %d failed\n", g_passed, g_failed);
    printf("====================================================\n");

    return (g_failed == 0) ? 0 : 1;
}
