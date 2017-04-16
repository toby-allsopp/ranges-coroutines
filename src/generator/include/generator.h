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

  template <class ElementType, class RefCountType = int>
  struct generator_sentinel;
  template <class ElementType, class RefCountType = int>
  struct generator_iterator;

  template <class ElementType, class RefCountType = int>
  class generator {
   public:
    struct promise_type;
    using sentinel = generator_sentinel<ElementType, RefCountType>;
    using iterator = generator_iterator<ElementType, RefCountType>;

    generator() = default;
    generator(std::experimental::coroutine_handle<promise_type> coro) : m_coro(coro) {}

    auto begin() {
      m_coro->resume();
      return iterator{this};
    }
    auto end() { return sentinel{}; }

   private:
    intrusive_coroutine_handle<promise_type> m_coro;

    friend struct generator_iterator<ElementType, RefCountType>;
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

  template <class ElementType, class RefCountType>
  struct generator_sentinel {
    // Range-V3-VS2015 still requires these:
    bool operator==(const generator_sentinel&) const { return true; }
    bool operator!=(const generator_sentinel&) const { return false; }
  };

  template <class ElementType, class RefCountType>
  struct generator_iterator {
    using value_type        = ElementType;
    using difference_type   = std::ptrdiff_t;
    using reference         = ElementType&;
    using pointer           = ElementType*;
    using iterator_category = std::input_iterator_tag;

    using generator      = generator<ElementType, RefCountType>;
    generator_iterator() = default;
    generator_iterator(generator* gen) : m_generator(gen) {}

    bool operator==(const generator_sentinel<ElementType, RefCountType>& other) const {
      return m_generator->m_coro->done();
    }
    bool operator!=(const generator_sentinel<ElementType, RefCountType>& other) const {
      return !(*this == other);
    }

    // Range-V3-VS2015 still requires these:
    bool operator==(const generator_iterator&) const { return true; }
    bool operator!=(const generator_iterator&) const { return false; }

    generator_iterator& operator++() {
      m_generator->m_coro->resume();
      return *this;
    }

    void operator++(int) { ++(*this); }

    reference operator*() const { return m_generator->m_coro->promise().currentElement; }

    generator* m_generator;
  };

  template <class ElementType, class RefCountType>
  bool operator==(const generator_sentinel<ElementType, RefCountType>& s,
                  const generator_iterator<ElementType, RefCountType>& it) {
    return it == s;
  }
  template <class ElementType, class RefCountType>
  bool operator!=(const generator_sentinel<ElementType, RefCountType>& s,
                  const generator_iterator<ElementType, RefCountType>& it) {
    return it != s;
  }

}  // namespace toby

#include <range/v3/range_fwd.hpp>

#if !defined(RANGE_V3_VERSION) || RANGE_V3_VERSION < 200
namespace ranges {
  namespace v3 {
    template <class ElementType, class RefCountType>
    struct common_type<toby::generator_iterator<ElementType, RefCountType>,
                       toby::generator_sentinel<ElementType, RefCountType>> {
      using type = common_iterator<toby::generator_iterator<ElementType, RefCountType>,
                                   toby::generator_sentinel<ElementType, RefCountType>>;
    };
    template <class ElementType, class RefCountType>
    struct common_type<toby::generator_sentinel<ElementType, RefCountType>,
                       toby::generator_iterator<ElementType, RefCountType>> {
      using type = common_iterator<toby::generator_iterator<ElementType, RefCountType>,
                                   toby::generator_sentinel<ElementType, RefCountType>>;
    };
  }
}
#endif
