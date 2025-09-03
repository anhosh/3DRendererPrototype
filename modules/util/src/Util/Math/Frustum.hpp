#pragma once

#include <array>

struct AABB;

struct Frustum {
  glm::vec3 farBottomLeft;
  glm::vec3 farBottomRight;
  glm::vec3 farTopLeft;
  glm::vec3 farTopRight;
  glm::vec3 nearBottomLeft;
  glm::vec3 nearBottomRight;
  glm::vec3 nearTopLeft;
  glm::vec3 nearTopRight;

  [[nodiscard]] static Frustum fromAABB(const AABB& box);

  [[nodiscard]] Frustum transform(const glm::mat4& transform) const;
  [[nodiscard]] std::array<glm::vec3, 8> asArray() const;
  [[nodiscard]] AABB bounds() const;
};
