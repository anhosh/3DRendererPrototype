#include <Camera.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

glm::vec3 Camera::forward() const {
  return {
    glm::cos(glm::radians(rotation.y)) * glm::cos(glm::radians(rotation.x)),
    glm::sin(glm::radians(rotation.y)),
    glm::cos(glm::radians(rotation.y)) * glm::sin(glm::radians(rotation.x)),
  };
}

glm::vec3 Camera::up() const {
  // return {
  //   glm::sin(glm::radians(rotation.z)) * glm::cos(glm::radians(rotation.y)),
  //   glm::cos(glm::radians(rotation.z)),
  //   glm::sin(glm::radians(rotation.z)) * glm::sin(glm::radians(rotation.y)),
  // };
  return { 0.0f, 1.0f, 0.0f };
}

glm::mat4 Camera::view() const {
  return glm::lookAt(position, position + this->forward(), this->up());
}

glm::mat4 Camera::projection(glm::uvec2 screenSize) const {
  return glm::perspective(glm::radians(fov),
                          static_cast<float>(screenSize.x) / static_cast<float>(screenSize.y),
                          near, far);
}
