#pragma once
#include <Util/Macros/Errors.hpp>

inline constexpr glm::vec3 DIRECTION_RIGHT   = { 1.0f, 0.0f, 0.0f };
inline constexpr glm::vec3 DIRECTION_UP      = { 0.0f, 1.0f, 0.0f };
inline constexpr glm::vec3 DIRECTION_FORWARD = { 0.0f, 0.0f, 1.0f };

template <int32_t N, typename T> requires (N >= 1)
bool vecXLess(const glm::vec<N, T> a, const glm::vec<N, T> b) { return a.x < b.x; }

template <int32_t N, typename T> requires (N >= 2)
bool vecYLess(const glm::vec<N, T> a, const glm::vec<N, T> b) { return a.y < b.y; }

template <int32_t N, typename T> requires (N >= 3)
bool vecZLess(const glm::vec<N, T> a, const glm::vec<N, T> b) { return a.z < b.z; }

template <int32_t N, typename T> requires (N >= 4)
bool vecWLess(const glm::vec<N, T> a, const glm::vec<N, T> b) { return a.w < b.w; }

template <int32_t N, typename T>
bool isInRangeInclusive(const glm::vec<N, T> vec, const glm::vec<N, T> min, const glm::vec<N, T> max) {
  if constexpr (N == 1) {
    return vec.x >= min.x && vec.x <= max.x;
  } else if constexpr (N == 2) {
    return vec.x >= min.x && vec.x <= max.x &&
           vec.y >= min.y && vec.y <= max.y;
  } else if constexpr (N == 3) {
    return vec.x >= min.x && vec.x <= max.x &&
           vec.y >= min.y && vec.y <= max.y &&
           vec.z >= min.z && vec.z <= max.z;
  } else if constexpr (N == 4) {
    return vec.x >= min.x && vec.x <= max.x &&
           vec.y >= min.y && vec.y <= max.y &&
           vec.z >= min.z && vec.z <= max.z &&
           vec.w >= min.w && vec.w <= max.w;
  }
  UNREACHABLE();
}

template <int32_t N, typename T>
bool isInRangeExclusive(const glm::vec<N, T> vec, const glm::vec<N, T> min, const glm::vec<N, T> max) {
  if constexpr (N == 1) {
    return vec.x > min.x && vec.x < max.x;
  } else if constexpr (N == 2) {
    return vec.x > min.x && vec.x < max.x &&
           vec.y > min.y && vec.y < max.y;
  } else if constexpr (N == 3) {
    return vec.x > min.x && vec.x < max.x &&
           vec.y > min.y && vec.y < max.y &&
           vec.z > min.z && vec.z < max.z;
  } else if constexpr (N == 4) {
    return vec.x > min.x && vec.x < max.x &&
           vec.y > min.y && vec.y < max.y &&
           vec.z > min.z && vec.z < max.z &&
           vec.w > min.w && vec.w < max.w;
  }
  UNREACHABLE();
}
