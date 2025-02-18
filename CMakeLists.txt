cmake_minimum_required(VERSION 3.12)

project(
    labwork11
    VERSION 1.0
    DESCRIPTION "C++ Year 2024 Labwork10"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Define your test sources
set(tests
    tests/tests.cpp
)

