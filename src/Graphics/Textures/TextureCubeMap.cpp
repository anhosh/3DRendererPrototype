#include <Graphics/Textures/TextureCubeMap.hpp>

#include <Assets/Bitmap.hpp>

#include <array>
#include <ranges>

TextureCubeMap::TextureCubeMap()
  : TextureBase(GL_TEXTURE_CUBE_MAP)
{
}

TextureCubeMap::TextureCubeMap(const TextureCubeMapBitmaps& bitmaps, const GLint internalFormat)
  : TextureBase(GL_TEXTURE_CUBE_MAP)
{
  this->generate(bitmaps, internalFormat);
}

void TextureCubeMap::generate(const TextureCubeMapBitmaps& bitmaps, const GLint internalFormat) {
  ZoneScoped;

  const auto faces = std::array { bitmaps.right, bitmaps.left, bitmaps.top, bitmaps.bottom, bitmaps.front, bitmaps.back };
  constexpr auto formats = std::array { GL_RED, GL_RG, GL_RGB, GL_RGBA };
  this->allocate2D(faces[0]->size(), internalFormat);
  for (const auto [i, face] : faces | std::views::enumerate) {
    const GLenum format = formats[face->channels() - 1];
    glTextureSubImage3D(this->id(), 0,
                        0, 0, static_cast<GLint>(i),
                        static_cast<GLsizei>(face->size().x), static_cast<GLsizei>(face->size().y), 1,
                        format, GL_UNSIGNED_BYTE, face->bytes());
  }
  glGenerateTextureMipmap(this->id());
}
