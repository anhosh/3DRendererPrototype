#include <Scene/Components/Camera.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

glm::mat4 CompCamera::projection(const glm::uvec2 screenSize) const {
  const glm::vec2 fScreenSize = screenSize;
  if (bOrthographic) {
    if (bUseFOVAsScreenSize) {
      return glm::ortho(-fov * 0.5f, fov * 0.5f,
                        -fov * 0.5f, fov * 0.5f,
                        near, far);
    }
    return glm::ortho(-fScreenSize.x * 0.5f, fScreenSize.x * 0.5f,
                      -fScreenSize.y * 0.5f, fScreenSize.y * 0.5f,
                      near, far);
  }
  return glm::perspective(glm::radians(fov), fScreenSize.x / fScreenSize.y, near, far);
}
