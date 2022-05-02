cmake_minimum_required(VERSION 3.18)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(
    MeshProcessing

)
set(TARGET meshprocessing PROPERTY C_STANDARD 11)
set(TARGET meshprocessing PROPERTY CXX_STANDARD 11)

add_subdirectory(apps)
