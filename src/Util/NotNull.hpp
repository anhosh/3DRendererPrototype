#pragma once

#include <cassert>
#include <cstddef>

template <typename T>
class NotNull {
public:
  NotNull(std::nullptr_t) = delete;
  NotNull(T* ptr) : mPtr(ptr) {
    assert(mPtr != nullptr);
  }
  NotNull(const NotNull&) = default;
  NotNull(NotNull&&) = default;

  const T* get() const { return mPtr; }
  T* get() { return mPtr; }

  operator const T*() const { return mPtr; }
  operator T*() { return mPtr; }

  explicit operator bool() const { return mPtr != nullptr; }

  const T* operator->() const { return mPtr; }
  T* operator->() { return mPtr; }

  const T& operator*() const { return *mPtr; }
  T& operator*() { return *mPtr; }

  NotNull& operator=(std::nullptr_t) = delete;
  NotNull& operator=(T* ptr) {
    assert(ptr != nullptr);
    mPtr = ptr;
    return *this;
  }
  NotNull& operator=(const NotNull&) = default;
  NotNull& operator=(NotNull&&) = default;

private:
  T* mPtr;
};
