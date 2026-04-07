cmake_minimum_required(VERSION 3.14)
project(AntiDebugTrap LANGUAGES CXX)

# ── Build type ────────────────────────────────────────────────────────────────
# Enforce Release so MSVC doesn't insert 0xCC padding (debug mode artefact).
set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)

# ── C++ Standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS  OFF)

# ── Capstone via FetchContent ─────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
  capstone
  GIT_REPOSITORY https://github.com/capstone-engine/capstone.git
  GIT_TAG        5.0.3
)
set(CAPSTONE_BUILD_TESTS  OFF CACHE BOOL "" FORCE)
set(CAPSTONE_BUILD_CSTOOL OFF CACHE BOOL "" FORCE)
set(CAPSTONE_BUILD_SHARED OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(capstone)

# ── Source file lists ─────────────────────────────────────────────────────────
set(DETECTOR_SOURCES
    src/detectors/TimingDetector.cpp
    src/detectors/HandleDetector.cpp
    src/detectors/HeapFlagDetector.cpp
)

set(EVASION_SOURCES
    src/evasion/MemoryEraser.cpp
)

set(UTILS_SOURCES
    src/utils/Logger.cpp
)

# Convenience: all "library" sources (shared between main exe and test binary)
set(LIB_SOURCES
    ${DETECTOR_SOURCES}
    ${EVASION_SOURCES}
    ${UTILS_SOURCES}
)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(AntiDebugTrap
    main.cpp
    ${LIB_SOURCES}
)

target_include_directories(AntiDebugTrap PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(AntiDebugTrap PRIVATE capstone)

# MSVC: enable SEH (/EHa) so __try/__except works in C++ translation units,
#       and enable intrinsics for __rdtsc / _mm_lfence.
if(MSVC)
    target_compile_options(AntiDebugTrap PRIVATE /EHa /Oi /W4)
    target_link_libraries(AntiDebugTrap PRIVATE ntdll)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/MinGW: enable SEH exceptions; ntdll is implicitly available on Windows
    target_compile_options(AntiDebugTrap PRIVATE -Wall -Wextra -msse2)
    if(WIN32)
        target_link_libraries(AntiDebugTrap PRIVATE ntdll)
    endif()
endif()

# ── Test executable ───────────────────────────────────────────────────────────
enable_testing()

add_executable(TestAntiDebug
    tests/test_detectors.cpp
    ${LIB_SOURCES}
)

target_include_directories(TestAntiDebug PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(TestAntiDebug PRIVATE capstone)

if(MSVC)
    target_compile_options(TestAntiDebug PRIVATE /EHa /Oi /W4)
    target_link_libraries(TestAntiDebug PRIVATE ntdll)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(TestAntiDebug PRIVATE -Wall -Wextra -msse2)
    if(WIN32)
        target_link_libraries(TestAntiDebug PRIVATE ntdll)
    endif()
endif()

# Register with CTest so "ctest" runs the binary automatically
add_test(NAME DetectorTests COMMAND TestAntiDebug)
