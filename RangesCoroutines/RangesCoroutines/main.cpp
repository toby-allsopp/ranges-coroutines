#define SPDLOG_FMT_EXTERNAL
#include "generator.h"

#include <range/v3/all.hpp>

#include <spdlog/spdlog.h>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <vector>

auto logger = spdlog::stdout_logger_mt("STDOUT");

generator<int> cofoo() {
  logger->info("before loop");
  for (int i = 0; i < 5; ++i) {
    logger->info("about to yield: {}", i);
    co_yield i;
  }
  logger->info("after loop");
}

template <typename InputRange, typename UnaryPredicate>
auto filter_co(InputRange range, UnaryPredicate pred)
    -> generator<ranges::range_value_t<InputRange>> {
  for (auto&& x : range) {
    if (pred(x)) {
      co_yield x;
    }
  }
}

template <typename UnaryPredicate>
auto filter_co(UnaryPredicate pred) {
  return ranges::make_pipeable([pred = std::move(pred)](auto&& rng) {
    return filter_co(std::forward<decltype(rng)>(rng), std::move(pred));
  });
}

// A class that adapts an existing range with a function
template <class Rng, class Fun>
class filter_view : public ranges::view_adaptor<filter_view<Rng, Fun>, Rng> {
  friend ranges::range_access;
  ranges::semiregular_t<Fun> fun_;  // Make Fun model SemiRegular if it doesn't
  class adaptor : public ranges::adaptor_base {
    ranges::semiregular_t<Fun> fun_;
    ranges::range_sentinel_t<Rng> end_;

   public:
    adaptor() = default;
    adaptor(ranges::semiregular_t<Fun> const& fun, ranges::range_sentinel_t<Rng> end)
        : fun_(fun), end_(end) {}
    auto begin(filter_view& rng) const {
      return ranges::find_if(ranges::begin(rng.mutable_base()), end_, fun_);
    }
    void next(ranges::range_iterator_t<Rng>& it) const {
      ++it;
      it = ranges::find_if(it, end_, fun_);
    }
  };

  adaptor begin_adaptor() const { return {fun_, ranges::end(this->mutable_base())}; }

 public:
  filter_view() = default;
  filter_view(Rng&& rng, Fun fun)
      : filter_view::view_adaptor{std::forward<Rng>(rng)}, fun_(std::move(fun)) {}
};

template <class Rng, class Fun>
filter_view<Rng, Fun> filter_rv(Rng&& rng, Fun fun) {
  return {std::forward<Rng>(rng), std::move(fun)};
}

template <class Fun>
auto filter_rv(Fun fun) {
  return ranges::make_pipeable([fun = std::move(fun)](auto&& rng) {
    return filter_rv(std::forward<decltype(rng)>(rng), std::move(fun));
  });
}

#include <iostream>

int main() {
  doctest::Context context;  // initialize
  int res = context.run();   // run

  if (context.shouldExit())  // important - query flags (and --exit) rely on the user
                             // doing this
    return res;              // propagate the result of the tests

  for (auto&& x : cofoo()) {
    std::cout << x << "\n";
  }

  auto pred = [](int x) { return x % 2 == 0; };

  // filter a vector
  for (auto&& x : filter_co(std::vector<int>{1, 2, 3, 4, 5}, pred)) {
    std::cout << x << "\n";
  }

  // filter a generator using a generator
  for (auto&& x : filter_co(cofoo(), pred)) {
    std::cout << x << "\n";
  }
  for (auto&& x : cofoo() | filter_co(pred)) {
    std::cout << x << "\n";
  }

  // filter a generator using a range view
  for (auto&& x : filter_rv(cofoo(), pred)) {
    std::cout << x << "\n";
  }

  // filter a generator using a range view
  for (auto&& x : cofoo() | filter_rv(pred)) {
    std::cout << x << "\n";
  }

  // filter a generator using a range view
  CONCEPT_ASSERT(ranges::v3::InputRange<decltype(cofoo())>::value);
  CONCEPT_ASSERT(ranges::v3::Range<decltype(cofoo())>());
  CONCEPT_ASSERT(ranges::v3::MoveConstructible<decltype(cofoo())>());
  CONCEPT_ASSERT(ranges::v3::Movable<decltype(cofoo())>());
  CONCEPT_ASSERT(ranges::v3::CopyConstructible<decltype(cofoo())>());
  CONCEPT_ASSERT(ranges::v3::Copyable<decltype(cofoo())>());
  CONCEPT_ASSERT(ranges::v3::SemiRegular<decltype(cofoo())>());
  CONCEPT_ASSERT(ranges::v3::View<decltype(cofoo())>());

  for (auto&& x : cofoo() | ranges::view::remove_if(pred)) {
    std::cout << x << "\n";
  }

  for (auto&& x : ranges::view::remove_if(cofoo(), pred)) {
    std::cout << x << "\n";
  }
}