# Helpers for Ranges + Coroutines #

This repository contains some helpers for and demonstrations of using Ranges and Coroutines together.

Currently this code has only been tested using Visual Studio 2017 as that is the only environment that easily supports Coroutines at the moment (March 2017). It should be possible to get this to work using recent nightly builds of Clang and libc++ also.

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

The `generator` class template included in this repository is intended to be a drop-in replacement for Microsoft's one. It has the advantage that it supports Ranges. It has the disadvantage that it is less efficient (it uses `std::shared_ptr`).

It allows you write code like:

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

# Dependencies #

To build the tests, you will need:

- Visual Studio 2017 (2015 might also work - not tested)
- range-v3 (I installed using vcpkg)
- spdlog
- doctest

The library itself is header-only and only depends on having an `<experimental/coroutine>` header available.