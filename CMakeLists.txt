cmake_minimum_required(VERSION 3.25)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")
set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_LIST_DIR})
set(VCPKG_TARGET_TRIPLET x86_64-linux-clang17)
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/clang17-toolchain.cmake)

project(Circuit)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)

find_package(Catch2 3 REQUIRED)

set(BUILD_FLAGS -Wall -Wextra -Werror -std=c++23 -stdlib=libc++)
set(LINKER_FLAGS -stdlib=libc++)

add_library(argv STATIC)
target_compile_options(argv PRIVATE ${BUILD_FLAGS})
target_include_directories(argv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pkg/argv)
target_sources(argv PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pkg/argv/argv.cpp)

add_library(log STATIC)
target_compile_options(log PRIVATE ${BUILD_FLAGS})
target_include_directories(log PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pkg/log)
target_sources(log PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pkg/log/log.cpp)

add_library(core STATIC)
target_compile_options(core PRIVATE ${BUILD_FLAGS})
target_link_libraries(core PRIVATE log)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(core PRIVATE graph.cpp graph_builder.cpp)
target_sources(core PRIVATE simple_builder.cpp)
target_sources(core PRIVATE testk.cpp)
target_sources(core PRIVATE min_adder_depth.cpp)
target_sources(core PRIVATE solver.cpp all_solvers.cpp)
target_sources(core PRIVATE csd.cpp csd_solver.cpp)
target_sources(core PRIVATE simple_solver.cpp)

add_executable(m main.cpp)
target_compile_options(m PRIVATE ${BUILD_FLAGS})
target_link_libraries(m PRIVATE core)
target_link_options(m PRIVATE ${LINKER_FLAGS})

add_executable(TotalTopologyCount bin/total_topology_count.cpp)
target_compile_options(TotalTopologyCount PRIVATE ${BUILD_FLAGS})
target_link_options(TotalTopologyCount PRIVATE ${LINKER_FLAGS})

add_executable(Runner bin/runner.cpp)
target_compile_options(Runner PRIVATE ${BUILD_FLAGS})
target_link_libraries(Runner PRIVATE core argv)
target_link_options(Runner PRIVATE ${LINKER_FLAGS})

add_executable(Bench bin/bench.cpp)
target_compile_options(Bench PRIVATE ${BUILD_FLAGS})
target_link_libraries(Bench PRIVATE core)
target_link_options(Bench PRIVATE ${LINKER_FLAGS})

add_executable(ut ut/graph_test.cpp ut/csd_test.cpp ut/csd_solver_test.cpp ut/simple_builder_test.cpp)
#target_sources(ut PRIVATE ut/testk_test.cpp)
target_compile_options(ut PRIVATE ${BUILD_FLAGS})
target_link_libraries(ut PRIVATE Catch2::Catch2WithMain core)
target_link_options(ut PRIVATE ${LINKER_FLAGS})