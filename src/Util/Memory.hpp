#pragma once

#include <concepts>

template <std::integral T>
T align(const T size, const T alignment) {
  return (size + alignment - 1) - ((size - 1) % alignment);
}

template <typename T = void>
T* ptrAtOffset(void* ptr, const std::size_t offset) {
  return reinterpret_cast<T*>(reinterpret_cast<std::size_t>(ptr) + offset);
}
