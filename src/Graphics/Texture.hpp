#pragma once

#include <Util/NoInit.hpp>

class Bitmap;

struct SamplerOptions {
  GLint wrapS = GL_REPEAT;
  GLint wrapT = GL_REPEAT;
  GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
  GLint magFilter = GL_LINEAR;
};

class Texture {
public:
  Texture();
  Texture(NoInit) {}
  explicit Texture(const Bitmap& bitmap, const SamplerOptions& options = {});

  void init();
  void generateColorTexture(const Bitmap& bitmap, const SamplerOptions& options = {}) const;
  bool destroy();

  void bind() const;
  void bind(GLuint slot) const;
  void unbind() const;
  void unbind(GLuint slot) const;

  [[nodiscard]] GLuint id() const { return mID; }

private:
  GLuint mID = 0;
};
