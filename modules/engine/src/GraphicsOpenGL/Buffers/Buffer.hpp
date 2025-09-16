#pragma once

#include <Util/Macros/Errors.hpp>
#include <Util/Registry.hpp>

#include <concepts>
#include <span>

namespace GraphicsOpenGL {
  template <typename BufferData>
  concept BufferObject = requires (BufferData t, std::span<uint8_t> buffer, size_t offset) {
    { t.size() } -> std::same_as<size_t>;
    { t.writeToBuffer(buffer, offset) } -> std::same_as<void>;
  };

  class Buffer {
  public:
    explicit Buffer(GLenum target);

    void init();
    void destroy();

    void allocate(size_t size);
    void reallocate(size_t newSize);
    void bindWhole(uint32_t bindPoint) const;
    void bindRange(uint32_t bindPoint, size_t offset, size_t size) const;

    void write(const BufferObject auto& data, const size_t offset = 0) {
      ZoneScoped;

      if (const size_t requiredSize = data.size(); mMapped.size_bytes() < offset + requiredSize) {
        this->reallocate(offset + requiredSize);
      }

      data.writeToBuffer(mMapped, offset);
    }

    [[nodiscard]] GLuint id() const { return mID; }
    [[nodiscard]] size_t size() const { return mMapped.size_bytes(); }
    [[nodiscard]] std::span<uint8_t> data() const { return mMapped; }

  private:
    GLuint mID = GL_NONE;
    GLenum mBindTarget = GL_NONE;
    std::span<uint8_t> mMapped;
  };

  using BufferHandle = Registry<Buffer>::Handle;
}
