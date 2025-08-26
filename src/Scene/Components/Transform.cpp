#include <Scene/Components/Transform.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

glm::vec3 CompTransform::forward() const {
  // return rotation.asMat3() * FORWARD_AXIS;
  return {
    glm::cos(glm::radians(rotation.pitch)) * glm::cos(glm::radians(rotation.yaw)),
    glm::sin(glm::radians(-rotation.pitch)),
    glm::cos(glm::radians(rotation.pitch)) * glm::sin(glm::radians(rotation.yaw)),
  };
}

glm::vec3 CompTransform::right() const {
  // return rotation.asMat3() * RIGHT_AXIS;
  return {
    -glm::sin(glm::radians(rotation.yaw)),
    0.0f,
    glm::cos(glm::radians(rotation.yaw)),
  };
}

glm::vec3 CompTransform::up() const {
  // return rotation.asMat3() * UP_AXIS;
  return {
    glm::sin(glm::radians(rotation.pitch)) * glm::cos(glm::radians(rotation.yaw)),
    glm::cos(glm::radians(rotation.pitch)),
    glm::sin(glm::radians(rotation.pitch)) * glm::sin(glm::radians(rotation.yaw)),
  };
}

glm::mat4 CompTransform::viewMatrix() const {
  return glm::lookAt(translation, translation + this->forward(), UP_AXIS);
}

glm::mat4 CompTransform::modelMatrix() const {
  glm::mat4 model(1.0f);
  model = glm::translate(model, translation);
  model = glm::scale(model, scale);
  model *= rotation.asMat4();
  return model;
}
