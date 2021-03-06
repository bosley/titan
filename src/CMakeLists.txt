cmake_minimum_required (VERSION 3.5)

project(titan)

set(CMAKE_CXX_STANDARD 17)

#
# Options
#
option(COMPILE_TESTS "Execute unit tests" ON)
option(WITH_ASAN     "Compile with ASAN" OFF)

#
# Setup build type 'Release vs Debug'
#
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

#
# Locate CPPUTest
#
if(DEFINED ENV{CPPUTEST_HOME})
    message(STATUS "Using CppUTest home: $ENV{CPPUTEST_HOME}")
    set(CPPUTEST_INCLUDE_DIRS $ENV{CPPUTEST_HOME}/include)
    set(CPPUTEST_LIBRARIES $ENV{CPPUTEST_HOME}/lib)
    set(CPPUTEST_LDFLAGS CppUTest CppUTestExt)
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(CPPUTEST REQUIRED cpputest>=3.8)
    message(STATUS "Found CppUTest version ${CPPUTEST_VERSION}")
endif()

#
# Setup ASAN
#
if(WITH_ASAN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
  set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()

#
# Build 
#
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set(PROJECT_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/alert/alert.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/error/error_manager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/lang/instructions.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/lang/lexer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/lang/parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/exec/exec.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/exec/env.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/exec/memory.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/exec/space.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/analyze/analyzer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/analyze/symbols.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/titan.cpp
)

add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        main.cpp)

#
# Tests
#
if(COMPILE_TESTS)
        add_subdirectory(tests)
endif()
