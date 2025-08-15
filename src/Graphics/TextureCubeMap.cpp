#include <Graphics/TextureCubeMap.hpp>

#include <Assets/Bitmap.hpp>

#include <array>
#include <ranges>

TextureCubeMap::TextureCubeMap() {
  ZoneScoped;

  this->init();
}

TextureCubeMap::TextureCubeMap(const TextureCubeMapBitmaps& bitmaps, const GLint internalFormat) {
  ZoneScoped;

  this->init();
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
  for (const auto [i, face] : faces | std::views::enumerate) {
    const GLenum format = formats[face->channels() - 1];
    glTextureSubImage3D(mID, 0,
                        0, 0, static_cast<GLint>(i),
                        static_cast<GLsizei>(face->size().x), static_cast<GLsizei>(face->size().y), 1,
                        format, GL_UNSIGNED_BYTE, face->bytes());
  }
  glGenerateTextureMipmap(mID);
}

void TextureCubeMap::destroy() {
  ZoneScoped;

  if (mID != GL_NONE) {
    glDeleteTextures(1, &mID);
    mID = GL_NONE;
  }
}

void TextureCubeMap::bind(const GLuint slot) const {
  glBindTextureUnit(slot, mID);
}

void TextureCubeMap::unbind(const GLuint slot) const {
  (void)mID;
  glBindTextureUnit(slot, GL_NONE);
}
