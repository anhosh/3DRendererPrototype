#include <Graphics/Components/Transform.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

glm::vec3 CompTransform::forward() const {
  return {
    glm::cos(glm::radians(rotation.y)) * glm::cos(glm::radians(rotation.x)),
    glm::sin(glm::radians(rotation.y)),
    glm::cos(glm::radians(rotation.y)) * glm::sin(glm::radians(rotation.x)),
  };
}

glm::vec3 CompTransform::up() const {
  // return {
  //   glm::sin(glm::radians(rotation.z)) * glm::cos(glm::radians(rotation.y)),
  //   glm::cos(glm::radians(rotation.z)),
  //   glm::sin(glm::radians(rotation.z)) * glm::sin(glm::radians(rotation.y)),
  // };
  return { 0.0f, 1.0f, 0.0f };
}

glm::mat4 CompTransform::viewMatrix() const {
  return glm::lookAt(translation, translation + this->forward(), this->up());
}

glm::mat4 CompTransform::modelMatrix() const {
  glm::mat4 model(1.0f);
  model = glm::translate(model, translation);
  model = glm::scale(model, scale);
  model *= glm::toMat4(glm::quat(glm::radians(rotation)));
  return model;
}
