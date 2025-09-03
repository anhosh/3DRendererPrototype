#pragma once

#include <Util/Math/AABB.hpp>
#include <Util/Math/Rectangle.hpp>

struct CompTransform;
struct Frustum;

struct CompCamera {
  AABB clipBox = { glm::vec3(0.0f, 0.0f, 0.01f), glm::vec3(1.0f, 1.0f, 50.0f) };
  float fov = 45.0f;

  [[nodiscard]] Rectangle screenBounds() const { return { clipBox.min.xy(), clipBox.max.xy() }; }
  [[nodiscard]] glm::vec2 screenSize() const { return clipBox.min.xy() - clipBox.max.xy(); }
  [[nodiscard]] glm::mat4 perspective() const;
  [[nodiscard]] glm::mat4 orthographic() const;
  [[nodiscard]] Frustum viewFrustumPerspective(const CompTransform& viewTransform) const;
  [[nodiscard]] Frustum viewFrustumOrthographic(const CompTransform& viewTransform) const;
};
