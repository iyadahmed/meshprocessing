cmake_minimum_required(VERSION 3.18)

project(meshprocessing)
add_executable(meshprocessing example.c stl.c stl.h)

set(TARGET meshprocessing PROPERTY C_STANDARD 11)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(meshprocessing m) # link libmath
    add_compile_options("-Ofast")
    add_compile_options("-march=native")
elseif(MSVC)
    add_compile_options("/O2")
endif()