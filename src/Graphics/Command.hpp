#pragma once

#include <Graphics/Framebuffer.hpp>
#include <Graphics/RenderPass.hpp>
#include <Util/Math/Frustum.hpp>

struct CmdDrawDebugFrustum {
  Frustum frustum;
};

struct CmdRenderPass {
  RenderPass renderPass;
};

struct CommandBuffer {
  using Command = std::variant<CmdDrawDebugFrustum,
                               CmdRenderPass>;

  std::vector<Command> commands;
};
