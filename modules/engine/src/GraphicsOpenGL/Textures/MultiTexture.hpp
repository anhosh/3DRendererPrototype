#pragma once

#include <GraphicsOpenGL/Textures/Texture2D.hpp>
#include <GraphicsOpenGL/Textures/Texture2DArray.hpp>
#include <GraphicsOpenGL/Textures/TextureCubeMap.hpp>
#include <Util/Registry.hpp>

#include <concepts>
#include <functional>
#include <type_traits>
#include <vector>

namespace GraphicsOpenGL {
  template <std::derived_from<TextureBase> Texture>
  class MultiTexture {
    using TextureHandle = Registry<Texture>::Handle;

  public:
    MultiTexture() = default;
    MultiTexture(const MultiTexture&) = default;
    MultiTexture(MultiTexture&&) = default;

    MultiTexture& operator=(const MultiTexture&) = default;
    MultiTexture& operator=(MultiTexture&&) = default;

    template <std::invocable<> FnRegisterTexture>
    explicit MultiTexture(const FnRegisterTexture& registerTexture, const size_t numTextures, const glm::uvec3 size, const GLint internalFormat) {
      mTextures.reserve(numTextures);
      for (size_t i = 0; i < numTextures; ++i) {
        if constexpr (std::is_same_v<Texture2D, Texture>) {
          Texture2DHandle texture = std::invoke(registerTexture);
          texture->allocate(size.xy(), internalFormat);
          mTextures.emplace_back(texture);
        } else if constexpr (std::is_same_v<Texture2DArray, Texture>) {
          Texture2DArrayHandle textureArray = std::invoke(registerTexture);
          textureArray->allocate(size.xy(), size.z, internalFormat);
          mTextures.emplace_back(textureArray);
        } else if constexpr (std::is_same_v<TextureCubeMap, Texture>) {
          TextureCubeMapHandle cubeMap = std::invoke(registerTexture);
          cubeMap->allocate(size.xy(), internalFormat);
          mTextures.emplace_back(cubeMap);
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
}
