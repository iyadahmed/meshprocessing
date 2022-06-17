# Based on https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/src/CMakeLists.txt
# From https://cliutils.gitlab.io/modern-cmake

add_library(string_buffer io/string_buffer/string_buffer.cc io/string_buffer/string_buffer.hh)
target_include_directories(string_buffer PUBLIC io/string_buffer)
target_compile_features(string_buffer PUBLIC cxx_std_11)

add_library(trimesh trimesh/trimesh.cc trimesh/trimesh.hh)
target_include_directories(trimesh PUBLIC trimesh)
target_compile_features(trimesh PUBLIC cxx_std_11)

add_library(stl
    io/stl/importer.cc
    io/stl/binary.cc
    io/stl/ascii.cc

    io/stl/importer.hh
    io/stl/binary.hh
    io/stl/ascii.hh
)
target_include_directories(stl PUBLIC io/stl)
target_link_libraries(stl PRIVATE trimesh string_buffer)
target_compile_features(stl PUBLIC cxx_std_17)

add_library(vec3 vec3/vec3.cc vec3/vec3.hh)
target_include_directories(vec3 PUBLIC vec3)
target_compile_features(vec3 PUBLIC cxx_std_11)

find_library(MATH_LIBRARY m)

if(MATH_LIBRARY)
    target_link_libraries(vec3 PUBLIC ${MATH_LIBRARY})
endif()