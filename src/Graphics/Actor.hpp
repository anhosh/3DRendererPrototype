#pragma once

#include <Graphics/SceneRenderer.hpp>
#include <Graphics/Transform.hpp>

#include <optional>
#include <vector>

struct Actor {
  std::string name = "Unnamed";
  Transform transform;
  std::vector<SceneRenderer::RenderData> renderData;

  void setShaderProgramInstance(const SceneRenderer::Handle<ShaderProgramInstance> shaderInstance) {
    for (SceneRenderer::RenderData& videoResource : renderData) {
      videoResource.shaderProgramInstance = shaderInstance;
    }
  }

  void setOutlineShaderInstance(std::optional<SceneRenderer::Handle<ShaderProgramInstance>> outlineShaderInstance) {
    for (SceneRenderer::RenderData& videoResource : renderData) {
      videoResource.renderOptions.outlineShaderInstance = outlineShaderInstance;
    }
  }
};
