add_executable(stl_stats stl_stats.cc)
target_compile_features(stl_stats PRIVATE cxx_std_17)
target_link_libraries(stl_stats PRIVATE stl vec3)

find_package(CGAL REQUIRED)
add_executable(boolean boolean.cc)
target_compile_features(boolean PRIVATE cxx_std_17)
target_link_libraries(boolean PRIVATE stl CGAL::CGAL)

find_package(CGAL REQUIRED)
add_executable(boolean2 boolean2.cc)
target_compile_features(boolean2 PRIVATE cxx_std_17)
target_link_libraries(boolean2 PRIVATE stl CGAL::CGAL)
