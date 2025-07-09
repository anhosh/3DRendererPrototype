#include <Graphics/VertexArray.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <cassert>

VertexArray::VertexArray() {
  this->init();
}

VertexArray::VertexArray(std::span<const Vertex> vertices, std::span<const GLuint> indices) {
  this->init();
  this->generateMesh(vertices, indices);
}

void VertexArray::init() {
  glGenVertexArrays(1, &mVAO);
  glGenBuffers(1, &mVBO);
  glGenBuffers(1, &mEBO);
}

void VertexArray::destroy() {
  glDeleteBuffers(1, &mVBO);
  glDeleteBuffers(1, &mEBO);
  glDeleteVertexArrays(1, &mVAO);

  mVAO = 0;
  mVBO = 0;
  mEBO = 0;
}

void VertexArray::generateMesh(std::span<const Vertex> vertices, std::span<const GLuint> indices) {
  assert(indices.size() % 3 == 0);

  glGenVertexArrays(1, &mVAO);
  glGenBuffers(1, &mVBO);
  glGenBuffers(1, &mEBO);

  glBindVertexArray(mVAO);

  glBindBuffer(GL_ARRAY_BUFFER, mVBO);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size_bytes()), vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size_bytes()), indices.data(), GL_STATIC_DRAW);

  Vertex::setupAttributes();

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  mIndexCount = static_cast<GLsizei>(indices.size());
}

void VertexArray::bind() const {
  glBindVertexArray(mVAO);
}

void VertexArray::unbind() const {
  (void)mVAO;
  glBindVertexArray(0);
}

void VertexArray::bindAndDraw() const {
  this->bind();
  this->draw();
  this->unbind();
}

void VertexArray::draw() const {
  glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, nullptr);
}
