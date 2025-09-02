#include <Scene/Components/Camera.hpp>

#include <Scene/Components/Transform.hpp>
#include <Util/Math/Frustum.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

glm::mat4 CompCamera::perspective() const {
  return glm::perspective(glm::radians(fov), clipBox.width() / clipBox.height(), clipBox.min.z, clipBox.max.z);
}

glm::mat4 CompCamera::orthographic() const {
  return glm::ortho(clipBox.min.x, clipBox.max.x, clipBox.min.y, clipBox.max.y, clipBox.min.z, clipBox.max.z);
}

Frustum CompCamera::viewFrustumPerspective(const CompTransform& viewTransform) const {
  const glm::vec3 lookDirection = viewTransform.forward();
  const glm::vec3 upDirection = viewTransform.up();
  const glm::vec3 rightDirection = viewTransform.right();
  const float aspect = clipBox.width() / clipBox.height();
  const float nearHeight = glm::tan(glm::radians(fov)) * clipBox.min.z;
  const float farHeight = glm::tan(glm::radians(fov)) * clipBox.max.z;
  const float nearWidth = aspect * nearHeight;
  const float farWidth = aspect * farHeight;

  const glm::vec3 nearCentre = viewTransform.translation + lookDirection * clipBox.min.z;
  const glm::vec3 farCentre = viewTransform.translation + lookDirection * clipBox.max.z;
  const glm::vec3 farUp = upDirection * farHeight;
  const glm::vec3 nearUp = upDirection * nearHeight;
  const glm::vec3 farRight = rightDirection * farWidth;
  const glm::vec3 nearRight = rightDirection * nearWidth;

  return {
    .farBottomLeft   = farCentre - farUp - farRight,
    .farBottomRight  = farCentre - farUp + farRight,
    .farTopLeft      = farCentre + farUp - farRight,
    .farTopRight     = farCentre + farUp + farRight,
    .nearBottomLeft  = nearCentre - nearUp - nearRight,
    .nearBottomRight = nearCentre - nearUp + nearRight,
    .nearTopLeft     = nearCentre + nearUp - nearRight,
    .nearTopRight    = nearCentre + nearUp + nearRight,
  };
}

Frustum CompCamera::viewFrustumOrthographic(const CompTransform& viewTransform) const {
  const glm::vec3 lookDirection = viewTransform.forward();
  const glm::vec3 upDirection = viewTransform.up();
  const glm::vec3 rightDirection = viewTransform.right();

  const glm::vec3 nearCentre = viewTransform.translation + lookDirection * clipBox.min.z;
  const glm::vec3 farCentre = viewTransform.translation + lookDirection * clipBox.max.z;

  return {
    .farBottomLeft   = farCentre + (upDirection * clipBox.min.y) + (rightDirection * clipBox.min.x),
    .farBottomRight  = farCentre + (upDirection * clipBox.min.y) + (rightDirection * clipBox.max.x),
    .farTopLeft      = farCentre + (upDirection * clipBox.max.y) + (rightDirection * clipBox.min.x),
    .farTopRight     = farCentre + (upDirection * clipBox.max.y) + (rightDirection * clipBox.max.x),
    .nearBottomLeft  = nearCentre + (upDirection * clipBox.min.y) + (rightDirection * clipBox.min.x),
    .nearBottomRight = nearCentre + (upDirection * clipBox.min.y) + (rightDirection * clipBox.max.x),
    .nearTopLeft     = nearCentre + (upDirection * clipBox.max.y) + (rightDirection * clipBox.min.x),
    .nearTopRight    = nearCentre + (upDirection * clipBox.max.y) + (rightDirection * clipBox.max.x),
  };
}
