#pragma once

#include <array>
#include <type_traits>

template <typename T>
union IntoBytes {
  T value;
  std::array<uint8_t, sizeof(T)> bytes;
};

template <
  typename T,
  typename ArrayElement = std::remove_extent_t<T>,
  size_t N = !std::is_bounded_array_v<T>         ? sizeof(T) :
             (std::is_same_v<ArrayElement, char> ? std::extent_v<T> - 1 : std::extent_v<T>)
>
std::array<uint8_t, N> asBytes(const T& value) {
  if constexpr (std::is_bounded_array_v<T>) {
    std::array<uint8_t, N * sizeof(ArrayElement)> res;
    std::copy_n(value, N, res.data());
    return res;
  } else {
    const IntoBytes res = { .value = value };
    return res.bytes;
  }
}
