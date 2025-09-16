#pragma once

#include <GraphicsOpenGL/Framebuffer.hpp>
#include <GraphicsOpenGL/ShaderProgramInstance.hpp>
#include <GraphicsOpenGL/Textures/Texture2DArray.hpp>
#include <GraphicsOpenGL/Viewport.hpp>
#include <Util/NotNull.hpp>

#include <entt/entity/entity.hpp>

class Scene;

namespace GraphicsOpenGL {
  enum class SceneRenderMode : int32_t {
    Full,
    Wireframe,
    SurfaceNormal,
    SurfaceDepth,
    DepthMap,
    LinearizedDepthMap,
    VertexNormals,
  };

  struct ShadowMaps {
    Texture2DArrayHandle directionalShadowMaps = Texture2DArrayHandle::null();
    Texture2DArrayHandle pointShadowMaps = Texture2DArrayHandle::null();
    Texture2DArrayHandle spotlightShadowMaps = Texture2DArrayHandle::null();
  };

  struct RenderPassScene {
    NotNull<Scene> scene;
    entt::entity entityCamera;
    SceneRenderMode mode;
    ShadowMaps shadowMaps = {};
    bool bClearFramebuffer = true;
  };

  struct PostProcessingPass {
    FramebufferHandle srcFramebuffer;
    ShaderProgramInstanceHandle postProcessingShader;
  };

  struct RenderPass {
    Viewport viewport = {};
    FramebufferHandle dstFramebuffer;
    std::variant<RenderPassScene, PostProcessingPass> pass;
  };
}
