#include <Graphics/TextureCubeMap.hpp>

#include <Assets/Bitmap.hpp>

#include <array>

TextureCubeMap::TextureCubeMap() {
  ZoneScoped;

  this->init();
}

TextureCubeMap::TextureCubeMap(const TextureCubeMapBitmaps& bitmaps, const SamplerOptions& options) : TextureCubeMap() {
  ZoneScoped;

  this->generate(bitmaps, options);
}

void TextureCubeMap::init() {
  ZoneScoped;

  glGenTextures(1, &mID);
}

void TextureCubeMap::generate(const TextureCubeMapBitmaps& bitmaps, const SamplerOptions& options) const {
  ZoneScoped;

  this->bind();

  const auto faces = std::array { bitmaps.right, bitmaps.left, bitmaps.top, bitmaps.bottom, bitmaps.front, bitmaps.back };
  static constexpr GLenum formats[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
  for (size_t i = 0; i < faces.size(); ++i) {
    const GLenum format = formats[faces[i]->channels() - 1];
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, faces[i]->size().x, faces[i]->size().y,
                 0, format, GL_UNSIGNED_BYTE, faces[i]->bytes());
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, options.minFilter);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, options.magFilter);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, options.wrapS);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, options.wrapT);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, options.wrapR);

  this->unbind();
}

void TextureCubeMap::destroy() {
  ZoneScoped;

  if (mID != GL_NONE) {
    glDeleteTextures(1, &mID);
    mID = GL_NONE;
  }
}

void TextureCubeMap::bind() const {
  glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
}

void TextureCubeMap::bind(const GLuint slot) const {
  glActiveTexture(slot);
  glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
}

void TextureCubeMap::unbind() const {
  glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
}

void TextureCubeMap::unbind(const GLuint slot) const {
  glActiveTexture(slot);
  glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
}
