#pragma once

#include <Graphics/RenderingEngine.hpp>
#include <Graphics/Transform.hpp>

#include <optional>
#include <vector>

struct Actor {
  std::string name = "Unnamed";
  Transform transform = {};
  std::vector<RenderingEngine::RenderData> renderData;
  //
  // explicit Actor(const std::string_view name, const Transform& transform = {}, std::vector<RenderingEngine::RenderData>&& renderData = {})
  //   : name(name)
  //   , transform(transform)
  //   , renderData(std::move(renderData))
  // {}
  //
  // Actor(const Actor&) = default;
  // Actor(Actor&&) = default;
  //
  // Actor& operator=(const Actor&) = default;
  // Actor& operator=(Actor&&) = default;

  void setShaderProgramInstance(const ShaderProgramInstanceHandle shaderInstance) {
    for (RenderingEngine::RenderData& videoResource : renderData) {
      videoResource.shaderProgramInstance = shaderInstance;
    }
  }

  void setOutlineShaderInstance(std::optional<ShaderProgramInstanceHandle> outlineShaderInstance) {
    for (RenderingEngine::RenderData& videoResource : renderData) {
      videoResource.renderOptions.outlineShaderInstance = outlineShaderInstance;
    }
  }
};
