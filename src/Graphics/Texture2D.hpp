#pragma once

#include <Assets/AssetHandle.hpp>
#include <Util/Registry.hpp>

class Bitmap;

class Texture2D {
public:
  Texture2D();
  explicit Texture2D(AssetHandle<Bitmap> bitmap, GLint internalFormat);

  void init();
  void allocate(glm::uvec2 size, GLint internalFormat) const;
  void generate(AssetHandle<Bitmap> bitmap, GLint internalFormat) const;
  void destroy();

  void bind(GLuint slot) const;
  void unbind(GLuint slot) const;

  [[nodiscard]] GLuint id() const { return mID; }

private:
  GLuint mID = 0;
};

using Texture2DHandle = Registry<Texture2D>::Handle;
