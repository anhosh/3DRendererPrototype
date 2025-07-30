#pragma once

#include <Graphics/RenderData.hpp>
#include <Graphics/ShaderProgramInstance.hpp>
#include <Graphics/Transform.hpp>

#include <optional>
#include <vector>

struct Actor {
  std::string name = "Unnamed";
  Transform transform = {};
  std::vector<RenderData> renderData;

  void setShaderProgramInstance(const ShaderProgramInstanceHandle shaderInstance) {
    ZoneScoped;

    for (RenderData& videoResource : renderData) {
      videoResource.shaderProgramInstance = shaderInstance;
    }
  }

  void setOutlineShaderInstance(const std::optional<ShaderProgramInstanceHandle>& outlineShaderInstance) {
    ZoneScoped;

    for (RenderData& videoResource : renderData) {
      videoResource.outlineShaderInstance = outlineShaderInstance;
    }
  }
};
