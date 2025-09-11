#pragma once

#include <array>

struct Triangle {
  std::array<glm::vec3, 3> points;

  [[nodiscard]] glm::vec3 a() const { return points[0]; }
  [[nodiscard]] glm::vec3 b() const { return points[1]; }
  [[nodiscard]] glm::vec3 c() const { return points[2]; }
  [[nodiscard]] float area() const;
  [[nodiscard]] bool operator==(const Triangle& other) const;
  [[nodiscard]] std::partial_ordering operator<=>(const Triangle& other) const;
};
