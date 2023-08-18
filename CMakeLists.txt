cmake_minimum_required(VERSION 3.14)

project(coro_expected
  VERSION 1.0
  DESCRIPTION "redo std::expected with coroutine control flow"
  LANGUAGES CXX
)
option(SANITIZE_ADDRESS "Enable address sanitizer" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(demo
    demo.cpp
)
target_compile_features(demo PUBLIC cxx_std_23)
target_compile_options(demo PUBLIC -Wall -Wextra -Wpedantic -Werror)

if(SANITIZE_ADDRESS)
    target_compile_options(demo PUBLIC "-fsanitize=address")
    target_link_options(demo PUBLIC "-fsanitize=address")
endif()

