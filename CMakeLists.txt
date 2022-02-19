cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -flto")
project(DobieStation)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Setup conan CMake integration
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Shared packages
find_package(Threads REQUIRED)

# Modules
add_subdirectory(src/core)
add_subdirectory(src/qt)

if (MSVC)
    # Use DobieQt as Startup Project instead of ALL_BUILD
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT DobieQt)
endif()

if (UNIX AND NOT APPLE)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/data/DobieStation.desktop.in
        ${CMAKE_CURRENT_BINARY_DIR}/DobieStation.desktop)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DobieStation.desktop DESTINATION share/applications/)
endif()
