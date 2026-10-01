#pragma once

#include <atomic>
#include <cstddef>
#include <utility>

namespace rushevich {
namespace detail {
struct ctrl_block {
    std::atomic<size_t> strongCount {};
    std::atomic<size_t> weakCount {};

    // Increments the strong reference counter
    void incrementStrong() { strongCount.fetch_add(1, std::memory_order_relaxed); }

    // Decrements the strong reference counter and returns the new value
    size_t decrementStrong() { return strongCount.fetch_sub(1, std::memory_order_acq_rel) - 1; }
};
} // namespace detail

template <typename T> class shared_ptr {
public:
    shared_ptr() = default;

    shared_ptr(T* ptr)
        : _ptr { ptr },
          _ctrl { new detail::ctrl_block { .strongCount = 1, .weakCount = 0 } } {}

    shared_ptr(shared_ptr const& other) : _ptr { other._ptr }, _ctrl { other._ctrl } {
        if (_ctrl != nullptr) {
            _ctrl->incrementStrong();
        }
    }

    shared_ptr& operator=(shared_ptr const& other) {
        if (this != &other) {
            _tryDelete(); // first try to clean up the current object’s fields if needed
            _ptr = other._ptr;
            _ctrl = other._ctrl;
            if (_ctrl != nullptr) {
                _ctrl->incrementStrong();
            }
        }
        return *this;
    }

    shared_ptr(shared_ptr&& other)
        : _ptr { std::exchange(other._ptr, nullptr) },
          _ctrl { std::exchange(other._ctrl, nullptr) } {}

    shared_ptr& operator=(shared_ptr&& other) {
        if (this != &other) {
            _tryDelete();
            _ptr = other._ptr;
            other._ptr = nullptr;
            _ctrl = other._ctrl;
            other._ctrl = nullptr;
        }
        return *this;
    }

    ~shared_ptr() { _tryDelete(); }

    size_t refCount() { return _ctrl->strongCount.load(std::memory_order_relaxed); }

    T& operator*() { return *_ptr; }

    T* operator->() { return _ptr; }

private:
    T* _ptr { nullptr };
    detail::ctrl_block* _ctrl { nullptr };

    // Decrement’s the strong count and checks if the object or control block need
    // deletion
    void _tryDelete() {
        if (_ctrl != nullptr) {
            auto const strongCnt = _ctrl->decrementStrong();
            if (strongCnt == 0) {
                delete _ptr;
                _ptr = nullptr;
                if (_ctrl->weakCount.load(std::memory_order_acquire) == 0) {
                    delete _ctrl;
                    _ctrl = nullptr;
                }
            }
        }
    }
};
} // namespace rushevich
