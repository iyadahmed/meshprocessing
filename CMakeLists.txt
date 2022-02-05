cmake_minimum_required(VERSION 3.18)

project(meshprocessing)
set(TARGET meshprocessing PROPERTY C_STANDARD 11)

add_executable(test_stl
               test_stl.c
               stl.c
               mesh.cpp
               list.h
               
               stl.h
               mesh.hpp
               list.c)

add_executable(test_mesh
               test_mesh.cpp
               mesh.cpp
               mesh.hpp
               )

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # link libmath
    target_link_libraries(test_stl m) 
    target_link_libraries(test_mesh m)
    add_compile_options("-Ofast")
    add_compile_options("-march=native")
elseif(MSVC)
    add_compile_options("/O2")
endif()