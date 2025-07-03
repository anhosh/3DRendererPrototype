#pragma once

#include <optional>
#include <span>
#include <vector>

class Bitmap;

struct SamplerOptions {
  GLint wrapS = GL_REPEAT;
  GLint wrapT = GL_REPEAT;
  GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
  GLint magFilter = GL_LINEAR;
};

class Textures {
public:
  explicit Textures(std::span<const Bitmap> bitmaps, std::span<const SamplerOptions> options = {});

  void generateTextures(std::span<const Bitmap> bitmaps, std::span<const SamplerOptions> options = {});
  bool destroy(std::optional<size_t> textureIndex = std::nullopt);

  void bind(size_t textureIndex) const;
  void bind(size_t textureIndex, GLuint slot) const;
  void bindAll() const;
  void unbind() const;
  void unbind(GLuint slot) const;
  void unbindAllSlots() const;

private:
  std::vector<GLuint> m2DTextures;
};
