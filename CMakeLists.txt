cmake_minimum_required(VERSION 3.18)

project(
    MeshProcessing

)
set(TARGET meshprocessing PROPERTY C_STANDARD 11)
set(TARGET meshprocessing PROPERTY CXX_STANDARD 11)

add_subdirectory(apps)
