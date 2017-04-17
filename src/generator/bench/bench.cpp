#include "bench.h"
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

void bench_ints_generator_toby(int n) {
  RANGES_FOR(int i, co_ints<toby::generator<int>>(0, n)) { consume(i); }
}

void bench_ints_generator_gor(int n) {
  for (int i : co_ints<gor::generator<int>>(0, n)) {
    consume(i);
  }
}

#ifdef HAS_EXPERIMENTAL_GENERATOR

#include <experimental/generator>

void bench_ints_generator_exp(int n) {
  for (int i : co_ints<std::experimental::generator<int>>(0, n)) {
    consume(i);
  }
}

#endif

void bench_ints_generator_toby_atomic(int n) {
  RANGES_FOR(int i, co_ints<toby::generator<int, std::atomic<int>>>(0, n)) { consume(i); }
}

void bench_ints_handrolled(int n) {
  for (int i = 0; i < n; ++i) {
    consume(i);
  }
}

void bench_ints_ranges(int n) {
  for (auto i : ranges::view::ints(0, n)) {
    consume(i);
  }
}

template <typename Generator, typename InputRange, typename UnaryPredicate>
auto co_remove_if(InputRange range, UnaryPredicate pred) -> Generator {
  RANGES_FOR(auto&& x, range) {
    if (pred(x)) {
      co_yield x;
    }
  }
}

template <typename Generator, typename UnaryPredicate>
auto co_remove_if(UnaryPredicate pred) {
  return ranges::make_pipeable([pred = std::move(pred)](auto&& rng) {
    return co_remove_if<Generator>(std::forward<decltype(rng)>(rng), std::move(pred));
  });
}

auto pred = [](int x) { return x % 2 == 0; };

void bench_filter_generator_toby(int n) {
  RANGES_FOR(
      int i,
      co_ints<toby::generator<int>>(0, n) | co_remove_if<toby::generator<int>>(pred)) {
    consume(i);
  }
}

void bench_filter_generator_gor(int n) {
  RANGES_FOR(
      int i,
      co_ints<gor::generator<int>>(0, n) | co_remove_if<gor::generator<int>>(pred)) {
    consume(i);
  }
}

#ifdef HAS_EXPERIMENTAL_GENERATOR
void bench_filter_generator_exp(int n) {
  RANGES_FOR(int i,
             co_ints<std::experimental::generator<int>>(0, n) |
                 co_remove_if<std::experimental::generator<int>>(pred)) {
    consume(i);
  }
}
#endif

void bench_filter_handrolled(int n) {
  for (auto i : ranges::view::ints(0, n)) {
    if (pred(i)) consume(i);
  }
}

void bench_filter_ranges(int n) {
  for (auto i : ranges::view::ints(0, n) | ranges::view::remove_if(pred)) {
    consume(i);
  }
}