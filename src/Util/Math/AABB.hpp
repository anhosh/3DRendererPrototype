#pragma once

struct AABB {
  glm::vec3 min = glm::vec3(INFINITY);
  glm::vec3 max = glm::vec3(-INFINITY);

  void transform(const glm::mat4& transform);
  void moveToCenter(glm::vec3 center);
  void includePoint(glm::vec3 point);
  void includeAABB(const AABB& other);

  AABB transformed(const glm::mat4& transform) const;
  AABB movedToCenter(glm::vec3 center) const;
  AABB includingPoint(glm::vec3 point) const;
  AABB includingAABB(const AABB& other) const;
};
