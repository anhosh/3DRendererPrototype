#pragma once

#include <Graphics/Framebuffer.hpp>
#include <Graphics/RenderPass.hpp>
#include <Graphics/Textures/Texture2DArray.hpp>

struct CmdRenderPass {
  RenderPass renderPass;
};

struct CmdCopyShadowMapsToArrayTexture {
  std::span<const FramebufferHandle> shadowMaps;
  Texture2DArrayHandle textureArray;
};

struct CommandBuffer {
  using Command = std::variant<CmdRenderPass, CmdCopyShadowMapsToArrayTexture>;

  std::vector<Command> commands;
};
