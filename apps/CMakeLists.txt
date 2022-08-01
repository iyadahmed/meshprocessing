include(libigl)
find_package(TBB CONFIG REQUIRED)
find_package(CGAL CONFIG REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Embree REQUIRED)

add_subdirectory(boolean_embree)

add_executable(bsp bsp.cc bsp.hh near_far_tree.hh MurmurHash3.h MurmurHash3.cpp vector_set.hh)
target_compile_features(bsp PRIVATE cxx_std_17)
target_link_libraries(bsp PRIVATE stl)

add_executable(self_intersection self_intersection.cc)
target_compile_features(self_intersection PRIVATE cxx_std_17)
target_link_libraries(self_intersection PRIVATE stl CGAL::CGAL)

add_executable(stl_stats stl_stats.cc)
target_compile_features(stl_stats PRIVATE cxx_std_17)
target_link_libraries(stl_stats PRIVATE stl vec3 timers)

add_executable(winding_numbers winding_numbers.cc)
target_compile_features(winding_numbers PRIVATE cxx_std_17)
target_link_libraries(winding_numbers PRIVATE stl vec3 TBB::tbb)

add_executable(bvhapp bvh.cc)
target_compile_features(bvhapp PRIVATE cxx_std_17)
target_link_libraries(bvhapp PRIVATE stl vec3 bvh timers)

add_executable(mcpip mcpip.cc)
target_compile_features(mcpip PRIVATE cxx_std_17)
target_link_libraries(mcpip PRIVATE timers stl vec3 CGAL::CGAL
                                    OpenMP::OpenMP_CXX)

add_executable(
  mcpip_embree
  mcpip_embree/mcpip_embree.cc mcpip_embree/embree_device.hh
  mcpip_embree/embree_do_intersect.hh mcpip_embree/embree_num_intersections.hh
  mcpip_embree/embree_scene.hh)
target_compile_features(mcpip_embree PRIVATE cxx_std_17)
target_link_libraries(
  mcpip_embree
  PRIVATE timers
          stl
          vec3
          ${EMBREE_LIBRARIES}
          OpenMP::OpenMP_CXX
          TBB::tbb
          igl::core)

add_executable(fast_winding_numbers fast_winding_numbers.cc)
target_compile_features(fast_winding_numbers PRIVATE cxx_std_17)
target_link_libraries(fast_winding_numbers PRIVATE timers stl vec3 CGAL::CGAL
                                                   OpenMP::OpenMP_CXX igl::core)

if(UNIX)
# Because of igl
  target_compile_options(fast_winding_numbers PRIVATE -Wno-unused-result)
endif()
