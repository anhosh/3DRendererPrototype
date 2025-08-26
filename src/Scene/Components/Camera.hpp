#pragma once

struct CompTransform;
struct Frustum;

struct CompCamera {
  float fov = 45.0f;
  float near = 0.1f;
  float far = 300.0f;
  bool bOrthographic = false;
  bool bUseFOVAsScreenSize = false;

  [[nodiscard]] glm::mat4 projection(glm::uvec2 screenSize) const;
  [[nodiscard]] Frustum viewFrustum(CompTransform frustumTransform, glm::uvec2 screenSize) const;
};
