#include <Graphics/Buffer.hpp>

Buffer::Buffer(const GLenum target) : mTarget(target) {
  ZoneScoped;

  this->init();
}

void Buffer::init() {
  ZoneScoped;

  glCreateBuffers(1, &mID);
}

void Buffer::destroy() {
  ZoneScoped;

  if (mID != GL_NONE) {
    glDeleteBuffers(1, &mID);
    mID = GL_NONE;
    mSize = 0;
  }
}

void Buffer::allocate(const size_t size) {
  ZoneScoped;

  glNamedBufferData(mID, static_cast<GLsizeiptr>(size), nullptr, GL_DYNAMIC_READ);
  mSize = size;
}

void Buffer::reallocate(const size_t newSize) {
  ZoneScoped;

  assert(mID != GL_NONE);

  const GLuint oldBuffer = mID;
  const size_t oldSize = mSize;

  this->init();
  this->allocate(newSize);
  if (oldSize > 0) {
    glCopyNamedBufferSubData(oldBuffer, mID, 0, 0, static_cast<GLsizeiptr>(oldSize));
  }
  glDeleteBuffers(1, &oldBuffer);
}

void Buffer::bindWhole(const uint32_t bindPoint) const {
  ZoneScoped;

  glBindBuffer(mTarget, mID);
  glBindBufferBase(mTarget, bindPoint, mID);
  glBindBuffer(mTarget, GL_NONE);
}

void Buffer::bindRange(const uint32_t bindPoint, const size_t offset, const size_t size) const {
  ZoneScoped;

  glBindBuffer(mTarget, mID);
  glBindBufferRange(mTarget, bindPoint, mID, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size));
  glBindBuffer(mTarget, GL_NONE);
}
