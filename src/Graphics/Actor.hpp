#pragma once

#include <Graphics/RenderingEngine.hpp>
#include <Graphics/Transform.hpp>

#include <optional>
#include <vector>

struct Actor {
  std::string name = "Unnamed";
  Transform transform;
  std::vector<RenderingEngine::RenderData> renderData;

  void setShaderProgramInstance(const RenderingEngine::Handle<ShaderProgramInstance> shaderInstance) {
    for (RenderingEngine::RenderData& videoResource : renderData) {
      videoResource.shaderProgramInstance = shaderInstance;
    }
  }

  void setOutlineShaderInstance(std::optional<RenderingEngine::Handle<ShaderProgramInstance>> outlineShaderInstance) {
    for (RenderingEngine::RenderData& videoResource : renderData) {
      videoResource.renderOptions.outlineShaderInstance = outlineShaderInstance;
    }
  }
};
