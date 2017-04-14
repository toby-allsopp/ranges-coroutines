#ifndef GOR_GENERATOR_H
#define GOR_GENERATOR_H

namespace gor {
  namespace coro = std::experimental;
  template <typename _Ty>
  struct generator {
    struct promise_type {
      _Ty current_value;
      coro::suspend_always yield_value(_Ty value) {
        this->current_value = value;
        return {};
      }
      coro::suspend_always initial_suspend() { return {}; }
      coro::suspend_always final_suspend() { return {}; }
      generator get_return_object() { return generator{this}; }
      void return_void() {}
    };

    struct iterator {
      coro::coroutine_handle<promise_type> _Coro;
      bool _Done;

      iterator(coro::coroutine_handle<promise_type> Coro, bool Done)
          : _Coro(Coro), _Done(Done) {}

      iterator& operator++() {
        _Coro.resume();
        _Done = _Coro.done();
        return *this;
      }

      bool operator==(iterator const& _Right) const { return _Done == _Right._Done; }

      bool operator!=(iterator const& _Right) const { return !(*this == _Right); }

      _Ty const& operator*() const { return _Coro.promise().current_value; }

      _Ty const* operator->() const { return &(operator*()); }
    };

    iterator begin() {
      p.resume();
      return {p, p.done()};
    }

    iterator end() { return {p, true}; }

    generator(generator&& rhs) : p(rhs.p) { rhs.p = nullptr; }

    ~generator() {
      if (p) p.destroy();
    }

   private:
    explicit generator(promise_type* p)
        : p(coro::coroutine_handle<promise_type>::from_promise(*p)) {}

    coro::coroutine_handle<promise_type> p;
  };
}

#endif  // GOR_GENERATOR_H
