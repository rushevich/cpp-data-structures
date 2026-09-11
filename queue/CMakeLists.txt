cmake_minimum_required(VERSION 3.20)
project(rushevich_queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(flags INTERFACE)
target_compile_options(flags INTERFACE -g)
add_executable(test main.cpp)
target_include_directories(test PRIVATE queue.hpp)
target_link_libraries(test PRIVATE flags)
