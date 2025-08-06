#pragma once

#include <Graphics/ShaderProgramInstance.hpp>

struct CompOutline {
  ShaderProgramInstanceHandle outlineShader;

  static void on_destroy(entt::registry& registry, const entt::entity entity) {
    registry.get<CompOutline>(entity).outlineShader.erase();
  }
};
