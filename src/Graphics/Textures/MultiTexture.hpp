#pragma once

#include <Graphics/Textures/Texture2D.hpp>
#include <Graphics/Textures/Texture2DArray.hpp>
#include <Graphics/Textures/TextureCubeMap.hpp>
#include <Util/Registry.hpp>

#include <concepts>
#include <type_traits>
#include <vector>

template <std::derived_from<TextureBase> Texture>
class MultiTexture {
  using TextureHandle = Registry<Texture>::Handle;

public:
  MultiTexture() = default;
  MultiTexture(const MultiTexture&) = default;
  MultiTexture(MultiTexture&&) = default;

  MultiTexture(Registry<Texture>& textureRegistry, const size_t numTextures, const glm::uvec3 size, const GLint internalFormat) {
    mTextures.reserve(numTextures);
    for (size_t i = 0; i < numTextures; ++i) {
      if constexpr (std::is_same_v<Texture2D, Texture>) {
        Texture2D texture;
        texture.allocate(size.xy(), internalFormat);
        mTextures.emplace_back(textureRegistry.add(texture));
      } else if constexpr (std::is_same_v<Texture2DArray, Texture>) {
        Texture2DArray textureArray;
        textureArray.allocate(size.xy(), size.z, internalFormat);
        mTextures.emplace_back(textureRegistry.add(textureArray));
      } else if constexpr (std::is_same_v<TextureCubeMap, Texture>) {
        TextureCubeMap cubeMap;
        cubeMap.allocate(size.xy(), internalFormat);
        mTextures.emplace_back(textureRegistry.add(cubeMap));
      }
    }
  }

  void init() {
    this->current()->init();
  }

  void destroy() {
    this->current()->destroy();
  }

  void bind(GLuint unit) const {
    this->current()->bind(unit);
  }

  void unbind(GLuint unit) const {
    this->current()->unbind(unit);
  }

  [[nodiscard]] GLuint id() const {
    return this->current()->id();
  }

  [[nodiscard]] GLuint target() const {
    return this->current()->target();
  }

  [[nodiscard]] glm::uvec3 size() const {
    return this->current()->size();
  }

  [[nodiscard]] TextureHandle current() const {
    return mTextures[mCurrentTexture];
  }

  void switchToNext() {
    mCurrentTexture = (mCurrentTexture + 1) % mTextures.size();
  }

private:
  std::vector<TextureHandle> mTextures;
  size_t mCurrentTexture = 0;
};
