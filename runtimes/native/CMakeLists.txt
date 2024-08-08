cmake_minimum_required(VERSION 3.7)
project(WASM4)

# Prevent BUILD_SHARED_LIBS and other options from being cleared by vendor CMakeLists
# https://stackoverflow.com/a/66342383
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# Enable LTO in release builds
if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.11)
cmake_minimum_required(VERSION 3.12)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    if (result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "IPO is not supported: ${output}")
    endif ()
endif ()
endif ()

# General options
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTS OFF)
set(BUILD_TOOLS OFF)

file(GLOB COMMON_SOURCES RELATIVE "${CMAKE_SOURCE_DIR}" "src/*.c")
file(GLOB M3_SOURCES RELATIVE "${CMAKE_SOURCE_DIR}" "vendor/wasm3/source/*.c")


set(DISPLAY_SOURCES
    src/main.c
    src/wasm_wasm3.c
    src/display.c
)
add_executable(wasm4 ${COMMON_SOURCES} ${DISPLAY_SOURCES} ${M3_SOURCES})

target_include_directories(wasm4 PRIVATE "${CMAKE_SOURCE_DIR}/vendor/wasm3/source")
set_target_properties(wasm4 PROPERTIES C_STANDARD 99)
install(TARGETS wasm4)

target_link_libraries(wasm4 m)

