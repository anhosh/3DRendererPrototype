#pragma once

#include <Assets/AssetHandle.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/Textures/Texture2D.hpp>
#include <Graphics/Textures/TextureBase.hpp>
#include <Util/Registry.hpp>

#include <span>

class Bitmap;

class Texture2DArray : public TextureBase {
public:
  Texture2DArray();
  explicit Texture2DArray(std::span<const AssetHandle<Bitmap>> bitmaps, GLint internalFormat);

  void allocate(glm::uvec2 size, uint32_t slices, GLint internalFormat);
  void generate(std::span<const AssetHandle<Bitmap>> bitmaps, GLint internalFormat);

  void copyFromTexture2D(const Texture2D& texture, uint32_t targetSlice) const;
  void copyFromTexture2D(Texture2DHandle texture, uint32_t targetSlice) const;
  void copyFromFramebuffer(const Framebuffer& framebuffer, uint32_t targetSlice) const;
  void copyFromFramebuffer(FramebufferHandle framebuffer, uint32_t targetSlice) const;
};

using Texture2DArrayHandle = Registry<Texture2DArray>::Handle;
