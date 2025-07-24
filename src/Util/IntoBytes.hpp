#pragma once

#include <Util/Macros/Errors.hpp>

#include <array>
#include <variant>

template <typename T>
union IntoBytes {
  T value;
  std::array<uint8_t, sizeof(T)> bytes;
};

template <typename T>
std::array<uint8_t, sizeof(T)> asBytes(const T& value) {
  const IntoBytes res = { .value = value };
  return res.bytes;
}
