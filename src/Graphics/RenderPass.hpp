#pragma once

#include <Graphics/Framebuffer.hpp>
#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Viewport.hpp>
#include <Util/NotNull.hpp>

#include <entt/entity/entity.hpp>
#include <Graphics/Textures/Texture2DArray.hpp>

struct CompCamera;
class Scene;

enum class SceneRenderMode : int32_t {
  Full,
  Wireframe,
  SurfaceNormal,
  SurfaceDepth,
  DepthMap,
  VertexNormals,
};

struct ShadowMaps {
  Texture2DArrayHandle directionalShadowMaps = Texture2DArrayHandle::null();
  Texture2DArrayHandle pointShadowMaps = Texture2DArrayHandle::null();
  Texture2DArrayHandle spotlightShadowMaps = Texture2DArrayHandle::null();
};

struct RenderScenePass {
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
  std::variant<RenderScenePass, PostProcessingPass> pass;
};
