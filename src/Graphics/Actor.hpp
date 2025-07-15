#pragma once

#include <Graphics/RenderingEngine.hpp>
#include <Graphics/Transform.hpp>

#include <optional>
#include <vector>

struct Actor {
  std::string name = "Unnamed";
  Transform transform = {};
  std::vector<RenderingEngine::RenderData> renderData;

  void setShaderProgramInstance(const ShaderProgramInstanceHandle shaderInstance) {
    for (RenderingEngine::RenderData& videoResource : renderData) {
      videoResource.shaderProgramInstance = shaderInstance;
    }
  }

  void setOutlineShaderInstance(const std::optional<ShaderProgramInstanceHandle>& outlineShaderInstance) {
    for (RenderingEngine::RenderData& videoResource : renderData) {
      videoResource.renderOptions.outlineShaderInstance = outlineShaderInstance;
    }
  }
};
