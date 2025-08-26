#pragma once

#include <Util/Math/Rotation.hpp>

struct CompTransform {
  glm::vec3 translation = glm::vec3(0.0f);
  Rotation rotation = {};
  glm::vec3 scale = glm::vec3(1.0f);

  [[nodiscard]] glm::vec3 forward() const;
  [[nodiscard]] glm::vec3 right() const;
  [[nodiscard]] glm::vec3 up() const;
  [[nodiscard]] glm::mat4 viewMatrix() const;
  [[nodiscard]] glm::mat4 modelMatrix() const;

  bool operator==(const CompTransform&) const = default;
};
