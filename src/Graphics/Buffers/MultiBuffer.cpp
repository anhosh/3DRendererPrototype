#include <Graphics/Buffers/MultiBuffer.hpp>

#include <Util/Memory.hpp>

MultiBuffer::MultiBuffer(const BufferHandle buffer, const uint32_t numBuffers)
  : mBuffer(buffer)
  , mNumBuffers(numBuffers)
{
  if (!sBufferAlignment.has_value()) {
    GLint alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
    sBufferAlignment.emplace(alignment);
  }
}

void MultiBuffer::init() {
  mBuffer->init();
}

void MultiBuffer::destroy() {
  mBuffer->destroy();
}

void MultiBuffer::allocate(const size_t size) {
  mBufferSize = size;
  mAlignedBufferSize = align(size, sBufferAlignment.value());
  mBuffer->allocate(mNumBuffers * mAlignedBufferSize);
}

void MultiBuffer::reallocate(const size_t newSize) {
  mBufferSize = newSize;
  mAlignedBufferSize = align(newSize, sBufferAlignment.value());
  mBuffer->reallocate(mNumBuffers * mAlignedBufferSize);
}

void MultiBuffer::bindWhole(const uint32_t bindPoint) const {
  mBuffer->bindRange(bindPoint, mCurrentBuffer * mAlignedBufferSize, mBufferSize);
}

void MultiBuffer::bindRange(const uint32_t bindPoint, const size_t offset, const size_t size) const {
  mBuffer->bindRange(bindPoint, mCurrentBuffer * mAlignedBufferSize + offset, size);
}

void MultiBuffer::switchToNext() {
  mCurrentBuffer = (mCurrentBuffer + 1) % mNumBuffers;
}
