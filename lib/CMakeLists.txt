
project(libvtitan)

set(TARGET_SOURCE_DIR ${PROJECT_SOURCE_DIR}/targets/${TARGET})

# Populates VTITAN_TARGET_SOURCES 
# VTITAN_TARGET_PRIVATE_INCLUDES
# and VTITAN_TARGET_PRIVATE_SOURCES
include(${TARGET_SOURCE_DIR}/Target.cmake)

set(SETTINGS_HEADER ${PROJECT_SOURCE_DIR}/include/vtitan/settings.hpp)
configure_file(${PROJECT_SOURCE_DIR}/settings.hpp.in ${SETTINGS_HEADER})

add_library(
        ${PROJECT_NAME}
        ${VTITAN_TARGET_SOURCES}
        ${VTITAN_TARGET_PRIVATE_SOURCES}
)

add_library(vtitan::lib ALIAS ${PROJECT_NAME})

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        PRIVATE
        ${VTITAN_TARGET_PRIVATE_INCLUDES}
)

target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC
        Threads::Threads
)