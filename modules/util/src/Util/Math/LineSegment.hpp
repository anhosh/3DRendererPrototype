#pragma once

template <int32_t DIMENSIONS>
struct LineSegment {
  glm::vec<DIMENSIONS, float> a, b;
};

using LineSegment2D = LineSegment<2>;
using LineSegment3D = LineSegment<3>;
