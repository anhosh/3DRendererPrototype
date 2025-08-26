#include <Scene/Components/Camera.hpp>

#include <Scene/Components/Transform.hpp>
#include <Util/Math/Frustum.hpp>

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

Frustum CompCamera::viewFrustum(CompTransform frustumTransform, const glm::uvec2 screenSize) const {
  const glm::vec3 lookDirection = frustumTransform.forward();
  const glm::vec3 upDirection = frustumTransform.up();
  const glm::vec3 rightDirection = frustumTransform.right();

  const glm::vec3 nearCentre = frustumTransform.translation + lookDirection * near;
  const glm::vec3 farCentre = frustumTransform.translation + lookDirection * far;

  const float aspect = static_cast<float>(screenSize.x) / static_cast<float>(screenSize.y);
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
