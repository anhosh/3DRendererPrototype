#pragma once

#include <Assets/AssetHandle.hpp>
#include <Graphics/Sampler.hpp>
#include <Util/Registry.hpp>

class Bitmap;

class Texture2D {
public:
  explicit Texture2D(SamplerHandle sampler);
  explicit Texture2D(AssetHandle<Bitmap> bitmap, SamplerHandle sampler);

  void init();
  void allocate(glm::uvec2 size, GLint format) const;
  void generate(AssetHandle<Bitmap> bitmap) const;
  void destroy();

  void bind(GLuint slot) const;
  void unbind(GLuint slot) const;

  [[nodiscard]] GLuint id() const { return mID; }

private:
  GLuint mID = 0;
  SamplerHandle mSampler;
};

using Texture2DHandle = Registry<Texture2D>::Handle;
