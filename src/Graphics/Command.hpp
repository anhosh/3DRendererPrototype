#pragma once

#include <Graphics/Framebuffer.hpp>
#include <Graphics/RenderPass.hpp>

struct CmdRenderPass {
  RenderPass renderPass;
};

struct CommandBuffer {
  using Command = std::variant<CmdRenderPass>;

  std::vector<Command> commands;
};
