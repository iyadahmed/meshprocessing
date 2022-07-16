# Based on https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/src/CMakeLists.txt
# From https://cliutils.gitlab.io/modern-cmake

add_library(string_buffer io/string_buffer/string_buffer.cc io/string_buffer/string_buffer.hh)
target_include_directories(string_buffer PUBLIC io/string_buffer)
target_compile_features(string_buffer PUBLIC cxx_std_11)

add_library(stl
    io/stl/stl_io.cc
    io/stl/importer/stl_importer_binary.cc
    io/stl/importer/stl_importer_ascii.cc
    io/stl/exporter/stl_exporter_binary.cc

    io/stl/stl_io.hh
    io/stl/importer/stl_importer_binary.hh
    io/stl/importer/stl_importer_ascii.hh
    io/stl/exporter/stl_exporter_binary.hh
)
target_include_directories(stl PUBLIC io/stl PRIVATE io/stl/importer io/stl/exporter)
target_link_libraries(stl PRIVATE string_buffer)
target_compile_features(stl PUBLIC cxx_std_17)

add_library(vec3 INTERFACE)
target_sources(vec3 INTERFACE vec3/vec3.hh)
target_include_directories(vec3 INTERFACE vec3)
target_compile_features(vec3 INTERFACE cxx_std_11)

add_library(timers INTERFACE)
target_sources(timers INTERFACE timers/timers.hh)
target_include_directories(timers INTERFACE timers)
target_compile_features(timers INTERFACE cxx_std_17)

add_library(bvh INTERFACE)
target_sources(bvh INTERFACE bvh/bvh.hh)
target_include_directories(bvh INTERFACE bvh)
target_link_libraries(bvh INTERFACE vec3 stl)
target_compile_features(bvh INTERFACE cxx_std_11)

find_library(MATH_LIBRARY m)

if(MATH_LIBRARY)
    target_link_libraries(vec3 INTERFACE ${MATH_LIBRARY})
endif()
