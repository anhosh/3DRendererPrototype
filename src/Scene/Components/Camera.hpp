#pragma once

#include <Util/Math/AABB.hpp>

struct CompTransform;
struct Frustum;

struct CompCamera {
  AABB clipBox = { glm::vec3(0.0f), glm::vec3(1.0f) };
  float fov = 45.0f;

  [[nodiscard]] glm::vec2 screenSize() const { return screenBounds.min - screenBounds.max; }
  [[nodiscard]] glm::mat4 perspective() const;
  [[nodiscard]] glm::mat4 orthographic() const;
  [[nodiscard]] Frustum viewFrustumPerspective(const CompTransform& viewTransform) const;
  [[nodiscard]] Frustum viewFrustumOrthographic(const CompTransform& viewTransform) const;
};
