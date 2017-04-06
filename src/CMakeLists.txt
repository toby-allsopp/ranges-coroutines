cmake_minimum_required(VERSION 3.4)

project(ranges-coroutines)

add_library(range-v3 INTERFACE)
if(MSVC)
  target_include_directories(range-v3 INTERFACE deps/Range-V3-VS2015/include)
else()
  target_include_directories(range-v3 INTERFACE deps/range-v3/include)
endif()

add_library(hayai INTERFACE)
target_include_directories(hayai INTERFACE deps/hayai/src)

enable_testing()
add_subdirectory(generator)
