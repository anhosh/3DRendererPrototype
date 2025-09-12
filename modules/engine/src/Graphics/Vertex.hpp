#pragma once

#include <glm/gtx/hash.hpp>

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec2 texCoord;

  static void setupVertexAttributes(GLuint vao);

  bool operator==(const Vertex& other) const = default;
  bool operator!=(const Vertex& other) const = default;
};

template <> struct std::hash<Vertex> {
  size_t operator()(const Vertex& vertex) const noexcept {
    return ((hash<glm::vec3>{}(vertex.position) ^
            (hash<glm::vec3>{}(vertex.normal) << 1)) >> 1) ^
           ((hash<glm::vec3>{}(vertex.tangent) ^
            (hash<glm::vec2>{}(vertex.texCoord) << 1)) >> 1);
  }
};
