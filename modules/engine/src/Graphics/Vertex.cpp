#include <Graphics/Vertex.hpp>

#include <tracy/TracyOpenGL.hpp>

void Vertex::setupVertexAttributes(const GLuint vao) {
  ZoneScoped;
  TracyGpuZone("Setup vertex attributes");

  glEnableVertexArrayAttrib(vao, 0);
  glEnableVertexArrayAttrib(vao, 1);
  glEnableVertexArrayAttrib(vao, 2);
  glEnableVertexArrayAttrib(vao, 3);

  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribBinding(vao, 1, 0);
  glVertexArrayAttribBinding(vao, 2, 0);
  glVertexArrayAttribBinding(vao, 3, 0);

  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
  glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
  glVertexArrayAttribFormat(vao, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent));
  glVertexArrayAttribFormat(vao, 3, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));
}
