add_library(generator
  src/generator.cpp)
target_include_directories(generator
  PUBLIC include
  PRIVATE src)
target_compile_features(generator
  PUBLIC cxx_generic_lambdas)
if(MSVC)
  target_compile_options(generator
    PUBLIC /await)
else() # clang
  target_compile_options(generator
    PUBLIC -fcoroutines-ts -stdlib=libc++)
  target_link_libraries(generator -stdlib=libc++)
endif()

add_executable(generator_test test/generator.cpp test/main.cpp)
target_link_libraries(generator_test generator range-v3)

add_test(generator_test generator_test)

add_subdirectory(bench)
