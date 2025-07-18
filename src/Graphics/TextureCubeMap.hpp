#pragma once

#include <Assets/AssetHandle.hpp>
#include <Graphics/SamplerOptions.hpp>
#include <Util/Registry.hpp>

class Bitmap;

struct TextureCubeMapBitmaps {
  AssetHandle<Bitmap> right;
  AssetHandle<Bitmap> left;
  AssetHandle<Bitmap> top;
  AssetHandle<Bitmap> bottom;
  AssetHandle<Bitmap> back;
  AssetHandle<Bitmap> front;
};

class TextureCubeMap {
public:
  TextureCubeMap();
  explicit TextureCubeMap(const TextureCubeMapBitmaps& bitmaps, const SamplerOptions& options = {});

  void init();
  void generate(const TextureCubeMapBitmaps& bitmaps, const SamplerOptions& options = {}) const;
  void destroy();

  void bind() const;
  void bind(GLuint slot) const;
  void unbind() const;
  void unbind(GLuint slot) const;

  [[nodiscard]] GLuint id() const { return mID; }

private:
  GLuint mID = 0;
};

using TextureCubeMapHandle = Registry<TextureCubeMap>::Handle;
