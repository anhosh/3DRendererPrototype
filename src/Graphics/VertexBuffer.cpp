#include <Graphics/VertexBuffer.hpp>

VertexBuffer::VertexBuffer(const size_t stride) : stride(static_cast<GLsizei>(stride)) {
  this->init();
}

void VertexBuffer::init() {
  glCreateBuffers(1, &vbo);
}

void VertexBuffer::destroy() {
  glDeleteBuffers(1, &vbo);
  vbo = GL_NONE;
}

void VertexBuffer::bind(const size_t binding, const size_t offset) const {
  glBindVertexBuffer(binding, vbo, static_cast<GLintptr>(offset), stride);
}
