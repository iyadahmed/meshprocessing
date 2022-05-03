add_executable(
    stl_stats
    stl_stats.cc
    ./stl/importer.cc

    ./stl/ascii.hh
    ./stl/binary.hh               
    ./stl/importer.hh
    vec3.hh)


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(stl_stats PUBLIC m)
endif()