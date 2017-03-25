#pragma once

#include <experimental/coroutine>

template <class PromiseType>
using shared_coroutine_handle =
    std::shared_ptr<std::experimental::coroutine_handle<PromiseType>>;

template <class PromiseType>
shared_coroutine_handle<PromiseType> make_shared_coroutine_handle(
    std::experimental::coroutine_handle<PromiseType> coro) {
  return {new std::experimental::coroutine_handle<PromiseType>(coro),
          [](std::experimental::coroutine_handle<PromiseType>* p) { p->destroy(); }};
}

template <class ElementType>
class generator {
 public:
  struct promise_type {
    ElementType currentElement;

    auto get_return_object() {
      return generator{
          std::experimental::coroutine_handle<promise_type>::from_promise(*this)};
    }
    auto initial_suspend() { return std::experimental::suspend_always{}; }
    auto yield_value(ElementType element) {
      currentElement = std::move(element);
      return std::experimental::suspend_always{};
    }
    auto final_suspend() { return std::experimental::suspend_always{}; }
  };

  generator() = default;
  generator(std::experimental::coroutine_handle<promise_type> coro)
      : coro(make_shared_coroutine_handle<promise_type>(coro)) {}

  auto begin() {
    coro->resume();
    return coro->done() ? end() : iterator{coro};
  }
  auto end() { return iterator{}; }

 private:
  struct iterator {
    using value_type        = ElementType;
    using difference_type   = std::ptrdiff_t;
    using reference         = ElementType&;
    using pointer           = ElementType const*;
    using iterator_category = std::input_iterator_tag;

    iterator() = default;
    iterator(shared_coroutine_handle<promise_type> coro) : coro(coro) {}

    bool operator==(const iterator& other) const { return coro == other.coro; }
    bool operator!=(const iterator& other) const { return !(*this == other); }
    iterator& operator++() {
      coro->resume();
      if (coro->done()) coro.reset(); // makes *this == end()
      return *this;
    }
    // we need to make auto x = *i++ equivalent to auto x = *i; ++i;
    auto operator++(int) {
      struct proxy {
        using value_type = ElementType;
        ElementType element;
        const ElementType& operator*() const { return element; }
        ElementType& operator*() { return element; }
      };
      proxy ret{std::move(**this)};
      ++(*this);
      return ret;
    }
    reference operator*() const { return coro->promise().currentElement; }
    //pointer operator->() const { return &coro->promise().currentElement; }

    shared_coroutine_handle<promise_type> coro;
  };

  shared_coroutine_handle<promise_type> coro;
};
