add_executable(generator_bench
  main.cpp
  bench.h
  bench.cpp
  consume.cpp
)
target_link_libraries(generator_bench generator range-v3 hayai)
if(MSVC)
  target_compile_definitions(generator_bench PRIVATE HAS_EXPERIMENTAL_GENERATOR)
else() # clang
endif()
