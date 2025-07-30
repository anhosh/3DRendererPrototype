#include <Graphics/Components/CompTransform.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

glm::mat4 CompTransform::matrix() const {
  glm::mat4 model(1.0f);
  model = glm::translate(model, translation);
  model = glm::scale(model, scale);
  model *= glm::toMat4(glm::quat(glm::radians(rotation)));
  return model;
}
