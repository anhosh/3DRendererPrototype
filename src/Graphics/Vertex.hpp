#pragma once

#include <Util/Macros.hpp>

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoord;

  static void setupAttributes() {
    VERTEX_ATTRIBUTE_FLOATS(0, position);
    VERTEX_ATTRIBUTE_FLOATS(1, normal);
    VERTEX_ATTRIBUTE_FLOATS(2, texCoord);
  }
};
