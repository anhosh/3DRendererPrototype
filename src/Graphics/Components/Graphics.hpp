#pragma once

#include <Graphics/RenderData.hpp>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <vector>

struct CompGraphics {
  std::vector<RenderData> renderData;

  static void on_update(entt::registry& registry, const entt::entity entity) {
    registry.emplace<CompDirty>(entity);
  }
};

inline void setShaderProgramInstance(entt::registry& registry, entt::entity entity, const ShaderProgramInstanceHandle shaderInstance) {
  ZoneScoped;

  assert(registry.valid(entity));

  registry.patch<CompGraphics>(entity, [=](CompGraphics& graphics) {
    for (RenderData& renderData : graphics.renderData) {
      renderData.shaderProgramInstance = shaderInstance;
    }
  });
}
