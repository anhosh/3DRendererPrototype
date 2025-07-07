#pragma once

class Bitmap;

struct SamplerOptions {
  GLint wrapS = GL_REPEAT;
  GLint wrapT = GL_REPEAT;
  GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
  GLint magFilter = GL_LINEAR;
};

class Texture {
public:
  explicit Texture(const Bitmap& bitmap, const SamplerOptions& options = {});

  void generateTextures(const Bitmap& bitmap, const SamplerOptions& options = {});
  bool destroy();

  void bind() const;
  void bind(GLuint slot) const;
  void unbind() const;
  void unbind(GLuint slot) const;

private:
  GLuint mID;
};
