#pragma once

#include <Util/Registry.hpp>

#include <concepts>
#include <vector>

class Buffer {
public:
  explicit Buffer(GLenum target);

  void init();
  void destroy();

  void allocate(size_t size);
  void reallocate(size_t newSize);
  void bindWhole(uint32_t bindPoint) const;
  void bindRange(uint32_t bindPoint, size_t offset, size_t size) const;

  template <typename BufferData> requires
    requires (BufferData t, GLenum target, size_t offset) {
      { t.size() } -> std::same_as<size_t>;
      { t.writeToBuffer(target, offset) } -> std::same_as<void>;
    }
  void write(const BufferData& data, const size_t offset = 0) {
    ZoneScoped;

    const size_t requiredSize = data.size();
    if (mSize < offset + requiredSize) {
      this->reallocate(offset + requiredSize);
    }

    glBindBuffer(mTarget, mID);
    data.writeToBuffer(mTarget, offset);
    glBindBuffer(mTarget, GL_NONE);
  }

  [[nodiscard]] GLuint id() const { return mID; }
  [[nodiscard]] size_t size() const { return mSize; }

private:
  GLuint mID = GL_NONE;
  GLenum mTarget = GL_NONE;
  size_t mSize = 0;
};

using BufferHandle = Registry<Buffer>::Handle;
