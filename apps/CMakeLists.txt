add_executable(stl_stats stl_stats.cc)
target_compile_features(stl_stats PRIVATE cxx_std_17)
target_link_libraries(stl_stats PRIVATE stl vec3)

find_package(CGAL REQUIRED)
add_executable(boolean boolean.cc)
target_compile_features(boolean PRIVATE cxx_std_17)
target_link_libraries(boolean PRIVATE stl CGAL::CGAL)

add_executable(boolean2 boolean2.cc)
target_compile_features(boolean2 PRIVATE cxx_std_17)
target_link_libraries(boolean2 PRIVATE stl CGAL::CGAL)

find_package(TBB REQUIRED)
add_executable(winding_numbers winding_numbers.cc)
target_compile_features(winding_numbers PRIVATE cxx_std_17)
target_link_libraries(winding_numbers PRIVATE stl vec3 TBB::tbb)

find_package(OpenMP REQUIRED)
add_executable(mcwn mcwn.cc)
target_compile_features(mcwn PRIVATE cxx_std_17)
target_link_libraries(mcwn PRIVATE stl vec3 CGAL::CGAL OpenMP::OpenMP_CXX)

add_executable(bvhapp bvh.cc)
target_compile_features(bvhapp PRIVATE cxx_std_17)
target_link_libraries(bvhapp PRIVATE stl vec3)
