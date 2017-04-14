#include "generator.h"
#include "gor_generator.h"

#include <hayai.hpp>
#include <range/v3/view/iota.hpp>

template <class Generator>
Generator co_ints(int start, int end) {
  for (int i = start; i < end; ++i) {
    co_yield i;
  }
}

template <class F>
void cb_ints(int start, int end, F&& f) {
  for (int i = start; i < end; ++i) {
    f(i);
  }
}

extern void consume(int);

static const int NUM = 1;

BENCHMARK(cofoo, co_ints, 1000, 100000 / NUM) {
  for (int i : co_ints<toby::generator<int>>(0, NUM)) {
    consume(i);
  }
}

BENCHMARK(cofoo, co_ints_gor, 1000, 100000 / NUM) {
  for (int i : co_ints<gor::generator<int>>(0, NUM)) {
    consume(i);
  }
}

BENCHMARK(cofoo, co_ints_atomic, 1000, 100000 / NUM) {
  for (int i : co_ints<toby::generator<int, std::atomic<int>>>(0, NUM)) {
    consume(i);
  }
}

BENCHMARK(cofoo, handrolled_ints, 1000, 100000 / NUM) {
  for (int i = 0; i < NUM; ++i) {
    consume(i);
  }
}

/*
BENCHMARK(co_ints, cb_ints, 1000, 100000/NUM) {
  cb_ints(0, NUM, [](int i) { consume(i); });
}
*/

BENCHMARK(cofoo, ranges, 1000, 1000000 / NUM) {
  for (auto i : ranges::view::ints(0, NUM)) {
    consume(i);
  }
}

int main() {
  hayai::ConsoleOutputter consoleOutputter;

  hayai::Benchmarker::AddOutputter(consoleOutputter);
  hayai::Benchmarker::RunAllTests();
  return 0;
}
