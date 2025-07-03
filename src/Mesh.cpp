#include <Mesh.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <cassert>

Mesh::Mesh() {
  this->init();
}

Mesh::Mesh(std::span<const Vertex> vertices, std::span<const GLuint> indices) {
  this->init();
  this->generateMesh(vertices, indices);
}

void Mesh::init() {
  glGenVertexArrays(1, &mVAO);
  glGenBuffers(1, &mVBO);
  glGenBuffers(1, &mEBO);
}

void Mesh::destroy() {
  glDeleteBuffers(1, &mVBO);
  glDeleteBuffers(1, &mEBO);
  glDeleteVertexArrays(1, &mVAO);

  mVAO = 0;
  mVBO = 0;
  mEBO = 0;
}

void Mesh::generateMesh(std::span<const Vertex> vertices, std::span<const GLuint> indices) {
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

void Mesh::bind() const {
  glBindVertexArray(mVAO);
}

void Mesh::unbind() const {
  (void)mVAO;
  glBindVertexArray(0);
}

void Mesh::bindAndDraw() const {
  this->bind();
  this->draw();
  this->unbind();
}

void Mesh::draw() const {
  glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, nullptr);
}
