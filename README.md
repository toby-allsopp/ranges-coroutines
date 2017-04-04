# Helpers for Ranges + Coroutines #

This repository contains some helpers for and demonstrations of using Ranges and Coroutines together.

Currently this code has only been tested using Visual Studio 2017 and a build of specific branches of clang and libc++.

## `generator` - a Range-friendly generator

Included in Microsoft's Coroutines support library is the class template `std::experimental::generator`. This allows a coroutine to return a lazy sequence of values using `co_yield`.

This allows code like:

```c++
generator<int> infinite_sequence() {
  for (int i = 0; ; ++i) {
    co_yield i;
  }
}

for (int i : infinite_sequence()) {
  cout << i;
  if (i > 10) break;
}
```

This is wonderful, however these `generator` objects can't be using alongside the also wonderful range-v3 library. This is due to `generator` not satisfying all of the requirements of the `Range` concept (see https://github.com/Microsoft/Range-V3-VS2015/issues/12).

The `generator` class template included in this repository is intended to be a drop-in replacement for Microsoft's one. It has the advantage that it supports Ranges. It has the disadvantage that it is ever-so-slightly less efficient because it has to maintain a reference count in order to support copying.

It allows you to write code like:

```c++
generator<int> infinite_sequence() {
  for (int i = 0; ; ++i) {
    co_yield i;
  }
}

infinite_sequence()
  | take(10)
  | for_each([](int x) { cout << x; });
```

Also, it's easy to write an adaptor using a coroutine:

```c++
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

infinite_sequence()
  | filter_co([](int x) { return x % 2 == 0; })
  | take(10)
  | for_each([](int x) { cout << x; });
```

# Dependencies #

To build the tests, you will need either:

- Visual Studio 2017 (2015 might also work - not tested)
- Clang and libc++ built from particular branches (see below)

The following are included:
- range-v3
- spdlog (currently not)
- doctest

The library itself is header-only and only depends on having an `<experimental/coroutine>` header available.

# Building #

## Visual Studio ##

## Clang ##

Currently, you will need to build your own clang and libc++. Hopefully this stuff will get merged into those projects' trunks soon.

### Checkout llvm, clang, libcxx and libcxxabi

    cd $SRC_ROOT # you choose this directory
    git clone -b merge0307 https://github.com/GorNishanov/llvm.git
    cd llvm/tools
    git clone -b merge0307 https://github.com/GorNishanov/clang.git
    cd ../projects
    git clone -b coroutines https://github.com/efcs/libcxx.git
    git clone https://github.com/llvm-mirror/libcxxabi.git
    
### Configure

    cd $SRC_ROOT
    mkdir build
    cd build
    cmake ..
    
### Build

    cd $SRC_ROOT/build
    make # use -jN if you have lots of RAM
    
### Configure generator

    cd $LOCATION_OF_THIS_FILE
    mkdir build
    cd build
    cmake CXX=$SRC_ROOT/build/bin/clang++ ..
    
### Build generator

    make
    
### Run tests

    LD_LIBRARY_PATH=$SRC_ROOT/build/lib ./generator/generator_test
