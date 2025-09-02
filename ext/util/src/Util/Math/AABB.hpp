#pragma once

struct Triangle;

struct AABB {
  glm::vec3 min = glm::vec3(INFINITY);
  glm::vec3 max = glm::vec3(-INFINITY);

  [[nodiscard]] float width() const { return max.x - min.x; }
  [[nodiscard]] float height() const { return max.y - min.y; }
  [[nodiscard]] float depth() const { return max.z - min.z; }
  [[nodiscard]] glm::vec3 size() const { return max - min; }
  [[nodiscard]] glm::vec3 center() const;
  [[nodiscard]] std::array<glm::vec3, 8> vertices(const glm::mat4& transform = glm::mat4(1.0f)) const;
  [[nodiscard]] std::array<Triangle, 12> triangulated(const glm::mat4& transform = glm::mat4(1.0f)) const;

  void transform(const glm::mat4& transform);
  void moveToCenter(glm::vec3 center);
  void includePoint(glm::vec3 point);
  void includeAABB(const AABB& other);

  AABB transformed(const glm::mat4& transform) const;
  AABB movedToCenter(glm::vec3 center) const;
  AABB includingPoint(glm::vec3 point) const;
  AABB includingAABB(const AABB& other) const;
};
