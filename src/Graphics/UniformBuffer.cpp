#include <Graphics/UniformBuffer.hpp>

UniformBuffer::UniformBuffer() {
  glGenBuffers(1, &mID);
}

void UniformBuffer::destroy() {
  if (mID != GL_NONE) {
    glDeleteBuffers(1, &mID);
    mID = GL_NONE;
  }
}

void UniformBuffer::allocate(const size_t size) {
  glBindBuffer(GL_UNIFORM_BUFFER, mID);
  glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, GL_NONE);
  mSize = size;
}

void UniformBuffer::bindWhole(const uint32_t bindPoint) const {
  this->bindRange(bindPoint, 0, mSize);
}

void UniformBuffer::bindRange(const uint32_t bindPoint, const size_t offset, const size_t size) const {
  glBindBuffer(GL_UNIFORM_BUFFER, mID);
  glBindBufferRange(GL_UNIFORM_BUFFER, bindPoint, mID, offset, size);
  glBindBuffer(GL_UNIFORM_BUFFER, GL_NONE);
}
