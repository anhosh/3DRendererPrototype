#pragma once

#include <array>

struct Triangle {
  union {
    std::array<glm::vec3, 3> points;
    struct { glm::vec3 a, b, c; };
  };

  [[nodiscard]] float area() const;
  [[nodiscard]] bool operator==(const Triangle& other) const;
  [[nodiscard]] std::partial_ordering operator<=>(const Triangle& other) const;
};
