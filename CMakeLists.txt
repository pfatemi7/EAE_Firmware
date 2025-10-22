cmake_minimum_required(VERSION 3.20)
project(eae_firmware VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable IPO/LTO if supported
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    message(STATUS "IPO/LTO is supported")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO/LTO is not supported: ${ipo_error}")
endif()

# Enable testing
enable_testing()

# Find GTest
find_package(GTest CONFIG REQUIRED)

# Main executable
add_executable(eae_firmware
    src/main.cpp
    src/can.cpp
    src/pid.cpp
    src/state_machine.cpp
)

target_include_directories(eae_firmware PRIVATE src)

# Unit tests
add_executable(test_pid
    tests/test_pid.cpp
    src/pid.cpp
)

target_link_libraries(test_pid 
    PRIVATE 
    GTest::gtest 
    GTest::gtest_main
)

target_include_directories(test_pid PRIVATE src)

# Set runtime library to match vcpkg static triplet
if(MSVC)
    set_property(TARGET test_pid PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Add tests
add_test(NAME pid_tests COMMAND test_pid)
