#include <Util/Math/Triangle.hpp>

float Triangle::area() const {
  const glm::vec3 ab = a() - b();
  const glm::vec3 ac = a() - c();
  return glm::length(glm::cross(ab, ac)) * 0.5f;
}

bool Triangle::operator==(const Triangle& other) const {
  return (a() == other.a() && b() == other.b() && c() == other.c()) ||
         (a() == other.b() && b() == other.c() && c() == other.a()) ||
         (a() == other.c() && b() == other.a() && c() == other.b()) ||
         (a() == other.a() && b() == other.c() && c() == other.b()) ||
         (a() == other.b() && b() == other.a() && c() == other.c()) ||
         (a() == other.c() && b() == other.b() && c() == other.a());
}

std::partial_ordering Triangle::operator<=>(const Triangle& other) const {
  if (*this == other) {
    return std::partial_ordering::equivalent;
  }
  if (this->area() > other.area()) {
    return std::partial_ordering::greater;
  }
  return std::partial_ordering::less;
}
