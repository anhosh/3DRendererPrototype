#pragma once

#include <Assets/AssetHandle.hpp>
#include <Graphics/SamplerOptions.hpp>
#include <Util/NoInit.hpp>
#include <Util/Registry.hpp>

class Bitmap;

class Texture2D {
public:
  Texture2D();
  Texture2D(NoInit) {}
  explicit Texture2D(AssetHandle<Bitmap> bitmap, const SamplerOptions& options = {});

  void init();
  void generate(AssetHandle<Bitmap> bitmap, const SamplerOptions& options = {}) const;
  void destroy();

  void bind() const;
  void bind(GLuint slot) const;
  void unbind() const;
  void unbind(GLuint slot) const;

  [[nodiscard]] GLuint id() const { return mID; }

private:
  GLuint mID = 0;
};

using Texture2DHandle = Registry<Texture2D>::Handle;
