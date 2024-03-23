cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_SUPPRESS_REGENERATION ON)

project(Circuit)

find_package(Catch2 3 CONFIG REQUIRED)

add_library(ArgParser STATIC)
target_include_directories(ArgParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pkg/Argv/include)
target_sources(ArgParser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pkg/Argv/argv.cpp)
target_compile_features(ArgParser PUBLIC cxx_std_20)

option(LOG_DISABLE "Disable all logging")
add_library(Log STATIC)
target_include_directories(Log PUBLIC pkg/Log/include)
configure_file(pkg/Log/config.h.in log-config/log-config.h)
target_include_directories(Log PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/log-config)
target_sources(Log PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pkg/Log/log.cpp)
target_compile_features(Log PUBLIC cxx_std_20)

add_library(ArrayVec STATIC)
target_include_directories(ArrayVec PUBLIC pkg/ArrayVec/include)
target_sources(ArrayVec PRIVATE pkg/ArrayVec/arrayvec.cpp)
target_compile_features(ArrayVec PUBLIC cxx_std_20)

add_library(Core STATIC)
target_link_libraries(Core PRIVATE Log)
target_include_directories(Core PUBLIC core)
target_compile_features(Core PUBLIC cxx_std_20)
target_sources(Core PRIVATE core/graph.cpp core/graph_builder.cpp)
target_sources(Core PRIVATE core/simple_builder.cpp)
target_sources(Core PRIVATE core/testk.cpp)
target_sources(Core PRIVATE core/min_adder_depth.cpp)
target_sources(Core PRIVATE core/solver.cpp core/all_solvers.cpp)
target_sources(Core PRIVATE core/csd.cpp core/csd_solver.cpp)
target_sources(Core PRIVATE core/simple_solver.cpp)

add_library(BruteForce STATIC)
target_include_directories(BruteForce PUBLIC pkg/BruteForce/include)
target_include_directories(BruteForce PRIVATE pkg/BruteForce)
target_link_libraries(BruteForce PUBLIC Core ArrayVec)
target_link_libraries(BruteForce PRIVATE Log)
target_compile_features(BruteForce PUBLIC cxx_std_20)
target_sources(BruteForce PRIVATE pkg/BruteForce/bruteforce.cpp pkg/BruteForce/isomorphism_key.cpp)

add_executable(M bin/main.cpp)
target_link_libraries(M PRIVATE Core)

add_executable(Runner bin/runner.cpp)
target_link_libraries(Runner PRIVATE Core ArgParser)

add_executable(Bench bin/bench.cpp)
target_link_libraries(Bench PRIVATE Core)

add_executable(FindAllOutputs bin/find_all_outputs.cpp)
target_link_libraries(FindAllOutputs PRIVATE BruteForce ArgParser)

add_executable(Test ut/graph_test.cpp ut/csd_test.cpp ut/csd_solver_test.cpp ut/simple_builder_test.cpp)
target_sources(Test PRIVATE ut/find_all_outputs_test.cpp)
#target_sources(ut PRIVATE ut/testk_test.cpp)
target_link_libraries(Test PRIVATE Catch2::Catch2WithMain Core BruteForce Log)
