cmake_minimum_required(VERSION 3.14)

project(ExpECS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ExpECS INTERFACE)
target_include_directories(ExpECS INTERFACE include)

# Only build tests when this is the top-level project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    # For Windows: Prevent GTest from overriding parent project compiler settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)
    enable_testing()

    add_executable(RunTests
        tests/entity_manager_tests.cpp
        tests/component_manager_tests.cpp
        tests/system_manager_tests.cpp
        tests/registry_tests.cpp
    )
    target_link_libraries(RunTests PRIVATE
        ExpECS
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(RunTests)

    # Benchmark if release build
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "Release build: adding Benchmark...")

        add_executable(Benchmarks
            tests/benchmarks.cpp
        )
        target_link_libraries(Benchmarks PRIVATE
            ExpECS
            GTest::gtest_main
        )

        gtest_discover_tests(Benchmarks)
    else()
        message(STATUS "Debug build: Skipping Benchmarks (Set CMAKE_BUILD_TYPE=Release to include them)")
    endif()
endif()