#include <Graphics/VertexArray.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <cassert>

VertexArray::VertexArray() {
  this->init();
}

VertexArray::VertexArray(const Mesh& mesh) {
  this->init();
  this->generateMesh(mesh);
}

void VertexArray::init() {
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
}

void VertexArray::destroy() {
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
  glDeleteVertexArrays(1, &vao);

  vao = 0;
  vbo = 0;
  ebo = 0;
}

void VertexArray::generateMesh(const Mesh& mesh) {
  assert(mesh.indices.size() % 3 == 0);

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(Vertex)), mesh.vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(uint32_t)), mesh.indices.data(), GL_STATIC_DRAW);

  Vertex::setupAttributes();

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  indexCount = static_cast<GLsizei>(mesh.indices.size());
}
