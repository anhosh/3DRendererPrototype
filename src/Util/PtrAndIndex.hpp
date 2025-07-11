#pragma once

#include <Util/NotNull.hpp>

template <typename T>
struct PtrAndIndex {
  T* ptr = nullptr;
  size_t index = SIZE_MAX;

  T& operator*() const {
    return *NotNull(ptr);
  }

  T* operator->() const {
    return NotNull(ptr);
  }
};
