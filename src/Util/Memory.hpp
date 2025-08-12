#pragma once

inline size_t align(const size_t size, const size_t alignment) {
  return (size + alignment - 1) - ((size - 1) % alignment);
}

template <typename T = void>
T* ptrAtOffset(void* ptr, const size_t offset) {
  return reinterpret_cast<T*>(reinterpret_cast<size_t>(ptr) + offset);
}
