#include <Scene/Components/Camera.hpp>

#include <Scene/Components/Transform.hpp>
#include <Util/Math/Frustum.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

glm::mat4 CompCamera::perspective() const {
  return glm::perspective(glm::radians(fov), screenSize.x / screenSize.y, near, far);
}

glm::mat4 CompCamera::orthographic() const {
  const glm::vec2 halfSize = screenSize * 0.5f;
  return glm::ortho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y, near, far);
}

Frustum CompCamera::viewFrustumPerspective(const CompTransform& viewTransform) const {
  const glm::vec3 lookDirection = viewTransform.forward();
  const glm::vec3 upDirection = viewTransform.up();
  const glm::vec3 rightDirection = viewTransform.right();

  const glm::vec3 nearCentre = viewTransform.translation + lookDirection * near;
  const glm::vec3 farCentre = viewTransform.translation + lookDirection * far;

  const float aspect = screenSize.x / screenSize.y;
  const float nearHeight = glm::tan(glm::radians(fov)) * near;
  const float farHeight = glm::tan(glm::radians(fov)) * far;
  const float nearWidth = aspect * nearHeight;
  const float farWidth = aspect * farHeight;

  const glm::vec3 farUp = upDirection * farHeight;
  const glm::vec3 farRight = rightDirection * farWidth;
  const glm::vec3 nearUp = upDirection * nearHeight;
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

  const glm::vec3 nearCentre = viewTransform.translation + lookDirection * near;
  const glm::vec3 farCentre = viewTransform.translation + lookDirection * far;

  const glm::vec3 up = upDirection * screenSize.y * 0.5f;
  const glm::vec3 right = rightDirection * screenSize.x * 0.5f;

  return {
    .farBottomLeft   = farCentre - up - right,
    .farBottomRight  = farCentre - up + right,
    .farTopLeft      = farCentre + up - right,
    .farTopRight     = farCentre + up + right,
    .nearBottomLeft  = nearCentre - up - right,
    .nearBottomRight = nearCentre - up + right,
    .nearTopLeft     = nearCentre + up - right,
    .nearTopRight    = nearCentre + up + right,
  };
}
