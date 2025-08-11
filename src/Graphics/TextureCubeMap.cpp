#include <Graphics/TextureCubeMap.hpp>

#include <Assets/Bitmap.hpp>

#include <array>

TextureCubeMap::TextureCubeMap(SamplerHandle sampler)
  : mSampler(sampler)
{
  ZoneScoped;

  this->init();
}

TextureCubeMap::TextureCubeMap(const TextureCubeMapBitmaps& bitmaps, SamplerHandle sampler, const GLint internalFormat)
  : TextureCubeMap(sampler)
{
  ZoneScoped;

  this->generate(bitmaps, internalFormat);
}

void TextureCubeMap::init() {
  ZoneScoped;

  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mID);
}

void TextureCubeMap::generate(const TextureCubeMapBitmaps& bitmaps, const GLint internalFormat) const {
  ZoneScoped;

  const auto faces = std::array { bitmaps.right, bitmaps.left, bitmaps.top, bitmaps.bottom, bitmaps.front, bitmaps.back };
  constexpr auto formats = std::array { GL_RED, GL_RG, GL_RGB, GL_RGBA };
  glTextureStorage2D(mID, 1, internalFormat,  static_cast<GLsizei>(faces[0]->size().x), static_cast<GLsizei>(faces[0]->size().y));
  for (size_t i = 0; i < faces.size(); ++i) {
    const GLenum format = formats[faces[i]->channels() - 1];
    glTextureSubImage3D(mID, 0,
                        0, 0, static_cast<GLint>(i),
                        static_cast<GLsizei>(faces[i]->size().x), static_cast<GLsizei>(faces[i]->size().y), 1,
                        format, GL_UNSIGNED_BYTE, faces[i]->bytes());
  }
  glGenerateTextureMipmap(mID);
}

void TextureCubeMap::destroy() {
  ZoneScoped;

  if (mID != GL_NONE) {
    glDeleteTextures(1, &mID);
    mID = GL_NONE;
  }
  mSampler->destroy();
}

void TextureCubeMap::bind(const GLuint slot) const {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
  mSampler->bind(slot);
}

void TextureCubeMap::unbind(const GLuint slot) const {
  (void)mID;
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
  mSampler->unbind(slot);
}
