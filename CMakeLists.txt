# Based on https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/CMakeLists.txt
# From https://cliutils.gitlab.io/modern-cmake/

cmake_minimum_required(VERSION 3.11...3.23)

project(
    MeshProcessing
    VERSION 0.1
    DESCRIPTION "Mesh Processing in C++"
    LANGUAGES CXX
)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_CXX_EXTENSIONS OFF) # Let's ensure -std=c++xx instead of -std=g++xx
endif()

# FetchContent added in CMake 3.11, downloads during the configure step
include(FetchContent)

# FetchContent_MakeAvailable was not added until CMake 3.14; use our shim
if(${CMAKE_VERSION} VERSION_LESS 3.14)
    include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()

# Formatting library
FetchContent_Declare(
    cgal
    GIT_REPOSITORY https://github.com/CGAL/cgal.git
    GIT_TAG v5.4.1)
FetchContent_MakeAvailable(cgal)

add_subdirectory(src)
add_subdirectory(apps)
