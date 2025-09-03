#include <Graphics/Textures/Texture2D.hpp>

#include <Assets/Bitmap.hpp>

Texture2D::Texture2D()
  : TextureBase(GL_TEXTURE_2D)
{
}

Texture2D::Texture2D(const AssetHandle<Bitmap> bitmap, const GLint internalFormat)
  : TextureBase(GL_TEXTURE_2D)
{
  this->generate(bitmap, internalFormat);
}

void Texture2D::allocate(const glm::uvec2 size, const GLint internalFormat) {
  this->allocate2D(size, internalFormat);
}

void Texture2D::generate(const AssetHandle<Bitmap> bitmap, const GLint internalFormat) {
  this->generate(bitmap.get(), internalFormat);
}

void Texture2D::generate(const Bitmap& bitmap, const GLint internalFormat) {
  ZoneScoped;

  constexpr auto formats = std::array { GL_RED, GL_RG, GL_RGB, GL_RGBA };
  const GLenum format = formats[bitmap.channels() - 1];
  const glm::ivec2 size = bitmap.size();

  this->allocate(size, internalFormat);
  glTextureSubImage2D(this->id(), 0, 0, 0, size.x, size.y, format, GL_UNSIGNED_BYTE, bitmap.bytes());
  glGenerateTextureMipmap(this->id());
}
