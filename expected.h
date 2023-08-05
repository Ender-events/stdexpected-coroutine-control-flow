#pragma once

#include <iostream>
#include <type_traits>
#include <utility>
#include <variant>

namespace ender
{
struct unexpect_t
{

    explicit unexpect_t() = default;
};
inline constexpr unexpect_t unexpect{};
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

    template <typename... Args>
    expected(std::in_place_t, Args&&... args)
        : data_{std::in_place_type_t<T>{}, std::forward<Args>(args)...}
    {}

    template <typename... Args>
    expected(unexpect_t, Args&&... args)
        : data_{std::in_place_type_t<unexpected<E>>{}, std::forward<Args>(args)...}
    {}
    constexpr auto error() const& noexcept -> const E&
    {
        return std::get<unexpected<E>>(data_).error();
    }
    constexpr auto error() & noexcept -> E&
    {
        return std::get<unexpected<E>>(data_).error();
    }
    constexpr auto error() const&& noexcept -> const E&&
    {
        return std::get<unexpected<E>>(std::move(data_)).error();
    }
    constexpr auto error() && noexcept -> E&&
    {
        return std::get<unexpected<E>>(std::move(data_)).error();
    }

private:
    std::variant<T, unexpected<E>> data_;
};

} // namespace ender
