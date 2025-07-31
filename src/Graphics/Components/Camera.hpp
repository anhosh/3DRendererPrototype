#pragma once

struct CompCamera {
  float fov = 45.0f;
  float near = 0.1f;
  float far = 1000.0f;
  float speed = 2.0f;

  [[nodiscard]] glm::mat4 projection(glm::uvec2 screenSize) const;
};
