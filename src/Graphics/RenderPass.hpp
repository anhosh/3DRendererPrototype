#pragma once

#include <Graphics/Framebuffer.hpp>
#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Viewport.hpp>
#include <Util/NotNull.hpp>

#include <entt/entity/entity.hpp>

struct CompCamera;
class Scene;

enum class SceneRenderMode : int32_t {
  Full,
  SurfaceNormal,
  SurfaceDepth,
  Wireframe,
  VertexNormals,
};

struct RenderScenePass {
  NotNull<Scene> scene;
  entt::entity entityCamera;
  SceneRenderMode mode;
};

struct PostProcessingPass {
  FramebufferHandle srcFramebuffer;
  ShaderProgramInstanceHandle postProcessingShader;
};

struct RenderPass {
  Viewport viewport;
  FramebufferHandle dstFramebuffer;
  std::variant<RenderScenePass, PostProcessingPass> pass;
};
