#include <Graphics/Texture.hpp>

#include <Assets/Bitmap.hpp>

Texture::Texture() {
  this->init();
}

Texture::Texture(const Bitmap& bitmap, const SamplerOptions& options)
  : Texture()
{
  this->generateColorTexture(bitmap, options);
}

void Texture::init() {
  glGenTextures(1, &mID);
}

void Texture::generateColorTexture(const Bitmap& bitmap, const SamplerOptions& options) const {
  this->bind();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options.wrapS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options.wrapT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.minFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.magFilter);

  static constexpr GLenum formats[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
  const GLenum format = formats[bitmap.channels() - 1];
  const glm::ivec2 size = bitmap.size();

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y,
               0, format, GL_UNSIGNED_BYTE, bitmap.bytes());
  glGenerateMipmap(GL_TEXTURE_2D);

  this->unbind();
}

bool Texture::destroy() {
  if (mID == GL_NONE) {
    return false;
  }

  glDeleteTextures(1, &mID);
  mID = GL_NONE;
  return true;
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
  glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void Texture::unbind(const GLuint slot) const {
  glActiveTexture(slot);
  this->unbind();
}
