#include <Graphics/Texture2D.hpp>

#include <Assets/Bitmap.hpp>

Texture2D::Texture2D() {
  ZoneScoped;

  this->init();
}

Texture2D::Texture2D(AssetHandle<Bitmap> bitmap, const SamplerOptions& options)
  : Texture2D()
{
  ZoneScoped;

  this->generate(bitmap, options);
}

void Texture2D::init() {
  ZoneScoped;

  glGenTextures(1, &mID);
}

void Texture2D::generate(AssetHandle<Bitmap> bitmap, const SamplerOptions& options) const {
  ZoneScoped;

  this->bind();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options.wrapS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options.wrapT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.minFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.magFilter);

  static constexpr GLenum formats[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
  const GLenum format = formats[bitmap->channels() - 1];
  const glm::ivec2 size = bitmap->size();

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y,
               0, format, GL_UNSIGNED_BYTE, bitmap->bytes());
  glGenerateMipmap(GL_TEXTURE_2D);

  this->unbind();
}

void Texture2D::destroy() {
  ZoneScoped;

  if (mID == GL_NONE) {
    glDeleteTextures(1, &mID);
    mID = GL_NONE;
  }
}

void Texture2D::bind() const {
  glBindTexture(GL_TEXTURE_2D, mID);
}

void Texture2D::bind(const GLuint slot) const {
  glActiveTexture(slot);
  this->bind();
}

void Texture2D::unbind() const {
  (void)mID;
  glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void Texture2D::unbind(const GLuint slot) const {
  glActiveTexture(slot);
  this->unbind();
}
