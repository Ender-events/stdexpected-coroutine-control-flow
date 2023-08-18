#include <expected>
#include <iostream>
#include <utility>

#include "expected.h"

namespace ender
{
expected<int, int> make_value(int i)
{
    co_return i;
}
expected<int, int> make_error(int i)
{
    co_return ender::unexpected<int>{std::move(i)};
}
expected<int, int> test_value()
{
    std::cout << "begin\n";
    auto v = co_await make_value(4);
    std::cout << "get value: " << v << '\n';
    co_return make_value(5);
}
expected<int, int> test_error()
{
    std::cout << "begin\n";
    auto v = co_await make_value(1);
    std::cout << "get value: " << v << '\n';
    v = co_await make_error(2);
    std::cout << "get error: " << v << '\n';
    co_return ender::unexpected{42};
    co_return co_await make_value(3);
}
} // namespace ender

auto main() -> int
{
    std::expected<int, int> tata{};
    auto exp = ender::test_error();
    std::cout << "main error: " << exp.error() << '\n';
    exp = ender::test_value();
    std::cout << "main value: " << exp.value() << '\n';
}
