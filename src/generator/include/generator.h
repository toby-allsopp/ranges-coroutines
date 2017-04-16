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
  /// Cooperation with the promise object is required in order to maintain the reference
  /// count, ala boost::intrusive_ptr.
  ///
  /// The reference count in increased by calling `intrusive_coroutine_handle_add_ref` and
  /// decreased by calling `intrusive_coroutine_handle_release`. These functions are found
  /// via ADL based on the promise type.
  template <class PromiseType>
  class intrusive_coroutine_handle {
   public:
    using handle = std::experimental::coroutine_handle<PromiseType>;

    intrusive_coroutine_handle() : m_coro(nullptr) {}
    intrusive_coroutine_handle(handle coro) : m_coro(coro) {
      if (m_coro) intrusive_coroutine_handle_add_ref(m_coro.promise());
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

    ~intrusive_coroutine_handle() {
      if (m_coro && intrusive_coroutine_handle_release(m_coro.promise()) == 0)
        m_coro.destroy();
    }

    handle& operator*() { return m_coro; }
    handle* operator->() { return &m_coro; }

   private:
    handle m_coro;
  };

  struct generator_sentinel {
    // Range-V3-VS2015 still requires these:
    bool operator==(const generator_sentinel&) const { return true; }
    bool operator!=(const generator_sentinel&) const { return false; }
  };

  template <class PromiseType>
  struct generator_iterator;

  template <class ElementType, class RefCountType = int>
  class generator {
   public:
    struct promise_type;

    generator() = default;
    generator(std::experimental::coroutine_handle<promise_type> coro) : m_coro(coro) {}

    auto begin() {
      m_coro->resume();
      return generator_iterator<promise_type>{*m_coro};
    }
    auto end() { return generator_sentinel{}; }

   private:
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
  void intrusive_coroutine_handle_add_ref(PromiseType& promise) {
    promise.add_ref();
  }

  template <typename PromiseType>
  int intrusive_coroutine_handle_release(PromiseType& promise) {
    return promise.del_ref();
  }

  template <class PromiseType>
  struct generator_iterator {
    using ElementType       = decltype(std::declval<PromiseType>().currentElement);
    using value_type        = ElementType;
    using difference_type   = std::ptrdiff_t;
    using reference         = ElementType&;
    using pointer           = ElementType*;
    using iterator_category = std::input_iterator_tag;

    generator_iterator() = default;
    generator_iterator(std::experimental::coroutine_handle<PromiseType> coro)
        : m_coro(coro) {}

    bool operator==(const generator_sentinel&) const { return m_coro.done(); }
    bool operator!=(const generator_sentinel& other) const { return !(*this == other); }

    // Range-V3-VS2015 still requires these:
    bool operator==(const generator_iterator&) const { return true; }
    bool operator!=(const generator_iterator&) const { return false; }

    generator_iterator& operator++() {
      m_coro.resume();
      return *this;
    }

    void operator++(int) { ++(*this); }

    reference operator*() const { return m_coro.promise().currentElement; }

    std::experimental::coroutine_handle<PromiseType> m_coro;
  };

  template <class PromiseType>
  bool operator==(const generator_sentinel& s,
                  const generator_iterator<PromiseType>& it) {
    return it == s;
  }
  template <class PromiseType>
  bool operator!=(const generator_sentinel& s,
                  const generator_iterator<PromiseType>& it) {
    return it != s;
  }

}  // namespace toby

#include <range/v3/range_fwd.hpp>

#if !defined(RANGE_V3_VERSION) || RANGE_V3_VERSION < 200
namespace ranges {
  namespace v3 {
    template <class PromiseType>
    struct common_type<toby::generator_iterator<PromiseType>, toby::generator_sentinel> {
      using type = common_iterator<toby::generator_iterator<PromiseType>,
                                   toby::generator_sentinel>;
    };
    template <class PromiseType>
    struct common_type<toby::generator_sentinel, toby::generator_iterator<PromiseType>> {
      using type = common_iterator<toby::generator_iterator<PromiseType>,
                                   toby::generator_sentinel>;
    };
  }
}
#endif
