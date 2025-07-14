#pragma once

#include <Util/Macros/VertexAttributes.hpp>

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoord;

  static void enableAttributes() {
    VERTEX_ATTRIBUTE_FLOATS(Vertex, 0, position);
    VERTEX_ATTRIBUTE_FLOATS(Vertex, 1, normal);
    VERTEX_ATTRIBUTE_FLOATS(Vertex, 2, texCoord);
  }
};
