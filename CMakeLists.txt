cmake_minimum_required(VERSION 3.14)
project(AntiDebugTrap)

# Sadece Release modunda derlenmesini zorla (MSVC paddingsiz)
set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)

# C++ Standardı
set(CMAKE_CXX_STANDARD 17)

# C++ Dosyası
add_executable(AntiDebugTrap src/main.cpp)

# Capstone Kütüphanesini CMake FetchContent ile dahil etme (En kolay yöntem)
include(FetchContent)
FetchContent_Declare(
  capstone
  GIT_REPOSITORY https://github.com/capstone-engine/capstone.git
  GIT_TAG        5.0.3
)

# Capstone için kullanılmayacak gereksiz hedef araçlarını ve testleri kapatıyoruz
set(CAPSTONE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CAPSTONE_BUILD_CSTOOL OFF CACHE BOOL "" FORCE)
set(CAPSTONE_BUILD_SHARED OFF CACHE BOOL "" FORCE) # Statik kullanılmasını tercih edelim

# Kütüphaneyi kullanılabilir hale getir
FetchContent_MakeAvailable(capstone)

# Projemizi Capstone (capstone) paketiyle linkliyoruz.
target_link_libraries(AntiDebugTrap PRIVATE capstone)
