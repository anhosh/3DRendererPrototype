#include <Graphics/TextureCubeMap.hpp>

#include <array>
#include <Assets/Bitmap.hpp>

TextureCubeMap::TextureCubeMap() {
  this->init();
}

TextureCubeMap::TextureCubeMap(const TextureCubeMapBitmaps& bitmaps, const SamplerOptions& options) : TextureCubeMap() {
  this->generate(bitmaps, options);
}

void TextureCubeMap::init() {
  glGenTextures(1, &mID);
}

void TextureCubeMap::generate(const TextureCubeMapBitmaps& bitmaps, const SamplerOptions& options) const {
  this->bind();

  const auto faces = std::array { bitmaps.right, bitmaps.left, bitmaps.top, bitmaps.bottom, bitmaps.front, bitmaps.back };
  for (size_t i = 0; i < faces.size(); ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, faces[i]->size().x, faces[i]->size().y,
                 0, GL_RGB, GL_UNSIGNED_BYTE, faces[i]->bytes());
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, options.minFilter);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, options.magFilter);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, options.wrapS);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, options.wrapT);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, options.wrapR);

  this->unbind();
}

void TextureCubeMap::destroy() {
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
