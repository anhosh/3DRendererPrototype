#pragma once

class Camera {
public:
  [[nodiscard]] glm::vec3 forward() const;
  [[nodiscard]] glm::vec3 up() const;

  [[nodiscard]] glm::mat4 view() const;
  [[nodiscard]] glm::mat4 projection(glm::uvec2 screenSize) const;

public:
  glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
  glm::vec3 rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
  float fov = 45.0f;
  float near = 0.1f;
  float far = 1000.0f;
  float speed = 2.0f;
};
