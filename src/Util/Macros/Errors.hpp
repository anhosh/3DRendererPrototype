#pragma once

#include <Util/Macros/Common.hpp>

#include <cassert>
#include <expected>
#include <print>

#define UNREACHABLE() \
  TO_STATEMENT( \
    std::println(stderr, "Reached unreachable code at {}:{}, {}:", __FILE__, __LINE__, __FUNCTION__); \
    assert(false); \
  )

#define PANIC(fmt, ...) \
  TO_STATEMENT( \
    std::println(stderr, "Program panicked at {}:{}, {}:", __FILE__, __LINE__, __FUNCTION__); \
    std::println(stderr, fmt __VA_OPT__(,) __VA_ARGS__); \
    assert(false); \
  )

#define RETURN_ERROR_IF_UNEXPECTED(expr) \
  TO_STATEMENT( \
    if (const auto& result = (expr); !result.has_value()) { \
      return std::unexpected(std::move(result.error())); \
    } \
  )

#define PANIC_IF_UNEXPECTED(expr) \
  TO_STATEMENT( \
    if (const auto& result = (expr); !result.has_value()) { \
      PANIC("{}", result.error()); \
    } \
  )

#define ASSIGN_EXPECTED_OR_RETURN(variable, expr) \
  TO_STATEMENT( \
    if (const auto& result = (expr); result.has_value()) { \
      variable = std::move(expr.value()); \
    } else { \
      return std::unexpected(std::move(result.error())); \
    } \
  )

#define ASSIGN_EXPECTED_OR_IGNORE(variable, expr) \
  TO_STATEMENT( \
    if (const auto& result = (expr); result.has_value()) { \
      variable = std::move(expr.value()); \
    } \
  )
