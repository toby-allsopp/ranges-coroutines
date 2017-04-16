#include "bench.h"

#include <hayai.hpp>

static const int NUM = 100;

BENCHMARK(cofoo, co_ints_toby, 1000, 100000 / NUM) { bench_co_ints_toby(NUM); }

BENCHMARK(cofoo, co_ints_gor, 1000, 100000 / NUM) { bench_co_ints_gor(NUM); }

#ifdef HAS_EXPERIMENTAL_GENERATOR
BENCHMARK(cofoo, co_ints_exp, 1000, 100000 / NUM) { bench_co_ints_exp(NUM); }
#endif

BENCHMARK(cofoo, co_ints_atomic, 1000, 100000 / NUM) { bench_co_ints_atomic(NUM); }

BENCHMARK(cofoo, handrolled_ints, 1000, 100000 / NUM) { bench_handrolled_ints(NUM); }

/*
BENCHMARK(co_ints, cb_ints, 1000, 100000/NUM) {
  cb_ints(0, NUM, [](int i) { consume(i); });
}
*/

BENCHMARK(cofoo, ranges, 1000, 100000 / NUM) { bench_ranges_ints(NUM); }

int main() {
  hayai::ConsoleOutputter consoleOutputter;

  hayai::Benchmarker::AddOutputter(consoleOutputter);
  hayai::Benchmarker::RunAllTests();
  return 0;
}
