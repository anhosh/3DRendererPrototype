#include <Graphics/Buffer.hpp>

Buffer::Buffer(const GLenum target) : mTarget(target) {
  ZoneScoped;

  this->init();
}

void Buffer::init() {
  ZoneScoped;

  glGenBuffers(1, &mID);
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

  glBindBuffer(mTarget, mID);
  glBufferData(mTarget, static_cast<GLsizeiptr>(size), nullptr, GL_DYNAMIC_READ);
  glBindBuffer(mTarget, GL_NONE);
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
    glBindBuffer(GL_COPY_READ_BUFFER, oldBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, mID);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, oldSize);
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
