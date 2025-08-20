#include <Graphics/Textures/Texture2DArray.hpp>

#include <Assets/Bitmap.hpp>

#include <tracy/TracyOpenGL.hpp>

#include <ranges>

Texture2DArray::Texture2DArray()
  : TextureBase(GL_TEXTURE_2D_ARRAY)
{
}

Texture2DArray::Texture2DArray(const std::span<const AssetHandle<Bitmap>> bitmaps, const GLint internalFormat)
  : TextureBase(GL_TEXTURE_2D_ARRAY)
{
  this->generate(bitmaps, internalFormat);
}

void Texture2DArray::allocate(const glm::uvec2 size, const uint32_t slices, const GLint internalFormat) {
  this->allocate3D(glm::uvec3(size, slices), internalFormat);
}

void Texture2DArray::generate(const std::span<const AssetHandle<Bitmap>> bitmaps, const GLint internalFormat) {
  ZoneScoped;

  if (bitmaps.empty()) {
    return;
  }

  assert(std::ranges::all_of(bitmaps, [=](const AssetHandle<Bitmap> bitmap) {
    return bitmap->size() == bitmaps.front()->size() &&
           bitmap->channels() == bitmaps.front()->channels() &&
           bitmap->bSRGB == bitmaps.front()->bSRGB;
  }));

  constexpr auto formats = std::array { GL_RED, GL_RG, GL_RGB, GL_RGBA };
  const GLenum format = formats[bitmaps.front()->channels() - 1];
  const glm::ivec2 size = bitmaps.front()->size();

  TracyGpuZone("Generate Texture2DArray");
  this->allocate(size, bitmaps.size(), internalFormat);
  for (const auto [slice, bitmap] : bitmaps | std::views::enumerate) {
    glTextureSubImage3D(this->id(), 0, 0, 0, static_cast<GLint>(slice),
                        size.x, size.y, 1, format, GL_UNSIGNED_BYTE, bitmap->bytes());
  }
  glGenerateTextureMipmap(this->id());
}

void Texture2DArray::copyFromTexture2D(const Texture2D& texture, const uint32_t targetSlice) const {
  ZoneScoped;

  assert(this->size() == texture.size());

  TracyGpuZone("Copy from Texture2D");
  glCopyImageSubData(texture.id(), GL_TEXTURE_2D, 0, 0, 0, 0,
                     this->id(), GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<GLint>(targetSlice),
                     static_cast<GLsizei>(texture.size().x), static_cast<GLsizei>(texture.size().y), 1);
  glGenerateTextureMipmap(this->id());
}

void Texture2DArray::copyFromTexture2D(const Texture2DHandle texture, const uint32_t targetSlice) const {
  this->copyFromTexture2D(texture.get(), targetSlice);
}

void Texture2DArray::copyFromFramebuffer(const Framebuffer& framebuffer, const uint32_t targetSlice) const {
  ZoneScoped;

  assert(glm::uvec2(this->size()) == framebuffer.size());

  TracyGpuZone("Copy from Texture2D");
  glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.fbo());
  glCopyTextureSubImage3D(this->id(), 0, 0, 0, targetSlice, 0, 0, framebuffer.size().x, framebuffer.size().y);
}

void Texture2DArray::copyFromFramebuffer(const FramebufferHandle framebuffer, const uint32_t targetSlice) const {
  this->copyFromFramebuffer(framebuffer.get(), targetSlice);
}
