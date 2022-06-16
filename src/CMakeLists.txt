# Based on https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/src/CMakeLists.txt
# From https://cliutils.gitlab.io/modern-cmake

add_library(trimesh io/trimesh.cc io/trimesh.hh)
target_compile_features(trimesh PUBLIC cxx_std_11) # All users of this library will need at least C++11

add_library(stl
    io/stl/importer.cc
    io/stl/binary.cc
    io/stl/ascii.cc
    io/stl/string_buffer.cc

    io/stl/importer.hh
    io/stl/binary.hh
    io/stl/ascii.hh
    io/stl/string_buffer.hh
)
target_link_libraries(stl PRIVATE trimesh)
target_compile_features(stl PUBLIC cxx_std_17)

add_library(vec3 io/vec3.cc io/vec3.hh)
target_compile_features(vec3 PUBLIC cxx_std_11)

find_library(MATH_LIBRARY m)

if(MATH_LIBRARY)
    target_link_libraries(vec3 PUBLIC ${MATH_LIBRARY})
endif()
