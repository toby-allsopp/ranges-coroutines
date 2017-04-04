cmake_minimum_required(VERSION 3.4)

project(ranges-coroutines)

add_library(range-v3 INTERFACE)
#set_property(TARGET range-v3 PROPERTY INTERFACE_INCLUDE_DIRECTORIES deps/range-v3/include)
target_include_directories(range-v3 INTERFACE deps/range-v3/include)

enable_testing()
add_subdirectory(generator)
