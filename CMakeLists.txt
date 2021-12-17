cmake_minimum_required (VERSION 3.5)

project(vtitan)

set(CMAKE_CXX_STANDARD 20)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERION_PATCH  0)
set(VERSION_INFO "${PROJECT_NAME} v${VERSION_MAJOR}.${VERSION_MINOR}.${VERION_PATCH}")

option(UNIT_TESTS        "Execute compile-time tests"    ON)
option(INTEGRATION_TESTS "Execute integration tests"     ON)

# Move this into a target cmake 
find_package(Threads REQUIRED)

if(UNIT_TESTS)
    include(cmake/FindCppuTest.cmake)
endif()

if(INTEGRATION_TESTS)
  # Find whatever will be used for integration tests
endif()

set(BUILD_PLATFORM "UNKNOWN")
if(WIN32)
  set(BUILD_PLATFORM "WINDOWS")
elseif(APPLE)
  set(BUILD_PLATFORM "APPLE")
elseif(UNIX AND NOT APPLE)
  set(BUILD_PLATFORM "LINUX")
endif()

if(NOT TARGET)
  set(TARGET standard)
endif()

add_subdirectory(lib)
add_subdirectory(apps)

message("VERSION_INFO: ${VERSION_INFO}")
message("BUILD PLATFORM: ${BUILD_PLATFORM}")
message("TARGET: ${TARGET}")



