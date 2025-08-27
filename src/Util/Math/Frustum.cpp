#include <Util/Math/Frustum.hpp>

Frustum Frustum::transform(const glm::mat4& transform) const {
  Frustum ret;
  ret.farBottomLeft = transform * glm::vec4(farBottomLeft, 1.0f);
  ret.farBottomRight = transform * glm::vec4(farBottomRight, 1.0f);
  ret.farTopLeft = transform * glm::vec4(farTopLeft, 1.0f);
  ret.farTopRight = transform * glm::vec4(farTopRight, 1.0f);
  ret.nearBottomLeft = transform * glm::vec4(nearBottomLeft, 1.0f);
  ret.nearBottomRight = transform * glm::vec4(nearBottomRight, 1.0f);
  ret.nearTopLeft = transform * glm::vec4(nearTopLeft, 1.0f);
  ret.nearTopRight = transform * glm::vec4(nearTopRight, 1.0f);
  return ret;
}

std::array<glm::vec3, 8> Frustum::asArray() const {
  return {
    farBottomLeft,
    farBottomRight,
    farTopLeft,
    farTopRight,
    nearBottomLeft,
    nearBottomRight,
    nearTopLeft,
    nearTopRight,
  };
}
