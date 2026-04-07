cmake_minimum_required(VERSION 3.14)
project(AntiDebugTrap LANGUAGES CXX)

# ── C++ Standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS  OFF)

# ── Default build type (single-config generators, e.g. MinGW Makefiles) ───────
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# ── Capstone via FetchContent (preserved) ─────────────────────────────────────
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

# All shared "library" sources used by both main exe and test binary
set(LIB_SOURCES
    ${DETECTOR_SOURCES}
    ${EVASION_SOURCES}
    ${UTILS_SOURCES}
)

# ─────────────────────────────────────────────────────────────────────────────
# Helper: apply compiler flags for a given target
# ─────────────────────────────────────────────────────────────────────────────
function(apply_compile_options TARGET_NAME)
    if(MSVC)
        # Base flags: high warning level, no warnings-as-errors, C++17, SEH, intrinsics
        target_compile_options(${TARGET_NAME} PRIVATE
            /W4         # Warning level 4
            /WX-        # Do NOT treat warnings as errors
            /std:c++17  # Explicit C++17 (belt-and-suspenders alongside CMAKE_CXX_STANDARD)
            /EHa        # Enable SEH so __try/__except catches hardware exceptions
            /Oi         # Enable intrinsic functions (__rdtsc, etc.)
        )

        # Configuration-specific flags (multi-config generators, e.g. VS)
        target_compile_options(${TARGET_NAME} PRIVATE
            $<$<CONFIG:Debug>:   /DDEBUG /Od /Zi>
            $<$<CONFIG:Release>: /O2 /DNDEBUG>
        )

        # ntdll for NtQueryInformationProcess (HandleDetector)
        target_link_libraries(${TARGET_NAME} PRIVATE ntdll)

    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # GCC / MinGW – equivalent warning and optimisation flags
        target_compile_options(${TARGET_NAME} PRIVATE
            -Wall -Wextra   # High warning level
            -msse2          # SSE2 for _mm_lfence / __rdtsc
        )

        # Configuration-specific flags (single-config generators, e.g. MinGW Makefiles)
        target_compile_options(${TARGET_NAME} PRIVATE
            $<$<CONFIG:Debug>:   -DDEBUG -O0 -g>
            $<$<CONFIG:Release>: -O2 -DNDEBUG>
        )

        if(WIN32)
            target_link_libraries(${TARGET_NAME} PRIVATE ntdll)
        endif()
    endif()
endfunction()

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(AntiDebugTrap main.cpp)

# Add all detector / evasion / utils sources via target_sources()
target_sources(AntiDebugTrap PRIVATE
    src/detectors/TimingDetector.cpp
    src/detectors/HandleDetector.cpp
    src/detectors/HeapFlagDetector.cpp
    src/evasion/MemoryEraser.cpp
    src/utils/Logger.cpp
)

# Include directories: src/ root + src/detectors/ for convenience
target_include_directories(AntiDebugTrap PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/detectors
)

target_link_libraries(AntiDebugTrap PRIVATE capstone)

apply_compile_options(AntiDebugTrap)

# ── Test executable ───────────────────────────────────────────────────────────
enable_testing()

add_executable(test_detectors
    tests/test_detectors.cpp
    src/detectors/TimingDetector.cpp
    src/detectors/HandleDetector.cpp
    src/detectors/HeapFlagDetector.cpp
    src/evasion/MemoryEraser.cpp
    src/utils/Logger.cpp
)

# Include directories: same as main target
target_include_directories(test_detectors PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/detectors
)

target_link_libraries(test_detectors PRIVATE capstone)

apply_compile_options(test_detectors)

# Register with CTest
add_test(NAME DetectorTests COMMAND test_detectors)
