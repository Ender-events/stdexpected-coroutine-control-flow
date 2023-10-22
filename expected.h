#pragma once

#include <cassert>
#include <coroutine>
#include <iostream>
#include <type_traits>
#include <utility>
#include <variant>

template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

namespace ender
{
template <typename E>
class unexpected
{
    E error_;

public:
    constexpr explicit unexpected(E&& error) : error_{std::move(error)}
    {}
    constexpr auto error() const& noexcept -> const E&
    {
        return error_;
    }
    constexpr auto error() & noexcept -> E&
    {
        return error_;
    }
    constexpr auto error() const&& noexcept -> const E&&
    {
        std::cout << "const E&&";
        return std::move(error_);
    }
    constexpr auto error() && noexcept -> E&&
    {
        std::cout << "E&&";
        return std::move(error_);
    }
    constexpr void swap(unexpected& other) noexcept(std::is_nothrow_swappable_v<E>)
    {
        std::swap(error_, other.error_);
    }
};

template <typename T, typename E>
class expected
{
public:
    using value_type = T;
    using error_type = E;

    expected(const expected& exp) noexcept = delete;
    expected& operator=(const expected& exp) noexcept = delete;
    expected(expected&& exp) noexcept // don't known why = default make a double free
    {
        std::swap(handle_, exp.handle_);
    };
    expected& operator=(expected&& exp) noexcept
    {
        std::swap(handle_, exp.handle_);
        return *this;
    };
    ~expected()
    {
        if (handle_)
        {
            handle_.destroy();
        }
    }

    constexpr auto error() const& noexcept -> const E&
    {
        // TODO: deducing this
        // std::cout << "error() " << this << " with " << handle_.address() << "\n";
        return std::get<unexpected<E>>(handle_.promise().data_).error();
    }
    constexpr auto error() & noexcept -> E&
    {
        return std::get<unexpected<E>>(handle_.promise().data_).error();
    }
    constexpr auto error() const&& noexcept -> const E&&
    {
        return std::get<unexpected<E>>(std::move(handle_.promise().data_)).error();
    }
    constexpr auto error() && noexcept -> E&&
    {
        return std::get<unexpected<E>>(std::move(handle_.promise().data_)).error();
    }
    constexpr auto value() const& noexcept -> const T&
    {
        // TODO: deducing this
        // std::cout << "error() " << this << " with " << handle_.address() << "\n";
        return std::get<T>(handle_.promise().data_);
    }
    constexpr auto value() & noexcept -> T&
    {
        return std::get<T>(handle_.promise().data_);
    }
    constexpr auto value() const&& noexcept -> const T&&
    {
        return std::get<T>(std::move(handle_.promise().data_));
    }
    constexpr auto value() && noexcept -> T&&
    {
        return std::get<T>(std::move(handle_.promise().data_));
    }

    constexpr auto to_std() && noexcept -> std::expected<T, E>
    {
        return std::visit(overloaded{[](T&& v) { return std::expected<T, E>{v}; },
                                     [](unexpected<E>&& e) {
                                         return std::expected<T, E>{
                                             std::unexpected<E>{std::move(e).error()}};
                                     }},
                          std::move(handle_.promise().data_));
    }

    struct promise_type
    {
        promise_type()
        {
            std::cout << "ctor promise_type\n";
        }
        promise_type(int a)
        {
            std::cout << "ctor promise_type " << a << "\n";
        }
        std::variant<T, unexpected<E>> data_;

        void* operator new(size_t c)
        {
            std::cout << "promise_type::new " << c << '\n';
            return std::malloc(c);
        }

        void operator delete(void* p)
        {
            std::cout << "promise_type::delete " << p << '\n';
            return std::free(p);
        }

        auto get_return_object() -> expected<T, E>
        {
            std::coroutine_handle<promise_type> handle =
                std::coroutine_handle<promise_type>::from_promise(*this);
            return expected<T, E>{handle};
        }
        auto initial_suspend() -> std::suspend_never
        {
            return {};
        }
        auto final_suspend() noexcept -> std::suspend_always
        {
            return {};
        }
        void return_value(unexpected<E>&& error)
        {
            // std::coroutine_handle<promise_type> handle =
            //     std::coroutine_handle<promise_type>::from_promise(*this);
            // std::cout << "return_value(unexpected<E>&& error): " << this << " with "
            //           << handle.address() << "\n";
            data_ = std::move(error);
        }
        void return_value(T&& value)
        {
            data_ = std::move(value);
        }
        void return_value(expected<T, E>&& exp)
        {
            data_ = std::move(exp.handle_.promise().data_);
        }
        void unhandled_exception()
        {}
    };

    auto await_ready() -> bool
    {
        assert(handle_);
        return std::holds_alternative<T>(handle_.promise().data_);
    }
    void await_suspend(std::coroutine_handle<promise_type> handle)
    {
        assert(handle_);
        // std::cout << "suspend error " << error() << " with " << handle.address() << "\n";
        handle.promise().data_ = std::move(handle_.promise().data_);
    }
    auto await_resume() -> T
    {
        assert(std::holds_alternative<T>(handle_.promise().data_) && "resume an unexpected value");
        return std::get<T>(handle_.promise().data_);
    }

private:
    friend promise_type;

    explicit expected(std::coroutine_handle<promise_type> handle) noexcept : handle_{handle}
    {
        std::cout << "ctor: " << this << " with " << handle.address() << "\n";
    }
    std::coroutine_handle<promise_type> handle_ = nullptr;
};

} // namespace ender
