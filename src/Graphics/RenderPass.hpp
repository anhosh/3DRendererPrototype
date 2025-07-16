#pragma once

#include <Graphics/Scene.hpp>
#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Viewport.hpp>
#include <Util/NotNull.hpp>

struct RenderScenePass {
  NotNull<Scene> scene;
  NotNull<Camera> camera;
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
