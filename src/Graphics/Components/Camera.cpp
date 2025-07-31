#include <Graphics/Components/Camera.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

glm::mat4 CompCamera::projection(const glm::uvec2 screenSize) const {
  return glm::perspective(glm::radians(fov),
                          static_cast<float>(screenSize.x) / static_cast<float>(screenSize.y),
                          near, far);
}
