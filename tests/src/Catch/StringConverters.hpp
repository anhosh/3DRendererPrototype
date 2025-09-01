#pragma once

#include <Util/Math/Triangle.hpp>

#include <catch2/catch_tostring.hpp>

namespace Catch {
  template <>
  struct StringMaker<glm::vec3> {
    static std::string convert(const glm::vec3& v) {
      return std::format("({} {} {})", v.x, v.y, v.z);
    }
  };

  template <>
  struct StringMaker<std::optional<glm::vec3>> {
    static std::string convert(const std::optional<glm::vec3>& v) {
      if (v.has_value()) {
        return std::format("({} {} {})", v->x, v->y, v->z);
      }
      return "std::nullopt";
    }
  };

  template <>
  struct StringMaker<Triangle> {
    static std::string convert(const Triangle& t) {
      return std::format("[A:({} {} {}) B:({} {} {}) C:({} {} {})]", t.a.x,  t.a.y, t.a.z, t.b.x, t.b.y, t.b.z, t.c.x, t.c.y, t.c.z);
    }
  };
}

