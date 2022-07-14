# Based on https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/CMakeLists.txt
# From https://cliutils.gitlab.io/modern-cmake/

cmake_minimum_required(VERSION 3.11...3.23)

project(
    MeshProcessing
    VERSION 0.1
    DESCRIPTION "Mesh Processing in C++"
    LANGUAGES CXX
)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_CXX_EXTENSIONS OFF) # Let's ensure -std=c++xx instead of -std=g++xx
endif()

add_subdirectory(src)
add_subdirectory(apps)
