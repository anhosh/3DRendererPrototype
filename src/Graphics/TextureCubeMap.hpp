#pragma once

#include <Assets/AssetHandle.hpp>
#include <Util/Registry.hpp>

class Bitmap;

struct TextureCubeMapBitmaps {
  AssetHandle<Bitmap> right;
  AssetHandle<Bitmap> left;
  AssetHandle<Bitmap> top;
  AssetHandle<Bitmap> bottom;
  AssetHandle<Bitmap> back;
  AssetHandle<Bitmap> front;
  bool bSRGB = false;
};

class TextureCubeMap {
public:
  TextureCubeMap();
  explicit TextureCubeMap(const TextureCubeMapBitmaps& bitmaps, GLint internalFormat);

  void init();
  void generate(const TextureCubeMapBitmaps& bitmaps, GLint internalFormat) const;
  void destroy();

  void bind(GLuint slot) const;
  void unbind(GLuint slot) const;

  [[nodiscard]] GLuint id() const { return mID; }

private:
  GLuint mID = 0;
};

using TextureCubeMapHandle = Registry<TextureCubeMap>::Handle;
