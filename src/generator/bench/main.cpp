#include "bench.h"

#include <hayai.hpp>

static const int NUM = 100;

BENCHMARK(ints, generator_toby, 1000, 100000 / NUM) { bench_ints_generator_toby(NUM); }

BENCHMARK(ints, generator_gor, 1000, 100000 / NUM) { bench_ints_generator_gor(NUM); }

#ifdef HAS_EXPERIMENTAL_GENERATOR
BENCHMARK(ints, generator_exp, 1000, 100000 / NUM) { bench_ints_generator_exp(NUM); }
#endif

BENCHMARK(ints, generator_toby_atomic, 1000, 100000 / NUM) { bench_ints_generator_toby_atomic(NUM); }

BENCHMARK(ints, handrolled, 1000, 100000 / NUM) { bench_ints_handrolled(NUM); }

/*
BENCHMARK(ints, callback, 1000, 100000/NUM) {
  cb_ints(0, NUM, [](int i) { consume(i); });
}
*/

BENCHMARK(ints, ranges, 1000, 100000 / NUM) { bench_ints_ranges(NUM); }

int main() {
  hayai::ConsoleOutputter consoleOutputter;

  hayai::Benchmarker::AddOutputter(consoleOutputter);
  hayai::Benchmarker::RunAllTests();
  return 0;
}
