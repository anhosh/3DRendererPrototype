#pragma once

#include <Graphics/ShaderUniform.hpp>
#include <Util/IntoBytes.hpp>
#include <Util/Registry.hpp>

#include <concepts>
#include <vector>

inline constexpr uint32_t UBO_BIND_POINT_CAMERA = 0;
inline constexpr uint32_t UBO_BIND_POINT_DIRECTIONAL_LIGHTS = 1;
inline constexpr uint32_t UBO_BIND_POINT_POINT_LIGHTS = 2;
inline constexpr uint32_t UBO_BIND_POINT_SPOTLIGHTS = 3;

class UniformBuffer {
public:
  UniformBuffer();

  void destroy();

  void allocate(size_t size);
  void bindWhole(uint32_t bindPoint) const;
  void bindRange(uint32_t bindPoint, size_t offset, size_t size) const;

  template <typename T> requires
    requires (T t, std::vector<uint8_t> buffer) {
      { t.size() } -> std::same_as<size_t>;
      { t.writeToBuffer(buffer) } -> std::same_as<void>;
    }
  void write(const T& value, const size_t offset = 0) {
    const size_t requiredSize = value.size();
    assert(mSize >= offset + requiredSize);

    std::vector<uint8_t> buffer;
    value.writeToBuffer(buffer);

    glBindBuffer(GL_UNIFORM_BUFFER, mID);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, requiredSize, buffer.data());
    glBindBuffer(GL_UNIFORM_BUFFER, GL_NONE);
  }

  [[nodiscard]] GLuint id() const { return mID; }
  [[nodiscard]] size_t size() const { return mSize; }

private:
  GLuint mID = GL_NONE;
  size_t mSize = 0;
};

using UniformBufferHandle = Registry<UniformBuffer>::Handle;
