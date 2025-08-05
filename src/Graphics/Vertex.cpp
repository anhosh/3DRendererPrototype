#include <Graphics/Vertex.hpp>
#include <tracy/TracyOpenGL.hpp>

void Vertex::setupVertexAttributes(const GLuint vao) {
  ZoneScoped;
  TracyGpuZone("Setup vertex attributes");

  glEnableVertexArrayAttrib(vao, 0);
  glEnableVertexArrayAttrib(vao, 1);
  glEnableVertexArrayAttrib(vao, 2);
  for (size_t i = 0; i < 4 + 3; ++i) {
    glEnableVertexArrayAttrib(vao, 3 + i);
  }

  // Vertex data
  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribBinding(vao, 1, 0);
  glVertexArrayAttribBinding(vao, 2, 0);
  // Instance data
  for (size_t i = 0; i < 4 + 3; ++i) {
    glVertexArrayAttribBinding(vao, 3 + i, 1);
  }

  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
  glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
  glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));
  // Model matrix
  for (size_t i = 0; i < 4; ++i) {
    const size_t offset = sizeof(glm::vec4) * i;
    glVertexArrayAttribFormat(vao, 3 + i, 4, GL_FLOAT, GL_FALSE, offset);
  }
  // Normal matrix
  for (size_t i = 0; i < 3; ++i) {
    const size_t offset = sizeof(glm::mat4) + sizeof(glm::vec3) * i;
    glVertexArrayAttribFormat(vao, 7 + i, 3, GL_FLOAT, GL_FALSE, offset);
  }

  glVertexArrayBindingDivisor(vao, 1, 1);
}
