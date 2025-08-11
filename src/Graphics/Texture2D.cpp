#include <Graphics/Texture2D.hpp>

#include <Assets/Bitmap.hpp>

Texture2D::Texture2D(SamplerHandle sampler)
  : mSampler(sampler)
{
  ZoneScoped;

  this->init();
}

Texture2D::Texture2D(AssetHandle<Bitmap> bitmap, const SamplerHandle sampler, const GLint internalFormat)
  : Texture2D(sampler)
{
  ZoneScoped;

  this->generate(bitmap, internalFormat);
}

void Texture2D::init() {
  if (mID ==  GL_NONE) {
    ZoneScoped;

    glCreateTextures(GL_TEXTURE_2D, 1, &mID);
  }
}

void Texture2D::allocate(const glm::uvec2 size, const GLint internalFormat) const {
  glTextureStorage2D(mID, 1, internalFormat, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
}

void Texture2D::generate(AssetHandle<Bitmap> bitmap, const GLint internalFormat) const {
  ZoneScoped;

  this->allocate(bitmap->size(), internalFormat);

  constexpr auto formats = std::array { GL_RED, GL_RG, GL_RGB, GL_RGBA };
  const GLenum format = formats[bitmap->channels() - 1];
  const glm::ivec2 size = bitmap->size();
  glTextureSubImage2D(mID, 0, 0, 0, size.x, size.y, format, GL_UNSIGNED_BYTE, bitmap->bytes());
  glGenerateTextureMipmap(mID);
}

void Texture2D::destroy() {
  ZoneScoped;

  if (mID != GL_NONE) {
    glDeleteTextures(1, &mID);
    mID = GL_NONE;
  }
}

void Texture2D::bind(const GLuint slot) const {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, mID);
  mSampler->bind(slot);
}

void Texture2D::unbind(const GLuint slot) const {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, GL_NONE);
  mSampler->unbind(slot);
}
