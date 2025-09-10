#pragma once

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec2 texCoord;

  static void setupVertexAttributes(GLuint vao);
};
