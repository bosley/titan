cmake_minimum_required (VERSION 3.5)

project(titan)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
        compiler/lexer.cpp
        main.cpp)


add_custom_target(copy-wc ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test_code ${CMAKE_BINARY_DIR}/test_code
        )