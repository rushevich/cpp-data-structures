#include <concepts>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace rushevich {

// a unique_ptr manages a, presumbably, dynamically allocated object and ensures that the underlying
// data is freed with the destruction of the unique_ptr itself
template <typename T, std::invocable<T*> Deleter = std::default_delete<T>> class unique_ptr {
public:
    using pointer = T*;
    explicit unique_ptr(T* raw, Deleter deleter = Deleter {})
        : _pointer { raw },
          _deleter { std::move(deleter) } {}

    template <typename U, typename E>
        requires std::is_convertible_v<typename unique_ptr<U, E>::pointer, pointer>
                     && std::is_assignable_v<Deleter&, E&&>
    explicit unique_ptr(unique_ptr<U, E>&& other) noexcept
        : _pointer { other.release() },
          _deleter { std::forward<E>(other.get_deleter()) } {}

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    unique_ptr(unique_ptr&& other) noexcept(
        std::is_nothrow_move_constructible_v<Deleter>
        && std::constructible_from<Deleter, decltype(other._deleter)>)
        : _pointer { other.release() },
          _deleter { std::move(other._deleter) } {}

    unique_ptr& operator=(unique_ptr&& other) noexcept(
        std::is_nothrow_move_assignable_v<Deleter>
        && std::assignable_from<Deleter, decltype(other._deleter)>) {
        if (this != &other) {
            reset(other._pointer);
            other._pointer = decltype(other._pointer) {};
            _deleter = std::move(other._deleter);
        }
        return *this;
    }

    ~unique_ptr() noexcept(std::is_nothrow_destructible_v<T>) {
        static_assert(std::is_nothrow_destructible_v<T>, "T must be nothrow destructible");
        reset();
    }

    T* release() { return std::exchange(_pointer, nullptr); }

    void reset(T* other = pointer()) noexcept(std::is_nothrow_destructible_v<T>) {
        T* old = std::exchange(_pointer, other);
        if (old != nullptr) {
            _deleter(old);
        }
    }

    T* get() noexcept { return _pointer; }

    auto get_deleter() noexcept { return _deleter; }

    std::add_lvalue_reference_t<T> operator*() const noexcept { return *_pointer; }

    T* operator->() const noexcept { return _pointer; }

    explicit operator bool() const noexcept { return _pointer != nullptr; }

    void swap(unique_ptr& other) noexcept {
        using std::swap;
        swap(_pointer, other._pointer);
        swap(_deleter, other._deleter);
        // we let ADL do the work... consider the case where the user provides their own deleter
        // that (for some reason), has a swap
    }

    friend void swap(unique_ptr& lhs, unique_ptr& rhs) { lhs.swap(rhs); }

private:
    T* _pointer { nullptr };
    [[no_unique_address]] Deleter
        _deleter {}; // We use no_unique_address in the event that this is a stateless deleter
};

template <typename T, typename... Args> unique_ptr<T> make_unique(Args&&... args) {
    static_assert(std::constructible_from<T, Args...>);
    return unique_ptr { new T { std::forward<Args>(args)... } };
}

} // namespace rushevich
