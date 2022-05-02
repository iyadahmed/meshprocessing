add_executable(
    stl_stats
    stl_stats.cpp
    stl.cpp
               
    stl.hpp
    vec3.hh)


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(stl_stats PUBLIC m)
    target_compile_options(stl_stats PUBLIC "-Ofast;-march=native;-mfma;-m64")
elseif(MSVC)
    target_compile_options(stl_stats PUBLIC "/O2")
endif()
