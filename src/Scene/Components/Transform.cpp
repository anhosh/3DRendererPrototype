#include <Scene/Components/Transform.hpp>

#include <Util/Math/Vectors.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

glm::vec3 CompTransform::forward() const {
  return rotation.asMat3() * DIRECTION_FORWARD;
}

glm::vec3 CompTransform::right() const {
  return rotation.asMat3() * DIRECTION_RIGHT;
}

glm::vec3 CompTransform::up() const {
  return rotation.asMat3() * DIRECTION_UP;
}

glm::mat4 CompTransform::viewMatrix() const {
  return glm::lookAt(translation, translation + this->forward(), DIRECTION_UP);
}

glm::mat4 CompTransform::modelMatrix() const {
  glm::mat4 model(1.0f);
  model = glm::translate(model, translation);
  model = glm::scale(model, scale);
  model *= rotation.asMat4();
  return model;
}
