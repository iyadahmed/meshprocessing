# Based on
# https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/src/CMakeLists.txt
# From https://cliutils.gitlab.io/modern-cmake

add_subdirectory(io)

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
target_link_libraries(bvh INTERFACE vec3)
target_compile_features(bvh INTERFACE cxx_std_11)

find_library(MATH_LIBRARY m)

if(MATH_LIBRARY)
  target_link_libraries(vec3 INTERFACE ${MATH_LIBRARY})
endif()
