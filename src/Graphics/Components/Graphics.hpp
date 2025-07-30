#pragma once

#include <Graphics/RenderData.hpp>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <vector>

struct CompGraphics {
  std::vector<RenderData> renderData;
};

inline void setShaderProgramInstance(entt::registry& registry, entt::entity entity, const ShaderProgramInstanceHandle shaderInstance) {
  ZoneScoped;

  assert(registry.valid(entity));

  for (RenderData& videoResource : registry.get<CompGraphics>(entity).renderData) {
    videoResource.shaderProgramInstance = shaderInstance;
  }
}

inline void setOutlineShaderInstance(entt::registry& registry, entt::entity entity, const std::optional<ShaderProgramInstanceHandle>& outlineShaderInstance) {
  ZoneScoped;

  assert(registry.valid(entity));

  for (RenderData& videoResource : registry.get<CompGraphics>(entity).renderData) {
    videoResource.outlineShaderInstance = outlineShaderInstance;
  }
}
