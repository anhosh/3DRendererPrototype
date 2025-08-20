#pragma once

#include <Assets/AssetHandle.hpp>
#include <Graphics/Textures/TextureBase.hpp>
#include <Util/Registry.hpp>

class Bitmap;

class Texture2D : public TextureBase {
public:
  Texture2D();
  explicit Texture2D(AssetHandle<Bitmap> bitmap, GLint internalFormat);

  void allocate(glm::uvec2 size, GLint internalFormat);
  void generate(AssetHandle<Bitmap> bitmap, GLint internalFormat);
  void generate(const Bitmap& bitmap, GLint internalFormat);
};

using Texture2DHandle = Registry<Texture2D>::Handle;
