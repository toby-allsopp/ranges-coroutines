#pragma once

#include <experimental/coroutine>

/// An RAII-style shared-ownership wrapper for std::experimental::coroutine_handle.
///
/// Cooperation with the promise object is required, ala boost::intrusive_ptr.  The
/// promise object is required to have two member functions: `void add_ref()` and `int
/// del_ref()`.
template <class PromiseType>
class shared_coroutine_handle {
 public:
  shared_coroutine_handle() : m_coro(nullptr) {}
  shared_coroutine_handle(std::experimental::coroutine_handle<PromiseType> coro)
      : m_coro(coro) {
    if (m_coro) {
      m_coro.promise().add_ref();
    }
  }

  shared_coroutine_handle(const shared_coroutine_handle& other)
      : shared_coroutine_handle(other.m_coro) {}
  shared_coroutine_handle(shared_coroutine_handle&& other) : shared_coroutine_handle() {
    std::swap(m_coro, other.m_coro);
  }

  shared_coroutine_handle& operator=(const shared_coroutine_handle& other) {
    this->~shared_coroutine_handle();
    new (this) shared_coroutine_handle(other);
    return *this;
  }
  shared_coroutine_handle& operator=(shared_coroutine_handle&& other) {
    this->~shared_coroutine_handle();
    new (this) shared_coroutine_handle(std::move(other));
    return *this;
  }

  ~shared_coroutine_handle() {
    if (m_coro && m_coro.promise().del_ref() == 0) {
      m_coro.destroy();
    }
  }

  std::experimental::coroutine_handle<PromiseType>& operator*() { return m_coro; }
  std::experimental::coroutine_handle<PromiseType>* operator->() { return &m_coro; }

 private:
  std::experimental::coroutine_handle<PromiseType> m_coro;
};

template <class ElementType>
class generator {
 public:
  struct promise_type {
    ElementType currentElement;
    int ref_count{0};

    void add_ref() { ++ref_count; }
    int del_ref() { return --ref_count; }

    generator get_return_object() {
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
  generator(std::experimental::coroutine_handle<promise_type> coro) : m_coro(coro) {}

  auto begin() {
    m_coro->resume();
    return m_coro->done() ? end() : iterator{this};
  }
  auto end() { return iterator{}; }

 private:
  struct iterator {
    using value_type        = ElementType;
    using difference_type   = std::ptrdiff_t;
    using reference         = ElementType&;
    using pointer           = ElementType const*;
    using iterator_category = std::input_iterator_tag;

    iterator() : m_generator(nullptr) {}
    iterator(generator* gen) : m_generator(gen) {}

    bool operator==(const iterator& other) const {
      return m_generator == other.m_generator;
    }
    bool operator!=(const iterator& other) const { return !(*this == other); }

    iterator& operator++() {
      m_generator->m_coro->resume();
      if (m_generator->m_coro->done()) {
        m_generator = nullptr;  // makes *this == end()
      }
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
    ElementType& operator*() const {
      return m_generator->m_coro->promise().currentElement;
    }

    generator* m_generator;
  };

  shared_coroutine_handle<promise_type> m_coro;
};
