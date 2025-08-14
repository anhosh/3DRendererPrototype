#include <Scene/Components/Camera.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

glm::mat4 CompCamera::projection(const glm::uvec2 screenSize) const {
  if (bOrthographic) {
    return glm::ortho(-0.01f * static_cast<float>(screenSize.x), 0.01f * static_cast<float>(screenSize.x),
                      -0.01f * static_cast<float>(screenSize.y), 0.01f * static_cast<float>(screenSize.y),
                      near, far);
  }
  return glm::perspective(glm::radians(fov),
                          static_cast<float>(screenSize.x) / static_cast<float>(screenSize.y),
                          near, far);
}
