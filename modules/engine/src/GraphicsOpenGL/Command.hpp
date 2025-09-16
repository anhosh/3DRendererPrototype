#pragma once

#include <GraphicsOpenGL/Framebuffer.hpp>
#include <GraphicsOpenGL/RenderPass.hpp>
#include <Util/Math/Frustum.hpp>

namespace GraphicsOpenGL {
  struct CmdDrawDebugFrustum {
    Frustum frustum;
    glm::vec3 color;
  };

  struct CmdRenderPass {
    RenderPass renderPass;
  };

  struct CommandBuffer {
    using Command = std::variant<CmdDrawDebugFrustum,
                                 CmdRenderPass>;

    std::vector<Command> commands;
  };
}
