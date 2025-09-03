#pragma once

#include <array>

template <int32_t DIMENSIONS>
struct LineSegment {
  union {
    std::array<glm::vec<DIMENSIONS, float>, 2> points;
    struct { glm::vec<DIMENSIONS, float> a, b; };
  };
};

using LineSegment2D = LineSegment<2>;
using LineSegment3D = LineSegment<3>;
