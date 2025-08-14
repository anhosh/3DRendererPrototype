#pragma once

#include <array>
#include <concepts>

template <typename T, size_t N>
class MultiBuffer {
public:
  template <typename> requires std::default_initializable<T>
  MultiBuffer()
  {
  }

  explicit MultiBuffer(T&& initial) : mBuffers(initial) {}

  template <typename... Ts> requires (std::same_as<T, Ts> && ...)
  explicit MultiBuffer(Ts&&... initialBuffers) : mBuffers({ initialBuffers... }) {}

  [[nodiscard]] T& current() {
    return mBuffers[mCurrent];
  }

  [[nodiscard]] const T& current() const {
    return mBuffers[mCurrent];
  }

  void switchToNext() {
    mCurrent = (mCurrent + 1) % N;
  }

private:
  std::array<T, N> mBuffers;
  size_t mCurrent = 0;
};

template <typename T>
using DoubleBuffer = MultiBuffer<T, 2>;
template <typename T>
using TripleBuffer = MultiBuffer<T, 3>;
