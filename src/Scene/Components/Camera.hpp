#pragma once

struct CompTransform;
struct Frustum;

struct CompCamera {
  glm::vec2 screenSize = glm::vec2(1.0f);
  float fov = 45.0f;
  float near = 0.1f;
  float far = 300.0f;

  [[nodiscard]] glm::mat4 perspective() const;
  [[nodiscard]] glm::mat4 orthographic() const;
  [[nodiscard]] Frustum viewFrustumPerspective(const CompTransform& viewTransform) const;
  [[nodiscard]] Frustum viewFrustumOrthographic(const CompTransform& viewTransform) const;
};
