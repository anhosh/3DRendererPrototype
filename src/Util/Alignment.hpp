#pragma once

inline size_t align(const size_t size, const size_t alignment) {
  return (size + alignment - 1) - ((size - 1) % alignment);
}
