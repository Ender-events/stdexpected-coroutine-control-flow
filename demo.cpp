#include "expected.h"
#include <utility>

auto main() -> int
{
    ender::expected<int, int> test{ender::unexpect, 42};
    return std::move(test).error();
}
