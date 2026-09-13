cmake_minimum_required(VERSION 3.20)
project(rushevich_trees LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(flags INTERFACE)
target_compile_options(flags INTERFACE -g)

add_executable(test main.cpp)
target_include_directories(test PRIVATE rb_tree.hpp)
target_link_libraries(test PRIVATE flags)


add_library(bench_flags INTERFACE)
target_compile_options(bench_flags INTERFACE)

# add_executable(bench bench.cpp)
# target_include_directories(bench PRIVATE queue.hpp)
# target_link_libraries(test PRIVATE bench_flags)
