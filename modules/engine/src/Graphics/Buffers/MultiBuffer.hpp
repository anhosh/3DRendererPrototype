#pragma once

#include <Graphics/Buffers/Buffer.hpp>

#include <optional>

class MultiBuffer {
public:
  MultiBuffer() = default;
  MultiBuffer(const MultiBuffer&) = default;
  MultiBuffer(MultiBuffer&&) = default;

  MultiBuffer& operator=(const MultiBuffer&) = default;
  MultiBuffer& operator=(MultiBuffer&&) = default;

  MultiBuffer(BufferHandle buffer, uint32_t numBuffers);

  void init();
  void destroy();

  void setNumBuffers(size_t newNumBuffers);
  void setCurrent(size_t index);

  void allocate(size_t size);
  void reallocate(size_t newSize);
  void bindWhole(uint32_t bindPoint) const;
  void bindRange(uint32_t bindPoint, size_t offset, size_t size) const;

  void write(const BufferObject auto& data, const size_t offset = 0) {
    ZoneScoped;

    if (const size_t requiredSize = data.size(); mAlignedBufferSize < offset + requiredSize) {
      this->reallocate(offset + requiredSize);
    }

    data.writeToBuffer(this->data(), offset);
  }

  [[nodiscard]] GLuint id() const { return mBuffer->id(); }
  [[nodiscard]] size_t size() const { return mBufferSize; }
  [[nodiscard]] std::span<uint8_t> data() const;
  [[nodiscard]] BufferHandle handle() const { return mBuffer; }
  [[nodiscard]] size_t numBuffers() const { return mNumBuffers; }

  void switchToNext();

private:
  BufferHandle mBuffer = BufferHandle::null();
  size_t mBufferSize = 0;
  size_t mAlignedBufferSize = 0;
  uint32_t mNumBuffers = 1;
  uint32_t mCurrentBuffer = 0;

  inline static std::optional<size_t> sBufferAlignment = std::nullopt;
};

