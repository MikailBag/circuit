cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

project(Circuit)

find_package(Catch2 3 REQUIRED)

add_library(Argv STATIC)
target_include_directories(Argv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pkg/argv/include)
target_sources(Argv PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pkg/argv/argv.cpp)

add_library(Log STATIC)
target_include_directories(Log PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pkg/log/include)
target_sources(Log PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pkg/log/log.cpp)
target_compile_features(Log PUBLIC cxx_constexpr)
target_compile_features(Log PUBLIC cxx_std_20) # inline constexpr variable

add_library(Core STATIC)
target_compile_options(Core PRIVATE ${BUILD_FLAGS})
target_link_libraries(Core PRIVATE Log)
target_include_directories(Core PUBLIC core)
target_compile_features(Core PUBLIC cxx_attributes) # [[maybe_uninit]]
target_compile_features(Core PUBLIC cxx_std_20) # std::span
target_sources(Core PRIVATE core/graph.cpp core/graph_builder.cpp)
target_sources(Core PRIVATE core/simple_builder.cpp)
target_sources(Core PRIVATE core/testk.cpp)
target_sources(Core PRIVATE core/min_adder_depth.cpp)
target_sources(Core PRIVATE core/solver.cpp core/all_solvers.cpp)
target_sources(Core PRIVATE core/csd.cpp core/csd_solver.cpp)
target_sources(Core PRIVATE core/simple_solver.cpp)

add_executable(M core/main.cpp)
target_link_libraries(M PRIVATE Core)

add_executable(TotalTopologyCount bin/total_topology_count.cpp)

add_executable(Runner bin/runner.cpp)
target_link_libraries(Runner PRIVATE Core Argv)

add_executable(Bench bin/bench.cpp)
target_link_libraries(Bench PRIVATE Core)

add_executable(Test ut/graph_test.cpp ut/csd_test.cpp ut/csd_solver_test.cpp ut/simple_builder_test.cpp)
#target_sources(ut PRIVATE ut/testk_test.cpp)
target_link_libraries(Test PRIVATE Catch2::Catch2WithMain Core)
