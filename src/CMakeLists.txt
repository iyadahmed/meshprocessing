add_executable(test_stl
               test_stl.cpp
               stl.cpp
               
               stl.hpp
               vec3.hh)
add_executable(test_mesh test_mesh.cpp mesh.hpp)
add_executable(test_mathutils test_mathutils.cpp mathutils.h)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # link libmath
    # target_link_libraries(test_stl m)
    add_compile_options("-Ofast")
    add_compile_options("-march=native")
    add_compile_options("-mfma")
    add_compile_options("-m64")
elseif(MSVC)
    add_compile_options("/O2")
endif()