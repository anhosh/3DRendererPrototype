#include <Util/Math/Frustum.hpp>

#include <Util/Math/AABB.hpp>

Frustum Frustum::fromAABB(const AABB& box) {
  return {
    .farBottomLeft   = glm::vec3(box.min.x, box.min.y, box.max.z),
    .farBottomRight  = glm::vec3(box.max.x, box.min.y, box.max.z),
    .farTopLeft      = glm::vec3(box.min.x, box.max.y, box.max.z),
    .farTopRight     = glm::vec3(box.max.x, box.max.y, box.max.z),
    .nearBottomLeft  = glm::vec3(box.min.x, box.min.y, box.min.z),
    .nearBottomRight = glm::vec3(box.max.x, box.min.y, box.min.z),
    .nearTopLeft     = glm::vec3(box.min.x, box.max.y, box.min.z),
    .nearTopRight    = glm::vec3(box.max.x, box.max.y, box.min.z),
  };
}

Frustum Frustum::transform(const glm::mat4& transform) const {
  return {
    .farBottomLeft   = transform * glm::vec4(farBottomLeft, 1.0f),
    .farBottomRight  = transform * glm::vec4(farBottomRight, 1.0f),
    .farTopLeft      = transform * glm::vec4(farTopLeft, 1.0f),
    .farTopRight     = transform * glm::vec4(farTopRight, 1.0f),
    .nearBottomLeft  = transform * glm::vec4(nearBottomLeft, 1.0f),
    .nearBottomRight = transform * glm::vec4(nearBottomRight, 1.0f),
    .nearTopLeft     = transform * glm::vec4(nearTopLeft, 1.0f),
    .nearTopRight    = transform * glm::vec4(nearTopRight, 1.0f),
  };
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

AABB Frustum::bounds() const {
  AABB box;
  for (const glm::vec3 point : this->asArray()) {
    box.includePoint(point);
  }
  return box;
}
