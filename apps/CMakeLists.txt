include(libigl)
find_package(TBB CONFIG REQUIRED)
find_package(CGAL CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

if(WIN32)
    find_package(embree 3 REQUIRED)
    set(EMBREE_LINK_TARGET embree)
elseif(UNIX)
    find_library(embree3 REQUIRED)
    set(EMBREE_LINK_TARGET embree3)
endif()

add_executable(stl_stats stl_stats.cc)
target_compile_features(stl_stats PRIVATE cxx_std_17)
target_link_libraries(stl_stats PRIVATE stl vec3)

add_executable(boolean boolean.cc)
target_compile_features(boolean PRIVATE cxx_std_17)
target_link_libraries(boolean PRIVATE stl CGAL::CGAL)

add_executable(boolean2 boolean2.cc)
target_compile_features(boolean2 PRIVATE cxx_std_17)
target_link_libraries(boolean2 PRIVATE stl CGAL::CGAL)

add_executable(boolean3 boolean3.cc)
target_compile_features(boolean3 PRIVATE cxx_std_17)
target_link_libraries(boolean3 PRIVATE stl CGAL::CGAL)

add_executable(boolean_embree boolean_embree/boolean_embree.cc boolean_embree/boolean_embree.hh)
target_compile_features(boolean_embree PRIVATE cxx_std_17)
target_link_libraries(boolean_embree PRIVATE stl vec3 CGAL::CGAL ${EMBREE_LINK_TARGET})

add_executable(boolean5 boolean5.cc boolean5.hh)
target_compile_features(boolean5 PRIVATE cxx_std_17)
target_link_libraries(boolean5 PRIVATE stl vec3 CGAL::CGAL OpenMP::OpenMP_CXX TBB::tbb)

add_executable(bvh3 bvh3/bvh3.cc bvh3/bvh3.hh bvh3/morton_codes.hh)
target_compile_features(bvh3 PRIVATE cxx_std_17)
target_link_libraries(bvh3 PRIVATE stl vec3 bvh CGAL::CGAL OpenMP::OpenMP_CXX TBB::tbb)

add_executable(minimal_embree_bvh_build minimal_embree_bvh_build.cc minimal_embree_bvh_build.hh)
target_compile_features(minimal_embree_bvh_build PRIVATE cxx_std_17)
target_link_libraries(minimal_embree_bvh_build PRIVATE stl vec3 ${EMBREE_LINK_TARGET})

add_executable(winding_numbers winding_numbers.cc)
target_compile_features(winding_numbers PRIVATE cxx_std_17)
target_link_libraries(winding_numbers PRIVATE stl vec3 TBB::tbb)

add_executable(mcwn mcwn.cc)
target_compile_features(mcwn PRIVATE cxx_std_17)
target_link_libraries(mcwn PRIVATE stl vec3 CGAL::CGAL OpenMP::OpenMP_CXX)

add_executable(bvhapp bvh.cc)
target_compile_features(bvhapp PRIVATE cxx_std_17)
target_link_libraries(bvhapp PRIVATE stl vec3 bvh)

add_executable(bvhapp2 bvh2.cc)
target_compile_features(bvhapp2 PRIVATE cxx_std_17)
target_link_libraries(bvhapp2 PRIVATE stl vec3 bvh)

add_executable(mcpip mcpip.cc)
target_compile_features(mcpip PRIVATE cxx_std_17)
target_link_libraries(mcpip PRIVATE stl vec3 CGAL::CGAL OpenMP::OpenMP_CXX)

add_executable(mcpip_embree
    mcpip_embree/mcpip_embree.cc
    mcpip_embree/embree_device.hh
    mcpip_embree/embree_do_intersect.hh
    mcpip_embree/embree_num_intersections.hh
    mcpip_embree/embree_scene.hh
)
target_compile_features(mcpip_embree PRIVATE cxx_std_17)
target_link_libraries(mcpip_embree PRIVATE stl vec3 ${EMBREE_LINK_TARGET} OpenMP::OpenMP_CXX TBB::tbb igl::core)

add_executable(fast_winding_numbers fast_winding_numbers.cc)
target_compile_features(fast_winding_numbers PRIVATE cxx_std_17)
target_link_libraries(fast_winding_numbers PRIVATE stl vec3 CGAL::CGAL OpenMP::OpenMP_CXX igl::core)

if(UNIX)
    target_compile_options(fast_winding_numbers PRIVATE -Wno-unused-result)
endif()

add_executable(boolean6 boolean6.cc)
target_compile_features(boolean6 PRIVATE cxx_std_17)
target_link_libraries(boolean6 PRIVATE stl vec3 CGAL::CGAL OpenMP::OpenMP_CXX)
