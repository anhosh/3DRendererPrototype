#pragma once

#include <Macros.hpp>

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 color;
  glm::vec2 texCoord;

  static void setupAttributes() {
    VERTEX_ATTRIBUTE_FLOATS(0, position);
    VERTEX_ATTRIBUTE_FLOATS(1, normal);
    VERTEX_ATTRIBUTE_FLOATS(2, color);
    VERTEX_ATTRIBUTE_FLOATS(3, texCoord);
  }
};
