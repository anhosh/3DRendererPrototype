#include <Util/Math/AABB.hpp>

#include <array>
#include <ranges>

void AABB::transform(const glm::mat4& transform) {
  const auto vertices = std::array {
    transform * glm::vec4(min.x, min.y, min.z, 1.0f),
    transform * glm::vec4(min.x, min.y, max.z, 1.0f),
    transform * glm::vec4(min.x, max.y, min.z, 1.0f),
    transform * glm::vec4(min.x, max.y, max.z, 1.0f),
    transform * glm::vec4(max.x, min.y, min.z, 1.0f),
    transform * glm::vec4(max.x, min.y, max.z, 1.0f),
    transform * glm::vec4(max.x, max.y, min.z, 1.0f),
    transform * glm::vec4(max.x, max.y, max.z, 1.0f),
  };

  const auto [minX, maxX] = std::ranges::minmax(vertices, [](const glm::vec4& a, const glm::vec4 b) { return a.x < b.x; });
  const auto [minY, maxY] = std::ranges::minmax(vertices, [](const glm::vec4& a, const glm::vec4 b) { return a.y < b.y; });
  const auto [minZ, maxZ] = std::ranges::minmax(vertices, [](const glm::vec4& a, const glm::vec4 b) { return a.z < b.z; });
  std::tie(min.x, max.x) = std::tie(minX.x, maxX.x);
  std::tie(min.y, max.y) = std::tie(minY.y, maxY.y);
  std::tie(min.z, max.z) = std::tie(minZ.z, maxZ.z);
}

void AABB::moveToCenter(const glm::vec3 center) {
  const glm::vec3 diagonal = max - min;
  min = center - diagonal * 0.5f;
  max = center + diagonal * 0.5f;
}

void AABB::includePoint(const glm::vec3 point) {
  min.x = glm::min(min.x, point.x);
  min.y = glm::min(min.y, point.y);
  min.z = glm::min(min.z, point.z);
  max.x = glm::max(max.x, point.x);
  max.y = glm::max(max.y, point.y);
  max.z = glm::max(max.z, point.z);
}

void AABB::includeAABB(const AABB& other) {
  min.x = glm::min(min.x, other.min.x);
  min.y = glm::min(min.y, other.min.y);
  min.z = glm::min(min.z, other.min.z);
  max.x = glm::max(max.x, other.max.x);
  max.y = glm::max(max.y, other.max.y);
  max.z = glm::max(max.z, other.max.z);
}

AABB AABB::movedToCenter(const glm::vec3 center) const {
  AABB ret = *this;
  ret.moveToCenter(center);
  return ret;
}

AABB AABB::includingPoint(const glm::vec3 point) const {
  AABB ret = *this;
  ret.includePoint(point);
  return ret;
}

AABB AABB::includingAABB(const AABB& other) const {
  AABB ret = *this;
  ret.includeAABB(other);
  return ret;
}

AABB AABB::transformed(const glm::mat4& transform) const {
  AABB ret = *this;
  ret.transform(transform);
  return ret;
}
