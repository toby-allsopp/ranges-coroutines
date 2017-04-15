#include "generator.h"
#include "gor_generator.h"

#include <range/v3/all.hpp>

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

void bench_co_ints_toby(int n) {
  RANGES_FOR(int i, co_ints<toby::generator<int>>(0, n)) { consume(i); }
}

void bench_co_ints_gor(int n) {
  for (int i : co_ints<gor::generator<int>>(0, n)) {
    consume(i);
  }
}

#ifdef HAS_EXPERIMENTAL_GENERATOR

#include <experimental/generator>

void bench_co_ints_exp(int n) {
  for (int i : co_ints<std::experimental::generator<int>>(0, n)) {
    consume(i);
  }
}

#endif

void bench_co_ints_atomic(int n) {
  RANGES_FOR(int i, co_ints<toby::generator<int, std::atomic<int>>>(0, n)) { consume(i); }
}

void bench_handrolled_ints(int n) {
  for (int i = 0; i < n; ++i) {
    consume(i);
  }
}

void bench_ranges_ints(int n) {
  for (auto i : ranges::view::ints(0, n)) {
    consume(i);
  }
}
