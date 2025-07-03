#include <Textures.hpp>

#include <Bitmap.hpp>

Textures::Textures(std::span<const Bitmap> bitmaps, std::span<const SamplerOptions> options) {
  this->generateTextures(bitmaps, options);
}

bool Textures::destroy(std::optional<size_t> textureIndex) {
  if (m2DTextures.empty()) {
    return false;
  }

  if (textureIndex.has_value()) {
    glDeleteTextures(1, &m2DTextures[textureIndex.value()]);
    return true;
  }

  glDeleteTextures(static_cast<GLsizei>(m2DTextures.size()), m2DTextures.data());
  m2DTextures.clear();
  return true;
}

void Textures::generateTextures(std::span<const Bitmap> bitmaps, std::span<const SamplerOptions> options) {
  assert(bitmaps.size() == options.size());

  m2DTextures.resize(bitmaps.size());
  glGenTextures(static_cast<GLsizei>(m2DTextures.size()), m2DTextures.data());

  for (size_t i = 0; i < m2DTextures.size(); ++i) {
    this->bind(i);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options[i].wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options[i].wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options[i].minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options[i].magFilter);

    static constexpr GLenum formats[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
    const GLenum format = formats[bitmaps[i].channels() - 1];
    const glm::ivec2 size = bitmaps[i].size();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y,
                 0, format, GL_UNSIGNED_BYTE, bitmaps[i].data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  this->unbind();
}

void Textures::bind(const size_t textureIndex) const {
  glBindTexture(GL_TEXTURE_2D, m2DTextures[textureIndex]);
}

void Textures::bind(const size_t textureIndex, const GLuint slot) const {
  glActiveTexture(slot);
  this->bind(textureIndex);
}

void Textures::bindAll() const {
  for (size_t i = 0; i < m2DTextures.size(); ++i) {
    this->bind(i, GL_TEXTURE0 + static_cast<GLuint>(i));
  }
}

void Textures::unbind() const {
  (void)m2DTextures;
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Textures::unbind(const GLuint slot) const {
  glActiveTexture(slot);
  this->unbind();
}

void Textures::unbindAllSlots() const {
  for (size_t i = 0; i < m2DTextures.size(); ++i) {
    this->unbind(GL_TEXTURE0 + static_cast<GLuint>(i));
  }
}
