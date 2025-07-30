#include <Graphics/VertexArray.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <cassert>

VertexArray::VertexArray() {
  ZoneScoped;

  this->init();
}

VertexArray::VertexArray(const Mesh& mesh) {
  ZoneScoped;

  this->init();
  this->generateMesh(mesh);
}

void VertexArray::init() {
  ZoneScoped;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
}

void VertexArray::destroy() {
  ZoneScoped;

  if (vbo != GL_NONE) {
    glDeleteBuffers(1, &vbo);
    vbo = GL_NONE;
  }
  if (ebo != GL_NONE) {
    glDeleteBuffers(1, &ebo);
    ebo = GL_NONE;
  }
  if (vao != GL_NONE) {
    glDeleteVertexArrays(1, &vao);
    vao = GL_NONE;
  }
}

void VertexArray::generateMesh(const Mesh& mesh) {
  ZoneScoped;

  assert(mesh.indices.size() % 3 == 0);

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(Vertex)), mesh.vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(uint32_t)), mesh.indices.data(), GL_STATIC_DRAW);

  Vertex::enableAttributes();

  glBindVertexArray(GL_NONE);
  glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);

  indexCount = static_cast<GLsizei>(mesh.indices.size());
}
