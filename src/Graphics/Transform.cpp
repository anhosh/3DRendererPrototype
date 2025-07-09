#include <Graphics/Transform.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

glm::mat4 Transform::matrix() const {
  glm::mat4 model(1.0f);
  model = glm::translate(model, translation);
  model = glm::scale(model, scale);
  model *= glm::toMat4(glm::quat(glm::radians(rotation)));
  return model;
}
