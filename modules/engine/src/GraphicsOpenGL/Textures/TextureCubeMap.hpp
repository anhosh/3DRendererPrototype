#pragma once

#include <Assets/AssetHandle.hpp>
#include <GraphicsOpenGL/Textures/TextureBase.hpp>
#include <Util/Registry.hpp>

class Bitmap;

namespace GraphicsOpenGL {
  struct TextureCubeMapBitmaps {
    AssetHandle<Bitmap> right;
    AssetHandle<Bitmap> left;
    AssetHandle<Bitmap> top;
    AssetHandle<Bitmap> bottom;
    AssetHandle<Bitmap> front;
    AssetHandle<Bitmap> back;
    bool bSRGB = false;
  };

  class TextureCubeMap : public TextureBase {
  public:
    TextureCubeMap();
    explicit TextureCubeMap(const TextureCubeMapBitmaps& bitmaps, GLint internalFormat);

    void allocate(glm::uvec2 size, GLint internalFormat);
    void generate(const TextureCubeMapBitmaps& bitmaps, GLint internalFormat);
  };

  using TextureCubeMapHandle = Registry<TextureCubeMap>::Handle;
}
