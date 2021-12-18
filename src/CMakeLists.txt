cmake_minimum_required (VERSION 3.5)

project(titan)

set(CMAKE_CXX_STANDARD 20)

option(COMPILE_TESTS "Execute unit tests" ON)

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

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set(PROJECT_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/compiler/lexer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/compiler/parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/compiler/parsetree.cpp
)

add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        main.cpp)

add_custom_target(copy-test-code ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test_code ${CMAKE_BINARY_DIR}/test_code
        )

if(COMPILE_TESTS)
        add_subdirectory(tests)
endif()