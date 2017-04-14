#pragma once

#include <iterator>
#include <utility>

#if !USE_MY_COROUTINE_HEADER
#include <experimental/coroutine>
#else
#include "coroutine.h"
#endif

namespace toby {
  /// An RAII-style shared-ownership wrapper for
  /// std::experimental::coroutine_handle.
  ///
  /// Cooperation with the promise object is required, ala boost::intrusive_ptr.
  /// The
  /// promise object is required to have two member functions: `void add_ref()`
  /// and `int
  /// del_ref()`.
  template <class PromiseType>
  class intrusive_coroutine_handle {
   public:
    intrusive_coroutine_handle() : m_coro(nullptr) {}
    intrusive_coroutine_handle(std::experimental::coroutine_handle<PromiseType> coro)
        : m_coro(coro) {
      intrusive_coroutine_handle_add_ref(m_coro);
    }

    intrusive_coroutine_handle(const intrusive_coroutine_handle& other)
        : intrusive_coroutine_handle(other.m_coro) {}
    intrusive_coroutine_handle(intrusive_coroutine_handle&& other)
        : intrusive_coroutine_handle() {
      std::swap(m_coro, other.m_coro);
    }

    intrusive_coroutine_handle& operator=(const intrusive_coroutine_handle& other) {
      this->~intrusive_coroutine_handle();
      new (this) intrusive_coroutine_handle(other);
      return *this;
    }
    intrusive_coroutine_handle& operator=(intrusive_coroutine_handle&& other) {
      this->~intrusive_coroutine_handle();
      new (this) intrusive_coroutine_handle(std::move(other));
      return *this;
    }

    ~intrusive_coroutine_handle() { intrusive_coroutine_handle_release(m_coro); }

    std::experimental::coroutine_handle<PromiseType>& operator*() { return m_coro; }
    std::experimental::coroutine_handle<PromiseType>* operator->() { return &m_coro; }

   private:
    std::experimental::coroutine_handle<PromiseType> m_coro;
  };

  template <class ElementType, class RefCountType = int>
  class generator {
   public:
    struct promise_type;

    generator() = default;
    generator(std::experimental::coroutine_handle<promise_type> coro) : m_coro(coro) {}

    auto begin() {
      m_coro->resume();
      return iterator{this, m_coro->done()};
    }
    auto end() { return iterator{this, true}; }

   private:
    struct iterator {
      using value_type        = ElementType;
      using difference_type   = std::ptrdiff_t;
      using reference         = ElementType&;
      using pointer           = ElementType*;
      using iterator_category = std::input_iterator_tag;

      iterator() : m_generator(nullptr), m_done(true) {}
      iterator(generator* gen, bool done) : m_generator(gen), m_done(done) {}

      bool operator==(const iterator& other) const { return m_done == other.m_done; }
      bool operator!=(const iterator& other) const { return !(*this == other); }

      iterator& operator++() {
        m_generator->m_coro->resume();
        m_done = m_generator->m_coro->done();
        return *this;
      }

      // we need to make auto x = *i++ equivalent to auto x = *i; ++i;
      // until P0541
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

      reference operator*() const {
        return m_generator->m_coro->promise().currentElement;
      }

      generator* m_generator;
      bool m_done;
    };

    intrusive_coroutine_handle<promise_type> m_coro;
  };

  template <class ElementType, class RefCountType>
  struct generator<ElementType, RefCountType>::promise_type {
    ElementType currentElement;
    RefCountType ref_count{0};

    void add_ref() { ++ref_count; }
    auto del_ref() { return --ref_count; }

    generator get_return_object() {
      return generator{
          std::experimental::coroutine_handle<promise_type>::from_promise(*this)};
    }
    auto initial_suspend() { return std::experimental::suspend_always{}; }
    auto yield_value(ElementType element) {
      currentElement = std::move(element);
      return std::experimental::suspend_always{};
    }
    void return_void() {}
    auto final_suspend() { return std::experimental::suspend_always{}; }
  };

  template <typename PromiseType>
  void intrusive_coroutine_handle_add_ref(
      std::experimental::coroutine_handle<PromiseType> coro) {
    if (coro) coro.promise().add_ref();
  }

  template <typename PromiseType>
  void intrusive_coroutine_handle_release(
      std::experimental::coroutine_handle<PromiseType> coro) {
    if (coro && coro.promise().del_ref() == 0) coro.destroy();
  }

}  // namespace toby
