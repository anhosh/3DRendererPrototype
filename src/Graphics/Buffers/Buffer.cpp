#include <Graphics/Buffers/Buffer.hpp>

#include <tracy/TracyOpenGL.hpp>

Buffer::Buffer(const GLenum target) : mBindTarget(target) {
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
    mMapped = {};
  }
}

void Buffer::allocate(const size_t size) {
  ZoneScoped;
  TracyGpuZone("Allocate buffer");

  constexpr GLbitfield mappingFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

  glNamedBufferStorage(mID, static_cast<GLsizeiptr>(size), nullptr, GL_DYNAMIC_STORAGE_BIT | mappingFlags);
  NotNull mapped = static_cast<uint8_t*>(glMapNamedBufferRange(mID, 0, size, mappingFlags));
  mMapped = std::span(mapped.get(), size);
}

void Buffer::reallocate(const size_t newSize) {
  ZoneScoped;

  assert(mID != GL_NONE);

  const GLuint oldBuffer = mID;
  const size_t oldSize = mMapped.size_bytes();

  this->init();
  this->allocate(newSize);
  if (oldSize > 0) [[likely]] {
    TracyGpuZone("Copy buffer");

    glCopyNamedBufferSubData(oldBuffer, mID, 0, 0, static_cast<GLsizeiptr>(oldSize));
  }
  glDeleteBuffers(1, &oldBuffer);
}

void Buffer::bindWhole(const uint32_t bindPoint) const {
  ZoneScoped;

  glBindBufferBase(mBindTarget, bindPoint, mID);
}

void Buffer::bindRange(const uint32_t bindPoint, const size_t offset, const size_t size) const {
  ZoneScoped;

  glBindBufferRange(mBindTarget, bindPoint, mID, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size));
}
