add_executable(generator_bench main.cpp bench.cpp consume.cpp)
target_link_libraries(generator_bench generator range-v3 hayai)
