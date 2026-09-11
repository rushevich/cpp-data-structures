#pragma once
#include <cassert>
#include <cstddef>
#include <exception>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace rushevich {

/*
  An allocator-aware queue implemented as a ring-buffer
 */
template <typename T, typename Allocator = std::allocator<T>> class queue {
public:
  explicit queue(size_t cap) : _cap{cap} {
    if (_cap < 1) {
      _cap = 1;
    }
    _cap++; // for
    const auto [ptr, actual_cap] = ATraits::allocate_at_least(
        _alloc,
        _cap); // C++23 allows us to use this without compile-time checks
    _buf = ptr;
    _cap = actual_cap;
  }

  template <typename... Args>
  [[nodiscard]] bool try_emplace(Args &&...args) noexcept(
      std::is_nothrow_constructible_v<T, Args &&...>) {
    static_assert(
        std::is_constructible_v<T, Args &&...>,
        "T must be constructible with Args&&..."); // this assertion ensures
                                                   // that the passed in args
                                                   // are actually valid
    if (full()) {
      return false;
    }
    ATraits::construct(_alloc, _buf + _writeIdx, std::forward<Args>(args)...);
    _writeIdx = advance(_writeIdx);
    return true;
  }

  [[nodiscard]] bool
  try_push(const T &obj) noexcept(std::is_nothrow_copy_constructible_v<T>) {
    static_assert(std::is_copy_constructible_v<T>);
    return try_emplace(obj);
  }

  [[nodiscard]] bool
  try_push(T &&obj) noexcept(std::is_nothrow_move_constructible_v<T>) {
    static_assert(std::is_move_constructible_v<T>);
    return try_emplace(std::move(obj));
  }

  [[nodiscard]] T *front() const {
    if (empty()) {
      return nullptr;
    }
    return &_buf[_readIdx];
  }

  void pop() noexcept {
    static_assert(std::is_nothrow_destructible_v<T>);
    assert(!empty() && "Cannot pop when the queue is empty");
    ATraits::destroy(_alloc, _buf + _readIdx);
    _readIdx = advance(_readIdx);
  }

  ~queue() noexcept {
    static_assert(std::is_nothrow_destructible_v<T>);
    while (front() != nullptr) {
      pop();
    }
    ATraits::deallocate(_alloc, _buf, _cap);
  }

  bool empty() const { return _readIdx == _writeIdx; }

  size_t advance(size_t index) const {
    size_t next = index + 1;
    return (next == _cap) ? 0 : next;
  }

  bool full() const { return advance(_writeIdx) == _readIdx; }

  size_t size() const {
    std::ptrdiff_t diff = _writeIdx - _readIdx;
    if (diff < 0) {
      diff += _cap;
    }
    return diff;
  }

  size_t capacity() const { return _cap - 1; }

  queue(const queue &) = delete;
  queue(queue &&) = delete;
  queue &operator=(const queue &) = delete;
  queue &operator=(queue &&) = delete;

private:
  using ATraits = std::allocator_traits<Allocator>;
  [[no_unique_address]] Allocator _alloc;
  T *_buf{nullptr};
  size_t _cap{};
  size_t _readIdx{}; // I prefer this read and write nomenclature
  size_t _writeIdx{};
};
} // namespace rushevich
