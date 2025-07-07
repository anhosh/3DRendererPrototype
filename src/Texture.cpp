#include <Texture.hpp>

#include <Bitmap.hpp>

Texture::Texture(const Bitmap& bitmap, const SamplerOptions& options) {
  this->generateTextures(bitmap, options);
}

bool Texture::destroy() {
  if (mID == 0) {
    return false;
  }

  glDeleteTextures(1, &mID);
  mID = 0;
  return true;
}

void Texture::generateTextures(const Bitmap& bitmap, const SamplerOptions& options) {
  glGenTextures(1, &mID);

  this->bind();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options.wrapS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options.wrapT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.minFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.magFilter);

  static constexpr GLenum formats[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
  const GLenum format = formats[bitmap.channels() - 1];
  const glm::ivec2 size = bitmap.size();

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y,
               0, format, GL_UNSIGNED_BYTE, bitmap.data.data());
  glGenerateMipmap(GL_TEXTURE_2D);

  this->unbind();
}

void Texture::bind() const {
  glBindTexture(GL_TEXTURE_2D, mID);
}

void Texture::bind(const GLuint slot) const {
  glActiveTexture(slot);
  this->bind();
}

void Texture::unbind() const {
  (void)mID;
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::unbind(const GLuint slot) const {
  glActiveTexture(slot);
  this->unbind();
}
